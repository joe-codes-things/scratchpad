# Build ScratchPad

## Requirements

- macOS with Xcode and its command-line tools
- CMake
- Git

The project uses JUCE as a Git submodule at `JUCE_framework/`.

## Build

From the repository root:

```bash
git submodule update --init --recursive
./Scripts/build.sh
```

The script configures an Xcode build in `build/`, builds the VST3 and AU
formats, and copies the resulting bundles to:

- `builds/nla_scratchpad.vst3`
- `builds/nla_scratchpad.component`

The build script does not install plugins or use `sudo`. To use a plugin,
copy the desired bundle to a plugin directory scanned by your DAW. A
system-wide installer can be built separately using the installer scripts.

## Clean Build

Remove the generated build directory and rebuild:

```bash
rm -rf build
./Scripts/build.sh
```

## Project Layout

- `Source/` — plugin processor, editor, and native file chooser
- `JUCE_framework/` — JUCE dependency
- `Scripts/` — build and installer scripts
- `build/` — generated CMake and Xcode files
- `builds/` — latest local VST3 and AU bundles
- `docs/` — project documentation
