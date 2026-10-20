# ScratchPad Installer System

## Overview

ScratchPad packages macOS plugins in a `.pkg` and `.dmg`, and the Windows
VST3 in an `.exe` installer and a ZIP distribution.

## Windows Installer

The combined `./scripts/build.sh` pipeline builds the Windows x64 VST3 and
installer on the Parallels Windows 11 VM, then creates
`exports/production-builds/Windows/v{VERSION}/ScratchPad_v{VERSION}_Windows.exe`
and
`exports/production-builds/Windows/v{VERSION}/ScratchPad_v{VERSION}_Windows.zip`,
containing the installer and project README on the Mac. It uses native MSVC and
Inno Setup 6; it does not need MinGW or Wine. SSH must be configured between
the Mac and VM. See
[BUILD_README.md](./BUILD_README.md) for the VM tool and connection
requirements.

To build only the Windows plugin and production distributions, run on the Mac:

```bash
./scripts/build_windows.sh
```

The installer places the versioned VST3 under the system VST3 plugin folder
and requests administrator privileges when a user runs it. The plugin and
installer are currently unsigned. The installer creates its own Windows
uninstaller; it is available through Windows Installed Apps/Apps & Features.
The ZIP contains `ScratchPad_v{VERSION}_Windows.exe` and `README.md`.

## macOS Installer

### What Gets Built

- **VST3 Plugin**: Installs to `/Library/Audio/Plug-Ins/VST3/`
- **AU Plugin**: Installs to `/Library/Audio/Plug-Ins/Components/`
- **Installer package**: Supports choosing VST3, AU, or both with Installer's
  Customize pane
- **Disk image**: Contains the installer package, uninstaller, and project
  README

### Prerequisites

- Xcode Command Line Tools (includes `pkgbuild`, `productbuild`, `hdiutil`,
  and `osacompile`)
- macOS plugins built in `build/macOS/ScratchpadPlugin_artefacts/Release/`

### Build

Build the macOS plugin targets:

```bash
./scripts/build_macos.sh
```

Build the installer package:

```bash
./scripts/build_installer.sh
```

Output: `exports/production-builds/macOS/v{VERSION}/ScratchPad_v{VERSION}_macOS.pkg`

Build the disk image:

```bash
./scripts/build_dmg.sh
```

Output: `exports/production-builds/macOS/v{VERSION}/ScratchPad_v{VERSION}_macOS.dmg`

The DMG includes the installer package, `ScratchPad Uninstaller.app`, and the
project-root `README.md` as Markdown. To choose which plugin formats to
install, open the package and click **Customize** (or **Custom Install**) on
the Installation Type screen, then select VST3, AU, or both.

### macOS File Structure

```
scripts/
├── build_installer.sh
└── build_dmg.sh

installer/
├── distribution.xml
├── welcome.html
├── license.html
└── uninstaller/
    ├── uninstall.sh
    └── UninstallerApp.applescript

exports/production-builds/macOS/v{VERSION}/
├── ScratchPad_v{VERSION}_macOS.pkg
└── ScratchPad_v{VERSION}_macOS.dmg
```

The DMG contains `ScratchPad_v{VERSION}_macOS.pkg`,
`ScratchPad Uninstaller.app`, and `README.md`.

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
project(ScratchpadPlugin VERSION 0.0.27 LANGUAGES C CXX)
```

Update macOS package UI text in `installer/welcome.html` and
`installer/license.html`. Change macOS install locations in
`installer/distribution.xml` and `scripts/build_installer.sh`. Change the
Windows installer in `installer/windows/ScratchPad.iss`.

## Code Signing

macOS packages use ad-hoc plugin signatures during installer packaging. Replace
these with Developer ID signing for distribution. Windows artifacts are
currently unsigned; signing support is deferred.
