#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

namespace raiu::presets {

struct Preset {
    const char* name;
    // {パラメーターID, 実値} — 列挙されないパラメーターはデフォルト値に戻る
    std::vector<std::pair<const char*, float>> values;
};

// ファクトリープリセット10種（設計書 §6）
inline const std::vector<Preset>& factoryPresets() {
    static const std::vector<Preset> presets = {
        {"Lovers Keys",  // 80s ラバーズレゲエのふわキラDX7キーボード
         {{"algorithm", 2}, {"feedback", 0.15f},
          {"op1_ratio", 1.0f}, {"op1_level", 0.9f}, {"op1_attack", 0.002f}, {"op1_decay", 1.2f}, {"op1_sustain", 0.15f}, {"op1_release", 0.4f}, {"op1_velsens", 0.3f},
          {"op2_ratio", 14.0f}, {"op2_level", 0.18f}, {"op2_attack", 0.001f}, {"op2_decay", 0.15f}, {"op2_sustain", 0.0f}, {"op2_release", 0.2f}, {"op2_velsens", 0.7f}, {"op2_keyscale", 4.0f},
          {"op3_ratio", 1.0f}, {"op3_detune", 6.0f}, {"op3_level", 0.85f}, {"op3_attack", 0.003f}, {"op3_decay", 1.8f}, {"op3_sustain", 0.2f}, {"op3_release", 0.45f},
          {"op4_ratio", 1.0f}, {"op4_level", 0.4f}, {"op4_decay", 0.8f}, {"op4_sustain", 0.0f}, {"op4_velsens", 0.6f}, {"op4_keyscale", 2.0f},
          {"chorus_mix", 0.6f}, {"chorus_depth", 0.35f}, {"chorus_rate", 0.8f},
          {"reverb_mix", 0.25f}, {"vibrato_rate", 5.5f}, {"vibrato_depth", 6.0f}}},

        {"Rub-a-Dub Bass",  // 80s打ち込みレゲエの太いDX7ベース
         {{"algorithm", 0}, {"feedback", 0.35f}, {"mono", 1.0f},
          {"op1_ratio", 0.5f}, {"op1_level", 1.0f}, {"op1_attack", 0.002f}, {"op1_decay", 0.5f}, {"op1_sustain", 0.6f}, {"op1_release", 0.15f}, {"op1_velsens", 0.2f},
          {"op2_ratio", 0.5f}, {"op2_level", 0.7f}, {"op2_attack", 0.001f}, {"op2_decay", 0.25f}, {"op2_sustain", 0.3f}, {"op2_release", 0.1f}, {"op2_velsens", 0.7f},
          {"op3_ratio", 1.0f}, {"op3_level", 0.15f}, {"op3_decay", 0.15f}, {"op3_sustain", 0.0f}, {"op3_release", 0.1f},
          {"op4_ratio", 1.0f}, {"op4_level", 0.1f}, {"op4_decay", 0.1f}, {"op4_sustain", 0.0f}, {"op4_release", 0.1f},
          {"chorus_mix", 0.0f}, {"reverb_mix", 0.05f}, {"master_gain", -4.0f}}},

        {"Glass EP",  // ワイングラスの縁を指でなぞって鳴らす共鳴音。ゆっくり「歌い出し」、
         // 押さえている間鳴り続け、離した後もグラスがしばらく響き続ける
         {{"algorithm", 7},
          {"op1_level", 0.8f}, {"op1_attack", 0.35f}, {"op1_decay", 0.5f}, {"op1_sustain", 0.95f}, {"op1_release", 1.8f}, {"op1_velsens", 0.15f},
          {"op2_ratio", 1.0f}, {"op2_detune", 4.0f}, {"op2_level", 0.35f}, {"op2_attack", 0.4f}, {"op2_decay", 0.6f}, {"op2_sustain", 0.9f}, {"op2_release", 1.9f}, {"op2_velsens", 0.15f},
          {"op3_ratio", 2.0f}, {"op3_level", 0.12f}, {"op3_attack", 0.5f}, {"op3_decay", 0.6f}, {"op3_sustain", 0.7f}, {"op3_release", 1.5f}, {"op3_velsens", 0.2f},
          {"op4_level", 0.0f},
          {"vibrato_rate", 3.0f}, {"vibrato_depth", 4.0f},
          {"chorus_mix", 0.25f}, {"reverb_mix", 0.4f}, {"reverb_size", 0.7f}}},

        {"Vibraphone",  // ビブラフォン風
         {{"algorithm", 2},
          {"op1_ratio", 1.0f}, {"op1_level", 0.9f}, {"op1_attack", 0.001f}, {"op1_decay", 2.5f}, {"op1_sustain", 0.0f}, {"op1_release", 2.0f}, {"op1_velsens", 0.4f},
          {"op2_ratio", 4.0f}, {"op2_level", 0.28f}, {"op2_attack", 0.001f}, {"op2_decay", 0.4f}, {"op2_sustain", 0.0f}, {"op2_release", 0.3f}, {"op2_velsens", 0.6f}, {"op2_keyscale", 3.0f},
          {"op3_ratio", 4.0f}, {"op3_level", 0.22f}, {"op3_attack", 0.001f}, {"op3_decay", 2.0f}, {"op3_sustain", 0.0f}, {"op3_release", 1.5f},
          {"op4_level", 0.0f},
          {"vibrato_rate", 4.0f}, {"vibrato_depth", 8.0f},
          {"chorus_mix", 0.15f}, {"reverb_mix", 0.3f}}},

        {"Music Box",  // オルゴール風の澄んだ高音
         {{"algorithm", 2},
          {"op1_ratio", 1.0f}, {"op1_level", 0.85f}, {"op1_attack", 0.001f}, {"op1_decay", 1.2f}, {"op1_sustain", 0.0f}, {"op1_release", 0.8f},
          {"op2_ratio", 3.5f}, {"op2_level", 0.35f}, {"op2_attack", 0.001f}, {"op2_decay", 0.3f}, {"op2_sustain", 0.0f}, {"op2_release", 0.2f}, {"op2_keyscale", 4.0f},
          {"op3_ratio", 2.0f}, {"op3_level", 0.3f}, {"op3_attack", 0.001f}, {"op3_decay", 1.0f}, {"op3_sustain", 0.0f}, {"op3_release", 0.7f},
          {"op4_ratio", 7.0f}, {"op4_level", 0.2f}, {"op4_decay", 0.1f}, {"op4_sustain", 0.0f}, {"op4_release", 0.1f},
          {"reverb_mix", 0.35f}, {"chorus_mix", 0.1f}}},

        {"Pure Sine",  // SINE波の丸い音 + Water NATURE OSCをごく薄く重ね、丸みに自然な揺らぎを添える
         {{"algorithm", 7},
          {"op1_level", 0.9f}, {"op1_attack", 0.01f}, {"op1_decay", 0.3f}, {"op1_sustain", 0.8f}, {"op1_release", 0.4f},
          {"op2_level", 0.0f}, {"op3_level", 0.0f}, {"op4_level", 0.0f},
          {"nature_type", 2}, {"nature_level", 0.15f}, {"nature_keytrack", 1.0f},
          {"nature_attack", 0.02f}, {"nature_decay", 0.4f}, {"nature_sustain", 0.6f}, {"nature_release", 0.5f},
          {"nature_cutoff", 3000.0f}, {"nature_res", 0.15f},
          {"chorus_mix", 0.1f}, {"reverb_mix", 0.15f}}},

        {"Forest EP",  // 再設計: 鳥の声（Birds NATURE OSC）が主役 — 弾いた音程でさえずりの高さが変わる。エレピとTEXTUREは背景装飾
         {{"chorus_mix", 0.4f}, {"reverb_mix", 0.3f},
          {"tex_birds", 0.12f}, {"tex_wind", 0.2f}, {"tex_resmix", 0.4f}, {"tex_distance", 0.4f}, {"tex_master", 0.5f},
          {"breath", 0.6f}, {"swell", 0.3f},
          {"nature_type", 3}, {"nature_level", 0.65f}, {"nature_keytrack", 1.0f},
          {"nature_attack", 0.08f}, {"nature_decay", 0.3f}, {"nature_sustain", 0.5f}, {"nature_release", 0.5f},
          {"nature_cutoff", 7000.0f}, {"nature_res", 0.3f}}},

        {"Rainy Bass",  // ベース + 雨。Rain NATURE OSCが主役化しベースの音程に雨粒のピッチが追従する（TEXTUREの雨は背景の霧雨に後退）
         {{"algorithm", 0}, {"feedback", 0.25f}, {"mono", 1.0f},
          {"op1_ratio", 0.5f}, {"op1_level", 0.95f}, {"op1_attack", 0.002f}, {"op1_decay", 0.6f}, {"op1_sustain", 0.5f}, {"op1_release", 0.2f},
          {"op2_ratio", 0.5f}, {"op2_level", 0.55f}, {"op2_decay", 0.3f}, {"op2_sustain", 0.25f}, {"op2_velsens", 0.6f},
          {"op3_level", 0.1f}, {"op3_decay", 0.15f}, {"op3_sustain", 0.0f},
          {"op4_level", 0.08f}, {"op4_decay", 0.1f}, {"op4_sustain", 0.0f},
          {"nature_type", 5}, {"nature_level", 0.5f}, {"nature_keytrack", 0.85f},
          {"nature_attack", 0.005f}, {"nature_decay", 0.4f}, {"nature_sustain", 0.5f}, {"nature_release", 0.3f},
          {"nature_cutoff", 2500.0f}, {"nature_res", 0.35f},
          {"tex_rain", 0.25f}, {"tex_rain_density", 0.35f}, {"tex_resmix", 0.4f}, {"tex_distance", 0.35f}, {"tex_master", 0.4f},
          {"vel_to_texture", 0.5f},
          {"chorus_mix", 0.0f}, {"reverb_mix", 0.1f}}},

        {"Thunder Pad",  // 遠雷パッド（ゆっくり開く和音 + 風・遠い雨）
         // 和音で弾いたときの音割れ・過大音量を抑えるためレベルとmaster_gainを下げた
         {{"algorithm", 5},
          {"op1_level", 0.55f}, {"op1_attack", 0.8f}, {"op1_decay", 2.0f}, {"op1_sustain", 0.8f}, {"op1_release", 2.0f},
          {"op2_level", 0.5f}, {"op2_detune", -7.0f}, {"op2_attack", 1.0f}, {"op2_decay", 2.0f}, {"op2_sustain", 0.8f}, {"op2_release", 2.2f},
          {"op3_level", 0.45f}, {"op3_detune", 7.0f}, {"op3_attack", 1.2f}, {"op3_decay", 2.0f}, {"op3_sustain", 0.8f}, {"op3_release", 2.4f},
          {"op4_level", 0.22f}, {"op4_attack", 1.5f}, {"op4_decay", 2.0f}, {"op4_sustain", 0.4f}, {"op4_release", 2.0f},
          {"tex_wind", 0.45f}, {"tex_rain", 0.3f}, {"tex_rain_density", 0.7f}, {"tex_resmix", 0.5f}, {"tex_distance", 0.7f}, {"tex_master", 0.6f},
          {"breath", 0.8f},
          {"chorus_mix", 0.4f}, {"reverb_mix", 0.45f}, {"reverb_size", 0.8f}, {"master_gain", -10.0f}}},

        {"Siren Lead",  // 遠くまで響くサイレン
         {{"algorithm", 7}, {"mono", 1.0f}, {"portamento", 0.35f},
          {"op1_level", 0.9f}, {"op1_attack", 0.05f}, {"op1_decay", 0.3f}, {"op1_sustain", 1.0f}, {"op1_release", 0.6f},
          {"op2_level", 0.0f}, {"op3_level", 0.0f}, {"op4_level", 0.0f},
          {"vibrato_rate", 0.25f}, {"vibrato_depth", 90.0f},
          {"chorus_mix", 0.2f}, {"reverb_mix", 0.5f}, {"reverb_size", 0.9f}}},

        {"Whistle Wind",  // 風笛リード: 風を音階で吹く（NATURE OSC主役）
         {{"algorithm", 7},
          {"op1_level", 0.25f}, {"op1_attack", 0.15f}, {"op1_sustain", 0.9f}, {"op1_release", 0.6f},
          {"op2_level", 0.0f}, {"op3_level", 0.0f}, {"op4_level", 0.0f},
          {"nature_type", 1}, {"nature_level", 0.85f}, {"nature_keytrack", 1.0f},
          {"nature_attack", 0.2f}, {"nature_decay", 0.5f}, {"nature_sustain", 0.9f}, {"nature_release", 0.7f},
          {"nature_cutoff", 6000.0f}, {"nature_res", 0.4f},
          {"tex_wind", 0.25f}, {"tex_resmix", 0.6f}, {"tex_distance", 0.5f}, {"tex_master", 0.5f},
          {"breath", 0.5f},
          {"chorus_mix", 0.25f}, {"reverb_mix", 0.4f}, {"reverb_size", 0.7f}}},

        {"Bubble Keys",  // 水の鍵盤: 音程付きの泡 + 丸いSINE
         {{"algorithm", 7},
          {"op1_level", 0.6f}, {"op1_attack", 0.005f}, {"op1_decay", 0.8f}, {"op1_sustain", 0.3f}, {"op1_release", 0.5f},
          {"op2_level", 0.0f}, {"op3_level", 0.0f}, {"op4_level", 0.0f},
          {"nature_type", 2}, {"nature_level", 0.7f}, {"nature_keytrack", 1.0f},
          {"nature_attack", 0.005f}, {"nature_decay", 0.6f}, {"nature_sustain", 0.5f}, {"nature_release", 0.4f},
          {"nature_cutoff", 9000.0f}, {"nature_res", 0.3f},
          {"tex_water", 0.3f}, {"tex_resmix", 0.6f}, {"tex_distance", 0.3f}, {"tex_master", 0.5f},
          {"chorus_mix", 0.3f}, {"reverb_mix", 0.3f}}},

        // ---- 拡張プリセット群（DX7直系のFM単体音、ナイロンギター/ピアノ、モノリード、
        //      NATURE OSC発想音、TEXTURE主体アンビエント） ----

        {"Crystal Bell",  // DX7直系のFMベル: (4→3→2)+1
         // コードで弾いたときの不協和音を抑えるため、上部倍音を整数次倍音に寄せ
         // （3.5→3, 7→5, 11→8）、デチューン/フィードバックも控えめにして
         // 「共鳴するハーモニー」寄りに調整。ベルらしい金属光沢はリバーブで補う
         {{"algorithm", 6},
          {"op1_level", 0.85f}, {"op1_attack", 0.001f}, {"op1_decay", 2.5f}, {"op1_sustain", 0.0f}, {"op1_release", 2.0f}, {"op1_velsens", 0.5f},
          {"op2_ratio", 3.0f}, {"op2_detune", 2.0f}, {"op2_level", 0.38f}, {"op2_attack", 0.001f}, {"op2_decay", 1.8f}, {"op2_sustain", 0.0f}, {"op2_release", 1.5f}, {"op2_velsens", 0.6f}, {"op2_keyscale", 2.0f},
          {"op3_ratio", 5.0f}, {"op3_level", 0.25f}, {"op3_attack", 0.001f}, {"op3_decay", 0.6f}, {"op3_sustain", 0.0f}, {"op3_release", 0.4f}, {"op3_keyscale", 4.0f},
          {"op4_ratio", 8.0f}, {"op4_level", 0.2f}, {"op4_attack", 0.001f}, {"op4_decay", 0.3f}, {"op4_sustain", 0.0f}, {"op4_release", 0.2f}, {"op4_keyscale", 4.0f},
          {"feedback", 0.05f},
          {"chorus_mix", 0.15f}, {"reverb_mix", 0.4f}, {"reverb_size", 0.7f}, {"master_gain", -8.0f}}},

        {"Brass Stab",  // 3モジュレーター重ねのDX7ブラス: (2+3+4)→1
         // 他プリセットと比べて大きすぎたためmaster_gainをさらに下げた
         {{"algorithm", 3}, {"feedback", 0.15f},
          {"op1_level", 0.95f}, {"op1_attack", 0.01f}, {"op1_decay", 0.3f}, {"op1_sustain", 0.75f}, {"op1_release", 0.25f}, {"op1_velsens", 0.4f},
          {"op2_ratio", 1.0f}, {"op2_level", 0.5f}, {"op2_attack", 0.02f}, {"op2_decay", 0.4f}, {"op2_sustain", 0.5f}, {"op2_release", 0.2f}, {"op2_velsens", 0.6f},
          {"op3_ratio", 2.0f}, {"op3_level", 0.4f}, {"op3_attack", 0.015f}, {"op3_decay", 0.35f}, {"op3_sustain", 0.4f}, {"op3_release", 0.2f}, {"op3_velsens", 0.6f},
          {"op4_ratio", 3.0f}, {"op4_level", 0.35f}, {"op4_attack", 0.005f}, {"op4_decay", 0.25f}, {"op4_sustain", 0.2f}, {"op4_release", 0.15f}, {"op4_velsens", 0.7f},
          {"chorus_mix", 0.2f}, {"reverb_mix", 0.25f}, {"master_gain", -9.5f}}},

        {"Synth Strings",  // アルゴリズム8（全並列）を4つのデチューン正弦で束ねたパッド弦
         {{"algorithm", 7},
          {"op1_ratio", 1.0f}, {"op1_detune", -6.0f}, {"op1_level", 0.5f}, {"op1_attack", 0.6f}, {"op1_decay", 1.0f}, {"op1_sustain", 0.85f}, {"op1_release", 1.2f}, {"op1_velsens", 0.1f},
          {"op2_ratio", 1.0f}, {"op2_detune", 6.0f}, {"op2_level", 0.5f}, {"op2_attack", 0.7f}, {"op2_decay", 1.0f}, {"op2_sustain", 0.85f}, {"op2_release", 1.3f}, {"op2_velsens", 0.1f},
          {"op3_ratio", 2.0f}, {"op3_detune", -3.0f}, {"op3_level", 0.15f}, {"op3_attack", 0.8f}, {"op3_decay", 1.2f}, {"op3_sustain", 0.5f}, {"op3_release", 1.0f}, {"op3_velsens", 0.1f},
          {"op4_ratio", 1.0f}, {"op4_detune", 3.0f}, {"op4_level", 0.3f}, {"op4_attack", 0.9f}, {"op4_decay", 1.0f}, {"op4_sustain", 0.8f}, {"op4_release", 1.4f}, {"op4_velsens", 0.1f},
          {"vibrato_rate", 5.0f}, {"vibrato_depth", 5.0f},
          {"chorus_mix", 0.6f}, {"chorus_depth", 0.5f}, {"chorus_rate", 0.5f},
          {"reverb_mix", 0.4f}, {"reverb_size", 0.7f}, {"master_gain", -8.0f}}},

        {"Church Organ",  // 倍音比1,2,3,4の全並列。減衰なしのオルガン的サステイン
         {{"algorithm", 7},
          {"op1_ratio", 1.0f}, {"op1_level", 0.6f}, {"op1_attack", 0.005f}, {"op1_decay", 0.05f}, {"op1_sustain", 1.0f}, {"op1_release", 0.05f},
          {"op2_ratio", 2.0f}, {"op2_level", 0.35f}, {"op2_attack", 0.005f}, {"op2_decay", 0.05f}, {"op2_sustain", 1.0f}, {"op2_release", 0.05f}, {"op2_keyscale", 0.0f},
          {"op3_ratio", 3.0f}, {"op3_level", 0.22f}, {"op3_attack", 0.005f}, {"op3_decay", 0.05f}, {"op3_sustain", 1.0f}, {"op3_release", 0.05f},
          {"op4_ratio", 4.0f}, {"op4_level", 0.15f}, {"op4_attack", 0.005f}, {"op4_decay", 0.05f}, {"op4_sustain", 1.0f}, {"op4_release", 0.05f}, {"op4_keyscale", 0.0f},
          {"chorus_mix", 0.3f}, {"reverb_mix", 0.3f}, {"reverb_size", 0.6f}, {"master_gain", -8.0f}}},

        {"Wood Marimba",  // 非整数比のモジュレーターで木質のアタックを出すマリンバ
         {{"op1_level", 0.9f}, {"op1_attack", 0.001f}, {"op1_decay", 0.5f}, {"op1_sustain", 0.0f}, {"op1_release", 0.3f}, {"op1_velsens", 0.5f},
          {"op2_ratio", 3.5f}, {"op2_level", 0.5f}, {"op2_attack", 0.001f}, {"op2_decay", 0.15f}, {"op2_sustain", 0.0f}, {"op2_release", 0.1f}, {"op2_velsens", 0.6f}, {"op2_keyscale", 4.0f},
          {"op3_level", 0.4f}, {"op3_attack", 0.001f}, {"op3_decay", 0.4f}, {"op3_sustain", 0.0f}, {"op3_release", 0.25f}, {"op3_velsens", 0.4f},
          {"op4_ratio", 3.5f}, {"op4_level", 0.25f}, {"op4_attack", 0.001f}, {"op4_decay", 0.1f}, {"op4_sustain", 0.0f}, {"op4_release", 0.08f}, {"op4_velsens", 0.6f}, {"op4_keyscale", 3.0f},
          {"chorus_mix", 0.1f}, {"reverb_mix", 0.25f}}},

        {"Metal Pad",  // 4→(1,2,3)。メタリックコードを狙い、比を1 / 1.5(完全5度) / 2.5に寄せて
         // 和音時の不協和音を抑えつつ金属的な非整数比の質感は残した。音量も少し下げた
         {{"algorithm", 5}, {"feedback", 0.05f},
          {"op1_level", 0.5f}, {"op1_attack", 0.5f}, {"op1_decay", 1.5f}, {"op1_sustain", 0.7f}, {"op1_release", 1.5f},
          {"op2_ratio", 2.0f}, {"op2_level", 0.35f}, {"op2_attack", 0.6f}, {"op2_decay", 1.5f}, {"op2_sustain", 0.65f}, {"op2_release", 1.6f}, {"op2_keyscale", 2.0f},
          {"op3_ratio", 2.5f}, {"op3_level", 0.28f}, {"op3_attack", 0.7f}, {"op3_decay", 1.5f}, {"op3_sustain", 0.6f}, {"op3_release", 1.7f}, {"op3_keyscale", 2.0f},
          {"op4_ratio", 1.5f}, {"op4_level", 0.32f}, {"op4_attack", 0.4f}, {"op4_decay", 1.0f}, {"op4_sustain", 0.5f}, {"op4_release", 1.0f},
          {"chorus_mix", 0.4f}, {"reverb_mix", 0.45f}, {"reverb_size", 0.8f}, {"master_gain", -11.0f}}},

        {"Harpsichord Pluck",  // 4→3→2→1の単一チェーンで作る鋭いプラック
         {{"algorithm", 0}, {"feedback", 0.05f},
          {"op1_level", 0.9f}, {"op1_attack", 0.001f}, {"op1_decay", 0.4f}, {"op1_sustain", 0.0f}, {"op1_release", 0.15f}, {"op1_velsens", 0.3f},
          {"op2_ratio", 2.0f}, {"op2_level", 0.6f}, {"op2_attack", 0.001f}, {"op2_decay", 0.2f}, {"op2_sustain", 0.0f}, {"op2_release", 0.1f}, {"op2_velsens", 0.5f}, {"op2_keyscale", 2.0f},
          {"op3_ratio", 3.0f}, {"op3_level", 0.5f}, {"op3_attack", 0.001f}, {"op3_decay", 0.15f}, {"op3_sustain", 0.0f}, {"op3_release", 0.08f}, {"op3_velsens", 0.6f}, {"op3_keyscale", 3.0f},
          {"op4_ratio", 5.0f}, {"op4_level", 0.4f}, {"op4_attack", 0.001f}, {"op4_decay", 0.08f}, {"op4_sustain", 0.0f}, {"op4_release", 0.05f}, {"op4_velsens", 0.7f}, {"op4_keyscale", 4.0f},
          {"reverb_mix", 0.15f}, {"master_gain", -5.0f}}},

        {"Analog E-Bass",  // グライド付きモノの太いFMベース（Rub-a-Dub Bassとは別系統）
         {{"algorithm", 0}, {"feedback", 0.2f}, {"mono", 1.0f}, {"portamento", 0.06f},
          {"op1_ratio", 1.0f}, {"op1_level", 1.0f}, {"op1_attack", 0.002f}, {"op1_decay", 0.4f}, {"op1_sustain", 0.7f}, {"op1_release", 0.15f}, {"op1_velsens", 0.2f},
          {"op2_ratio", 1.0f}, {"op2_level", 0.5f}, {"op2_attack", 0.002f}, {"op2_decay", 0.3f}, {"op2_sustain", 0.4f}, {"op2_release", 0.1f}, {"op2_velsens", 0.5f},
          {"op3_ratio", 2.0f}, {"op3_level", 0.2f}, {"op3_attack", 0.002f}, {"op3_decay", 0.2f}, {"op3_sustain", 0.1f}, {"op3_release", 0.08f},
          {"op4_level", 0.15f}, {"op4_attack", 0.002f}, {"op4_decay", 0.15f}, {"op4_sustain", 0.0f}, {"op4_release", 0.05f},
          {"reverb_mix", 0.05f}, {"master_gain", -4.0f}}},

        {"Nylon Whisper",  // ナイロン弦ギター風のFMプラック
         {{"algorithm", 0}, {"feedback", 0.08f},
          {"op1_level", 0.85f}, {"op1_attack", 0.002f}, {"op1_decay", 1.0f}, {"op1_sustain", 0.0f}, {"op1_release", 0.5f}, {"op1_velsens", 0.4f},
          {"op2_ratio", 1.0f}, {"op2_detune", 3.0f}, {"op2_level", 0.35f}, {"op2_attack", 0.001f}, {"op2_decay", 0.35f}, {"op2_sustain", 0.0f}, {"op2_release", 0.2f}, {"op2_velsens", 0.6f}, {"op2_keyscale", 2.0f},
          {"op3_ratio", 2.0f}, {"op3_level", 0.25f}, {"op3_attack", 0.001f}, {"op3_decay", 0.5f}, {"op3_sustain", 0.0f}, {"op3_release", 0.3f}, {"op3_velsens", 0.4f},
          {"op4_level", 0.15f}, {"op4_attack", 0.001f}, {"op4_decay", 0.2f}, {"op4_sustain", 0.0f}, {"op4_release", 0.1f}, {"op4_velsens", 0.6f},
          {"chorus_mix", 0.2f}, {"reverb_mix", 0.25f}, {"master_gain", -6.0f}}},

        {"Crystal Piano",  // 澄んだアコースティックピアノ寄りの明るいエレピ
         {{"op1_level", 0.95f}, {"op1_attack", 0.002f}, {"op1_decay", 2.0f}, {"op1_sustain", 0.0f}, {"op1_release", 1.0f}, {"op1_velsens", 0.5f},
          {"op2_ratio", 7.0f}, {"op2_level", 0.15f}, {"op2_attack", 0.001f}, {"op2_decay", 0.2f}, {"op2_sustain", 0.0f}, {"op2_release", 0.15f}, {"op2_velsens", 0.7f}, {"op2_keyscale", 5.0f},
          {"op3_level", 0.6f}, {"op3_attack", 0.002f}, {"op3_decay", 2.5f}, {"op3_sustain", 0.0f}, {"op3_release", 1.2f}, {"op3_velsens", 0.4f},
          {"op4_ratio", 2.0f}, {"op4_level", 0.12f}, {"op4_attack", 0.001f}, {"op4_decay", 0.3f}, {"op4_sustain", 0.0f}, {"op4_release", 0.2f}, {"op4_velsens", 0.5f}, {"op4_keyscale", 3.0f},
          {"reverb_mix", 0.2f}}},

        {"Analog Glide Lead",  // モノ+ポルタメントのシンセリード。(2+3+4)→1で厚みのある波形を作る
         {{"algorithm", 3}, {"feedback", 0.3f}, {"mono", 1.0f}, {"portamento", 0.15f},
          {"op1_level", 0.9f}, {"op1_attack", 0.005f}, {"op1_decay", 0.2f}, {"op1_sustain", 0.9f}, {"op1_release", 0.2f}, {"op1_velsens", 0.3f},
          {"op2_ratio", 1.0f}, {"op2_level", 0.5f}, {"op2_attack", 0.005f}, {"op2_decay", 0.3f}, {"op2_sustain", 0.5f}, {"op2_release", 0.15f}, {"op2_velsens", 0.4f},
          {"op3_ratio", 2.0f}, {"op3_level", 0.35f}, {"op3_attack", 0.005f}, {"op3_decay", 0.25f}, {"op3_sustain", 0.4f}, {"op3_release", 0.15f}, {"op3_velsens", 0.4f},
          {"op4_level", 0.3f}, {"op4_attack", 0.005f}, {"op4_decay", 0.2f}, {"op4_sustain", 0.3f}, {"op4_release", 0.1f}, {"op4_velsens", 0.5f},
          {"vibrato_rate", 5.5f}, {"vibrato_depth", 25.0f},
          {"chorus_mix", 0.15f}, {"reverb_mix", 0.2f}, {"master_gain", -6.0f}}},

        {"Talking Lead",  // 強めのフィードバックで喋るように揺れるモノリード。(3+4)→2→1
         {{"algorithm", 1}, {"feedback", 0.35f}, {"mono", 1.0f}, {"portamento", 0.05f},
          {"op1_level", 0.9f}, {"op1_attack", 0.005f}, {"op1_decay", 0.3f}, {"op1_sustain", 0.85f}, {"op1_release", 0.3f}, {"op1_velsens", 0.3f},
          {"op2_ratio", 1.0f}, {"op2_level", 0.6f}, {"op2_attack", 0.005f}, {"op2_decay", 0.3f}, {"op2_sustain", 0.6f}, {"op2_release", 0.2f}, {"op2_velsens", 0.5f}, {"op2_keyscale", 2.0f},
          {"op3_ratio", 2.0f}, {"op3_level", 0.5f}, {"op3_attack", 0.005f}, {"op3_decay", 0.25f}, {"op3_sustain", 0.4f}, {"op3_release", 0.15f}, {"op3_velsens", 0.5f},
          {"op4_ratio", 3.0f}, {"op4_level", 0.45f}, {"op4_attack", 0.005f}, {"op4_decay", 0.2f}, {"op4_sustain", 0.3f}, {"op4_release", 0.1f}, {"op4_velsens", 0.6f},
          {"vibrato_rate", 6.0f}, {"vibrato_depth", 15.0f},
          {"chorus_mix", 0.1f}, {"reverb_mix", 0.15f}, {"master_gain", -6.0f}}},

        {"Insect Pluck",  // 虫の音（Insects）をキャリアにしたパーカッシブなプラック。FMは短いクリックのみ
         // 他プリセットに比べて体感音量が小さすぎたため底上げ
         {{"op1_level", 0.35f}, {"op1_attack", 0.001f}, {"op1_decay", 0.08f}, {"op1_sustain", 0.0f}, {"op1_release", 0.05f}, {"op1_velsens", 0.5f},
          {"op2_level", 0.0f}, {"op3_level", 0.0f}, {"op4_level", 0.0f},
          {"nature_type", 4}, {"nature_level", 1.0f}, {"nature_keytrack", 1.0f},
          {"nature_attack", 0.001f}, {"nature_decay", 0.15f}, {"nature_sustain", 0.0f}, {"nature_release", 0.08f},
          {"nature_cutoff", 5000.0f}, {"nature_res", 0.5f},
          {"chorus_mix", 0.1f}, {"reverb_mix", 0.2f}, {"master_gain", 3.0f}}},

        {"Thunder Fixed Bass",  // 雷鳴の低音ベース: 鍵盤の音程に追従（以前は両opがFixed周波数で固定され、
         // 何を弾いても55Hzのままだったバグを修正 — キャリアop1を音階追従にし、
         // op2はモジュレーターとして倍音比を保ったまま追従させ、Rainの荒々しいアタックだけを"固定色"として残す
         {{"algorithm", 0}, {"feedback", 0.1f},
          {"op1_ratio", 0.5f}, {"op1_level", 1.0f}, {"op1_attack", 0.05f}, {"op1_decay", 1.5f}, {"op1_sustain", 0.6f}, {"op1_release", 1.2f}, {"op1_velsens", 0.2f},
          {"op2_ratio", 1.0f}, {"op2_level", 0.3f}, {"op2_attack", 0.03f}, {"op2_decay", 0.8f}, {"op2_sustain", 0.2f}, {"op2_release", 0.5f},
          {"op3_level", 0.0f}, {"op4_level", 0.0f},
          {"nature_type", 5}, {"nature_level", 0.7f}, {"nature_keytrack", 0.1f},
          {"nature_attack", 0.001f}, {"nature_decay", 0.3f}, {"nature_sustain", 0.2f}, {"nature_release", 0.3f},
          {"nature_cutoff", 1200.0f}, {"nature_res", 0.6f},
          {"tex_wind", 0.2f}, {"tex_master", 0.4f},
          {"reverb_mix", 0.4f}, {"reverb_size", 0.8f}, {"master_gain", -9.0f}}},

        {"Wind Choir",  // 風（Wind NATURE OSC）だけで作るパッド。FMオペレーターは全休止
         {{"op1_level", 0.0f}, {"op2_level", 0.0f}, {"op3_level", 0.0f}, {"op4_level", 0.0f},
          {"nature_type", 1}, {"nature_level", 0.9f}, {"nature_keytrack", 1.0f},
          {"nature_attack", 1.2f}, {"nature_decay", 1.0f}, {"nature_sustain", 0.85f}, {"nature_release", 2.0f},
          {"nature_cutoff", 2500.0f}, {"nature_res", 0.3f},
          {"chorus_mix", 0.5f}, {"chorus_depth", 0.4f}, {"chorus_rate", 0.4f},
          {"reverb_mix", 0.55f}, {"reverb_size", 0.9f}}},

        {"Creek Pluck",  // 水（Water NATURE OSC）の音程付き泡プラック + 軽いFMクリック
         // 他プリセットに比べて体感音量がかなり小さすぎたため底上げ
         {{"op1_level", 0.35f}, {"op1_attack", 0.001f}, {"op1_decay", 0.1f}, {"op1_sustain", 0.0f}, {"op1_release", 0.05f},
          {"op2_level", 0.0f}, {"op3_level", 0.0f}, {"op4_level", 0.0f},
          {"nature_type", 2}, {"nature_level", 1.0f}, {"nature_keytrack", 1.0f},
          {"nature_attack", 0.001f}, {"nature_decay", 0.25f}, {"nature_sustain", 0.0f}, {"nature_release", 0.15f},
          {"nature_cutoff", 6000.0f}, {"nature_res", 0.35f},
          {"chorus_mix", 0.15f}, {"reverb_mix", 0.25f}, {"master_gain", 6.0f}}},

        {"Cricket Veil",  // 鍵盤のFM音を完全に消し、虫の音素（Insects NATURE OSC）だけを
         // 完全キートラックで鍵盤演奏する。虫の鳴き声そのものをメロディとして弾ける
         {{"op1_level", 0.0f}, {"op2_level", 0.0f}, {"op3_level", 0.0f}, {"op4_level", 0.0f},
          {"nature_type", 4}, {"nature_level", 0.95f}, {"nature_keytrack", 1.0f},
          {"nature_attack", 0.01f}, {"nature_decay", 0.2f}, {"nature_sustain", 0.6f}, {"nature_release", 0.3f},
          {"nature_cutoff", 6000.0f}, {"nature_res", 0.3f},
          {"chorus_mix", 0.3f}, {"reverb_mix", 0.4f}, {"reverb_size", 0.6f}, {"master_gain", -3.0f}}},

        {"Sparrow Whistle",  // 鳥の声（Birds）だけの独奏リード。完全キートラックでさえずりがメロディを歌う
         {{"op1_level", 0.0f}, {"op2_level", 0.0f}, {"op3_level", 0.0f}, {"op4_level", 0.0f},
          {"nature_type", 3}, {"nature_level", 0.9f}, {"nature_keytrack", 1.0f},
          {"nature_attack", 0.02f}, {"nature_decay", 0.3f}, {"nature_sustain", 0.5f}, {"nature_release", 0.4f},
          {"nature_cutoff", 9000.0f}, {"nature_res", 0.25f},
          {"chorus_mix", 0.2f}, {"reverb_mix", 0.3f}, {"reverb_size", 0.6f}, {"master_gain", -6.0f}}},

        {"Pebble Kalimba",  // 雨（Rain）を高レゾナンスで石のように調律したカリンバ風パーカッション
         // 他プリセットに比べて体感音量が小さすぎたため底上げ
         {{"op1_level", 0.5f}, {"op1_attack", 0.001f}, {"op1_decay", 0.3f}, {"op1_sustain", 0.0f}, {"op1_release", 0.15f}, {"op1_velsens", 0.4f},
          {"op2_level", 0.0f}, {"op3_level", 0.0f}, {"op4_level", 0.0f},
          {"nature_type", 5}, {"nature_level", 0.85f}, {"nature_keytrack", 1.0f},
          {"nature_attack", 0.001f}, {"nature_decay", 0.4f}, {"nature_sustain", 0.0f}, {"nature_release", 0.2f},
          {"nature_cutoff", 3500.0f}, {"nature_res", 0.7f},
          {"chorus_mix", 0.1f}, {"reverb_mix", 0.2f}, {"master_gain", 3.0f}}},

        {"Leaf Rustle Guitar",  // ナイロンギター風プラック + 草木のざわめき（Wind、低キートラック）
         {{"algorithm", 0}, {"feedback", 0.05f},
          {"op1_level", 0.8f}, {"op1_attack", 0.002f}, {"op1_decay", 0.9f}, {"op1_sustain", 0.0f}, {"op1_release", 0.4f}, {"op1_velsens", 0.4f},
          {"op2_ratio", 1.0f}, {"op2_detune", 4.0f}, {"op2_level", 0.3f}, {"op2_attack", 0.001f}, {"op2_decay", 0.3f}, {"op2_sustain", 0.0f}, {"op2_release", 0.15f}, {"op2_velsens", 0.6f}, {"op2_keyscale", 2.0f},
          {"op3_ratio", 2.0f}, {"op3_level", 0.2f}, {"op3_attack", 0.001f}, {"op3_decay", 0.4f}, {"op3_sustain", 0.0f}, {"op3_release", 0.2f},
          {"op4_level", 0.12f}, {"op4_attack", 0.001f}, {"op4_decay", 0.15f}, {"op4_sustain", 0.0f}, {"op4_release", 0.08f},
          {"nature_type", 1}, {"nature_level", 0.25f}, {"nature_keytrack", 0.2f},
          {"nature_attack", 0.01f}, {"nature_decay", 0.3f}, {"nature_sustain", 0.1f}, {"nature_release", 0.3f},
          {"nature_cutoff", 4000.0f}, {"nature_res", 0.25f},
          {"tex_wind", 0.15f}, {"tex_master", 0.35f},
          {"chorus_mix", 0.15f}, {"reverb_mix", 0.2f}, {"master_gain", -6.0f}}},

        {"Distant Thunder Swell",  // グローバルTEXTURE主体: 遠雷とともにゆっくり開くパッド
         {{"algorithm", 4},
          {"op1_level", 0.6f}, {"op1_attack", 1.0f}, {"op1_decay", 1.5f}, {"op1_sustain", 0.8f}, {"op1_release", 2.0f},
          {"op2_ratio", 1.0f}, {"op2_level", 0.4f}, {"op2_attack", 1.2f}, {"op2_decay", 1.0f}, {"op2_sustain", 0.5f}, {"op2_release", 1.0f},
          {"op3_ratio", 1.0f}, {"op3_detune", -7.0f}, {"op3_level", 0.5f}, {"op3_attack", 1.3f}, {"op3_decay", 1.5f}, {"op3_sustain", 0.8f}, {"op3_release", 2.2f},
          {"op4_ratio", 1.0f}, {"op4_detune", 7.0f}, {"op4_level", 0.45f}, {"op4_attack", 1.5f}, {"op4_decay", 1.5f}, {"op4_sustain", 0.75f}, {"op4_release", 2.4f},
          {"tex_wind", 0.6f}, {"tex_rain", 0.5f}, {"tex_rain_density", 0.6f}, {"tex_resmix", 0.55f}, {"tex_distance", 0.75f}, {"tex_master", 0.85f},
          {"breath", 0.7f}, {"swell", 0.5f},
          {"chorus_mix", 0.35f}, {"reverb_mix", 0.5f}, {"reverb_size", 0.85f}, {"master_gain", -8.0f}}},

        {"Rainforest Canopy",  // エレピ + 鳥/虫/風のTEXTURE層 + 控えめなBirds NATURE OSCの複合アンビエンス
         {{"chorus_mix", 0.3f}, {"reverb_mix", 0.3f},
          {"tex_birds", 0.3f}, {"tex_insects", 0.25f}, {"tex_wind", 0.3f}, {"tex_water", 0.15f},
          {"tex_resmix", 0.5f}, {"tex_distance", 0.5f}, {"tex_master", 0.7f},
          {"breath", 0.4f}, {"swell", 0.2f},
          {"nature_type", 3}, {"nature_level", 0.35f}, {"nature_keytrack", 0.8f},
          {"nature_attack", 0.1f}, {"nature_decay", 0.4f}, {"nature_sustain", 0.5f}, {"nature_release", 0.6f},
          {"nature_cutoff", 7000.0f}, {"nature_res", 0.25f},
          {"master_gain", -7.0f}}},

        {"Playground Echo",  // 遠くから聞こえる子供たちの声を思わせる音: Birdsを低め移調し距離感/残響で包む
         {{"algorithm", 7},
          {"op1_level", 0.35f}, {"op1_attack", 0.5f}, {"op1_decay", 1.0f}, {"op1_sustain", 0.6f}, {"op1_release", 1.2f},
          {"op2_level", 0.1f},
          {"op3_ratio", 1.0f}, {"op3_detune", 5.0f}, {"op3_level", 0.3f}, {"op3_attack", 0.6f}, {"op3_decay", 1.0f}, {"op3_sustain", 0.55f}, {"op3_release", 1.3f},
          {"op4_level", 0.1f},
          {"nature_type", 3}, {"nature_pitch", -12.0f}, {"nature_level", 0.5f}, {"nature_keytrack", 0.6f},
          {"nature_attack", 0.15f}, {"nature_decay", 0.4f}, {"nature_sustain", 0.4f}, {"nature_release", 0.6f},
          {"nature_cutoff", 3500.0f}, {"nature_res", 0.2f},
          {"tex_birds", 0.2f}, {"tex_distance", 0.8f}, {"tex_master", 0.5f},
          {"chorus_mix", 0.5f}, {"chorus_depth", 0.4f}, {"reverb_mix", 0.55f}, {"reverb_size", 0.9f}, {"master_gain", -8.0f}}},

        {"Submarine Sub",  // ノートトラックのサブベース + Fixed低音ドローン + 低いWaterの泡
         {{"algorithm", 0}, {"feedback", 0.15f}, {"mono", 1.0f}, {"portamento", 0.03f},
          {"op1_ratio", 0.5f}, {"op1_level", 1.0f}, {"op1_attack", 0.01f}, {"op1_decay", 1.0f}, {"op1_sustain", 0.7f}, {"op1_release", 0.3f}, {"op1_velsens", 0.2f},
          {"op2_fixed", 1.0f}, {"op2_fixedhz", 40.0f}, {"op2_level", 0.3f}, {"op2_attack", 0.02f}, {"op2_decay", 1.2f}, {"op2_sustain", 0.4f}, {"op2_release", 0.5f},
          {"op3_level", 0.1f}, {"op3_attack", 0.02f}, {"op3_decay", 0.3f}, {"op3_sustain", 0.1f}, {"op3_release", 0.15f},
          {"op4_level", 0.08f},
          {"nature_type", 2}, {"nature_pitch", -12.0f}, {"nature_level", 0.4f}, {"nature_keytrack", 0.7f},
          {"nature_attack", 0.02f}, {"nature_decay", 0.5f}, {"nature_sustain", 0.3f}, {"nature_release", 0.3f},
          {"nature_cutoff", 1200.0f}, {"nature_res", 0.3f},
          {"reverb_mix", 0.1f}, {"master_gain", -5.0f}}},
    };
    return presets;
}

// プリセット適用: 全パラメーターをデフォルトへ戻してから上書きする
inline void applyPreset(juce::AudioProcessorValueTreeState& apvts, int index) {
    const auto& list = factoryPresets();
    if (index < 0 || index >= (int) list.size()) return;

    for (auto* p : apvts.processor.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(p))
            rp->setValueNotifyingHost(rp->getDefaultValue());
    for (const auto& [id, value] : list[(size_t) index].values)
        if (auto* rp = apvts.getParameter(id))
            rp->setValueNotifyingHost(rp->convertTo0to1(value));
}

} // namespace raiu::presets
