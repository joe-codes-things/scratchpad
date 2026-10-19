#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class NLALookAndFeel : public juce::LookAndFeel_V4
{
public:
    NLALookAndFeel()
    {
        // Set default color scheme
        setColour (juce::ResizableWindow::backgroundColourId, deepBackground);
        setColour (juce::TextEditor::backgroundColourId, surface1);
        setColour (juce::TextEditor::textColourId, primaryText);
        setColour (juce::TextEditor::highlightColourId, neonBlue.withAlpha (0.3f));
        setColour (juce::TextEditor::outlineColourId, border.withAlpha (0.5f));
        setColour (juce::TextEditor::focusedOutlineColourId, border.withAlpha (0.5f));  // No blue glow on focus
        setColour (juce::CaretComponent::caretColourId, primaryText);
        
        setColour (juce::ComboBox::backgroundColourId, surface2);
        setColour (juce::ComboBox::textColourId, primaryText);
        setColour (juce::ComboBox::outlineColourId, border);
        setColour (juce::ComboBox::buttonColourId, surface3);
        setColour (juce::ComboBox::arrowColourId, neonCyan);
        
        setColour (juce::PopupMenu::backgroundColourId, surface2);
        setColour (juce::PopupMenu::textColourId, primaryText);
        setColour (juce::PopupMenu::headerTextColourId, neonBlue);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, surface3);
        setColour (juce::PopupMenu::highlightedTextColourId, neonBlue);
        
        setColour (juce::ScrollBar::backgroundColourId, surface1);
        setColour (juce::ScrollBar::thumbColourId, surface3);
        setColour (juce::ScrollBar::trackColourId, surface2);
    }
    
    // NLA Color Palette
    const juce::Colour deepBackground {0xff0a0e14};
    const juce::Colour surface1 {0xff11161d};
    const juce::Colour surface2 {0xff181e29};
    const juce::Colour surface3 {0xff202634};
    const juce::Colour border {0xff2a3342};
    
    const juce::Colour neonBlue {0xff00e6ff};
    const juce::Colour neonCyan {0xff00ffc6};
    const juce::Colour neonGreen {0xff39ff14};
    const juce::Colour limeGreen {0xffa8ff00};
    const juce::Colour neonPurple {0xffb300ff};
    const juce::Colour neonMagenta {0xffff00f0};
    const juce::Colour brightBlue {0xff2979ff};
    
    const juce::Colour warmWhite {0xffffdca8};
    const juce::Colour amber {0xffffb84d};
    const juce::Colour gold {0xffffc23c};
    const juce::Colour deepAmber {0xffff7a00};
    
    const juce::Colour primaryText {0xffe6f1ff};
    const juce::Colour secondaryText {0xffa6b3c7};
    const juce::Colour dimText {0xff6b778c};
    
    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
        auto cornerSize = 4.0f;
        
        // Determine button type by text
        juce::String buttonText = button.getButtonText();
        bool isAddButton = buttonText == "New Note";
        bool isRemoveButton = buttonText == "Delete Note";
        bool isExportButton = buttonText == "Export Note";
        bool isTabButton = !isAddButton && !isRemoveButton && !isExportButton;
        
        // Base color
        juce::Colour baseColour = surface2;
        juce::Colour glowColour = neonBlue;
        
        if (isAddButton)
            glowColour = neonCyan;
        else if (isRemoveButton)
            glowColour = neonMagenta.withAlpha (0.8f);
        else if (isExportButton)
            glowColour = deepAmber;  // Deep amber for export button
        
        // Active tab gets neon blue glow
        if (isTabButton && button.getToggleState())
        {
            baseColour = surface3;
            glowColour = neonBlue;
            
            // Draw outer glow for active tab
            g.setColour (glowColour.withAlpha (0.15f));
            g.fillRoundedRectangle (bounds.expanded (3.0f), cornerSize + 2.0f);
            
            g.setColour (glowColour.withAlpha (0.25f));
            g.fillRoundedRectangle (bounds.expanded (1.5f), cornerSize + 1.0f);
        }
        
        // Hover state - neon green glow
        if (shouldDrawButtonAsHighlighted)
        {
            // Neon green hover glow
            g.setColour (limeGreen.withAlpha (0.12f));
            g.fillRoundedRectangle (bounds.expanded (2.0f), cornerSize + 1.0f);
            
            baseColour = baseColour.brighter (0.15f);
        }
        
        // Pressed state
        if (shouldDrawButtonAsDown)
        {
            baseColour = baseColour.darker (0.2f);
        }
        
        // Draw main button background with subtle gradient
        juce::ColourGradient gradient (baseColour.brighter (0.05f), bounds.getX(), bounds.getY(),
                                       baseColour.darker (0.05f), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill (gradient);
        g.fillRoundedRectangle (bounds, cornerSize);
        
        // Draw border with neon accent
        float borderAlpha = shouldDrawButtonAsHighlighted ? 0.6f : 0.3f;
        if (isTabButton && button.getToggleState())
            borderAlpha = 0.8f;
        
        g.setColour (glowColour.withAlpha (borderAlpha));
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    }
    
    void drawButtonText (juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::String buttonText = button.getButtonText();
        bool isTabButton = buttonText != "New Note" && buttonText != "Delete Note" && buttonText != "Export Note";
        
        // Text color - keep white for all buttons (no color change for active tab or hover)
        juce::Colour textColour = primaryText;
        
        g.setColour (textColour);
        
        // Font sizing - same size for all buttons
        float fontSize = 14.0f;
        g.setFont (juce::Font (fontSize, juce::Font::plain));
        
        g.drawText (button.getButtonText(), button.getLocalBounds(),
                   juce::Justification::centred, true);
    }
    
    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat();
        auto cornerSize = 4.0f;
        
        // Check if mouse is over the component
        bool isMouseOver = box.isMouseOver (true);
        
        // Hover glow (neon green like other buttons) - draw FIRST so it's behind everything
        if (isMouseOver)
        {
            g.setColour (limeGreen.withAlpha (0.12f));
            g.fillRoundedRectangle (bounds.expanded (2.0f), cornerSize + 1.0f);
        }
        
        // Fill background to cover any white corners
        g.setColour (surface2);
        g.fillRoundedRectangle (bounds, cornerSize);
        
        // Background with subtle gradient - brighten on hover
        auto baseColour = isMouseOver ? surface2.brighter (0.15f) : surface2;
        auto innerBounds = bounds.reduced (1.0f);
        juce::ColourGradient gradient (baseColour.brighter (0.05f), 0.0f, 0.0f,
                                       baseColour.darker (0.05f), 0.0f, (float) height, false);
        g.setGradientFill (gradient);
        g.fillRoundedRectangle (innerBounds, cornerSize);
        
        // Border with neon accent - increase alpha on hover or focus
        float borderAlpha = box.hasKeyboardFocus (true) ? 0.6f : (isMouseOver ? 0.5f : 0.3f);
        g.setColour (neonBlue.withAlpha (borderAlpha));
        g.drawRoundedRectangle (innerBounds, cornerSize, 1.0f);
        
        // Arrow button area
        auto arrowZone = juce::Rectangle<int> (buttonX, buttonY, buttonW, buttonH).toFloat();
        
        // Draw arrow
        juce::Path path;
        path.startNewSubPath (arrowZone.getCentreX() - 3.0f, arrowZone.getCentreY() - 2.0f);
        path.lineTo (arrowZone.getCentreX() + 3.0f, arrowZone.getCentreY() - 2.0f);
        path.lineTo (arrowZone.getCentreX(), arrowZone.getCentreY() + 2.0f);
        path.closeSubPath();
        
        g.setColour (neonCyan.withAlpha (0.8f));
        g.fillPath (path);
    }
    
    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override
    {
        auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat();
        
        // Fill entire area first to eliminate white corners
        g.fillAll (surface2);
        
        // Dark background with subtle glow
        g.setColour (surface2);
        g.fillRoundedRectangle (bounds, 4.0f);
        
        // Neon border
        g.setColour (neonBlue.withAlpha (0.4f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);
    }
    
    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                           bool hasSubMenu, const juce::String& text,
                           const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColourToUse) override
    {
        if (isSeparator)
        {
            auto r = area.reduced (5, 0);
            r.removeFromTop (r.getHeight() / 2 - 1);
            g.setColour (border);
            g.fillRect (r.removeFromTop (1));
        }
        else
        {
            auto textColour = primaryText;
            
            if (isHighlighted)
            {
                // Highlight with surface3 background
                g.setColour (surface3);
                g.fillRect (area);
                
                // Draw border on left and right sides using the same neon blue as the dropdown border
                g.setColour (neonBlue.withAlpha (0.5f));
                g.fillRect (area.getX(), area.getY(), 1, area.getHeight());  // Left border
                g.fillRect (area.getRight() - 1, area.getY(), 1, area.getHeight());  // Right border
                
                textColour = primaryText;  // Keep text white on hover
            }
            
            if (!isActive)
                textColour = dimText;
            
            auto r = area.reduced (1);
            
            if (isTicked)
            {
                g.setColour (neonCyan);
                auto tick = juce::Rectangle<float> (r.getX() + 4.0f, r.getCentreY() - 4.0f, 8.0f, 8.0f);
                g.fillEllipse (tick);
            }
            
            r.removeFromLeft (20);
            
            g.setColour (textColour);
            g.setFont (14.0f);
            g.drawFittedText (text, r, juce::Justification::centredLeft, 1);
        }
    }
    
    void drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override
    {
        if (textEditor.isEnabled())
        {
            auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);
            
            // Same subtle border whether focused or not
            g.setColour (border.withAlpha (0.5f));
            g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
        }
    }
    
    void drawScrollbar (juce::Graphics& g, juce::ScrollBar& scrollbar, int x, int y, int width, int height,
                       bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown) override
    {
        juce::Rectangle<int> thumbBounds;
        
        if (isScrollbarVertical)
            thumbBounds = {x, thumbStartPosition, width, thumbSize};
        else
            thumbBounds = {thumbStartPosition, y, thumbSize, height};
        
        auto cornerSize = (float) juce::jmin (width, height) * 0.5f;
        
        // Track background
        g.setColour (surface1);
        g.fillRect (x, y, width, height);
        
        // Thumb
        auto thumbColour = surface3;
        if (isMouseOver || isMouseDown)
            thumbColour = surface3.brighter (0.2f);
        
        g.setColour (thumbColour);
        g.fillRoundedRectangle (thumbBounds.toFloat().reduced (1.0f), cornerSize);
        
        // Subtle neon accent on thumb
        if (isMouseOver || isMouseDown)
        {
            g.setColour (neonCyan.withAlpha (0.3f));
            g.drawRoundedRectangle (thumbBounds.toFloat().reduced (1.0f), cornerSize, 1.0f);
        }
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NLALookAndFeel)
};
