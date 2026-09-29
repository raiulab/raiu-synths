#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/FMVoiceEngine.h"
#include "dsp/texture/TextureEngine.h"

namespace raiu::params {

inline juce::String opId(int opIndex1Based, const char* name) {
    return "op" + juce::String(opIndex1Based) + "_" + name;
}

// デフォルト音色: アルゴリズム3 (2→1)+(4→3) のエレクトリックピアノ
//   ペアA op1(キャリア r=1) + op2(モジュレーター r=14, 短い減衰) = ティーンという立ち上がり
//   ペアB op3(キャリア r=1) + op4(モジュレーター r=1, 長い減衰)  = 胴鳴りの温かさ
struct OpDefaults {
    float ratio, level, attack, decay, sustain, release, velSens, keyScale;
};
inline constexpr OpDefaults kOpDefaults[4] = {
    {1.0f, 0.90f, 0.003f, 1.4f, 0.00f, 0.35f, 0.30f, 0.0f},
    {14.0f, 0.22f, 0.001f, 0.25f, 0.00f, 0.20f, 0.60f, 3.0f},
    {1.0f, 0.80f, 0.003f, 1.6f, 0.05f, 0.40f, 0.30f, 0.0f},
    {1.0f, 0.30f, 0.003f, 1.0f, 0.00f, 0.30f, 0.50f, 2.0f},
};

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout() {
    using FloatParam = juce::AudioParameterFloat;
    using BoolParam = juce::AudioParameterBool;
    using ChoiceParam = juce::AudioParameterChoice;
    using Range = juce::NormalisableRange<float>;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    for (int i = 1; i <= 4; ++i) {
        const auto& d = kOpDefaults[i - 1];
        auto prefix = "OP" + juce::String(i) + " ";
        Range ratioRange(0.25f, 16.0f); ratioRange.setSkewForCentre(2.0f);
        Range timeRange(0.001f, 8.0f); timeRange.setSkewForCentre(0.5f);
        Range fixedHzRange(0.1f, 4000.0f); fixedHzRange.setSkewForCentre(100.0f);

        layout.add(
            std::make_unique<FloatParam>(juce::ParameterID{opId(i, "ratio"), 1}, prefix + "Ratio", ratioRange, d.ratio),
            std::make_unique<FloatParam>(juce::ParameterID{opId(i, "detune"), 1}, prefix + "Detune", Range(-50.0f, 50.0f), 0.0f),
            std::make_unique<BoolParam>(juce::ParameterID{opId(i, "fixed"), 1}, prefix + "Fixed Mode", false),
            std::make_unique<FloatParam>(juce::ParameterID{opId(i, "fixedhz"), 1}, prefix + "Fixed Hz", fixedHzRange, 440.0f),
            std::make_unique<FloatParam>(juce::ParameterID{opId(i, "level"), 1}, prefix + "Level", Range(0.0f, 1.0f), d.level),
            std::make_unique<FloatParam>(juce::ParameterID{opId(i, "attack"), 1}, prefix + "Attack", timeRange, d.attack),
            std::make_unique<FloatParam>(juce::ParameterID{opId(i, "decay"), 1}, prefix + "Decay", timeRange, d.decay),
            std::make_unique<FloatParam>(juce::ParameterID{opId(i, "sustain"), 1}, prefix + "Sustain", Range(0.0f, 1.0f), d.sustain),
            std::make_unique<FloatParam>(juce::ParameterID{opId(i, "release"), 1}, prefix + "Release", timeRange, d.release),
            std::make_unique<FloatParam>(juce::ParameterID{opId(i, "velsens"), 1}, prefix + "Velocity Sens", Range(0.0f, 1.0f), d.velSens),
            std::make_unique<FloatParam>(juce::ParameterID{opId(i, "keyscale"), 1}, prefix + "Key Scale dB/oct", Range(0.0f, 12.0f), d.keyScale));
    }

    juce::StringArray algoNames;
    for (int a = 1; a <= 8; ++a) algoNames.add("Algorithm " + juce::String(a));
    Range portaRange(0.0f, 2.0f); portaRange.setSkewForCentre(0.3f);

    layout.add(
        std::make_unique<ChoiceParam>(juce::ParameterID{"algorithm", 1}, "Algorithm", algoNames, 2),
        std::make_unique<FloatParam>(juce::ParameterID{"feedback", 1}, "Feedback", Range(0.0f, 1.0f), 0.0f),
        std::make_unique<FloatParam>(juce::ParameterID{"portamento", 1}, "Portamento Time", portaRange, 0.0f),
        std::make_unique<BoolParam>(juce::ParameterID{"mono", 1}, "Mono Mode", false),
        std::make_unique<FloatParam>(juce::ParameterID{"vibrato_rate", 1}, "Vibrato Rate", Range(0.1f, 12.0f), 5.0f),
        std::make_unique<FloatParam>(juce::ParameterID{"vibrato_depth", 1}, "Vibrato Depth", Range(0.0f, 100.0f), 0.0f),
        std::make_unique<FloatParam>(juce::ParameterID{"master_gain", 1}, "Master Gain dB", Range(-24.0f, 6.0f), -6.0f));

    layout.add(
        std::make_unique<FloatParam>(juce::ParameterID{"tex_wind", 1}, "Texture Wind", Range(0.0f, 1.0f), 0.0f),
        std::make_unique<FloatParam>(juce::ParameterID{"tex_water", 1}, "Texture Water", Range(0.0f, 1.0f), 0.0f),
        std::make_unique<FloatParam>(juce::ParameterID{"tex_birds", 1}, "Texture Birds", Range(0.0f, 1.0f), 0.0f),
        std::make_unique<FloatParam>(juce::ParameterID{"tex_insects", 1}, "Texture Insects", Range(0.0f, 1.0f), 0.0f),
        std::make_unique<FloatParam>(juce::ParameterID{"tex_rain", 1}, "Texture Rain", Range(0.0f, 1.0f), 0.0f),
        std::make_unique<FloatParam>(juce::ParameterID{"tex_rain_density", 1}, "Rain Density", Range(0.0f, 1.0f), 0.5f),
        std::make_unique<FloatParam>(juce::ParameterID{"tex_resonance", 1}, "Texture Resonance Q", Range(5.0f, 120.0f), 60.0f),
        std::make_unique<FloatParam>(juce::ParameterID{"tex_resmix", 1}, "Texture Resonance Mix", Range(0.0f, 1.0f), 0.7f),
        std::make_unique<FloatParam>(juce::ParameterID{"tex_distance", 1}, "Texture Distance", Range(0.0f, 1.0f), 0.3f),
        std::make_unique<FloatParam>(juce::ParameterID{"tex_master", 1}, "Texture Level", Range(0.0f, 1.0f), 0.8f));

    // NATURE OSC — 自然音の音素レイヤー
    {
        juce::StringArray natureModes{"Off", "Wind", "Water", "Birds", "Insects", "Rain"};
        Range natTimeRange(0.001f, 8.0f); natTimeRange.setSkewForCentre(0.5f);
        Range cutoffRange(100.0f, 12000.0f); cutoffRange.setSkewForCentre(2000.0f);
        layout.add(
            std::make_unique<ChoiceParam>(juce::ParameterID{"nature_type", 1}, "Nature Type", natureModes, 0),
            std::make_unique<FloatParam>(juce::ParameterID{"nature_level", 1}, "Nature Level", Range(0.0f, 1.0f), 0.5f),
            std::make_unique<FloatParam>(juce::ParameterID{"nature_keytrack", 1}, "Nature Key Track", Range(0.0f, 1.0f), 1.0f),
            std::make_unique<FloatParam>(juce::ParameterID{"nature_pitch", 1}, "Nature Pitch (semi)", Range(-24.0f, 24.0f), 0.0f),
            std::make_unique<FloatParam>(juce::ParameterID{"nature_attack", 1}, "Nature Attack", natTimeRange, 0.05f),
            std::make_unique<FloatParam>(juce::ParameterID{"nature_decay", 1}, "Nature Decay", natTimeRange, 0.3f),
            std::make_unique<FloatParam>(juce::ParameterID{"nature_sustain", 1}, "Nature Sustain", Range(0.0f, 1.0f), 0.7f),
            std::make_unique<FloatParam>(juce::ParameterID{"nature_release", 1}, "Nature Release", natTimeRange, 0.4f),
            std::make_unique<FloatParam>(juce::ParameterID{"nature_cutoff", 1}, "Nature Cutoff", cutoffRange, 8000.0f),
            std::make_unique<FloatParam>(juce::ParameterID{"nature_res", 1}, "Nature Resonance", Range(0.0f, 1.0f), 0.2f));
    }

    // エフェクト
    Range chorusRateRange(0.05f, 4.0f); chorusRateRange.setSkewForCentre(0.8f);
    layout.add(
        std::make_unique<FloatParam>(juce::ParameterID{"chorus_rate", 1}, "Chorus Rate", chorusRateRange, 0.9f),
        std::make_unique<FloatParam>(juce::ParameterID{"chorus_depth", 1}, "Chorus Depth", Range(0.0f, 1.0f), 0.25f),
        std::make_unique<FloatParam>(juce::ParameterID{"chorus_mix", 1}, "Chorus Mix", Range(0.0f, 1.0f), 0.35f),
        std::make_unique<FloatParam>(juce::ParameterID{"reverb_size", 1}, "Reverb Size", Range(0.0f, 1.0f), 0.5f),
        std::make_unique<FloatParam>(juce::ParameterID{"reverb_mix", 1}, "Reverb Mix", Range(0.0f, 1.0f), 0.2f));

    // 2層の相互作用
    layout.add(
        std::make_unique<FloatParam>(juce::ParameterID{"breath", 1}, "Breath (Texture->FM)", Range(0.0f, 1.0f), 0.0f),
        std::make_unique<FloatParam>(juce::ParameterID{"vel_to_texture", 1}, "Velocity->Texture", Range(0.0f, 1.0f), 0.0f),
        std::make_unique<FloatParam>(juce::ParameterID{"swell", 1}, "Note Swell", Range(0.0f, 1.0f), 0.0f));

    return layout;
}

// APVTSの現在値からエンジン用パラメーター構造体を作る
inline FMVoiceEngine::Params readEngineParams(const juce::AudioProcessorValueTreeState& s) {
    FMVoiceEngine::Params p;
    auto get = [&s](const juce::String& id) { return s.getRawParameterValue(id)->load(); };
    for (int i = 0; i < 4; ++i) {
        auto& op = p.ops[(size_t) i];
        const int n = i + 1;
        op.ratio = get(opId(n, "ratio"));
        op.detuneCents = get(opId(n, "detune"));
        op.fixed = get(opId(n, "fixed")) > 0.5f;
        op.fixedHz = get(opId(n, "fixedhz"));
        op.level = get(opId(n, "level"));
        op.attack = get(opId(n, "attack"));
        op.decay = get(opId(n, "decay"));
        op.sustain = get(opId(n, "sustain"));
        op.release = get(opId(n, "release"));
        op.velSens = get(opId(n, "velsens"));
        op.keyScaleDbPerOct = get(opId(n, "keyscale"));
    }
    p.algorithm = (int) get("algorithm") + 1;  // Choiceは0始まり
    p.feedback = get("feedback");
    return p;
}

inline NatureOscillator::Params readNatureParams(const juce::AudioProcessorValueTreeState& s) {
    NatureOscillator::Params p;
    auto get = [&s](const char* id) { return s.getRawParameterValue(id)->load(); };
    p.mode = (NatureOscillator::Mode) (int) get("nature_type");
    p.level = get("nature_level");
    p.keyTrack = get("nature_keytrack");
    p.semitones = get("nature_pitch");
    p.attack = get("nature_attack");
    p.decay = get("nature_decay");
    p.sustain = get("nature_sustain");
    p.release = get("nature_release");
    p.cutoff = get("nature_cutoff");
    p.resonance = get("nature_res");
    return p;
}

inline TextureEngine::Params readTextureParams(const juce::AudioProcessorValueTreeState& s) {
    TextureEngine::Params p;
    auto get = [&s](const char* id) { return s.getRawParameterValue(id)->load(); };
    p.windLevel = get("tex_wind");
    p.waterLevel = get("tex_water");
    p.birdsLevel = get("tex_birds");
    p.insectsLevel = get("tex_insects");
    p.rainLevel = get("tex_rain");
    p.rainDensity = get("tex_rain_density");
    p.resonance = get("tex_resonance");
    p.resonanceMix = get("tex_resmix");
    p.distance = get("tex_distance");
    p.master = get("tex_master");
    return p;
}

} // namespace raiu::params
