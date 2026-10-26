cmake_minimum_required(VERSION 3.22)
project(TSMANIA_TUNE VERSION 0.1.0)

# JUCE is fetched at configure time. An internet connection is required the first time.
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(TSMANIA_TUNE
    COMPANY_NAME "Tsmania Audio"
    BUNDLE_ID com.tsmania.tune
    PLUGIN_MANUFACTURER_CODE Tsma
    PLUGIN_CODE Tstn
    FORMATS VST3 Standalone
    PRODUCT_NAME "TSMANIA TUNE"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(TSMANIA_TUNE PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_features(TSMANIA_TUNE PRIVATE cxx_std_17)
target_compile_definitions(TSMANIA_TUNE PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(TSMANIA_TUNE PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags
)
