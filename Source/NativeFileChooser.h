#pragma once

#include <juce_core/juce_core.h>

/**
 * Shows a native save dialog and returns the selected file path.
 * @param defaultFileName The default filename to suggest
 * @param fileContent The content to write if user confirms
 * @return True if file was successfully saved, false if cancelled or error
 */
bool showNativeSaveDialog (const juce::String& defaultFileName, const juce::String& fileContent);
