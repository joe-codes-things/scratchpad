# ScratchPad Project Overview

ScratchPad is a JUCE-based text-notepad audio plugin for macOS and Windows. It
provides multiple notes per DAW project, adjustable text size, note renaming,
and text export. The plugin formats are VST3 on both platforms and Audio Unit
(AU) on macOS.

## Current Configuration

- Product name: `ScratchPad`
- CMake project: `ScratchpadPlugin`
- Version: `0.0.24` (from the root `CMakeLists.txt`)
- Language standard: C++17
- JUCE dependency: `JUCE_framework/` Git submodule

## Source Layout

- `Source/PluginProcessor.*` — plugin processor and DAW state persistence
- `Source/PluginEditor.*` — shared interface and cross-platform note export
- `CMakeLists.txt` — plugin metadata, platform formats, and JUCE linkage
- `Scripts/build.sh` — combined macOS and Parallels Windows build entry point
- `Scripts/build_macos.sh` — local macOS build
- `Scripts/build_windows.sh` — source sync and remote Windows build orchestration
- `Scripts/build_windows.ps1` — native MSVC and Inno Setup Windows build
- `Scripts/build_installer.sh`, `Scripts/build_dmg.sh` — macOS distribution
- `installer/windows/ScratchPad.iss` — Windows installer configuration

## Development

Initialize the JUCE submodule and build from the repository root. The
combined build needs SSH access to the Windows VM and the tools listed in
[BUILD_README.md](./BUILD_README.md):

```bash
git submodule update --init --recursive
./Scripts/build.sh
```

macOS and Windows artifacts are copied into separate subfolders under
`builds/`. The build scripts do not install plugins, request administrator
privileges, or change system plugin directories.

Change the plugin version in the root `CMakeLists.txt` when preparing a
release. See [BUILD_README.md](./BUILD_README.md) for build details and
[INSTALLER-README.md](./INSTALLER-README.md) for distribution packages.
