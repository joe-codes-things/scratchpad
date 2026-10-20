#!/bin/bash
# build_dmg.sh
# Builds the uninstaller app and packages it with the installer and README into a .dmg.
# Run after build_installer.sh.
# Requires: osacompile and hdiutil (included with macOS).

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
INSTALLER_DIR="$PROJECT_DIR/installer"
WORK_DIR="$PROJECT_DIR/dmg_work"

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

# ── Resolve version ──────────────────────────────────────────────────────────
VERSION=$(grep "project(ScratchpadPlugin VERSION" "$PROJECT_DIR/CMakeLists.txt" \
          | sed 's/.*VERSION \([0-9.]*\).*/\1/')

PKG_NAME="ScratchPad_v${VERSION}_macOS.pkg"
DMG_NAME="ScratchPad_v${VERSION}_macOS.dmg"
PRODUCTION_OUTPUT_DIR="$PROJECT_DIR/exports/production-builds/macOS/v${VERSION}"
PKG_PATH="$PRODUCTION_OUTPUT_DIR/$PKG_NAME"
FINAL_DMG="$PRODUCTION_OUTPUT_DIR/$DMG_NAME"
README_SRC="$PROJECT_DIR/README.md"

echo "========================================="
echo "ScratchPad DMG Builder"
echo "Version: $VERSION"
echo "========================================="
echo ""

# ── Preflight ────────────────────────────────────────────────────────────────
if [ ! -f "$PKG_PATH" ]; then
    echo -e "${RED}Error: Installer pkg not found at:${NC}"
    echo "  $PKG_PATH"
    echo "Run ./scripts/build_installer.sh first."
    exit 1
fi

if [ ! -f "$README_SRC" ]; then
    echo -e "${RED}Error: README not found at:${NC}"
    echo "  $README_SRC"
    exit 1
fi

for command in osacompile hdiutil; do
    if ! command -v "$command" >/dev/null 2>&1; then
        echo -e "${RED}Error: Required command '$command' was not found.${NC}" >&2
        exit 1
    fi
done

if [ -e "$FINAL_DMG" ]; then
    echo "Error: Output already exists; refusing to overwrite: $FINAL_DMG" >&2
    exit 1
fi

# ── Clean work dir ───────────────────────────────────────────────────────────
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR/dmg_contents"

# ── Build Uninstaller.app ────────────────────────────────────────────────────
echo "Building Uninstaller.app..."

APP_STAGE="$WORK_DIR/Uninstaller.app"

# osacompile requires the output path to not already exist
OSAC_OUT="$WORK_DIR/UninstallerRaw.app"

osacompile \
    -o "$OSAC_OUT" \
    "$INSTALLER_DIR/uninstaller/UninstallerApp.applescript"

# Copy the uninstall shell script into Resources
mkdir -p "$OSAC_OUT/Contents/Resources"
cp "$INSTALLER_DIR/uninstaller/uninstall.sh" "$OSAC_OUT/Contents/Resources/uninstall.sh"
chmod +x "$OSAC_OUT/Contents/Resources/uninstall.sh"

# Patch Info.plist display name
/usr/libexec/PlistBuddy -c "Set :CFBundleName 'ScratchPad Uninstaller'" \
    "$OSAC_OUT/Contents/Info.plist" 2>/dev/null || true
/usr/libexec/PlistBuddy -c "Set :CFBundleDisplayName 'ScratchPad Uninstaller'" \
    "$OSAC_OUT/Contents/Info.plist" 2>/dev/null || true

# Rename to final app name
mv "$OSAC_OUT" "$APP_STAGE"

echo -e "${GREEN}Uninstaller.app built${NC}"
echo ""

# ── Stage DMG contents ───────────────────────────────────────────────────────
echo "Staging DMG contents..."

DMG_STAGE="$WORK_DIR/dmg_contents"

# Installer package
cp "$PKG_PATH" "$DMG_STAGE/$PKG_NAME"

# Uninstaller app
cp -R "$APP_STAGE" "$DMG_STAGE/ScratchPad Uninstaller.app"

# Project README
cp "$README_SRC" "$DMG_STAGE/README.md"
echo -e "${GREEN}README.md included in DMG${NC}"

echo -e "${GREEN}DMG contents staged${NC}"
echo ""

# ── Create DMG ───────────────────────────────────────────────────────────────
echo "Creating DMG..."

TEMP_DMG="$WORK_DIR/temp.dmg"

# Create a writable DMG from the staged folder
hdiutil create \
    -volname "ScratchPad v${VERSION}" \
    -srcfolder "$DMG_STAGE" \
    -ov \
    -format UDRW \
    "$TEMP_DMG"

# Convert to compressed read-only DMG
hdiutil convert "$TEMP_DMG" \
    -ov \
    -format UDZO \
    -imagekey zlib-level=9 \
    -o "$FINAL_DMG"

echo ""
echo -e "${GREEN}=========================================${NC}"
echo -e "${GREEN}DMG built successfully!${NC}"
echo -e "${GREEN}=========================================${NC}"
echo ""
echo "Output: $FINAL_DMG"
SIZE=$(du -sh "$FINAL_DMG" | cut -f1)
echo "Size:   $SIZE"
echo ""

# ── Cleanup ──────────────────────────────────────────────────────────────────
rm -rf "$WORK_DIR"
echo "Scratch files cleaned up."
