#include "NativeFileChooser.h"

#if JUCE_MAC
#include <Cocoa/Cocoa.h>

bool showNativeSaveDialog (const juce::String& defaultFileName, const juce::String& fileContent)
{
    @autoreleasepool
    {
        NSSavePanel* savePanel = [NSSavePanel savePanel];
        
        NSString* defaultName = [NSString stringWithUTF8String: defaultFileName.toUTF8()];
        [savePanel setNameFieldStringValue: defaultName];
        [savePanel setMessage: @"Export current notepad as text file"];
        
        if ([savePanel runModal] == NSModalResponseOK)
        {
            NSURL* url = [savePanel URL];
            NSString* filePath = [url path];
            const char* pathCStr = [filePath UTF8String];
            
            juce::File selectedFile { juce::String (pathCStr) };
            return selectedFile.replaceWithText (fileContent);
        }
    }
    return false;
}

#else

bool showNativeSaveDialog (const juce::String&, const juce::String&)
{
    return false;
}

#endif
