#!/bin/bash
# build_installer.sh
# Builds a macOS .pkg installer for the ScratchPad VST3 and AU plugins.
# Requires: pkgbuild, productbuild (both included with Xcode Command Line Tools)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build/macOS"
PRODUCTION_OUTPUT_DIR="$PROJECT_DIR/exports/production-builds/macOS"
INSTALLER_DIR="$PROJECT_DIR/installer"
WORK_DIR="$PROJECT_DIR/installer_work"  # scratch space, cleaned up after

# ── Resolve version ──────────────────────────────────────────────────────────
VERSION=$(grep "project(ScratchpadPlugin VERSION" "$PROJECT_DIR/CMakeLists.txt" \
          | sed 's/.*VERSION \([0-9.]*\).*/\1/')
INSTALLER_NAME="ScratchPad_v${VERSION}_macOS"

# ── Source plugin paths ──────────────────────────────────────────────────────
VST3_SRC="$BUILD_DIR/ScratchpadPlugin_artefacts/Release/VST3/ScratchPad.vst3"
AU_SRC="$BUILD_DIR/ScratchpadPlugin_artefacts/Release/AU/ScratchPad.component"

# ── Destination paths (inside the installer payload) ────────────────────────
VST3_DEST_DIR="/Library/Audio/Plug-Ins/VST3"
AU_DEST_DIR="/Library/Audio/Plug-Ins/Components"

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

echo "========================================="
echo "ScratchPad Installer Builder"
echo "Version: $VERSION"
echo "========================================="
echo ""

# ── Preflight checks ─────────────────────────────────────────────────────────
if [ ! -d "$VST3_SRC" ]; then
    echo -e "${RED}Error: VST3 plugin not found at:${NC}"
    echo "  $VST3_SRC"
    echo "Run ./scripts/build_macos.sh first."
    exit 1
fi

if [ ! -d "$AU_SRC" ]; then
    echo -e "${RED}Error: AU plugin not found at:${NC}"
    echo "  $AU_SRC"
    echo "Run ./scripts/build_macos.sh first."
    exit 1
fi

# ── Clean and create work directory ─────────────────────────────────────────
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR/vst3_payload/$VST3_DEST_DIR"
mkdir -p "$WORK_DIR/au_payload/$AU_DEST_DIR"
mkdir -p "$WORK_DIR/pkgs"
mkdir -p "$PRODUCTION_OUTPUT_DIR"

# ── Sign plugins (ad-hoc) ────────────────────────────────────────────────────
echo "Signing plugins..."
codesign --force --sign - --timestamp=none "$VST3_SRC" 2>&1 | grep -v "replacing existing signature" || true
codesign --force --sign - --timestamp=none "$AU_SRC"   2>&1 | grep -v "replacing existing signature" || true
echo -e "${GREEN}Signed${NC}"
echo ""

# ── Copy payloads ────────────────────────────────────────────────────────────
echo "Staging payloads..."

# Copy and rename VST3 to versioned name
cp -R "$VST3_SRC" "$WORK_DIR/vst3_payload/$VST3_DEST_DIR/nla_scratchpad_v${VERSION}.vst3"

# Copy and rename AU to versioned name
cp -R "$AU_SRC" "$WORK_DIR/au_payload/$AU_DEST_DIR/nla_scratchpad_v${VERSION}.component"

echo -e "${GREEN}Done${NC}"
echo ""

# ── Build component packages ─────────────────────────────────────────────────
echo "Building component packages..."

# VST3
pkgbuild \
    --root "$WORK_DIR/vst3_payload" \
    --identifier "com.ninelivesaudio.scratchpad.vst3" \
    --version "$VERSION" \
    --install-location "/" \
    "$WORK_DIR/pkgs/ScratchPad_VST3.pkg"
echo -e "${GREEN}VST3 component pkg built${NC}"

# AU
pkgbuild \
    --root "$WORK_DIR/au_payload" \
    --identifier "com.ninelivesaudio.scratchpad.au" \
    --version "$VERSION" \
    --install-location "/" \
    "$WORK_DIR/pkgs/ScratchPad_AU.pkg"
echo -e "${GREEN}AU component pkg built${NC}"

# ── Build final product installer ────────────────────────────────────────────
echo "Building final installer..."

OUTPUT_PKG="$PRODUCTION_OUTPUT_DIR/${INSTALLER_NAME}.pkg"

productbuild \
    --distribution "$INSTALLER_DIR/distribution.xml" \
    --resources "$INSTALLER_DIR" \
    --package-path "$WORK_DIR/pkgs" \
    "$OUTPUT_PKG"

echo ""
echo -e "${GREEN}=========================================${NC}"
echo -e "${GREEN}Installer built successfully!${NC}"
echo -e "${GREEN}=========================================${NC}"
echo ""
echo "Output: $OUTPUT_PKG"
echo ""

# ── Cleanup ──────────────────────────────────────────────────────────────────
rm -rf "$WORK_DIR"
echo "Scratch files cleaned up."
echo ""
