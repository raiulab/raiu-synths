#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace raiu {

// ピークが出力へ到達する前にゲインを下げる、ステレオリンクの先読みリミッター。
// juce::dsp::Limiter は処理末尾で 0 dBFS にハードクリップするため、短いFMの
// アタックや自然音の過渡成分を大きく入力すると、出力値は安全でも波形が平らに
// 切られて音割れになる。ここではピークを先に検出し、遅延させた信号へ滑らかな
// ゲインカーブを適用することで、ハードクリップへ到達する前に保護する。
class LookaheadPeakLimiter {
public:
    void prepare(const juce::dsp::ProcessSpec& spec) {
        sampleRate = spec.sampleRate > 0.0 ? spec.sampleRate : 48000.0;
        numChannels = (int) spec.numChannels;
        lookaheadSamples = std::max(1, (int) std::ceil(sampleRate * lookaheadSeconds));
        delaySize = lookaheadSamples + 1;
        delay.assign((size_t) numChannels * (size_t) delaySize, 0.0f);
        gainSchedule.assign((size_t) delaySize, 1.0f);
        releaseCoefficient = std::exp(-1.0f / (releaseSeconds * (float) sampleRate));
        reset();
    }

    void reset() noexcept {
        std::fill(delay.begin(), delay.end(), 0.0f);
        std::fill(gainSchedule.begin(), gainSchedule.end(), 1.0f);
        writeIndex = 0;
        currentGain = 1.0f;
    }

    void process(juce::AudioBuffer<float>& buffer) noexcept {
        jassert(buffer.getNumChannels() <= numChannels);
        if (delaySize <= 1 || buffer.getNumChannels() == 0) return;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float linkedPeak = 0.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                const float input = buffer.getSample(ch, sample);
                linkedPeak = std::max(linkedPeak, std::abs(input));
                delay[(size_t) ch * (size_t) delaySize + (size_t) writeIndex] = input;
            }

            const int readIndex = (writeIndex + 1) % delaySize;
            if (linkedPeak > ceiling) {
                const float requiredGain = ceiling / linkedPeak;
                // 現在の出力位置から、検出したピークが出力される位置まで線形に
                // ゲインを下げる。重なるピークには最も安全な（小さい）値を採用。
                for (int ahead = 0; ahead <= lookaheadSamples; ++ahead) {
                    const int index = (readIndex + ahead) % delaySize;
                    const float proportion = (float) ahead / (float) lookaheadSamples;
                    const float scheduledGain = 1.0f + (requiredGain - 1.0f) * proportion;
                    gainSchedule[(size_t) index] =
                        std::min(gainSchedule[(size_t) index], scheduledGain);
                }
            }

            const float targetGain = gainSchedule[(size_t) readIndex];
            if (targetGain < currentGain)
                currentGain = targetGain;  // 先読みで既に傾斜済みのアタック
            else
                currentGain = releaseCoefficient * currentGain
                            + (1.0f - releaseCoefficient) * targetGain;

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
                const float delayed =
                    delay[(size_t) ch * (size_t) delaySize + (size_t) readIndex];
                buffer.setSample(ch, sample, delayed * currentGain);
            }

            gainSchedule[(size_t) readIndex] = 1.0f;
            writeIndex = (writeIndex + 1) % delaySize;
        }
    }

    int getLatencySamples() const noexcept { return lookaheadSamples; }
    static constexpr float getCeiling() noexcept { return ceiling; }

private:
    static constexpr double lookaheadSeconds = 0.001;
    static constexpr float releaseSeconds = 0.060f;
    static constexpr float ceiling = 0.89125094f;  // -1 dBFS

    std::vector<float> delay;
    std::vector<float> gainSchedule;
    double sampleRate = 48000.0;
    float releaseCoefficient = 0.0f;
    float currentGain = 1.0f;
    int numChannels = 0;
    int lookaheadSamples = 0;
    int delaySize = 0;
    int writeIndex = 0;
};

// [FM+テクスチャ] → コーラス → リバーブ → リミッター
// コーラスは80年代ラバーズレゲエのエレピに必須の要素。
class FXChain {
public:
    struct Params {
        float chorusRate = 0.9f;    // Hz
        float chorusDepth = 0.25f;
        float chorusMix = 0.35f;
        float reverbSize = 0.5f;
        float reverbMix = 0.2f;
    };

    void prepare(const juce::dsp::ProcessSpec& spec) {
        chorus.prepare(spec);
        chorus.setCentreDelay(12.0f);  // BBD風の揺れの中心ディレイ
        chorus.setFeedback(0.0f);
        reverb.prepare(spec);
        limiter.prepare(spec);
    }

    void reset() {
        chorus.reset();
        reverb.reset();
        limiter.reset();
    }

    int getLatencySamples() const noexcept { return limiter.getLatencySamples(); }

    void setParams(const Params& p) {
        chorus.setRate(p.chorusRate);
        chorus.setDepth(p.chorusDepth);
        chorus.setMix(p.chorusMix);

        juce::Reverb::Parameters rp;
        rp.roomSize = 0.3f + 0.65f * p.reverbSize;
        rp.damping = 0.5f;
        rp.width = 1.0f;
        rp.wetLevel = p.reverbMix;
        rp.dryLevel = 1.0f - p.reverbMix * 0.5f;  // ウェットを上げても芯を残す
        reverb.setParameters(rp);
    }

    void process(juce::AudioBuffer<float>& buffer) {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        chorus.process(ctx);
        reverb.process(ctx);

        // 旧JUCEリミッターが暗黙に加えていた大きなメイクアップゲインを外すと
        // 全プリセットが約6 dB以上小さくなる。音量設計との互換性を保ちつつ、
        // 以前より控えめな+6 dBを明示して、直後の先読み段で安全に制御する。
        buffer.applyGain(juce::Decibels::decibelsToGain(6.0f));
        limiter.process(buffer);

        // 非有限値の汚染と、実装ミス時の0 dBFS超過だけは最終段でも止める。
        // 通常時の天井(-1 dBFS)より高い値はここで隠さず、回帰テストで検出する。
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            auto* data = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                data[i] = std::isfinite(data[i])
                    ? juce::jlimit(-1.0f, 1.0f, data[i]) : 0.0f;
        }
    }

private:
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Reverb reverb;
    LookaheadPeakLimiter limiter;
};

} // namespace raiu
