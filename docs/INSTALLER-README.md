# ScratchPad Installer System

## Overview

The ScratchPad installer system creates professional macOS distribution packages (.pkg) and disk images (.dmg) for easy plugin installation and distribution.

## What Gets Built

### Installer Package (.pkg)
- **VST3 Plugin**: Installs to `/Library/Audio/Plug-Ins/VST3/`
- **AU Plugin**: Installs to `/Library/Audio/Plug-Ins/Components/`

### Disk Image (.dmg)
Contains:
- Installer package (.pkg)
- Uninstaller app
- Project `README.md`

## Building the Installer

### Prerequisites
- Xcode Command Line Tools (includes pkgbuild, productbuild, hdiutil, osacompile)
- Built plugins in `build/ScratchpadPlugin_artefacts/Release/`

### Step 1: Build the Plugin
```bash
./Scripts/build.sh
```

### Step 2: Build the Installer Package
```bash
./Scripts/build_installer.sh
```

This creates: `exports/ScratchPad_v{VERSION}_macOS.pkg`

### Step 3: Build the DMG
```bash
./Scripts/build_dmg.sh
```

This creates: `exports/ScratchPad_v{VERSION}_macOS.dmg`

The DMG includes the project-root `README.md` as Markdown alongside the installer package and uninstaller app.

### Quick Build (All Steps)
```bash
./Scripts/build.sh && ./Scripts/build_installer.sh && ./Scripts/build_dmg.sh
```

## File Structure

```
scratchpad/
├── Scripts/
│   ├── build_installer.sh        # Builds .pkg installer
│   └── build_dmg.sh              # Builds .dmg with uninstaller
│
├── installer/
│   ├── distribution.xml          # Installer configuration
│   ├── welcome.html              # Welcome screen
│   ├── license.html              # License agreement
│   └── uninstaller/
│       ├── uninstall.sh          # Uninstall logic
│       └── UninstallerApp.applescript  # Uninstaller UI
│
└── exports/
    ├── ScratchPad_v{VERSION}_macOS.pkg
    └── ScratchPad_v{VERSION}_macOS.dmg
```

## Installer Features

### Component Selection
Users can choose whether to install the VST3 and AU plugin formats.

### Versioned Installation
Plugins are installed with version numbers:
- `nla_scratchpad_v{VERSION}.vst3`
- `nla_scratchpad_v{VERSION}.component`

This allows multiple versions to coexist if needed.

### System Requirements
- macOS 11.0 (Big Sur) or later
- Administrator privileges for plugin installation

## Uninstaller

The uninstaller removes:
- All installed VST3 plugins (versioned and current)
- All installed AU plugins (versioned and current)

### Using the Uninstaller
1. Open the DMG
2. Double-click "ScratchPad Uninstaller.app"
3. Confirm uninstallation
4. Enter admin password when prompted

## Customization

### Changing Version
Version is automatically read from `CMakeLists.txt`:
```cmake
project(ScratchpadPlugin VERSION 0.0.23 LANGUAGES CXX)
```

### Updating Welcome/License Text
Edit these files:
- `installer/welcome.html`
- `installer/license.html`

### Modifying Install Locations
Edit `installer/distribution.xml` and `Scripts/build_installer.sh`

## Distribution

### For End Users
Distribute the `.dmg` file:
- `ScratchPad_v{VERSION}_macOS.dmg`

Users can:
1. Mount the DMG
2. Run the installer package
3. Keep the uninstaller for later use
4. Open `README.md` for project and usage information

### For Testing
Use the `.pkg` file directly:
- `ScratchPad_v{VERSION}_macOS.pkg`

## Troubleshooting

### "Installer package not found"
Run `./Scripts/build.sh` first to build the plugins.

### Installer Won't Run
Check that you have Xcode Command Line Tools:
```bash
xcode-select --install
```

### DMG Won't Mount
Verify the DMG integrity:
```bash
hdiutil verify exports/ScratchPad_v{VERSION}_macOS.dmg
```

## Technical Details

### Code Signing
Plugins are ad-hoc signed during installer build:
```bash
codesign --force --sign - --timestamp=none [plugin]
```

For distribution, replace with proper Developer ID signing.

### Compression
DMG uses UDZO format with zlib level 9 compression for optimal size.

## Version History

### v0.0.23 (May 27, 2026)
- Initial installer system implementation
- Includes .pkg installer, .dmg distribution, and uninstaller app
