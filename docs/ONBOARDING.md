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

- `source/PluginProcessor.*` — plugin processor and DAW state persistence
- `source/PluginEditor.*` — shared interface and cross-platform note export
- `CMakeLists.txt` — plugin metadata, platform formats, and JUCE linkage
- `scripts/build.sh` — combined macOS and Parallels Windows build entry point
- `scripts/build_macos.sh` — local macOS build
- `scripts/build_windows.sh` — source sync and remote Windows build orchestration
- `scripts/build_windows.ps1` — native MSVC and Inno Setup Windows build
- `scripts/build_installer.sh`, `scripts/build_dmg.sh` — macOS distribution
- `installer/windows/ScratchPad.iss` — Windows installer configuration

## Development

Initialize the JUCE submodule and build from the repository root. The
combined build needs SSH access to the Windows VM and the tools listed in
[BUILD_README.md](./BUILD_README.md):

```bash
git submodule update --init --recursive
./scripts/build.sh
```

Use `./scripts/build.sh production` for Release builds or
`./scripts/build.sh development` for Debug builds. Artifacts are placed under
version-specific folders in `exports/production-builds/` or
`exports/development-builds/`, respectively. Development builds contain plugin
bundles only; production builds also create a Windows installer. The separate
macOS installer scripts package production builds. Existing versioned outputs
are not overwritten.

The local DAW test projects are
`daw-testing-mac/scratchpad_CubaseTest/` and
`daw-testing-mac/scratchpad_LogicTest.logicx/`. Their parent directory is
ignored by Git, so these local sessions are not part of the published source.

The build scripts do not install plugins, request administrator privileges, or
change system plugin directories.

Change the plugin version in the root `CMakeLists.txt` when preparing a
release. See [BUILD_README.md](./BUILD_README.md) for build details and
[INSTALLER-README.md](./INSTALLER-README.md) for distribution packages.

## Bug Reporting Convention

When adding or updating reported bugs, maintain [`BUGS.md`](./BUGS.md). Assign
each new entry the next unused sequential identifier in the form `B001`,
`B002`, and so on. Never reuse or renumber IDs; move resolved entries to the
Resolved Bugs section instead of deleting them.
