#!/bin/bash
# Build and stage the macOS VST3 and AU plugins.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/macOS"
BUILD_TYPE="${1:-production}"

if [[ "$#" -gt 1 || ( "$BUILD_TYPE" != "development" && "$BUILD_TYPE" != "production" ) ]]; then
    echo "Usage: $0 [development|production]" >&2
    exit 2
fi

BUILD_CONFIGURATION=Release
if [[ "$BUILD_TYPE" == "development" ]]; then
    BUILD_CONFIGURATION=Debug
fi

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

OUTPUT_DIR="$PROJECT_ROOT/exports/${BUILD_TYPE}-builds/macOS/v${VERSION}"
if [[ -e "$OUTPUT_DIR" ]]; then
    echo "Error: Output already exists; refusing to overwrite: $OUTPUT_DIR" >&2
    exit 1
fi

PLUGIN_NAME=nla_scratchpad
if [[ "$BUILD_TYPE" == "development" ]]; then
    PLUGIN_NAME="${PLUGIN_NAME}_dev"
fi

mkdir -p "$BUILD_DIR"
cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" -G Xcode

xcodebuild -project "$BUILD_DIR/ScratchpadPlugin.xcodeproj" \
    -scheme ScratchpadPlugin_VST3 -configuration "$BUILD_CONFIGURATION" -quiet
xcodebuild -project "$BUILD_DIR/ScratchpadPlugin.xcodeproj" \
    -scheme ScratchpadPlugin_AU -configuration "$BUILD_CONFIGURATION" -quiet

VST3_SRC="$BUILD_DIR/ScratchpadPlugin_artefacts/$BUILD_CONFIGURATION/VST3/ScratchPad.vst3"
AU_SRC="$BUILD_DIR/ScratchpadPlugin_artefacts/$BUILD_CONFIGURATION/AU/ScratchPad.component"
VST3_DEST="$OUTPUT_DIR/${PLUGIN_NAME}.vst3"
AU_DEST="$OUTPUT_DIR/${PLUGIN_NAME}.component"

if [[ ! -d "$VST3_SRC" || ! -d "$AU_SRC" ]]; then
    echo "Error: Expected macOS plugin outputs were not produced." >&2
    echo "  VST3: $VST3_SRC" >&2
    echo "  AU:   $AU_SRC" >&2
    exit 1
fi

mkdir -p "$OUTPUT_DIR"
cp -R "$VST3_SRC" "$VST3_DEST"
cp -R "$AU_SRC" "$AU_DEST"

echo "macOS $BUILD_TYPE plugins built:"
echo "  VST3: $VST3_DEST"
echo "  AU:   $AU_DEST"
echo "  Version: $VERSION"
