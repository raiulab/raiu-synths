#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

namespace raiu {

Raiu1Processor::Raiu1Processor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", params::createLayout()) {
    synth.addSound(new FMSound());
    for (int i = 0; i < kNumVoices; ++i)
        synth.addVoice(new FMSynthVoice());
}

void Raiu1Processor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 48000.0;
    synth.setCurrentPlaybackSampleRate(sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<FMSynthVoice*>(synth.getVoice(i)))
            v->prepare(sampleRate, samplesPerBlock);
    monoEngine.setSampleRate(sampleRate);
    monoScratch.resize((size_t) samplesPerBlock);
    textureEngine.setSampleRate(sampleRate);
    texScratch.resize((size_t) samplesPerBlock);
    texFollower.setSampleRate(sampleRate, 0.01f, 0.2f);
    // ベロシティ開き: 約1.5秒、Swell: 約0.8秒で減衰（ブロック単位で適用）
    velEnvDecay = std::exp(-(float) samplesPerBlock / (1.5f * (float) sampleRate));
    swellEnvDecay = std::exp(-(float) samplesPerBlock / (0.8f * (float) sampleRate));
    heldNotes.reserve(32);
    masterGain.reset(sampleRate, 0.02);
    polyHeadroom.reset(sampleRate, 0.02);
    polyHeadroom.setCurrentAndTargetValue(1.0f);
    fx.prepare({sampleRate, (juce::uint32) samplesPerBlock, 2});
    setLatencySamples(fx.getLatencySamples());
}

void Raiu1Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // ホストが prepareToPlay で申告したサイズより大きいブロックを送ってくる
    // 可能性へのガード（通常はno-op。スクラッチバッファの範囲外書き込みを防ぐ）
    const int hostBlockSize = buffer.getNumSamples();
    if ((int) texScratch.size() < hostBlockSize) texScratch.resize((size_t) hostBlockSize);
    if ((int) monoScratch.size() < hostBlockSize) monoScratch.resize((size_t) hostBlockSize);

    const auto engineParams = params::readEngineParams(apvts);
    const auto natureParams = params::readNatureParams(apvts);
    const float porta = apvts.getRawParameterValue("portamento")->load();
    const bool mono = apvts.getRawParameterValue("mono")->load() > 0.5f;

    // ピッチベンド更新 + テクスチャ共鳴へのノート転送
    for (const auto meta : midi) {
        const auto msg = meta.getMessage();
        if (msg.isPitchWheel())
            bendCents = (float) (msg.getPitchWheelValue() - 8192) / 8192.0f * 200.0f;
        else if (msg.isNoteOn()) {
            textureEngine.noteOn(msg.getNoteNumber());
            velEnv = std::max(velEnv, msg.getFloatVelocity());
            swellEnv = 1.0f;
        }
        else if (msg.isNoteOff())
            textureEngine.noteOff(msg.getNoteNumber());
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            textureEngine.allNotesOff();
    }

    // ビブラートLFO（ブロック単位で更新）
    const float vibRate = apvts.getRawParameterValue("vibrato_rate")->load();
    const float vibDepth = apvts.getRawParameterValue("vibrato_depth")->load();
    const float vibCents = vibDepth * (float) std::sin(vibratoPhase);
    vibratoPhase += 2.0 * juce::MathConstants<double>::pi * vibRate
                    * buffer.getNumSamples() / currentSampleRate;
    if (vibratoPhase > 2.0 * juce::MathConstants<double>::pi)
        vibratoPhase -= 2.0 * juce::MathConstants<double>::pi;

    // Breath: 前ブロックのテクスチャ揺らぎがFMモジュレーターを開く
    const float breath = apvts.getRawParameterValue("breath")->load();
    const float texNorm = juce::jlimit(0.0f, 1.0f, texFollower.current() / 0.25f);
    const float modBoost = 1.0f + breath * 0.5f * texNorm;

    const float pitchOffset = bendCents + vibCents;
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<FMSynthVoice*>(synth.getVoice(i))) {
            v->applyParams(engineParams, porta);
            v->applyNatureParams(natureParams);
            v->setPitchOffsetCents(pitchOffset);
            v->setModulatorBoost(modBoost);
        }
    monoEngine.setParams(engineParams);
    monoEngine.setNatureParams(natureParams);
    monoEngine.setPortamentoTime(porta);
    monoEngine.setPitchOffsetCents(pitchOffset);
    monoEngine.setModulatorBoost(modBoost);

    if (mono != wasMono) {  // モード切替時は鳴っている音を整理する
        synth.allNotesOff(0, true);
        if (!mono) monoEngine.noteOff();
        heldNotes.clear();
        wasMono = mono;
    }

    if (mono) {
        juce::MidiBuffer empty;
        synth.renderNextBlock(buffer, empty, 0, buffer.getNumSamples());  // 残響中のポリ音
        processMonoMidi(buffer, midi);
    } else {
        synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());
    }

    // ポリフォニー加算のためのヘッドルーム（複数キャリア×和音でのクリップ防止）。
    // 単音では基準の0.3を維持しつつ、同時発音数が増える和音・パッド系プリセットでは
    // その分だけ追加で頭打ちを避ける（√で緩やかに効かせ、和音の迫力は残す）。
    // リリースの長いプリセットを素早く連打すると同時発音数がブロックをまたいで
    // 頻繁に増減するため、ここをブロック単位の即時切り替えにすると逆にクリックの
    // 原因になる。SmoothedValueで滑らかに追従させる。
    int activeVoices = 0;
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (synth.getVoice(i)->isVoiceActive()) ++activeVoices;
    polyHeadroom.setTargetValue(activeVoices > 3 ? std::sqrt(3.0f / (float) activeVoices) : 1.0f);
    buffer.applyGain(0.3f);
    polyHeadroom.applyGain(buffer, buffer.getNumSamples());

    // テクスチャレイヤーを加算
    textureEngine.setParams(params::readTextureParams(apvts));
    const int numSamples = buffer.getNumSamples();
    std::fill(texScratch.begin(), texScratch.begin() + numSamples, 0.0f);
    textureEngine.process(texScratch.data(), numSamples);
    for (int i = 0; i < numSamples; ++i)
        texFollower.process(texScratch[(size_t) i]);

    // ベロシティ開き + Swell によるテクスチャゲイン
    const float velAmt = apvts.getRawParameterValue("vel_to_texture")->load();
    const float swellAmt = apvts.getRawParameterValue("swell")->load();
    const float texGain = (1.0f - velAmt + velAmt * velEnv) + swellAmt * swellEnv;
    velEnv *= velEnvDecay;
    swellEnv *= swellEnvDecay;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.addFrom(ch, 0, texScratch.data(), numSamples, texGain);

    // マスターゲインはFXチェーンより先に適用する。こうすることで
    // チェーン末尾のリミッターが常に最終出力を保護する「最後の砦」になる
    // （以前はリミッター→マスターゲインの順で、Masterを上げるとリミッター通過後の
    // 信号がそのまま0dBFSを超えてクリップするバグがあった）
    masterGain.setTargetValue(
        juce::Decibels::decibelsToGain(apvts.getRawParameterValue("master_gain")->load()));
    masterGain.applyGain(buffer, buffer.getNumSamples());

    // エフェクトチェーン（Distanceが上がるとリバーブも深くなる）
    FXChain::Params fxParams;
    fxParams.chorusRate = apvts.getRawParameterValue("chorus_rate")->load();
    fxParams.chorusDepth = apvts.getRawParameterValue("chorus_depth")->load();
    fxParams.chorusMix = apvts.getRawParameterValue("chorus_mix")->load();
    fxParams.reverbSize = apvts.getRawParameterValue("reverb_size")->load();
    const float texDistance = apvts.getRawParameterValue("tex_distance")->load();
    const float texMaster = apvts.getRawParameterValue("tex_master")->load();
    fxParams.reverbMix = juce::jlimit(
        0.0f, 1.0f,
        apvts.getRawParameterValue("reverb_mix")->load() + 0.25f * texDistance * texMaster);
    fx.setParams(fxParams);
    fx.process(buffer);
}

void Raiu1Processor::processMonoMidi(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi) {
    int pos = 0;
    for (const auto meta : midi) {
        const auto msg = meta.getMessage();
        const int t = juce::jlimit(0, buffer.getNumSamples(), (int) meta.samplePosition);
        renderMonoSegment(buffer, pos, t - pos);
        pos = t;

        if (msg.isNoteOn()) {
            const int note = msg.getNoteNumber();
            const float vel = msg.getFloatVelocity();
            std::erase_if(heldNotes, [note](const auto& p) { return p.first == note; });
            heldNotes.emplace_back(note, vel);
            monoEngine.noteOn(note, vel);  // 発音中ならポルタメントでグライド
        } else if (msg.isNoteOff()) {
            const int note = msg.getNoteNumber();
            std::erase_if(heldNotes, [note](const auto& p) { return p.first == note; });
            if (monoEngine.currentNote() == note) {
                if (!heldNotes.empty())  // 後着優先: 直前に押していた鍵へ戻る
                    monoEngine.noteOn(heldNotes.back().first, heldNotes.back().second);
                else
                    monoEngine.noteOff();
            }
        } else if (msg.isAllNotesOff() || msg.isAllSoundOff()) {
            heldNotes.clear();
            monoEngine.noteOff();
        }
    }
    renderMonoSegment(buffer, pos, buffer.getNumSamples() - pos);
}

void Raiu1Processor::renderMonoSegment(juce::AudioBuffer<float>& buffer, int start, int numSamples) {
    if (numSamples <= 0 || !monoEngine.isActive()) return;
    std::fill(monoScratch.begin(), monoScratch.begin() + numSamples, 0.0f);
    monoEngine.render(monoScratch.data(), numSamples);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.addFrom(ch, start, monoScratch.data(), numSamples);
}

int Raiu1Processor::getNumPrograms() {
    return (int) presets::factoryPresets().size();
}

void Raiu1Processor::setCurrentProgram(int index) {
    if (index < 0 || index >= getNumPrograms()) return;
    currentProgram = index;
    presets::applyPreset(apvts, index);
}

const juce::String Raiu1Processor::getProgramName(int index) {
    const auto& list = presets::factoryPresets();
    if (index < 0 || index >= (int) list.size()) return {};
    return list[(size_t) index].name;
}

juce::AudioProcessorEditor* Raiu1Processor::createEditor() {
    return new Raiu1Editor(*this);
}

void Raiu1Processor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    state.setProperty("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void Raiu1Processor::setStateInformation(const void* data, int sizeInBytes) {
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType())) {
            auto state = juce::ValueTree::fromXml(*xml);
            currentProgram = (int) state.getProperty("program", 0);
            apvts.replaceState(state);
        }
}

} // namespace raiu

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new raiu::Raiu1Processor();
}
