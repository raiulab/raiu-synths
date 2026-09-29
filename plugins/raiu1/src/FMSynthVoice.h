#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "dsp/FMVoiceEngine.h"

namespace raiu {

// 全MIDIノート・全チャンネルを受けるサウンド定義
struct FMSound : public juce::SynthesiserSound {
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class FMSynthVoice : public juce::SynthesiserVoice {
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        return dynamic_cast<FMSound*>(sound) != nullptr;
    }

    void prepare(double sampleRate, int maxBlockSize) {
        engine.setSampleRate(sampleRate);
        scratch.resize((size_t) maxBlockSize);
    }

    void applyParams(const FMVoiceEngine::Params& p, float portamentoTime) {
        engine.setParams(p);
        engine.setPortamentoTime(portamentoTime);
    }

    void applyNatureParams(const NatureOscillator::Params& p) { engine.setNatureParams(p); }

    void setPitchOffsetCents(float cents) { engine.setPitchOffsetCents(cents); }
    void setModulatorBoost(float boost) { engine.setModulatorBoost(boost); }

    void startNote(int midiNote, float velocity, juce::SynthesiserSound*, int) override {
        engine.noteOn(midiNote, velocity);
    }

    void stopNote(float, bool allowTailOff) override {
        engine.noteOff();
        if (!allowTailOff) {
            engine.reset();
            clearCurrentNote();
        }
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& output, int startSample, int numSamples) override {
        if (!engine.isActive()) {
            if (getCurrentlyPlayingNote() >= 0) clearCurrentNote();
            return;
        }
        if ((int) scratch.size() < numSamples) scratch.resize((size_t) numSamples);
        std::fill(scratch.begin(), scratch.begin() + numSamples, 0.0f);
        engine.render(scratch.data(), numSamples);
        for (int ch = 0; ch < output.getNumChannels(); ++ch)
            output.addFrom(ch, startSample, scratch.data(), numSamples);
        if (!engine.isActive()) clearCurrentNote();
    }

private:
    FMVoiceEngine engine;
    std::vector<float> scratch;
};

} // namespace raiu
