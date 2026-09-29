// AUコンポーネントのホスティングキャッシュを完全に迂回し、Raiu1Processorを
// 直接インスタンス化してクリッピング・NaNを検査するスタンドアロン実行ファイル。
#include "../src/PluginProcessor.h"
#include <cstdio>
#include <cmath>
#include <cstring>

int main(int argc, char** argv) {
    juce::String mode = argc > 1 ? juce::String(argv[1]) : juce::String("full");

    if (mode == "presets") {
        constexpr double presetSr = 48000.0;
        constexpr int presetBlockSize = 512;
        int failed = 0;

        std::printf("%-24s %8s %8s %10s %10s\n", "Preset", "RMS", "Peak", ">0.90", ">0.95");
        for (int program = 0;; ++program) {
            raiu::Raiu1Processor presetProc;
            if (program >= presetProc.getNumPrograms()) break;
            presetProc.setRateAndBufferSizeDetails(presetSr, presetBlockSize);
            presetProc.prepareToPlay(presetSr, presetBlockSize);
            presetProc.setCurrentProgram(program);

            juce::AudioBuffer<float> presetBuffer(2, presetBlockSize);
            juce::MidiBuffer presetMidi;
            for (int note : {48, 55, 60, 64})
                presetMidi.addEvent(juce::MidiMessage::noteOn(1, note, (juce::uint8) 110), 0);

            double sumSquares = 0.0;
            long long sampleCount = 0, over90 = 0, over95 = 0, nonFinite = 0;
            float peak = 0.0f;
            const int totalBlocks = (int) (presetSr * 4.0 / presetBlockSize);
            for (int b = 0; b < totalBlocks; ++b) {
                presetBuffer.clear();
                presetProc.processBlock(presetBuffer, presetMidi);
                presetMidi.clear();
                if (b == totalBlocks * 3 / 4) {
                    for (int note : {48, 55, 60, 64})
                        presetMidi.addEvent(juce::MidiMessage::noteOff(1, note), 0);
                }
                for (int ch = 0; ch < presetBuffer.getNumChannels(); ++ch) {
                    const float* data = presetBuffer.getReadPointer(ch);
                    for (int i = 0; i < presetBuffer.getNumSamples(); ++i) {
                        const float s = data[i];
                        if (!std::isfinite(s)) { ++nonFinite; continue; }
                        const float magnitude = std::abs(s);
                        peak = std::max(peak, magnitude);
                        sumSquares += (double) s * s;
                        ++sampleCount;
                        if (magnitude > 0.90f) ++over90;
                        if (magnitude > 0.95f) ++over95;
                    }
                }
            }

            const double rms = sampleCount > 0 ? std::sqrt(sumSquares / (double) sampleCount) : 0.0;
            const juce::String name = presetProc.getProgramName(program);
            std::printf("%-24s %8.4f %8.4f %10lld %10lld%s\n", name.toRawUTF8(), rms, (double) peak,
                        over90, over95, nonFinite > 0 ? "  NON-FINITE" : "");
            if (nonFinite > 0 || over90 > 0) ++failed;
        }

        std::printf("Preset overloads: %d\n", failed);
        return failed == 0 ? 0 : 1;
    }

    raiu::Raiu1Processor proc;
    constexpr double sr = 48000.0;
    constexpr int blockSize = 512;
    if (mode != "no_hostdetails") proc.setRateAndBufferSizeDetails(sr, blockSize);
    proc.prepareToPlay(sr, blockSize);

    auto setByName = [&](const char* paramID, float value) {
        if (auto* rp = proc.apvts.getParameter(paramID)) {
            rp->setValueNotifyingHost(rp->convertTo0to1(value));
            return;
        }
        std::fprintf(stderr, "param not found: %s\n", paramID);
        std::exit(2);
    };

    if (mode != "no_mastergain") setByName("master_gain", 6.0f);
    if (mode == "full" || mode == "texture") {
        setByName("tex_wind", 0.8f);
        setByName("tex_rain", 0.8f);
        setByName("tex_resonance", 120.0f);
        setByName("tex_resmix", 1.0f);
        setByName("tex_master", 1.0f);
    }

    juce::AudioBuffer<float> buffer(2, blockSize);
    juce::MidiBuffer midi;
    if (mode == "one_note")
        midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8) 110), 0);
    else if (mode != "texture_nonotes")
        for (int note : {36, 40, 43, 48, 52, 55, 60, 64})
            midi.addEvent(juce::MidiMessage::noteOn(1, note, (juce::uint8) 110), 0);

    float peak = 0.0f;
    long long overCount = 0, nanCount = 0;
    const int totalBlocks = (int) (sr * 4.0 / blockSize);
    for (int b = 0; b < totalBlocks; ++b) {
        buffer.clear();
        proc.processBlock(buffer, midi);
        midi.clear();  // ノートオンは最初のブロックのみ
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            const float* data = buffer.getReadPointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                float s = data[i];
                if (!std::isfinite(s)) { ++nanCount; continue; }
                peak = std::max(peak, std::abs(s));
                if (std::abs(s) > 1.0f) ++overCount;
            }
        }
    }

    std::printf("Peak: %.4f  OverCount(|x|>1.0): %lld  NaN/Inf: %lld\n", (double) peak, overCount,
               nanCount);
    if (nanCount > 0) { std::printf("NaN/Inf DETECTED\n"); return 1; }
    if (overCount > 0) { std::printf("CLIPPING DETECTED\n"); return 1; }
    std::printf("NO CLIPPING\n");
    return 0;
}
