// AUを実際にインスタンス化し、ノートを鳴らしてオフラインレンダリングし、
// 無音でないことを確認するスモークテスト。
// 使い方: swift tools/render_test.swift [出力wavパス]
import AVFoundation

let engine = AVAudioEngine()
let desc = AudioComponentDescription(
    componentType: kAudioUnitType_MusicDevice,
    componentSubType: fourCC("Rai1"),
    componentManufacturer: fourCC("Raiu"),
    componentFlags: 0, componentFlagsMask: 0)

func fourCC(_ s: String) -> OSType {
    var result: OSType = 0
    for c in s.utf8 { result = (result << 8) | OSType(c) }
    return result
}

let unit = AVAudioUnitMIDIInstrument(audioComponentDescription: desc)
engine.attach(unit)
engine.connect(unit, to: engine.mainMixerNode, format: nil)

let format = AVAudioFormat(standardFormatWithSampleRate: 48000, channels: 2)!
try engine.enableManualRenderingMode(.offline, format: format, maximumFrameCount: 4096)
try engine.start()

// C3, E3, G3 のコードを弾く
for note: UInt8 in [48, 52, 55] {
    unit.startNote(note, withVelocity: 100, onChannel: 0)
}

let totalFrames: AVAudioFrameCount = 48000 * 2  // 2秒
let buffer = AVAudioPCMBuffer(pcmFormat: engine.manualRenderingFormat,
                              frameCapacity: 4096)!
var rendered: AVAudioFrameCount = 0
var sumSquares = 0.0
var count = 0.0
var peak: Float = 0.0
var outFile: AVAudioFile? = nil
if CommandLine.arguments.count > 1 {
    outFile = try AVAudioFile(forWriting: URL(fileURLWithPath: CommandLine.arguments[1]),
                              settings: format.settings)
}

while rendered < totalFrames {
    // 1秒経過したらノートオフしてリリースを確認
    if rendered >= 48000 && rendered < 48000 + 4096 {
        for note: UInt8 in [48, 52, 55] { unit.stopNote(note, onChannel: 0) }
    }
    let frames = min(4096, totalFrames - rendered)
    let status = try engine.renderOffline(frames, to: buffer)
    guard status == .success else { fatalError("render failed: \(status)") }
    for ch in 0..<Int(buffer.format.channelCount) {
        let data = buffer.floatChannelData![ch]
        for i in 0..<Int(buffer.frameLength) {
            let s = data[i]
            if !s.isFinite { fatalError("NaN/inf detected at frame \(rendered)") }
            sumSquares += Double(s * s)
            count += 1
            peak = max(peak, abs(s))
        }
    }
    try outFile?.write(from: buffer)
    rendered += frames
}

let rms = (sumSquares / count).squareRoot()
print(String(format: "RMS: %.4f  Peak: %.4f", rms, peak))
if rms < 0.001 { fatalError("SILENT OUTPUT — synth is not producing sound") }
if peak > 1.0 { print("WARNING: output clips (peak > 1.0)") }
print("RENDER TEST PASSED")
