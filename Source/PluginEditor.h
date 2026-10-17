#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "NLALookAndFeel.h"

// Custom ComboBox that forces a repaint on mouseExit so hover glow clears reliably
class HoverComboBox : public juce::ComboBox
{
public:
    HoverComboBox() = default;
    
private:
    void mouseExit (const juce::MouseEvent&) override
    {
        repaint();
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HoverComboBox)
};

// Gear icon button component
class GearButton : public juce::Component
{
public:
    GearButton (juce::Colour normalColor, juce::Colour hoverColor)
        : normalColor_ (normalColor), hoverColor_ (hoverColor) {}
    
    void paint (juce::Graphics& g) override
    {
        auto c = isHovered_ ? hoverColor_ : normalColor_;
        auto b = getLocalBounds().toFloat().reduced (2.0f);
        float cx = b.getCentreX(), cy = b.getCentreY();
        float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
        
        g.setColour (c);
        
        float outerR = r, innerR = r * 0.55f;
        int teeth = 8;
        juce::Path gear;
        for (int i = 0; i < teeth; ++i)
        {
            float a0 = juce::MathConstants<float>::twoPi * i / teeth;
            float a1 = a0 + juce::MathConstants<float>::twoPi / teeth * 0.4f;
            float a2 = a0 + juce::MathConstants<float>::twoPi / teeth * 0.6f;
            float a3 = a0 + juce::MathConstants<float>::twoPi / teeth;
            auto pt = [&](float angle, float rad) {
                return juce::Point<float> (cx + rad * std::cos (angle),
                                          cy + rad * std::sin (angle));
            };
            if (i == 0) gear.startNewSubPath (pt (a0, innerR));
            gear.lineTo (pt (a0, outerR));
            gear.lineTo (pt (a1, outerR));
            gear.lineTo (pt (a2, outerR));
            gear.lineTo (pt (a3, innerR));
        }
        gear.closeSubPath();
        g.fillPath (gear);
        
        // Centre hole
        g.setColour (isHovered_ ? juce::Colour (0xff202634) : juce::Colour (0xff181e29));
        g.fillEllipse (cx - r * 0.28f, cy - r * 0.28f, r * 0.56f, r * 0.56f);
    }
    
    void mouseEnter (const juce::MouseEvent&) override { isHovered_ = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override { isHovered_ = false; repaint(); }
    void mouseDown (const juce::MouseEvent&) override { if (onClick) onClick(); }
    
    std::function<void()> onClick;
    
private:
    juce::Colour normalColor_, hoverColor_;
    bool isHovered_ { false };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GearButton)
};

// Hover label component
class HoverLabel : public juce::Component
{
public:
    HoverLabel (const juce::String& text, float fontSize,
               juce::Colour normalColor, juce::Colour hoverColor,
               juce::Justification justification = juce::Justification::centredRight)
        : text_ (text), fontSize_ (fontSize),
          normalColor_ (normalColor), hoverColor_ (hoverColor),
          justification_ (justification) {}
    
    void paint (juce::Graphics& g) override
    {
        g.setColour (isHovered_ ? hoverColor_ : normalColor_);
        g.setFont (juce::Font (fontSize_, juce::Font::plain));
        g.drawText (text_, getLocalBounds(), justification_, true);
    }
    
    void mouseEnter (const juce::MouseEvent&) override { isHovered_ = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override { isHovered_ = false; repaint(); }
    void mouseDown (const juce::MouseEvent&) override { if (onClick) onClick(); }
    
    std::function<void()> onClick;
    
private:
    juce::String text_;
    float fontSize_;
    juce::Colour normalColor_, hoverColor_;
    juce::Justification justification_;
    bool isHovered_ { false };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HoverLabel)
};

// Plugin Info Popup (gear menu)
class PluginInfoPopup : public juce::Component
{
public:
    PluginInfoPopup()
    {
        setSize (220, 90);
    }
    
    void paint (juce::Graphics& g) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent& e) override;
    
    std::function<void()> onAboutClicked;
    std::function<void()> onStyleGuideClicked;
    
private:
    bool aboutHovered_ { false };
    bool styleGuideHovered_ { false };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginInfoPopup)
};

// About Popup
class AboutPopup : public juce::Component
{
public:
    AboutPopup (const juce::String& pluginFormat, const juce::String& architecture,
               const juce::String& version, const juce::String& buildDate);
    
    void paint (juce::Graphics& g) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent& e) override;
    
    std::function<void()> onCloseClicked;
    
private:
    juce::String line1_, line2_, line3_;
    bool xHovered_ { false };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AboutPopup)
};

// Nine Lives Audio Popup
class NLAPopup : public juce::Component
{
public:
    NLAPopup();
    void paint (juce::Graphics& g) override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NLAPopup)
};

// Custom tab button that detects double-click for renaming
class TabButton : public juce::TextButton
{
public:
    TabButton (const juce::String& name) : juce::TextButton (name) {}
    
    std::function<void()> onDoubleClick;
    
private:
    void mouseDown (const juce::MouseEvent& e) override
    {
        // Only respond to left mouse button
        if (e.mods.isLeftButtonDown())
            juce::TextButton::mouseDown (e);
    }
    
    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        // Only respond to left mouse button double-click
        if (e.mods.isLeftButtonDown() && onDoubleClick)
            onDoubleClick();
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabButton)
};

// Custom rename editor that handles Escape key to cancel and Tab key to create new notepad
class TabRenameEditor : public juce::TextEditor
{
public:
    TabRenameEditor()
    {
        // Enable single-line text editor behavior
        setMultiLine (false);
        setReturnKeyStartsNewLine (false);
        
        // Enable horizontal scrolling for long text
        setScrollbarsShown (false);
        setScrollToShowCursor (true);
        
        // Enable text selection and cursor positioning
        setReadOnly (false);
        setCaretVisible (true);
    }
    
    ~TabRenameEditor() override = default;
    
    std::function<void()> onEscapeKey;
    std::function<void()> onTabKey;
    std::function<void()> onReturnKey;
    std::function<void()> onFocusLost;
    
    // Set this before triggering any action that removes the editor,
    // so focusLost doesn't fire a save on top of it
    bool suppressFocusLost { false };
    
private:
    bool keyPressed (const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::escapeKey)
        {
            suppressFocusLost = true;
            if (onEscapeKey)
                onEscapeKey();
            return true;
        }
        if (key == juce::KeyPress::tabKey)
        {
            suppressFocusLost = true;
            if (onTabKey)
                onTabKey();
            return true;
        }
        if (key == juce::KeyPress::returnKey)
        {
            suppressFocusLost = true;
            if (onReturnKey)
                onReturnKey();
            return true;
        }
        return juce::TextEditor::keyPressed (key);
    }
    
    void focusLost (FocusChangeType cause) override
    {
        if (!suppressFocusLost && onFocusLost)
            onFocusLost();
        juce::TextEditor::focusLost (cause);
    }
    
    TabRenameEditor (const TabRenameEditor&) = delete;
    TabRenameEditor& operator= (const TabRenameEditor&) = delete;
};

class NotepadPluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Button::Listener,
                                          private juce::TextEditor::Listener,
                                          private juce::ComboBox::Listener,
                                          private juce::KeyListener
{
public:
    NotepadPluginAudioProcessorEditor (NotepadPluginAudioProcessor&);
    ~NotepadPluginAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key, juce::Component* originatingComponent) override;
    void mouseDown (const juce::MouseEvent& e) override;

private:
    NotepadPluginAudioProcessor& processorRef;
    int renamingTabIndex = -1;
    NLALookAndFeel nlaLookAndFeel;
    
    // Buttons
    juce::TextButton addTabButton {"New Note"};
    juce::TextButton removeTabButton {"Delete Note"};
    juce::TextButton exportButton {"Export Note"};
    HoverComboBox fontSizeDropdown;
    
    float currentFontSize = 18.0f;
    
    // Tabs display with scrolling
    juce::Viewport tabViewport;
    juce::Component tabButtonsContainer;
    juce::TextEditor textEditor;
    
    std::unique_ptr<GearButton> gearButton_;
    std::unique_ptr<juce::Label> titleLabel_;
    std::unique_ptr<HoverLabel> nlaLabel_;
    std::unique_ptr<PluginInfoPopup> pluginInfoPopup_;
    std::unique_ptr<AboutPopup> aboutPopup_;
    std::unique_ptr<NLAPopup> nlaPopup_;
    std::unique_ptr<juce::AlertWindow> styleGuideWindow_;
    
    int lastCalculatedTabRows = 1;  // Track number of tab button rows
    
    void buttonClicked (juce::Button* button) override;
    void textEditorTextChanged (juce::TextEditor&) override;
    void comboBoxChanged (juce::ComboBox* comboBox) override;
    void updateTabButtons();
    void switchToTab (int tabIndex);
    void createTabButton (int tabIndex);
    void startRenamingTab (int tabIndex);
    int calculateButtonWidth (const juce::String& tabName);
    int calculateTabContainerHeight();
    void layoutTabButtons();
    void drawSettingsIcon (juce::Graphics& g, juce::Rectangle<int> area);
    void drawExportIcon (juce::Graphics& g, juce::Rectangle<int> area);
    void updateFontSize (float newSize);
    void applyFontSizeToSelection (float newSize);
    void setupFontSizeDropdown();
    void exportCurrentTab();
    void togglePluginInfoPopup();
    void hidePluginInfoPopup();
    void showAboutPopup();
    void hideAboutPopup();
    void showNLAPopup();
    void hideNLAPopup();
    void showStyleGuidePopup();
    void hideStyleGuidePopup();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotepadPluginAudioProcessorEditor)
};
