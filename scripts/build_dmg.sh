#!/bin/bash
# build_dmg.sh
# Builds the installer and uninstaller apps and packages them with the README into a .dmg.
# Run after build_installer.sh.
# Requires: Swift, Xcode Command Line Tools, osacompile, and hdiutil.

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
INSTALLER_APP_SOURCE="$INSTALLER_DIR/macOS/ScratchPadInstaller.swift"

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

if [ ! -f "$INSTALLER_APP_SOURCE" ]; then
    echo -e "${RED}Error: Installer app source not found at:${NC}"
    echo "  $INSTALLER_APP_SOURCE"
    exit 1
fi

for command in swiftc lipo codesign osacompile hdiutil; do
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

# ── Build format-selection installer app ──────────────────────────────────────
echo "Building ScratchPad Installer.app..."

INSTALLER_APP="$WORK_DIR/ScratchPad Installer.app"
INSTALLER_APP_CONTENTS="$INSTALLER_APP/Contents"
mkdir -p "$INSTALLER_APP_CONTENTS/MacOS" "$INSTALLER_APP_CONTENTS/Resources"

swiftc -target arm64-apple-macos11.0 \
    "$INSTALLER_APP_SOURCE" \
    -o "$WORK_DIR/ScratchPadInstaller.arm64"
swiftc -target x86_64-apple-macos11.0 \
    "$INSTALLER_APP_SOURCE" \
    -o "$WORK_DIR/ScratchPadInstaller.x86_64"
lipo -create \
    "$WORK_DIR/ScratchPadInstaller.arm64" \
    "$WORK_DIR/ScratchPadInstaller.x86_64" \
    -output "$INSTALLER_APP_CONTENTS/MacOS/ScratchPadInstaller"

cat > "$INSTALLER_APP_CONTENTS/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>ScratchPadInstaller</string>
    <key>CFBundleIdentifier</key>
    <string>com.ninelivesaudio.scratchpad.installer</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>ScratchPad Installer</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>$VERSION</string>
    <key>CFBundleVersion</key>
    <string>$VERSION</string>
    <key>LSMinimumSystemVersion</key>
    <string>11.0</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
</dict>
</plist>
PLIST

cp "$PKG_PATH" "$INSTALLER_APP_CONTENTS/Resources/ScratchPad.pkg"
codesign --force --sign - --timestamp=none "$INSTALLER_APP"

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

# Format-selection installer app
cp -R "$INSTALLER_APP" "$DMG_STAGE/ScratchPad Installer.app"

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
