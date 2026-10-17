# Build ScratchPad

## Requirements

### Mac

- macOS with Xcode and its command-line tools
- CMake and Git
- SSH access to the Windows build VM

### Windows 11 VM

- A running Parallels Windows 11 VM reachable from the Mac over SSH
- Visual Studio 2022 with both the MSVC x64 and ARM64 C++ tools and a Windows
  SDK. The x64 tools build the plugin; the ARM64 tools build JUCE's native
  helper utilities on an ARM64 VM.
- CMake
- Inno Setup 6

The project uses JUCE as a Git submodule at `JUCE_framework/`.

## Build Both Platforms

From the repository root on the Mac:

```bash
git submodule update --init --recursive
./Scripts/build.sh
```

The script first checks the VM connection and its build tools, builds macOS
locally, syncs the source to the VM over SSH, builds the Windows VST3 with
MSVC, and packages it with Inno Setup. Both platforms share project source
and JUCE, but use independent generated build state and outputs:

- `build/macOS/` — local Xcode project and macOS CMake state
- `build/Windows/transfer/` — temporary source-transfer staging on the Mac
- `Coding Projects\ScratchPadBuild\source\build\Windows\x64\` under the VM
  user's profile — native Windows CMake state on the VM
- `builds/macOS/nla_scratchpad.vst3`
- `builds/macOS/nla_scratchpad.component`
- `builds/Windows/nla_scratchpad.vst3`
- `builds/Windows/ScratchPad_v{VERSION}_Windows.exe`

The Windows installer installs the x64 VST3 to the system VST3 plug-in
directory. The installer and plugin are unsigned. The build scripts do not
install plugins or use `sudo`; to use a plugin without the installer, copy the
bundle to a directory scanned by your DAW.

The Windows VM must be running and SSH-reachable whenever the combined build
runs. Put its address, account name, and private-key path in the ignored
`.windows-vm.env` file at the repository root:

```bash
WINDOWS_VM_HOST=your-vm-address
WINDOWS_VM_USER=your-windows-account
WINDOWS_VM_SSH_KEY=/absolute/path/to/private-key
```

The script also accepts these values from the environment; the local
configuration file is deliberately excluded from Git. Keep private keys
outside the repository and never add private-key contents to this file.

## Build One Platform

Build just the local macOS plugins:

```bash
./Scripts/build_macos.sh
```

Build just the Windows plugin and installer through the VM:

```bash
./Scripts/build_windows.sh
```

To run the native Windows build directly in a source copy on the VM, execute
`Scripts\build_windows.ps1` from PowerShell.

## Clean Build

Remove only the platform build directory that needs reconfiguration, then run
its build script again. For example, the macOS build tree can be recreated
with:

```bash
rm -rf build/macOS
./Scripts/build_macos.sh
```

The Windows CMake tree is in the VM user's
`Coding Projects\ScratchPadBuild\source\build\Windows\x64` directory.

## Project Layout

- `Source/` — shared plugin processor and editor
- `JUCE_framework/` — JUCE dependency
- `Scripts/` — combined, per-platform, and installer build scripts
- `Scripts/build_windows.ps1` — native MSVC and Inno Setup Windows build
- `installer/windows/` — Windows Inno Setup configuration
- `build/macOS/` — local macOS generated build tree
- `builds/macOS/`, `builds/Windows/` — staged platform artifacts
- `docs/` — build and project documentation
