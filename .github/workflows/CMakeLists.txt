cmake_minimum_required(VERSION 3.15)
project(NeonDrive85 VERSION 1.0.0)
add_subdirectory(JUCE)
juce_add_plugin(NeonDrive85
    COMPANY_NAME "YourName"
    PLUGIN_MANUFACTURER_CODE Ndrv
    PLUGIN_CODE Nd85
    FORMATS VST3
    PRODUCT_NAME "Neon Drive 85")
target_sources(NeonDrive85 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.h
    Source/Arpeggiator.h)
target_link_libraries(NeonDrive85 PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp)
