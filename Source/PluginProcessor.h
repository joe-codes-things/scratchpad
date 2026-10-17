#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

// Tab data structure
struct NotepadTab
{
    juce::String name;
    juce::String text;
};

class NotepadPluginAudioProcessor : public juce::AudioProcessor
{
public:
    NotepadPluginAudioProcessor();
    ~NotepadPluginAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Tab management
    std::vector<NotepadTab> tabs;
    int currentTabIndex = 0;
    
    void addTab (const juce::String& tabName = "");
    void removeTab (int tabIndex);
    void renameTab (int tabIndex, const juce::String& newName);
    juce::String getCurrentTabText() const;
    void setCurrentTabText (const juce::String& text);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotepadPluginAudioProcessor)
};
