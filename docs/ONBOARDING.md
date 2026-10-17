# ScratchPad Project Overview

ScratchPad is a JUCE-based text-notepad audio plugin for macOS. It provides
multiple notes per DAW project, adjustable text size, note renaming, and text
export. The plugin formats are VST3 and Audio Unit (AU).

## Current Configuration

- Product name: `ScratchPad`
- CMake project: `ScratchpadPlugin`
- Version: `0.0.24` (from the root `CMakeLists.txt`)
- Language standard: C++17
- JUCE dependency: `JUCE_framework/` Git submodule

## Source Layout

- `Source/PluginProcessor.*` — audio processor and DAW state persistence
- `Source/PluginEditor.*` — user interface and note interactions
- `Source/NativeFileChooser.mm` — macOS export file chooser
- `CMakeLists.txt` — plugin metadata, version, formats, and JUCE linkage
- `Scripts/build.sh` — local build and artifact copy
- `Scripts/build_installer.sh`, `Scripts/build_dmg.sh` — optional macOS distribution

## Development

Initialize the JUCE submodule and build from the repository root:

```bash
git submodule update --init --recursive
./Scripts/build.sh
```

The VST3 and AU bundles are copied to `builds/`. The build script does not
install them, request administrator privileges, or change system plugin
directories.

Change the plugin version in the root `CMakeLists.txt` when preparing a
release. See [BUILD_README.md](./BUILD_README.md) for build details and
[INSTALLER-README.md](./INSTALLER-README.md) for distribution packages.
