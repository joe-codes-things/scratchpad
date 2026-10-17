#!/bin/bash
# uninstall.sh — core uninstall logic for ScratchPad
# Called by the Uninstaller.app wrapper with admin privileges.

VERSION_GLOB="*"  # removes any installed version

VST3_SYSTEM="/Library/Audio/Plug-Ins/VST3/ScratchPad.vst3"
AU_SYSTEM="/Library/Audio/Plug-Ins/Components/ScratchPad.component"
VST3_USER="$HOME/Library/Audio/Plug-Ins/VST3/ScratchPad.vst3"
AU_USER="$HOME/Library/Audio/Plug-Ins/Components/ScratchPad.component"

# Also remove versioned exports that the installer may have placed
VST3_VERSIONED_GLOB="/Library/Audio/Plug-Ins/VST3/nla_scratchpad_v*.vst3"
AU_VERSIONED_GLOB="/Library/Audio/Plug-Ins/Components/nla_scratchpad_v*.component"

# Also remove current naming convention
VST3_CURRENT="/Library/Audio/Plug-Ins/VST3/nla_scratchpad.vst3"
AU_CURRENT="/Library/Audio/Plug-Ins/Components/nla_scratchpad.component"

removed=0
errors=0

remove_item() {
    local path="$1"
    if [ -e "$path" ] || [ -d "$path" ]; then
        rm -rf "$path" && removed=$((removed + 1)) || errors=$((errors + 1))
    fi
}

# System-level (requires sudo — called via osascript do shell script ... with administrator privileges)
remove_item "$VST3_SYSTEM"
remove_item "$AU_SYSTEM"
remove_item "$VST3_CURRENT"
remove_item "$AU_CURRENT"

# Versioned names
for f in $VST3_VERSIONED_GLOB; do remove_item "$f"; done
for f in $AU_VERSIONED_GLOB;   do remove_item "$f"; done

# User-level (no sudo needed)
remove_item "$VST3_USER"
remove_item "$AU_USER"

echo "$removed items removed, $errors errors"
exit $errors
