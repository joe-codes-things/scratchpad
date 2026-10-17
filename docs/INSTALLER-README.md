# ScratchPad Installer System

## Overview

ScratchPad packages macOS plugins in a `.pkg` and `.dmg`, and the Windows
VST3 in an `.exe` installer.

## Windows Installer

The combined `./Scripts/build.sh` pipeline builds the Windows x64 VST3 and
creates `builds/Windows/ScratchPad_v{VERSION}_Windows.exe` on the Parallels
Windows 11 VM. It uses native MSVC and Inno Setup 6; it does not need MinGW or
Wine. SSH must be configured between the Mac and VM. See
[BUILD_README.md](./BUILD_README.md) for the VM tool and connection
requirements.

To build only the Windows plugin and installer, run on the Mac:

```bash
./Scripts/build_windows.sh
```

The installer places the versioned VST3 under the system VST3 plugin folder
and requests administrator privileges when a user runs it. The plugin and
installer are currently unsigned.

## macOS Installer

### What Gets Built

- **VST3 Plugin**: Installs to `/Library/Audio/Plug-Ins/VST3/`
- **AU Plugin**: Installs to `/Library/Audio/Plug-Ins/Components/`
- **Disk image**: Contains the package, uninstaller, and project README

### Prerequisites

- Xcode Command Line Tools (includes `pkgbuild`, `productbuild`, `hdiutil`,
  and `osacompile`)
- macOS plugins built in `build/macOS/ScratchpadPlugin_artefacts/Release/`

### Build

Build the macOS plugin targets:

```bash
./Scripts/build_macos.sh
```

Build the installer package:

```bash
./Scripts/build_installer.sh
```

Output: `exports/ScratchPad_v{VERSION}_macOS.pkg`

Build the disk image:

```bash
./Scripts/build_dmg.sh
```

Output: `exports/ScratchPad_v{VERSION}_macOS.dmg`

The DMG includes the project-root `README.md` as Markdown alongside the
installer package and uninstaller app.

### macOS File Structure

```
Scripts/
├── build_installer.sh
└── build_dmg.sh

installer/
├── distribution.xml
├── welcome.html
├── license.html
└── uninstaller/
    ├── uninstall.sh
    └── UninstallerApp.applescript

exports/
├── ScratchPad_v{VERSION}_macOS.pkg
└── ScratchPad_v{VERSION}_macOS.dmg
```

## Versioned Installation

Plugins are installed with version numbers so multiple versions can coexist:

- Windows: `nla_scratchpad_v{VERSION}.vst3`
- macOS VST3: `nla_scratchpad_v{VERSION}.vst3`
- macOS AU: `nla_scratchpad_v{VERSION}.component`

## System Requirements

- macOS 11.0 (Big Sur) or later
- Windows 11 for the Windows installer
- Administrator privileges to install system-wide

## macOS Uninstaller

The macOS uninstaller removes installed VST3 and AU plugins. Open the DMG,
launch "ScratchPad Uninstaller.app", confirm uninstallation, and enter the
administrator password when prompted.

## Customization

The version is read from the root `CMakeLists.txt`:

```cmake
project(ScratchpadPlugin VERSION 0.0.24 LANGUAGES C CXX)
```

Update macOS installer UI text in `installer/welcome.html` and
`installer/license.html`. Change macOS install locations in
`installer/distribution.xml` and `Scripts/build_installer.sh`. Change the
Windows installer in `installer/windows/ScratchPad.iss`.

## Code Signing

macOS packages use ad-hoc plugin signatures during installer packaging. Replace
these with Developer ID signing for distribution. Windows artifacts are
currently unsigned; signing support is deferred.
