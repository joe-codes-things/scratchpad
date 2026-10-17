cmake_minimum_required(VERSION 3.15)
project(ScratchpadPlugin VERSION 0.0.23 LANGUAGES CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/JUCE_framework JUCE)

juce_add_plugin(ScratchpadPlugin
    COMPANY_NAME "Nine Lives Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE NinL
    PLUGIN_CODE SPad
    FORMATS VST3 AU
    PRODUCT_NAME "ScratchPad"
    VST3_CATEGORIES "Utility")

target_sources(ScratchpadPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/NativeFileChooser.mm)

target_compile_features(ScratchpadPlugin PRIVATE cxx_std_17)

# Enable Objective-C++ for .mm files
set_source_files_properties(Source/NativeFileChooser.mm PROPERTIES
    LANGUAGE OBJCXX
    COMPILE_FLAGS "-fno-objc-arc")

target_compile_definitions(ScratchpadPlugin PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(ScratchpadPlugin PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_gui_extra
    juce::juce_gui_basics
    juce::juce_dsp)
