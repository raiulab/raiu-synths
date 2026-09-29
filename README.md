# RAIU Synths

## 日本語

RAIU-1 “Breeze FM”は、4オペレーターFM音源と、サンプルを使わない風・水・鳥・虫・雨のプロシージャル環境音を組み合わせた実験的なC++ソフトウェアシンセサイザーです。

### 主な特徴

- 8種類のFMアルゴリズムと16ボイス・ポリフォニー
- モノ／レガートモードとポルタメント
- 5種類のプロシージャル自然音ジェネレーター
- キートラッキング対応レゾネーターと、独立したテクスチャ用エンベロープ／フィルター
- コーラス、リバーブ、出力リミッター
- JUCEとCMakeによるAU、VST3、スタンドアロンの各ターゲット
- ユニットテスト、プリセット負荷試験、オフラインレンダリング、Audio Unit検証ツール

### 必要環境

- 記載しているAUワークフローではApple Silicon搭載macOS
- CMake 3.24以上
- Xcode Command Line Tools
- 初回構成時に、固定バージョンのJUCEとCatch2を取得するためのインターネット接続

### ビルド

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target RAIU1_AU RAIU1_Standalone -j 8
```

### テスト

```bash
cmake --build build --target raiu1_tests -j 8
./build/plugins/raiu1/raiu1_tests

./build/plugins/raiu1/raiu1_stress presets
auval -v aumu Rai1 Raiu
swift tools/render_test.swift
swift tools/preset_test.swift
```

### リポジトリ構成

- `plugins/raiu1/dsp/`：再利用可能なDSP部品
- `plugins/raiu1/src/`：プロセッサー、エディター、プリセット、エフェクト、UI
- `plugins/raiu1/tests/`：Catch2によるユニットテストと負荷試験
- `tools/`：macOS用オフラインレンダリング・プリセット検証ツール
- `docs/plans/`：RAIU-1の実装資料

### ライセンス

本プロジェクトのソースコードはAGPL-3.0-onlyで提供します。

JUCEは構成時に取得され、AGPLv3または商用JUCEライセンスのデュアルライセンスで提供されています。利用・配布形態がAGPLv3に準拠しない場合、商用JUCEライセンスが必要になる可能性があります。ビルドまたはバイナリ配布前に、JUCEの最新ライセンスを確認してください：<https://github.com/juce-framework/JUCE/blob/master/LICENSE.md>。

Catch2は独自のBoost Software Licenseで取得されます。このリポジトリにはJUCEとCatch2のソースコードを同梱していません。

---

## English

RAIU-1 “Breeze FM” is an experimental C++ software synthesizer that combines a four-operator FM
engine with sample-free procedural wind, water, bird, insect, and rain textures.

## Highlights

- Eight FM algorithms and 16-voice polyphony
- Mono/legato mode and portamento
- Five procedural nature-texture generators
- Key-tracked resonators and independent texture envelope/filter controls
- Chorus, reverb, and output limiting
- AU, VST3, and standalone targets through JUCE and CMake
- Unit, preset-stress, offline-render, and Audio Unit validation tools

## Requirements

- macOS on Apple Silicon for the documented AU workflow
- CMake 3.24 or later
- Xcode command-line tools
- Internet access during the first configure step to fetch pinned JUCE and Catch2 sources

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target RAIU1_AU RAIU1_Standalone -j 8
```

## Test

```bash
cmake --build build --target raiu1_tests -j 8
./build/plugins/raiu1/raiu1_tests

./build/plugins/raiu1/raiu1_stress presets
auval -v aumu Rai1 Raiu
swift tools/render_test.swift
swift tools/preset_test.swift
```

## Repository layout

- `plugins/raiu1/dsp/`: reusable DSP building blocks
- `plugins/raiu1/src/`: processor, editor, presets, effects, and UI
- `plugins/raiu1/tests/`: Catch2 unit and stress tests
- `tools/`: macOS offline-render and preset-validation tools
- `docs/plans/`: implementation notes for RAIU-1

## Licensing

The project source is released under AGPL-3.0-only.

JUCE is fetched during configuration and is dual-licensed under AGPLv3 and the commercial JUCE
license. A commercial JUCE license may be required if your use or distribution does not comply
with AGPLv3. Review JUCE's current license before building or distributing binaries:
<https://github.com/juce-framework/JUCE/blob/master/LICENSE.md>.

Catch2 is fetched under its own Boost Software License. No JUCE or Catch2 source is vendored in
this repository.
