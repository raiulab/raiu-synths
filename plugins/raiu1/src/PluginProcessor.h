#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "Params.h"
#include "FMSynthVoice.h"
#include "dsp/EnvFollower.h"
#include "FXChain.h"

namespace raiu {

class Raiu1Processor : public juce::AudioProcessor {
public:
    Raiu1Processor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "RAIU-1 Breeze FM"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static constexpr int kNumVoices = 16;

    void processMonoMidi(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi);
    void renderMonoSegment(juce::AudioBuffer<float>& buffer, int start, int numSamples);

    juce::Synthesiser synth;
    juce::SmoothedValue<float> masterGain;
    juce::SmoothedValue<float> polyHeadroom;  // 同時発音数に応じたヘッドルーム（急変によるクリック防止のため平滑化）
    int currentProgram = 0;

    FXChain fx;

    // 環境音テクスチャレイヤー
    TextureEngine textureEngine;
    std::vector<float> texScratch;

    // モノモード（レガート・後着優先）
    FMVoiceEngine monoEngine;
    std::vector<std::pair<int, float>> heldNotes;  // {note, velocity} 押鍵順
    std::vector<float> monoScratch;
    bool wasMono = false;

    // グローバルなピッチ変調
    double vibratoPhase = 0.0;
    float bendCents = 0.0f;
    // ホストが setRateAndBufferSizeDetails を呼ばずに processBlock を呼ぶ場合
    // getSampleRate() が 0 を返しうるため、prepareToPlay で受け取った値を
    // 自前で保持する（0除算によるNaN汚染を防ぐ）
    double currentSampleRate = 48000.0;

    // 2層の相互作用
    EnvFollower texFollower;   // テクスチャの揺らぎ → FM音色のBreath変調
    float velEnv = 0.0f;       // ベロシティ → テクスチャ開き（ゆっくり減衰）
    float swellEnv = 0.0f;     // ノートオンでテクスチャがふわっと持ち上がる
    float velEnvDecay = 0.999f, swellEnvDecay = 0.999f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Raiu1Processor)
};

} // namespace raiu
