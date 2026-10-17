#include "PluginProcessor.h"
#include "PluginEditor.h"

NotepadPluginAudioProcessor::NotepadPluginAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Initialize with one default tab
    addTab ("Untitled 1");
}

NotepadPluginAudioProcessor::~NotepadPluginAudioProcessor()
{
}

const juce::String NotepadPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NotepadPluginAudioProcessor::acceptsMidi() const
{
    return false;
}

bool NotepadPluginAudioProcessor::producesMidi() const
{
    return false;
}

bool NotepadPluginAudioProcessor::isMidiEffect() const
{
    return false;
}

double NotepadPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NotepadPluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int NotepadPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NotepadPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String NotepadPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void NotepadPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void NotepadPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void NotepadPluginAudioProcessor::releaseResources()
{
}

bool NotepadPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void NotepadPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Pass-through: copy input to output
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        juce::ignoreUnused (channelData);
    }
}

bool NotepadPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NotepadPluginAudioProcessor::createEditor()
{
    return new NotepadPluginAudioProcessorEditor (*this);
}

void NotepadPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Serialize all tabs
    auto xmlState = std::make_unique<juce::XmlElement> ("ScratchPadState");
    xmlState->setAttribute ("currentTabIndex", currentTabIndex);
    xmlState->setAttribute ("numTabs", static_cast<int> (tabs.size()));
    
    for (size_t i = 0; i < tabs.size(); ++i)
    {
        auto tabElement = xmlState->createNewChildElement ("Tab");
        tabElement->setAttribute ("name", tabs[i].name);
        tabElement->setAttribute ("text", tabs[i].text);
    }
    
    copyXmlToBinary (*xmlState, destData);
}

void NotepadPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Deserialize tabs
    auto xmlState = getXmlFromBinary (data, sizeInBytes);
    
    if (xmlState != nullptr && xmlState->hasTagName ("ScratchPadState"))
    {
        tabs.clear();
        int numTabs = xmlState->getIntAttribute ("numTabs", 1);
        
        for (int i = 0; i < numTabs; ++i)
        {
            auto tabElement = xmlState->getChildElement (i);
            if (tabElement != nullptr)
            {
                NotepadTab tab;
                tab.name = tabElement->getStringAttribute ("name", "Untitled");
                tab.text = tabElement->getStringAttribute ("text", "");
                tabs.push_back (tab);
            }
        }
        
        currentTabIndex = xmlState->getIntAttribute ("currentTabIndex", 0);
        if (currentTabIndex >= static_cast<int> (tabs.size()))
            currentTabIndex = 0;
    }
}

void NotepadPluginAudioProcessor::addTab (const juce::String& tabName)
{
    NotepadTab tab;
    tab.name = tabName.isEmpty() ? "Untitled " + juce::String (tabs.size() + 1) : tabName;
    tab.text = "";
    tabs.push_back (tab);
}

void NotepadPluginAudioProcessor::removeTab (int tabIndex)
{
    if (tabIndex >= 0 && tabIndex < static_cast<int> (tabs.size()))
    {
        tabs.erase (tabs.begin() + tabIndex);
        
        if (currentTabIndex >= static_cast<int> (tabs.size()) && !tabs.empty())
            currentTabIndex = static_cast<int> (tabs.size()) - 1;
    }
}

void NotepadPluginAudioProcessor::renameTab (int tabIndex, const juce::String& newName)
{
    if (tabIndex >= 0 && tabIndex < static_cast<int> (tabs.size()))
    {
        tabs[tabIndex].name = newName;
    }
}

juce::String NotepadPluginAudioProcessor::getCurrentTabText() const
{
    if (currentTabIndex >= 0 && currentTabIndex < static_cast<int> (tabs.size()))
    {
        return tabs[currentTabIndex].text;
    }
    return "";
}

void NotepadPluginAudioProcessor::setCurrentTabText (const juce::String& text)
{
    if (currentTabIndex >= 0 && currentTabIndex < static_cast<int> (tabs.size()))
    {
        tabs[currentTabIndex].text = text;
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NotepadPluginAudioProcessor();
}
