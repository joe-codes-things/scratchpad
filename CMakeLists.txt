cmake_minimum_required(VERSION 3.15)
project(ScratchpadPlugin VERSION 0.0.25 LANGUAGES C CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/JUCE_framework JUCE)

if(APPLE)
    set(SCRATCHPAD_PLUGIN_FORMATS VST3 AU)
elseif(WIN32)
    set(SCRATCHPAD_PLUGIN_FORMATS VST3)
else()
    message(FATAL_ERROR "ScratchPad currently supports macOS and Windows builds only.")
endif()

juce_add_plugin(ScratchpadPlugin
    COMPANY_NAME "Nine Lives Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    BUNDLE_ID "com.ninelivesaudio.scratchpad"
    PLUGIN_MANUFACTURER_CODE NinL
    PLUGIN_CODE SPad
    FORMATS ${SCRATCHPAD_PLUGIN_FORMATS}
    PRODUCT_NAME "ScratchPad"
    VST3_CATEGORIES "Utility")

target_sources(ScratchpadPlugin PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp)

target_compile_features(ScratchpadPlugin PRIVATE cxx_std_17)

target_compile_definitions(ScratchpadPlugin PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(ScratchpadPlugin PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_gui_extra
    juce::juce_gui_basics
    juce::juce_dsp)
