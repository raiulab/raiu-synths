#pragma once

#include "TextureCommon.h"

namespace raiu {

// 雨: ポアソン発生の粒（インパルス→共鳴バンドパスで「ピチッ」）+ 背景ヒス。
// densityで小雨〜土砂降りを操作。
class RainGenerator {
public:
    void setSampleRate(double sr) noexcept {
        sampleRate = sr;
        dropBand.set(1800.0f, 1.0f, sr);
        hissLP.setCutoff(4000.0f, sr);
        dcBlock.setCutoff(20.0f, sr);
        updateDensity();
    }

    // 0..1（雨の強さ）
    void setDensity(float d) noexcept {
        density = d < 0.0f ? 0.0f : (d > 1.0f ? 1.0f : d);
        updateDensity();
    }

    float next() noexcept {
        // 粒: ランダム振幅のインパルスを共鳴フィルターに通す
        float excitation = 0.0f;
        if (dropTrigger.fire(rng)) {
            float a = rng.uniform();
            excitation = rng.bipolar() * (0.5f + 2.5f * a * a);  // 大粒は稀
        }
        float drops = dropBand.process(excitation);

        // 背景ヒス: 遠くの無数の雨粒の合算
        float hiss = hissLP.process(rng.bipolar()) * (0.02f + 0.10f * density);

        return dcBlock.process(drops * 0.8f + hiss);
    }

private:
    void updateDensity() noexcept {
        dropTrigger.setRate(30.0f + 570.0f * density * density, sampleRate);
    }

    double sampleRate = 48000.0;
    float density = 0.5f;
    tex::Rng rng{1223u};
    tex::PoissonTrigger dropTrigger;
    tex::BiquadBP dropBand;
    tex::OnePoleLP hissLP;
    tex::OnePoleHP dcBlock;
};

} // namespace raiu
