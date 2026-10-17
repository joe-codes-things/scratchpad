#!/bin/bash
# Build and stage the macOS VST3 and AU plugins.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/macOS"
BUILDS_DIR="$PROJECT_ROOT/builds/macOS"

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "Error: The macOS build requires macOS with Xcode." >&2
    exit 1
fi

command -v cmake >/dev/null || { echo "Error: CMake is required." >&2; exit 1; }
command -v xcodebuild >/dev/null || { echo "Error: Xcode command-line tools are required." >&2; exit 1; }

VERSION=$(sed -nE 's/^project\(ScratchpadPlugin VERSION ([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' "$PROJECT_ROOT/CMakeLists.txt")
if [[ -z "$VERSION" ]]; then
    echo "Error: Could not read the project version from CMakeLists.txt." >&2
    exit 1
fi

mkdir -p "$BUILD_DIR"
cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" -G Xcode -DCMAKE_BUILD_TYPE=Release

xcodebuild -project "$BUILD_DIR/ScratchpadPlugin.xcodeproj" \
    -scheme ScratchpadPlugin_VST3 -configuration Release -quiet
xcodebuild -project "$BUILD_DIR/ScratchpadPlugin.xcodeproj" \
    -scheme ScratchpadPlugin_AU -configuration Release -quiet

VST3_SRC="$BUILD_DIR/ScratchpadPlugin_artefacts/Release/VST3/ScratchPad.vst3"
AU_SRC="$BUILD_DIR/ScratchpadPlugin_artefacts/Release/AU/ScratchPad.component"
VST3_DEST="$BUILDS_DIR/nla_scratchpad.vst3"
AU_DEST="$BUILDS_DIR/nla_scratchpad.component"

if [[ ! -d "$VST3_SRC" || ! -d "$AU_SRC" ]]; then
    echo "Error: Expected macOS plugin outputs were not produced." >&2
    echo "  VST3: $VST3_SRC" >&2
    echo "  AU:   $AU_SRC" >&2
    exit 1
fi

mkdir -p "$BUILDS_DIR"
rm -rf "$VST3_DEST" "$AU_DEST"
cp -R "$VST3_SRC" "$VST3_DEST"
cp -R "$AU_SRC" "$AU_DEST"

echo "macOS plugins built:"
echo "  VST3: $VST3_DEST"
echo "  AU:   $AU_DEST"
echo "  Version: $VERSION"
