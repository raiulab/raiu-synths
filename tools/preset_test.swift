// 全ファクトリープリセットを順に選択し、それぞれ発音して
// 無音・NaN・過大クリップがないことを確認する。
// 使い方: swift tools/preset_test.swift
import AVFoundation

func fourCC(_ s: String) -> OSType {
    var result: OSType = 0
    for c in s.utf8 { result = (result << 8) | OSType(c) }
    return result
}

let engine = AVAudioEngine()
let desc = AudioComponentDescription(
    componentType: kAudioUnitType_MusicDevice,
    componentSubType: fourCC("Rai1"),
    componentManufacturer: fourCC("Raiu"),
    componentFlags: 0, componentFlagsMask: 0)

let unit = AVAudioUnitMIDIInstrument(audioComponentDescription: desc)
engine.attach(unit)
engine.connect(unit, to: engine.mainMixerNode, format: nil)
let format = AVAudioFormat(standardFormatWithSampleRate: 48000, channels: 2)!
try engine.enableManualRenderingMode(.offline, format: format, maximumFrameCount: 4096)
try engine.start()

let presets = unit.auAudioUnit.factoryPresets ?? []
print("factory presets: \(presets.count)")
guard !presets.isEmpty else { fatalError("NO FACTORY PRESETS EXPOSED") }

var failures = 0
for preset in presets {
    unit.auAudioUnit.currentPreset = preset

    // ベース系プリセットも鳴るよう低めの音も混ぜる
    for note: UInt8 in [36, 48, 60] {
        unit.startNote(note, withVelocity: 100, onChannel: 0)
    }

    let totalFrames: AVAudioFrameCount = 96000  // 2秒
    let buffer = AVAudioPCMBuffer(pcmFormat: engine.manualRenderingFormat, frameCapacity: 4096)!
    var rendered: AVAudioFrameCount = 0
    var sumSquares = 0.0
    var count = 0.0
    var peak: Float = 0.0
    var bad = false
    while rendered < totalFrames {
        if rendered >= 72000 && rendered < 72000 + 4096 {
            for note: UInt8 in [36, 48, 60] { unit.stopNote(note, onChannel: 0) }
        }
        let frames = min(4096, totalFrames - rendered)
        let status = try engine.renderOffline(frames, to: buffer)
        guard status == .success else { fatalError("render failed") }
        for ch in 0..<Int(buffer.format.channelCount) {
            let data = buffer.floatChannelData![ch]
            for i in 0..<Int(buffer.frameLength) {
                let s = data[i]
                if !s.isFinite { bad = true }
                sumSquares += Double(s * s)
                count += 1
                peak = max(peak, abs(s))
            }
        }
        rendered += frames
    }
    // 次のプリセットへの残響持ち越しを軽減
    for _ in 0..<12 { _ = try engine.renderOffline(4096, to: buffer) }

    let rms = (sumSquares / count).squareRoot()
    var verdict = "OK"
    if bad { verdict = "NaN/INF"; failures += 1 }
    else if rms < 0.0005 { verdict = "SILENT"; failures += 1 }
    else if peak > 1.05 { verdict = "CLIPPING"; failures += 1 }
    print(String(format: "%-16s RMS: %.4f  Peak: %.4f  %@",
                 (preset.name as NSString).utf8String!, rms, peak, verdict))
}

if failures > 0 { fatalError("\(failures) preset(s) failed") }
print("ALL PRESETS PASSED")
