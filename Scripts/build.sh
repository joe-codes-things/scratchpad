#!/bin/bash
# Build ScratchPad plugin and copy the artifacts to builds/

set -e

# Resolve script location (handles symlinks)
SCRIPT_PATH="$0"
while [ -L "$SCRIPT_PATH" ]; do
    SCRIPT_PATH=$(readlink "$SCRIPT_PATH")
done
PROJECT_ROOT="$( cd "$( dirname "$SCRIPT_PATH" )/.." && pwd )"
BUILD_DIR="$PROJECT_ROOT/build"
BUILDS_DIR="$PROJECT_ROOT/builds"

# Read version from CMakeLists.txt
VERSION=$(grep "project.*VERSION" "$PROJECT_ROOT/CMakeLists.txt" | sed -E 's/.*VERSION ([0-9]+\.[0-9]+\.[0-9]+).*/\1/')
echo "🔨 Building ScratchPad Plugin v$VERSION..."

# Configure and build
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure if needed
if [ ! -f "CMakeCache.txt" ]; then
    echo "📋 Configuring CMake..."
    cmake -G Xcode -DCMAKE_BUILD_TYPE=Release ..
fi

# Build VST3
echo "🎵 Building VST3..."
xcodebuild -scheme ScratchpadPlugin_VST3 -configuration Release -quiet

# Build AU
echo "🎵 Building AU..."
xcodebuild -scheme ScratchpadPlugin_AU -configuration Release -quiet

# Create builds folder if it doesn't exist
mkdir -p "$BUILDS_DIR"

# Copy with versioning
echo "📁 Copying plugins to builds folder..."

VST3_SRC="$BUILD_DIR/ScratchpadPlugin_artefacts/Release/VST3/ScratchPad.vst3"
AU_SRC="$BUILD_DIR/ScratchpadPlugin_artefacts/Release/AU/ScratchPad.component"
VST3_DEST="$BUILDS_DIR/nla_scratchpad.vst3"
AU_DEST="$BUILDS_DIR/nla_scratchpad.component"

# Confirm build outputs before updating distributable artifacts.
if [ ! -d "$VST3_SRC" ]; then
    echo "❌ VST3 not found: $VST3_SRC" >&2
    exit 1
fi

if [ ! -d "$AU_SRC" ]; then
    echo "❌ AU not found: $AU_SRC" >&2
    exit 1
fi

rm -rf "$VST3_DEST" "$AU_DEST"
cp -R "$VST3_SRC" "$VST3_DEST"
cp -R "$AU_SRC" "$AU_DEST"

echo ""
echo "✨ Build complete!"
echo "   VST3: $VST3_DEST"
echo "   AU:   $AU_DEST"
echo "   Artifacts are in builds/. Install them manually in your DAW's plugin folder if desired."
