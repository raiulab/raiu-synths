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

target_sources(RAIU1 PRIVATE
  src/PluginProcessor.cpp
  src/PluginEditor.cpp
  src/ui/RaiuLookAndFeel.cpp
  src/ui/FMAlgorithmView.cpp
  src/ui/NatureOscPad.cpp
  src/ui/BreathOrb.cpp
  src/ui/PlayTab.cpp
  src/ui/EditTab.cpp)
target_include_directories(RAIU1 PRIVATE .)
target_compile_definitions(RAIU1 PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(RAIU1 PRIVATE
  juce::juce_audio_utils juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

add_executable(raiu1_stress
  tests/stress_main.cpp
  src/PluginProcessor.cpp
  src/PluginEditor.cpp
  src/ui/RaiuLookAndFeel.cpp
  src/ui/FMAlgorithmView.cpp
  src/ui/NatureOscPad.cpp
  src/ui/BreathOrb.cpp
  src/ui/PlayTab.cpp
  src/ui/EditTab.cpp)
target_include_directories(raiu1_stress PRIVATE .)
target_compile_definitions(raiu1_stress PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(raiu1_stress PRIVATE juce::juce_audio_utils juce::juce_dsp)

add_executable(raiu1_tests
  tests/test_sanity.cpp
  tests/test_sine_table.cpp
  tests/test_adsr.cpp
  tests/test_fm_operator.cpp
  tests/test_fm_algorithm.cpp
  tests/test_fm_voice.cpp
  tests/test_textures.cpp
  tests/test_resonator.cpp
  tests/test_texture_engine.cpp
  tests/test_env_follower.cpp
  tests/test_nature_osc.cpp
  tests/test_fx_chain.cpp)
target_include_directories(raiu1_tests PRIVATE .)
target_link_libraries(raiu1_tests PRIVATE Catch2::Catch2WithMain juce::juce_dsp)
