# RAIU-1 "Breeze FM" Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Logic Pro で動く AU 音源プラグイン「RAIU-1 Breeze FM」（4オペFM + プロシージャル環境音テクスチャ）を完成させる。

**Architecture:** DSP コアは JUCE 非依存のプレーン C++ ヘッダー（`plugins/raiu1/dsp/`）として書き、Catch2 で単体テストする。JUCE はプラグインの器（`PluginProcessor`）・パラメーター管理（APVTS）・エフェクト（juce::dsp）にのみ使う。ボイス管理は `juce::Synthesiser` + カスタム Voice。設計書: `docs/plans/2026-07-20-raiu1-breeze-fm-design.md`

**Tech Stack:** C++20 / JUCE 8.0.9 (FetchContent) / Catch2 v3 (FetchContent) / CMake ≥3.24 / macOS arm64 / AU + VST3 + Standalone

**検証コマンド（頻用）:**
- テスト: `cmake --build build --target raiu1_tests && ./build/plugins/raiu1/raiu1_tests`
- AU検証: `auval -v aumu Rai1 Raiu`（ビルドで `~/Library/Audio/Plug-Ins/Components/` に自動コピーされる）

---

## Task 1: CMake 骨格 + 空プラグインのビルド

**Files:**
- Create: `CMakeLists.txt`
- Create: `plugins/raiu1/CMakeLists.txt`
- Create: `plugins/raiu1/src/PluginProcessor.h` / `.cpp`
- Create: `.gitignore`

**Step 1: ルート CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.24)
project(RAIU_SYNTHS VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES arm64)

include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.9 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.7.1 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(Catch2)

add_subdirectory(plugins/raiu1)
```

**Step 2: plugins/raiu1/CMakeLists.txt**

```cmake
juce_add_plugin(RAIU1
  PRODUCT_NAME "RAIU-1 Breeze FM"
  COMPANY_NAME "RAIU"
  BUNDLE_ID com.raiu.raiu1
  PLUGIN_MANUFACTURER_CODE Raiu
  PLUGIN_CODE Rai1
  FORMATS AU VST3 Standalone
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  COPY_PLUGIN_AFTER_BUILD TRUE)

target_sources(RAIU1 PRIVATE src/PluginProcessor.cpp)
target_compile_definitions(RAIU1 PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(RAIU1 PRIVATE
  juce::juce_audio_utils juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
```

**Step 3: 最小の PluginProcessor**（無音を出すシンセ。`juce::AudioProcessor` 派生、
`isMidiEffect=false`, `acceptsMidi=true`、`createEditor` は `GenericAudioProcessorEditor`）

**Step 4: ビルド確認**

Run: `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --target RAIU1_AU RAIU1_Standalone`
Expected: ビルド成功、`~/Library/Audio/Plug-Ins/Components/RAIU-1 Breeze FM.component` が生成される

**Step 5: auval 確認**

Run: `auval -v aumu Rai1 Raiu`
Expected: `AU VALIDATION SUCCEEDED`

**Step 6: Commit**（`.gitignore` に `build/` を追加してから）

---

## Task 2: テストハーネス

**Files:**
- Modify: `plugins/raiu1/CMakeLists.txt`（`raiu1_tests` ターゲット追加）
- Create: `plugins/raiu1/tests/test_sanity.cpp`

**Step 1:** Catch2 テストターゲットを追加

```cmake
add_executable(raiu1_tests tests/test_sanity.cpp)
target_include_directories(raiu1_tests PRIVATE .)
target_link_libraries(raiu1_tests PRIVATE Catch2::Catch2WithMain)
```

**Step 2:** `TEST_CASE("sanity") { REQUIRE(1 + 1 == 2); }` を書き、ビルド・実行して PASS を確認

**Step 3: Commit**

---

## Task 3: SineTable（正弦波テーブル）— TDD

**Files:**
- Create: `plugins/raiu1/dsp/SineTable.h`
- Create: `plugins/raiu1/tests/test_sine_table.cpp`

**Step 1: 失敗するテストを書く**

```cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "dsp/SineTable.h"
#include <cmath>

TEST_CASE("SineTable matches std::sin within 1e-4") {
    for (double p = 0.0; p < 1.0; p += 0.001)  // phase in [0,1)
        REQUIRE(raiu::SineTable::lookup(p)
                == Catch::Approx(std::sin(p * 2.0 * M_PI)).margin(1e-4));
}
TEST_CASE("SineTable handles phase outside [0,1)") {
    REQUIRE(raiu::SineTable::lookup(1.25)
            == Catch::Approx(raiu::SineTable::lookup(0.25)).margin(1e-6));
    REQUIRE(std::isfinite(raiu::SineTable::lookup(-0.3)));
}
```

**Step 2:** 実行して FAIL（ヘッダー不在）を確認

**Step 3: 実装** — 4096 点テーブル + 線形補間。`static` テーブルを一度だけ生成。
位相は `p - std::floor(p)` で折り返す。

**Step 4:** テスト PASS を確認 → **Step 5: Commit**

---

## Task 4: ADSREnvelope — TDD

**Files:**
- Create: `plugins/raiu1/dsp/ADSREnvelope.h`
- Create: `plugins/raiu1/tests/test_adsr.cpp`

**Step 1: 失敗するテスト**

```cpp
TEST_CASE("ADSR reaches ~1.0 at end of attack") {
    raiu::ADSREnvelope env; env.setSampleRate(48000.0);
    env.setParams(0.01f, 0.1f, 0.5f, 0.2f);  // A,D,S,R (sec, sec, level, sec)
    env.noteOn();
    float v = 0; for (int i = 0; i < 480; ++i) v = env.next();
    REQUIRE(v > 0.95f);
}
TEST_CASE("ADSR decays to sustain level") { /* 0.5±0.05 после A+D×5 */ }
TEST_CASE("ADSR releases below 0.001 and reports inactive") {
    // noteOff 後 release×6 経過で isActive()==false
}
TEST_CASE("ADSR output is always in [0,1] and finite") { /* 全区間サンプル検査 */ }
```

**Step 2:** FAIL 確認
**Step 3: 実装** — 状態機械（Idle/Attack/Decay/Sustain/Release）。各段は
1-pole 指数近似（`coef = 1 - exp(-1/(time*sr/ 3))` 型）で滑らかに。
**Step 4:** PASS → **Step 5: Commit**

---

## Task 5: FMOperator — TDD

**Files:**
- Create: `plugins/raiu1/dsp/FMOperator.h`
- Create: `plugins/raiu1/tests/test_fm_operator.cpp`

仕様: `float next(float modInput)` — `phase += (fixed ? fixedHz : baseHz * ratio * detuneFactor) / sr`、
出力 `= level * env.next() * SineTable::lookup(phase + modInput + feedback * prevOut)`。
`modInput` は位相単位（1.0 = 1周期）。feedback は自己出力を位相へ帰還。

**Step 1: 失敗するテスト**

```cpp
TEST_CASE("Operator with ratio 1 produces base frequency") {
    // 440Hz, sr=48000, mod=0, env=最速アタック・サスティン1
    // 4800サンプル生成し、ゼロクロス数から周波数を推定 → 440±1Hz
}
TEST_CASE("Operator with ratio 2 doubles frequency") { /* 880±2Hz */ }
TEST_CASE("Fixed mode ignores note frequency") { /* fixedHz=100 → 100Hz */ }
TEST_CASE("Level 0 gives silence") { /* 全サンプル == 0 */ }
TEST_CASE("Modulation input changes the spectrum") {
    // mod=0 の出力と、正弦mod入力ありの出力が一致しないこと
}
```

**Step 2:** FAIL → **Step 3: 実装** → **Step 4:** PASS → **Step 5: Commit**

---

## Task 6: FMAlgorithm（8アルゴリズムのルーティング）— TDD

**Files:**
- Create: `plugins/raiu1/dsp/FMAlgorithm.h`
- Create: `plugins/raiu1/tests/test_fm_algorithm.cpp`

仕様: アルゴリズム = 4×4 変調行列 + キャリアマスク。定義（op番号は 1..4、`→` は変調）:

| # | 構成 | キャリア | 用途 |
|---|---|---|---|
| 1 | 4→3→2→1 | 1 | 深い直列。ブラス・ハード系 |
| 2 | (3+4)→2→1 | 1 | 複雑な倍音 |
| 3 | (2→1) + (4→3) | 1,3 | **エレピ定番**（2ペア） |
| 4 | (2+3+4)→1 | 1 | 密集変調。ベル |
| 5 | (2→1) + 3 + 4 | 1,3,4 | FM+オルガン混合 |
| 6 | 4→(1,2,3) | 1,2,3 | 1モジュレーター3キャリア。パッド |
| 7 | (4→3→2) + 1 | 1,2 | 直列+純音 |
| 8 | 1+2+3+4 | 1,2,3,4 | 全並列（加算/オルガン） |

feedback は常に op4 に適用。

**Step 1: 失敗するテスト**

```cpp
TEST_CASE("Algorithm 8 sums all carriers") {
    // 全op同一設定・mod無し → 出力は単一opの4倍（±1%）
}
TEST_CASE("Algorithm 1 outputs only op1") {
    // op1 の level=0 にすると出力が完全に無音になる
}
TEST_CASE("Algorithm 3 has two independent pairs") {
    // op1とop3のlevelを0 → 無音。op2とop4のlevel0 → 非無音（キャリアは生きる）
}
TEST_CASE("All 8 algorithms produce finite bounded output") { /* NaN/inf無し, |x|<=4 */ }
```

**Step 2〜5:** FAIL → 実装（行列データ駆動の `renderSample()`）→ PASS → Commit

---

## Task 7: FMVoiceEngine（1ボイス統合）— TDD

**Files:**
- Create: `plugins/raiu1/dsp/FMVoiceEngine.h`
- Create: `plugins/raiu1/tests/test_fm_voice.cpp`

仕様: noteOn(note, velocity) / noteOff() / `render(float* out, int n)`。
- MIDIノート→周波数（A4=440）
- ベロシティ感度: `opLevel *= mix(1, velocity, velSens)`
- キースケーリング: `opLevel *= dB→linear(-ksDbPerOct * (note - 60) / 12)`（ksは op ごと 0〜12dB/oct）
- ポルタメント: 目標周波数へ 1-pole グライド（time パラメーター）
- 全オペのエンベロープが inactive になったら `isActive()==false`

**テスト:** ピッチ正確性 / velSens=1 で velocity 0.5 の出力レベル半減 /
キースケーリングで高音ほど小さい / noteOff 後に必ず無音化 / NaN無し。

FAIL → 実装 → PASS → Commit。

---

## Task 8: プラグイン統合（M2: FMコアが Logic で鳴る）

**Files:**
- Create: `plugins/raiu1/src/Params.h`（APVTS レイアウト定義）
- Create: `plugins/raiu1/src/FMSound.h` / `FMSynthVoice.h`（juce::Synthesiser 接続）
- Modify: `plugins/raiu1/src/PluginProcessor.h/.cpp`

**Step 1: APVTS パラメーター定義**（ID は安定名で固定。後から順序変更可）
- op1..op4 × { ratio, detune, fixed, fixedHz, level, attack, decay, sustain, release, velSens, keyScale }
- グローバル: algorithm(1..8), feedback, portamentoTime, monoMode, vibratoRate, vibratoDepth, masterGain

**Step 2:** `FMSynthVoice : juce::SynthesiserVoice` が `FMVoiceEngine` を内包。
`renderNextBlock` で加算ミックス。16 ボイス。processBlock 冒頭で APVTS 値を各エンジンへ反映。

**Step 3:** ビルドして Standalone を起動、MIDI キーボードまたは画面キーボードで発音確認。

**Step 4:** `auval -v aumu Rai1 Raiu` → PASS 確認

**Step 5:** Logic Pro で挿してデフォルト音色（アルゴ3のエレピ設定を初期値にする）を確認

**Step 6: Commit**（マイルストーン M2 達成）

---

## Task 9: ビブラート LFO + ピッチベンド + モノモード

**Files:**
- Modify: `plugins/raiu1/dsp/FMVoiceEngine.h`（pitchMod 入力）
- Modify: `plugins/raiu1/src/PluginProcessor.cpp`（LFO・ベンド配線、mono 時は最後のノート優先レガート）
- Test: `plugins/raiu1/tests/test_fm_voice.cpp` に追加

**テスト:** vibratoDepth>0 で瞬時周波数が周期変動する / ベンド+2半音で周波数が 2^(2/12) 倍。
FAIL → 実装 → PASS → Standalone で耳確認 → Commit。

---

## Task 10: WindGenerator（風）— TDD

**Files:**
- Create: `plugins/raiu1/dsp/texture/WindGenerator.h`
- Create: `plugins/raiu1/dsp/texture/TextureCommon.h`（乱数 xorshift、1-pole LP/HP、biquad BP、Poissonトリガー等の共通部品）
- Create: `plugins/raiu1/tests/test_textures.cpp`

仕様: 白色ノイズ → 2次LP（カットオフ 200〜1500Hz を 0.1〜0.4Hz の平滑ランダムで揺らす）
＋「葉ずれ」= 2〜5kHz の BP ノイズを別の低速ランダムで振幅変調。2 成分をミックス。

**全テクスチャ共通テスト（このタスクで雛形を作る）:**

```cpp
// 各ジェネレーターに対して:
// 1) 5秒分レンダリングして NaN/inf が無い
// 2) 全サンプル |x| <= 1.0
// 3) RMS が 0.01〜0.5 の範囲（無音でも爆音でもない）
// 4) 出力に長期的な変化がある（1秒ごとのRMSの分散 > 0 → 「揺らぎ」の存在確認）
```

FAIL → 実装 → PASS → Commit。

---

## Task 11: WaterGenerator(水) / Task 12: BirdsGenerator(鳥) / Task 13: InsectsGenerator(虫) / Task 14: RainGenerator(雨)

各タスクとも Task 10 と同一手順（共通テストに追加 → FAIL → 実装 → PASS → Standalone で耳確認 → Commit）。合成仕様:

- **Water**: Poisson トリガー（密度可変）で「泡」を生成。泡 = 開始 400〜1200Hz から
  指数的に上昇する正弦チャープ（80ms、指数減衰）。+ 低域の「せせらぎ」= 300〜800Hz BP
  ノイズを 2〜6Hz のランダムで振幅変調。
- **Birds**: Poisson トリガー（平均 2〜4秒間隔）で「さえずり」= 2〜5kHz の正弦、
  周波数は 3 点（開始/頂点/終了）を 2 次補間でスイープ、長さ 60〜250ms、
  30% の確率で 2 連続。ステレオ位置ランダム（後段で対応）。
- **Insects**: キャリア 4.2kHz ± ランダムの正弦を 25〜35Hz パルストレインで AM。
  「鳴き」は 0.5秒鳴いて 0.3〜1秒休む周期パターン + 個体を 2〜3 重ねる。
- **Rain**: Poisson インパルス（密度＝雨の強さ）→ 各滴は 1〜3ms の減衰ノイズ +
  2kHz LP。+ 背景ヒス（薄い広帯域ノイズ）。密度パラメーターで小雨〜土砂降り。

---

## Task 15: ResonatorBank（キートラッキング共鳴）— TDD

**Files:**
- Create: `plugins/raiu1/dsp/texture/ResonatorBank.h`
- Test: `plugins/raiu1/tests/test_resonator.cpp`

仕様: 最大 8 音の押鍵周波数それぞれに 2 次共鳴 BP（Q 可変 20〜120）を割り当て、
入力（テクスチャバス）を通して合算。ノートオフでゆっくり減衰（クリック防止のQ緩和）。

**テスト:**

```cpp
TEST_CASE("Resonator concentrates noise energy at note frequency") {
    // 白色ノイズ入力、A4(440Hz)をセット
    // Goertzel で 440Hz と 1000Hz の帯域エネルギーを比較 → 440Hz が 10 倍以上
}
TEST_CASE("Resonator is silent with no active notes") { /* RMS < 1e-4 */ }
TEST_CASE("Resonator output is stable (no blowup) at high Q") { /* 10秒レンダで有界 */ }
```

FAIL → 実装 → PASS → Commit。

---

## Task 16: テクスチャバス統合（M3）

**Files:**
- Create: `plugins/raiu1/dsp/texture/TextureEngine.h`（5ジェネレーター + ResonatorBank + ミックス）
- Modify: `src/Params.h`（wind/water/birds/insects/rain の Level、rainDensity、distance、resonanceMix、textureMaster）
- Modify: `src/PluginProcessor.cpp`（processBlock でシンセ出力に加算、押鍵情報を ResonatorBank へ通知）

Standalone で「エレピ + 鳥 + 風」を耳確認 → auval PASS → Commit（M3 達成）。

---

## Task 17: 2層の相互作用（Breath / ベロシティ / Swell）

**Files:**
- Create: `plugins/raiu1/dsp/EnvFollower.h`（abs → 1-pole、attack 10ms / release 200ms）
- Modify: `dsp/FMVoiceEngine.h`（外部から modDepthScale を注入できるように）
- Modify: `src/PluginProcessor.cpp` + `src/Params.h`（breath, velToTexture, swellAmount）
- Test: `tests/test_env_follower.cpp`

仕様:
- Breath: テクスチャバスの EnvFollower 出力 f(0..1) → 全モジュレーターopの
  level を `×(1 + breath × 0.3 × f)` — 音色がわずかに開く
- velToTexture: ノートオンのベロシティでテクスチャ音量ゲインを押し上げ（減衰 1〜2 秒）
- Swell: ノートオンで textureGain に +swellAmount のパルス（attack 50ms / decay 800ms）

**テスト:** EnvFollower がステップ入力に規定時間で追従 / 定常値が入力振幅に一致。
FAIL → 実装 → PASS → 耳確認（Breath を回して「呼吸」を感じるか）→ Commit。

---

## Task 18: エフェクトチェーン（M4）

**Files:**
- Create: `plugins/raiu1/src/FXChain.h`（juce::dsp::Chorus / juce::dsp::Reverb / juce::dsp::Limiter の直列）
- Modify: `src/Params.h`（chorusMix, chorusRate, chorusDepth, reverbMix, reverbSize）
- Modify: `src/PluginProcessor.cpp`

配線: [FM + テクスチャ] → Chorus → Reverb → Limiter。
Distance ノブは「テクスチャの LPF カットオフ低下 + テクスチャのリバーブセンド増加」に同時マップ。
v1 は JUCE 標準 DSP を使う（BBD 風カスタムコーラスは音決め後に差し替え検討）。

auval PASS → Logic で耳確認 → Commit（M4 達成）。

---

## Task 19: プリセット機構 + ファクトリープリセット 8 種（M5）

**Files:**
- Create: `plugins/raiu1/src/Presets.h`（プリセット = 全パラメーター値の表。C++ 構造体の配列として埋め込み）
- Modify: `src/PluginProcessor.cpp`（getNumPrograms / setCurrentProgram 実装 → Logic のプリセットメニューに出る）

設計書 §6 の 8 プリセットを作成。**音決めは Standalone で耳を使って行う**（各プリセット
のパラメーター値はこの段階で調整して確定させる。Lovers Keys はアルゴ3 + コーラス深め、
Rub-a-Dub Bass はアルゴ1 + モノ + ポルタメント少し、等）。

getStateInformation / setStateInformation の保存復元テスト（APVTS の XML 往復）を追加。
auval PASS → Logic でプリセット切替確認 → Commit（M5 達成）。

---

## Task 20: シンプル UI（第1段階）

**Files:**
- Create: `plugins/raiu1/src/PluginEditor.h/.cpp`

GenericAudioProcessorEditor を卒業し、セクション分け（OPERATORS / TEXTURE / INTERACTION / FX）
した最小限のカスタムレイアウト（juce::Slider + SliderAttachment のグリッド）。
凝った見た目は M6（別プラン）で。auval PASS → Commit。

---

## Task 21: 最終検証

**Step 1:** 全テスト実行 → 全 PASS
**Step 2:** `auval -v aumu Rai1 Raiu` → SUCCEEDED
**Step 3:** pluginval を取得して実行:
`pluginval --strictness-level 8 --validate ~/Library/Audio/Plug-Ins/Components/"RAIU-1 Breeze FM.component"`
**Step 4:** Logic Pro 実機チェックリスト: 発音 / 16和音 / プリセット8種 / オートメーション書き込み再生 / プロジェクト保存→再読込で状態復元
**Step 5:** `README.md`（ビルド方法・インストール方法）を作成
**Step 6: Commit** — RAIU-1 v0.1.0 完成

---

## 備考

- 各タスク完了ごとに必ずコミット。テストが赤いままのコミット禁止
- 耳確認（Standalone）はテストで代替できない工程として明示的に実施する
- DSP ヘッダーは JUCE 非依存を保つ（2〜4号機で再利用するため）
- リアルタイムスレッドでのメモリ確保・ロック禁止（prepareToPlay で全確保）
