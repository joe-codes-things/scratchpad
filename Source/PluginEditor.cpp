#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "NativeFileChooser.h"

NotepadPluginAudioProcessorEditor::NotepadPluginAudioProcessorEditor (NotepadPluginAudioProcessor& p)
    : AudioProcessorEditor (p), processorRef (p)
{
    // Apply NLA LookAndFeel
    setLookAndFeel (&nlaLookAndFeel);
    
    // Add key listener to handle Escape key for closing popups
    addKeyListener (this);
    setWantsKeyboardFocus (true);
    
    setSize (800, 500);  // Expanded width to accommodate right sidebar
    setResizeLimits (600, 250, 1400, 1000);  // Min width increased for new layout
    
    // Configure gear button
    gearButton_ = std::make_unique<GearButton> (juce::Colour (0xff6b778c), juce::Colour (0xffa6b3c7));
    gearButton_->onClick = [this]() {
        if (nlaPopup_ && nlaPopup_->isVisible())
            hideNLAPopup();
        togglePluginInfoPopup();
    };
    addAndMakeVisible (*gearButton_);
    
    // Configure plugin title
    titleLabel_ = std::make_unique<juce::Label> ("Title", "ScratchPad");
    titleLabel_->setFont (juce::Font (20.0f, juce::Font::bold));
    titleLabel_->setJustificationType (juce::Justification::centredLeft);
    titleLabel_->setColour (juce::Label::textColourId, juce::Colour (0xffe6f1ff));  // PrimaryText
    addAndMakeVisible (*titleLabel_);
    
    // Configure Nine Lives Audio label
    nlaLabel_ = std::make_unique<HoverLabel> (
        "Nine Lives Audio",
        13.0f,
        juce::Colour (0xff6b778c),  // DimText
        juce::Colour (0xffa6b3c7),  // SecondaryText
        juce::Justification::centredRight);
    nlaLabel_->setMouseCursor (juce::MouseCursor::PointingHandCursor);
    nlaLabel_->onClick = [this]() {
        if (aboutPopup_ && aboutPopup_->isVisible())
            return;
        if (pluginInfoPopup_ && pluginInfoPopup_->isVisible())
            hidePluginInfoPopup();
        if (nlaPopup_ && nlaPopup_->isVisible())
            hideNLAPopup();
        else
            showNLAPopup();
    };
    addAndMakeVisible (*nlaLabel_);
    
    // Configure tab buttons - explicitly set text to ensure it's applied
    addTabButton.setButtonText ("New Note");
    removeTabButton.setButtonText ("Delete Note");
    exportButton.setButtonText ("Export Note");
    
    addTabButton.addListener (this);
    removeTabButton.addListener (this);
    exportButton.addListener (this);
    
    addAndMakeVisible (addTabButton);
    addAndMakeVisible (removeTabButton);
    addAndMakeVisible (tabButtonsContainer);
    addAndMakeVisible (exportButton);
    
    // Configure font size dropdown
    setupFontSizeDropdown();
    fontSizeDropdown.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible (fontSizeDropdown);
    
    // Setup scrollable tab container for vertical list
    tabViewport.setViewedComponent (&tabButtonsContainer, false);
    tabViewport.setScrollBarsShown (true, false);  // Show vertical scrollbar, hide horizontal
    tabViewport.addMouseListener (this, true);  // Add mouse listener to close popups and dismiss rename editor
    addAndMakeVisible (tabViewport);
    
    // Configure text editor with initial font size
    textEditor.setMultiLine (true);
    textEditor.setReturnKeyStartsNewLine (true);
    textEditor.setReadOnly (false);
    textEditor.setScrollbarsShown (true);
    textEditor.setCaretVisible (true);
    textEditor.setPopupMenuEnabled (true);
    textEditor.addListener (this);
    textEditor.setFont (juce::Font (currentFontSize));
    
    // Add mouse listener to text editor to close popups when clicking in it
    textEditor.addMouseListener (this, false);
    
    // Load current tab
    switchToTab (0);
    
    addAndMakeVisible (textEditor);
    
    updateTabButtons();
}

NotepadPluginAudioProcessorEditor::~NotepadPluginAudioProcessorEditor()
{
    removeKeyListener (this);
    setLookAndFeel (nullptr);
}

void NotepadPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void NotepadPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (10);  // Edge padding
    
    // Top bar for gear icon, title, buttons, and NLA label
    auto topBar = bounds.removeFromTop (35);
    
    // Gear icon - top left
    int gearWidth = 20;
    if (gearButton_)
        gearButton_->setBounds (topBar.removeFromLeft (gearWidth).reduced (2));
    else
        topBar.removeFromLeft (gearWidth);
    
    // Nine Lives Audio - top right (will be repositioned after we know note button positions)
    int nlaWidth = 110;
    auto nlaArea = topBar.removeFromRight (nlaWidth);
    
    // Calculate button positions to find where New Note button starts
    auto controlButtonBounds = topBar;
    int totalButtonWidth = 90 + 95 + 95 + 60;  // addTab + removeTab + export + fontDropdown (increased padding)
    int buttonSpacing = 10;
    int totalWidth = totalButtonWidth + (buttonSpacing * 3);
    
    // Center the buttons, then shift right by 50px
    int leftMargin = (controlButtonBounds.getWidth() - totalWidth) / 2 + 50;
    int newNoteButtonX = 10 + gearWidth + leftMargin;  // 10 (edge padding) + gear width + left margin
    
    // Calculate space between right of gear icon and left of New Note button
    int gearRightX = 10 + gearWidth;
    int spaceForTitle = newNoteButtonX - gearRightX;
    
    // Center the title in that space, then shift right by 30px, then left by 18px (net +12px)
    int titleWidth = 130;
    int titleLeftMargin = (spaceForTitle - titleWidth) / 2 + 12;
    
    // Position title
    if (titleLabel_)
    {
        topBar.removeFromLeft (titleLeftMargin);
        titleLabel_->setBounds (topBar.removeFromLeft (titleWidth).reduced (0, 2));
    }
    else
    {
        topBar.removeFromLeft (titleLeftMargin + titleWidth);
    }
    
    // Remaining space before buttons
    int remainingSpace = leftMargin - titleLeftMargin - titleWidth;
    topBar.removeFromLeft (remainingSpace);
    
    // Position control buttons
    addTabButton.setBounds (topBar.removeFromLeft (90).reduced (2));
    topBar.removeFromLeft (buttonSpacing);
    removeTabButton.setBounds (topBar.removeFromLeft (95).reduced (2));
    topBar.removeFromLeft (buttonSpacing);
    exportButton.setBounds (topBar.removeFromLeft (95).reduced (2));
    topBar.removeFromLeft (buttonSpacing);
    fontSizeDropdown.setBounds (topBar.removeFromLeft (60).reduced (2));
    
    const int spacing = 9;
    bounds.removeFromTop (spacing);
    
    // Calculate note button area dimensions
    const int noteButtonWidth = 150;
    const int noteButtonHeight = 30;
    const int noteButtonSpacing = 8;  // Increased spacing between buttons
    const int noteButtonSidePadding = 10;  // Space on left and right of note buttons
    
    // Calculate total width needed for note buttons area
    int noteButtonAreaWidth = noteButtonWidth + (noteButtonSidePadding * 2);
    
    // Position note buttons on the right side with equal padding
    auto noteButtonArea = bounds.removeFromRight (noteButtonAreaWidth);
    
    // Add spacing between notepad and note button area
    bounds.removeFromRight (noteButtonSidePadding);
    
    // Position "Nine Lives Audio" text centered in the right area
    if (nlaLabel_)
    {
        // Calculate the total width available for NLA text area (from right of notepad to right of plugin window)
        int rightAreaWidth = noteButtonAreaWidth + noteButtonSidePadding + 10;  // note button area + spacing + edge padding
        
        // Calculate spacing needed on each side to center the NLA text
        int totalSpacing = rightAreaWidth - nlaWidth;
        int sideSpacing = totalSpacing / 2;
        
        // Position NLA text with calculated spacing from the right edge
        int nlaX = getWidth() - 10 - rightAreaWidth + sideSpacing;
        nlaLabel_->setBounds (nlaX, 10, nlaWidth, 35);
    }
    
    // Add top spacing to match the spacing between top buttons and note buttons
    noteButtonArea.removeFromTop (spacing);
    
    // Inset the note buttons by the side padding amount
    noteButtonArea = noteButtonArea.reduced (noteButtonSidePadding, 0);
    
    // Set up the tab viewport to show vertical list of buttons
    tabViewport.setBounds (noteButtonArea);
    
    // Calculate total height needed for all buttons
    int totalButtonsHeight = (processorRef.tabs.size() * noteButtonHeight) + 
                            ((processorRef.tabs.size() - 1) * noteButtonSpacing) + 10;
    tabButtonsContainer.setSize (noteButtonWidth, totalButtonsHeight);
    
    // Layout buttons vertically
    int yPos = 5;
    for (int i = 0; i < tabButtonsContainer.getNumChildComponents(); ++i)
    {
        auto* button = tabButtonsContainer.getChildComponent (i);
        button->setBounds (0, yPos, noteButtonWidth, noteButtonHeight);
        yPos += noteButtonHeight + noteButtonSpacing;
    }
    
    // Text editor takes remaining space (notepad)
    textEditor.setBounds (bounds);
}

bool NotepadPluginAudioProcessorEditor::keyPressed (const juce::KeyPress& key, juce::Component* originatingComponent)
{
    // Handle Escape key to close popups
    if (key == juce::KeyPress::escapeKey)
    {
        bool closedSomething = false;
        
        // Close popups in priority order
        if (aboutPopup_ && aboutPopup_->isVisible())
        {
            hideAboutPopup();
            closedSomething = true;
        }
        else if (pluginInfoPopup_ && pluginInfoPopup_->isVisible())
        {
            hidePluginInfoPopup();
            closedSomething = true;
        }
        else if (nlaPopup_ && nlaPopup_->isVisible())
        {
            hideNLAPopup();
            closedSomething = true;
        }
        
        if (closedSomething)
            return true;  // Consume the key event
    }
    
    return false;  // Let other components handle the key
}

void NotepadPluginAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button == &addTabButton)
    {
        processorRef.addTab();
        processorRef.currentTabIndex = static_cast<int> (processorRef.tabs.size()) - 1;
        updateTabButtons();
        switchToTab (processorRef.currentTabIndex);
    }
    else if (button == &removeTabButton)
    {
        if (processorRef.tabs.size() > 1)
        {
            processorRef.removeTab (processorRef.currentTabIndex);
            updateTabButtons();
            switchToTab (processorRef.currentTabIndex);
        }
    }
    else if (button == &exportButton)
    {
        exportCurrentTab();
    }
}

void NotepadPluginAudioProcessorEditor::comboBoxChanged (juce::ComboBox* comboBox)
{
    if (comboBox == &fontSizeDropdown)
    {
        int selectedIndex = fontSizeDropdown.getSelectedItemIndex();
        if (selectedIndex >= 0 && selectedIndex < fontSizeDropdown.getNumItems())
        {
            float newSize = static_cast<float> (fontSizeDropdown.getItemText (selectedIndex).getIntValue());
            updateFontSize (newSize);
        }
    }
}

void NotepadPluginAudioProcessorEditor::textEditorTextChanged (juce::TextEditor&)
{
    processorRef.setCurrentTabText (textEditor.getText());
}

void NotepadPluginAudioProcessorEditor::updateTabButtons()
{
    tabButtonsContainer.removeAllChildren();
    
    for (size_t i = 0; i < processorRef.tabs.size(); ++i)
    {
        createTabButton (static_cast<int> (i));
    }
    
    layoutTabButtons();
    resized();
}

int NotepadPluginAudioProcessorEditor::calculateButtonWidth (const juce::String& tabName)
{
    // Minimum width ~90px, maximum ~190px, scales with text
    // Designed to fit 2 buttons comfortably in 415px minimum width
    const int minWidth = 90;
    const int maxWidth = 190;
    const int charWidth = 7; // approximate pixels per character
    
    int calculatedWidth = minWidth + (tabName.length() * charWidth);
    return juce::jlimit (minWidth, maxWidth, calculatedWidth);
}

void NotepadPluginAudioProcessorEditor::switchToTab (int tabIndex)
{
    if (tabIndex >= 0 && tabIndex < static_cast<int> (processorRef.tabs.size()))
    {
        processorRef.currentTabIndex = tabIndex;
        textEditor.removeListener (this);
        textEditor.setText (processorRef.getCurrentTabText());
        textEditor.addListener (this);
        updateTabButtons();
    }
}

void NotepadPluginAudioProcessorEditor::layoutTabButtons()
{
    // Buttons are now laid out vertically in resized() method
    // This function is kept for compatibility but layout is handled in resized()
}

void NotepadPluginAudioProcessorEditor::createTabButton (int tabIndex)
{
    auto* button = new TabButton (processorRef.tabs[tabIndex].name);
    button->setClickingTogglesState (true);
    
    button->onClick = [this, tabIndex]()
    {
        switchToTab (tabIndex);
    };
    
    button->onDoubleClick = [this, tabIndex]()
    {
        startRenamingTab (tabIndex);
    };
    
    // Set toggle state for current tab (this triggers the neon glow in LookAndFeel)
    button->setToggleState (tabIndex == processorRef.currentTabIndex, juce::dontSendNotification);
    
    // Size will be set in resized() method - all buttons same width in sidebar
    button->setSize (135, 30);  // Default size, will be adjusted in resized()
    
    tabButtonsContainer.addAndMakeVisible (button);
}

int NotepadPluginAudioProcessorEditor::calculateTabContainerHeight()
{
    // Note: This function is kept for compatibility but height is now calculated dynamically
    // in resized() using lastCalculatedTabRows to avoid wasted space
    return 65;
}

void NotepadPluginAudioProcessorEditor::startRenamingTab (int tabIndex)
{
    if (tabIndex >= 0 && tabIndex < static_cast<int> (processorRef.tabs.size()))
    {
        auto originalName = processorRef.tabs[tabIndex].name;
        
        // Get position from the actual button if it exists
        int xPosition = 0;
        int yPosition = 0;
        int buttonWidth = 135;  // Fixed width for sidebar buttons
        int buttonHeight = 30;
        
        if (tabIndex < tabButtonsContainer.getNumChildComponents())
        {
            auto* existingButton = tabButtonsContainer.getChildComponent (tabIndex);
            xPosition = existingButton->getX();
            yPosition = existingButton->getY();
            buttonWidth = existingButton->getWidth();
            buttonHeight = existingButton->getHeight();
        }
        
        auto* renameEditor = new TabRenameEditor();
        renameEditor->setText (originalName);
        renameEditor->selectAll();
        renameEditor->setBounds (xPosition, yPosition, buttonWidth, buttonHeight);
        tabButtonsContainer.addAndMakeVisible (renameEditor);
        renameEditor->grabKeyboardFocus();
        
        // Handle Escape key to cancel without saving (revert to original name)
        renameEditor->onEscapeKey = [this, renameEditor]()
        {
            // Don't save changes - just close the editor
            if (tabButtonsContainer.isParentOf (renameEditor))
            {
                tabButtonsContainer.removeChildComponent (renameEditor);
                delete renameEditor;
            }
            updateTabButtons();
        };
        
        // Handle Tab key to create new notepad and move to editing its name
        renameEditor->onTabKey = [this, tabIndex, renameEditor, originalName]()
        {
            auto newName = renameEditor->getText();
            if (newName != originalName)
            {
                processorRef.renameTab (tabIndex, newName);
            }
            tabButtonsContainer.removeChildComponent (renameEditor);
            delete renameEditor;
            
            // Create new notepad
            processorRef.addTab();
            int newTabIndex = static_cast<int> (processorRef.tabs.size()) - 1;
            processorRef.currentTabIndex = newTabIndex;
            updateTabButtons();
            switchToTab (newTabIndex);
            
            // Start editing the new tab's name immediately
            startRenamingTab (newTabIndex);
        };
        
        // Handle text confirmation when Return is pressed
        renameEditor->onReturnKey = [this, tabIndex, renameEditor, originalName]()
        {
            auto newName = renameEditor->getText();
            if (newName != originalName)
            {
                processorRef.renameTab (tabIndex, newName);
            }
            tabButtonsContainer.removeChildComponent (renameEditor);
            delete renameEditor;
            updateTabButtons();
        };
        
        // Handle text confirmation when focus is lost
        renameEditor->onFocusLost = [this, tabIndex, renameEditor, originalName]()
        {
            auto newName = renameEditor->getText();
            if (newName != originalName)
            {
                processorRef.renameTab (tabIndex, newName);
            }
            if (tabButtonsContainer.isParentOf (renameEditor))
            {
                tabButtonsContainer.removeChildComponent (renameEditor);
                // Use MessageManager to defer deletion and updateTabButtons to avoid use-after-free
                juce::MessageManager::callAsync ([this, renameEditor]()
                {
                    delete renameEditor;
                    updateTabButtons();
                });
            }
        };
    }
}

void NotepadPluginAudioProcessorEditor::exportCurrentTab()
{
    if (processorRef.currentTabIndex >= 0 && 
        processorRef.currentTabIndex < static_cast<int> (processorRef.tabs.size()))
    {
        juce::String tabName = processorRef.tabs[processorRef.currentTabIndex].name;
        juce::String tabText = processorRef.getCurrentTabText();
        
        // Create header with tab name
        juce::String fileContent = "=== " + tabName + " ===\n\n" + tabText;
        
        // Sanitize filename
        juce::String fileName = tabName + ".txt";
        fileName = fileName.replaceCharacters ("<>:\"|?*", "_");
        
        if (showNativeSaveDialog (fileName, fileContent))
        {
            juce::AlertWindow::showMessageBoxAsync (
                juce::AlertWindow::InfoIcon,
                "Export Successful",
                "Your note has been exported."
            );
        }
        else
        {
            // User cancelled or error occurred
        }
    }
}

void NotepadPluginAudioProcessorEditor::drawExportIcon (juce::Graphics& g, juce::Rectangle<int> area)
{
    // Icons are now drawn via button text
}

void NotepadPluginAudioProcessorEditor::drawSettingsIcon (juce::Graphics& g, juce::Rectangle<int> area)
{
    // Icons are now drawn via button text
}

void NotepadPluginAudioProcessorEditor::updateFontSize (float newSize)
{
    currentFontSize = juce::jlimit (8.0f, 32.0f, newSize);
    applyFontSizeToSelection (currentFontSize);
}

void NotepadPluginAudioProcessorEditor::applyFontSizeToSelection (float newSize)
{
    // Workaround for JUCE TextEditor limitation: setFont() only affects newly typed text.
    // To apply font to existing text, we must clear and restore content.
    
    // Save current state
    juce::String currentText = textEditor.getText();
    int caretPosition = textEditor.getCaretPosition();
    
    // Clear editor and set new font
    textEditor.clear();
    textEditor.setFont (juce::Font (newSize));
    
    // Restore text and cursor position
    textEditor.removeListener (this);  // Prevent unnecessary text changed callbacks
    textEditor.setText (currentText, false);  // false = don't send change notification
    textEditor.setCaretPosition (juce::jmin (caretPosition, textEditor.getText().length()));
    textEditor.addListener (this);
}

void NotepadPluginAudioProcessorEditor::setupFontSizeDropdown()
{
    fontSizeDropdown.addItem ("8", 1);
    fontSizeDropdown.addItem ("10", 2);
    fontSizeDropdown.addItem ("12", 3);
    fontSizeDropdown.addItem ("14", 4);
    fontSizeDropdown.addItem ("16", 5);
    fontSizeDropdown.addItem ("18", 6);
    fontSizeDropdown.addItem ("20", 7);
    fontSizeDropdown.addItem ("24", 8);
    fontSizeDropdown.addItem ("28", 9);
    fontSizeDropdown.addItem ("32", 10);
    
    fontSizeDropdown.setSelectedItemIndex (5); // Default to 18
    fontSizeDropdown.addListener (this);
    fontSizeDropdown.setWantsKeyboardFocus (false); // Prevent focus loss for text editor
}

void NotepadPluginAudioProcessorEditor::togglePluginInfoPopup()
{
    if (aboutPopup_ && aboutPopup_->isVisible())
        return;
    
    if (pluginInfoPopup_ && pluginInfoPopup_->isVisible())
    {
        hidePluginInfoPopup();
    }
    else
    {
        if (!pluginInfoPopup_)
        {
            pluginInfoPopup_ = std::make_unique<PluginInfoPopup>();
            pluginInfoPopup_->onAboutClicked = [this]() {
                hidePluginInfoPopup();
                showAboutPopup();
            };
            pluginInfoPopup_->onStyleGuideClicked = [this]() {
                hidePluginInfoPopup();

                if (! juce::URL ("https://github.com/joe-codes-things/scratchpad/blob/main/README.md")
                         .launchInDefaultBrowser())
                {
                    juce::AlertWindow::showMessageBoxAsync (
                        juce::AlertWindow::WarningIcon,
                        "Unable to Open Setup Guide",
                        "The setup guide could not be opened in your default browser."
                    );
                }
            };
        }
        
        pluginInfoPopup_->setBounds (10, 10, 220, 90);
        addAndMakeVisible (*pluginInfoPopup_);
        pluginInfoPopup_->toFront (false);
    }
}

void NotepadPluginAudioProcessorEditor::hidePluginInfoPopup()
{
    if (pluginInfoPopup_)
    {
        removeChildComponent (pluginInfoPopup_.get());
        pluginInfoPopup_->setVisible (false);
    }
}

void NotepadPluginAudioProcessorEditor::showAboutPopup()
{
    juce::String pluginFormat = "Unknown";
    auto wrapperType = juce::PluginHostType::getPluginLoadedAs();
    
    switch (wrapperType)
    {
        case juce::AudioProcessor::wrapperType_VST3:
            pluginFormat = "VST3";
            break;
        case juce::AudioProcessor::wrapperType_AudioUnit:
        case juce::AudioProcessor::wrapperType_AudioUnitv3:
            pluginFormat = "AU";
            break;
        default:
            pluginFormat = "Standalone";
            break;
    }
    
    juce::String architecture;
    #if JUCE_ARM
        architecture = "Apple Silicon";
    #elif JUCE_INTEL
        architecture = "Intel";
    #else
        architecture = "Unknown";
    #endif
    
    juce::String version = JucePlugin_VersionString;
    juce::String buildDate = __DATE__;
    
    aboutPopup_ = std::make_unique<AboutPopup> (pluginFormat, architecture, version, buildDate);
    aboutPopup_->onCloseClicked = [this]() { hideAboutPopup(); };
    
    int x = (getWidth() - aboutPopup_->getWidth()) / 2;
    int y = (getHeight() - aboutPopup_->getHeight()) / 2;
    aboutPopup_->setBounds (x, y, 250, 140);
    addAndMakeVisible (*aboutPopup_);
    aboutPopup_->toFront (false);
}

void NotepadPluginAudioProcessorEditor::hideAboutPopup()
{
    if (aboutPopup_)
    {
        removeChildComponent (aboutPopup_.get());
        aboutPopup_.reset();
    }
}

void NotepadPluginAudioProcessorEditor::showNLAPopup()
{
    if (!nlaPopup_)
        nlaPopup_ = std::make_unique<NLAPopup>();
    
    int x = getWidth() - 10 - nlaPopup_->getWidth();
    int y = 10;
    nlaPopup_->setBounds (x, y, 200, 50);
    addAndMakeVisible (*nlaPopup_);
    nlaPopup_->toFront (false);
}

void NotepadPluginAudioProcessorEditor::hideNLAPopup()
{
    if (nlaPopup_)
    {
        removeChildComponent (nlaPopup_.get());
        nlaPopup_->setVisible (false);
    }
}

void NotepadPluginAudioProcessorEditor::showStyleGuidePopup()
{
    // Do nothing for now - placeholder for future functionality
}

void NotepadPluginAudioProcessorEditor::hideStyleGuidePopup()
{
    // Not needed since we're opening in browser
}

void NotepadPluginAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    // Get the position relative to this component
    auto pos = e.getEventRelativeTo (this).getPosition();
    
    // Close popups when clicking anywhere in the window (not just inside the popup bounds)
    if (pluginInfoPopup_ && pluginInfoPopup_->isVisible())
    {
        if (!pluginInfoPopup_->getBounds().contains (pos) &&
            (!gearButton_ || !gearButton_->getBounds().contains (pos)))
            hidePluginInfoPopup();
    }
    
    if (nlaPopup_ && nlaPopup_->isVisible())
    {
        if (!nlaPopup_->getBounds().contains (pos) &&
            (!nlaLabel_ || !nlaLabel_->getBounds().contains (pos)))
            hideNLAPopup();
    }
    
    if (aboutPopup_ && aboutPopup_->isVisible())
    {
        if (!aboutPopup_->getBounds().contains (pos))
            hideAboutPopup();
    }
    
    // Dismiss any active rename editor when clicking in the tab viewport area
    // Check if there's a rename editor active
    for (int i = 0; i < tabButtonsContainer.getNumChildComponents(); ++i)
    {
        auto* comp = tabButtonsContainer.getChildComponent (i);
        if (auto* renameEditor = dynamic_cast<TabRenameEditor*> (comp))
        {
            // Trigger the focus lost callback which will save and clean up
            renameEditor->giveAwayKeyboardFocus();
            break;
        }
    }
}

// ── Popup Component Implementations ──────────────────────────────────────

void PluginInfoPopup::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    juce::Colour surface2 (0xff181e29);
    juce::Colour surface3 (0xff202634);
    juce::Colour border (0xff2a3342);
    juce::Colour secondaryText (0xffa6b3c7);
    juce::Colour dimText (0xff6b778c);
    
    auto gradient = juce::ColourGradient (surface2, 0, 0, surface3, 0, bounds.getBottom(), false);
    g.setGradientFill (gradient);
    g.fillRoundedRectangle (bounds, 4.0f);
    
    g.setColour (border);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    
    g.setColour (secondaryText);
    g.setFont (juce::Font (14.0f, juce::Font::bold));
    g.drawText ("Plugin Info", 12, 10, getWidth() - 24, 20, juce::Justification::centredLeft, true);
    
    g.setFont (juce::Font (14.0f, juce::Font::plain));
    
    g.setColour (aboutHovered_ ? secondaryText : dimText);
    g.drawText ("About ScratchPad", 18, 35, getWidth() - 30, 20, juce::Justification::centredLeft, true);
    
    g.setColour (styleGuideHovered_ ? secondaryText : dimText);
    g.drawText ("ScratchPad Guide", 18, 60, getWidth() - 30, 20, juce::Justification::centredLeft, true);
}

void PluginInfoPopup::mouseMove (const juce::MouseEvent& e)
{
    bool wasAboutHovered = aboutHovered_;
    bool wasStyleGuideHovered = styleGuideHovered_;
    
    aboutHovered_ = e.y >= 35 && e.y < 55;
    styleGuideHovered_ = e.y >= 60 && e.y < 80;
    
    if (wasAboutHovered != aboutHovered_ || wasStyleGuideHovered != styleGuideHovered_)
        repaint();
}

void PluginInfoPopup::mouseExit (const juce::MouseEvent&)
{
    if (aboutHovered_ || styleGuideHovered_)
    {
        aboutHovered_ = false;
        styleGuideHovered_ = false;
        repaint();
    }
}

void PluginInfoPopup::mouseDown (const juce::MouseEvent& e)
{
    if (e.y >= 35 && e.y < 55)
    {
        if (onAboutClicked) onAboutClicked();
    }
    else if (e.y >= 60 && e.y < 80)
    {
        if (onStyleGuideClicked) onStyleGuideClicked();
    }
}

AboutPopup::AboutPopup (const juce::String& pluginFormat, const juce::String& architecture,
                       const juce::String& version, const juce::String& buildDate)
{
    setSize (250, 140);
    line1_ = pluginFormat + " " + architecture;
    line2_ = "Version " + version;
    line3_ = "Built on " + buildDate;
}

void AboutPopup::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    juce::Colour surface2 (0xff181e29);
    juce::Colour surface3 (0xff202634);
    juce::Colour border (0xff2a3342);
    juce::Colour primaryText (0xffe6f1ff);
    juce::Colour secondaryText (0xffa6b3c7);
    juce::Colour dimText (0xff6b778c);
    
    auto gradient = juce::ColourGradient (surface2, 0, 0, surface3, 0, bounds.getBottom(), false);
    g.setGradientFill (gradient);
    g.fillRoundedRectangle (bounds, 4.0f);
    
    g.setColour (border);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    
    // X button
    float xSize = 20.0f;
    float xPadding = 8.0f;
    juce::Rectangle<float> xBounds (bounds.getWidth() - xSize - xPadding, xPadding, xSize, xSize);
    g.setColour (xHovered_ ? secondaryText : dimText);
    float xInset = 6.0f;
    g.drawLine (xBounds.getX() + xInset, xBounds.getY() + xInset,
               xBounds.getRight() - xInset, xBounds.getBottom() - xInset, 2.0f);
    g.drawLine (xBounds.getRight() - xInset, xBounds.getY() + xInset,
               xBounds.getX() + xInset, xBounds.getBottom() - xInset, 2.0f);
    
    static constexpr int titleH = 22;
    static constexpr int titleGap = 6;
    static constexpr int infoLineH = 18;
    static constexpr int infoLines = 3;
    int contentH = titleH + titleGap + infoLineH * infoLines;
    int padding = (getHeight() - contentH) / 2;
    
    int titleY = padding;
    int infoY = titleY + titleH + titleGap;
    
    g.setColour (primaryText);
    g.setFont (juce::Font (16.0f, juce::Font::bold));
    g.drawText ("About ScratchPad", 20, titleY, getWidth() - 40, titleH, juce::Justification::centred, true);
    
    g.setColour (secondaryText);
    g.setFont (juce::Font (14.0f, juce::Font::plain));
    g.drawText (line1_, 20, infoY, getWidth() - 40, infoLineH, juce::Justification::centred, true);
    g.drawText (line2_, 20, infoY + infoLineH, getWidth() - 40, infoLineH, juce::Justification::centred, true);
    g.drawText (line3_, 20, infoY + infoLineH * 2, getWidth() - 40, infoLineH, juce::Justification::centred, true);
}

void AboutPopup::mouseMove (const juce::MouseEvent& e)
{
    float xSize = 20.0f;
    float xPadding = 8.0f;
    juce::Rectangle<float> xBounds (getWidth() - xSize - xPadding, xPadding, xSize, xSize);
    bool wasHovered = xHovered_;
    xHovered_ = xBounds.contains (e.position);
    if (wasHovered != xHovered_) repaint();
}

void AboutPopup::mouseExit (const juce::MouseEvent&)
{
    if (xHovered_) { xHovered_ = false; repaint(); }
}

void AboutPopup::mouseDown (const juce::MouseEvent& e)
{
    float xSize = 20.0f;
    float xPadding = 8.0f;
    juce::Rectangle<float> xBounds (getWidth() - xSize - xPadding, xPadding, xSize, xSize);
    if (xBounds.contains (e.position))
    {
        if (onCloseClicked) onCloseClicked();
    }
}

NLAPopup::NLAPopup()
{
    setSize (200, 50);
}

void NLAPopup::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    juce::Colour surface2 (0xff181e29);
    juce::Colour surface3 (0xff202634);
    juce::Colour border (0xff2a3342);
    juce::Colour secondaryText (0xffa6b3c7);
    
    auto gradient = juce::ColourGradient (surface2, 0, 0, surface3, 0, bounds.getBottom(), false);
    g.setGradientFill (gradient);
    g.fillRoundedRectangle (bounds, 4.0f);
    
    g.setColour (border);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    
    g.setColour (secondaryText);
    g.setFont (juce::Font (14.0f, juce::Font::plain));
    
    int lineHeight = 18;
    int totalTextHeight = lineHeight * 2;
    int startY = (getHeight() - totalTextHeight) / 2;
    
    g.drawText ("Developed by Joe Sanders", 10, startY, getWidth() - 20, lineHeight,
               juce::Justification::centred, true);
    g.drawText (juce::CharPointer_UTF8 ("\xC2\xA9 2026 Nine Lives Audio"), 10, startY + lineHeight,
               getWidth() - 20, lineHeight, juce::Justification::centred, true);
}
