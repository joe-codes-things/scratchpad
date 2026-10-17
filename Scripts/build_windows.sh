#!/bin/bash
# Sync the project to the Parallels Windows VM, build with MSVC, and retrieve artifacts.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/Windows/transfer"
BUILDS_DIR="$PROJECT_ROOT/builds/Windows"
SOURCE_ARCHIVE="$BUILD_DIR/scratchpad-source.tar.gz"

CONFIG_FILE="$PROJECT_ROOT/.windows-vm.env"
CONFIG_HOST=""
CONFIG_USER=""
CONFIG_SSH_KEY=""
if [[ -f "$CONFIG_FILE" ]]; then
    while IFS= read -r line || [[ -n "$line" ]]; do
        [[ -z "$line" || "$line" == \#* ]] && continue
        if [[ "$line" != *=* ]]; then
            echo "Error: Invalid line in $CONFIG_FILE; expected NAME=value." >&2
            exit 1
        fi
        name="${line%%=*}"
        value="${line#*=}"
        case "$name" in
            WINDOWS_VM_HOST) CONFIG_HOST="$value" ;;
            WINDOWS_VM_USER) CONFIG_USER="$value" ;;
            WINDOWS_VM_SSH_KEY) CONFIG_SSH_KEY="$value" ;;
            *)
                echo "Error: Unsupported setting '$name' in $CONFIG_FILE." >&2
                exit 1
                ;;
        esac
    done < "$CONFIG_FILE"
fi

VM_HOST="${WINDOWS_VM_HOST:-$CONFIG_HOST}"
VM_USER="${WINDOWS_VM_USER:-$CONFIG_USER}"
SSH_KEY="${WINDOWS_VM_SSH_KEY:-$CONFIG_SSH_KEY}"
if [[ -z "$VM_HOST" || -z "$VM_USER" ]]; then
    echo "Error: Set WINDOWS_VM_HOST and WINDOWS_VM_USER in $CONFIG_FILE or the environment." >&2
    exit 1
fi
REMOTE="$VM_USER@$VM_HOST"

for command in ssh scp tar iconv; do
    if ! command -v "$command" >/dev/null 2>&1; then
        echo "Error: Required command '$command' was not found." >&2
        exit 1
    fi
done

if [[ -n "$SSH_KEY" && ! -f "$SSH_KEY" ]]; then
    echo "Error: Windows VM SSH key not found: $SSH_KEY" >&2
    exit 1
fi

ssh_options=(-o BatchMode=yes -o ConnectTimeout=10)
if [[ -n "$SSH_KEY" ]]; then
    ssh_options+=(-i "$SSH_KEY")
fi

encode_powershell() {
    printf '%s' "$1" | iconv -f UTF-8 -t UTF-16LE | base64 | tr -d '\n'
}

CHECK_SCRIPT=$(cat <<'POWERSHELL'
$ErrorActionPreference = "Stop"
$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\\Installer\\vswhere.exe"
if (-not (Test-Path $vswhere)) { throw "Visual Studio vswhere.exe was not found." }
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -requires Microsoft.VisualStudio.Component.VC.Tools.ARM64 -property installationPath
if (-not $vs) { throw "Visual Studio x64 and ARM64 C++ build tools are missing." }
$iscc = Join-Path $env:LOCALAPPDATA "Programs\\Inno Setup 6\\ISCC.exe"
if (-not (Test-Path $iscc)) { throw "Inno Setup 6 was not found at $iscc." }
if (-not (Get-Command cmake.exe -ErrorAction SilentlyContinue)) { throw "CMake is not on PATH." }
Write-Output "Windows VM is ready: MSVC x64, CMake, and Inno Setup."
POWERSHELL
)

if [[ "${1:-}" == "--check-only" ]]; then
    if [[ "$#" -ne 1 ]]; then
        echo "Usage: $0 [--check-only]" >&2
        exit 2
    fi
    CHECK_ENCODED="$(encode_powershell "$CHECK_SCRIPT")"
    ssh "${ssh_options[@]}" "$REMOTE" powershell.exe -NoProfile -EncodedCommand "$CHECK_ENCODED"
    exit 0
elif [[ "$#" -gt 0 ]]; then
    echo "Usage: $0 [--check-only]" >&2
    exit 2
fi

mkdir -p "$BUILD_DIR" "$BUILDS_DIR"
REMOTE_TRANSFER_CREATED=0
REMOTE_TRANSFER="ScratchPadBuild-transfer-$$-$RANDOM"
REMOTE_CLEANUP_SCRIPT=$(cat <<'POWERSHELL'
$ErrorActionPreference = "Stop"
$transfer = Join-Path $env:USERPROFILE "__REMOTE_TRANSFER__"
if (Test-Path -LiteralPath $transfer) {
    Remove-Item -LiteralPath $transfer -Recurse -Force
}
POWERSHELL
)
REMOTE_CLEANUP_SCRIPT="${REMOTE_CLEANUP_SCRIPT//__REMOTE_TRANSFER__/$REMOTE_TRANSFER}"
REMOTE_CLEANUP_ENCODED="$(encode_powershell "$REMOTE_CLEANUP_SCRIPT")"
cleanup() {
    local status=$?
    rm -f "$SOURCE_ARCHIVE"
    if [[ "$REMOTE_TRANSFER_CREATED" == 1 ]]; then
        if ! ssh "${ssh_options[@]}" "$REMOTE" powershell.exe -NoProfile -EncodedCommand "$REMOTE_CLEANUP_ENCODED"; then
            echo "Warning: Could not remove remote temporary artifacts at ~/$REMOTE_TRANSFER." >&2
        fi
    fi
    return "$status"
}
trap cleanup EXIT
tar -czf "$SOURCE_ARCHIVE" \
    --exclude='JUCE_framework/.git' \
    -C "$PROJECT_ROOT" \
    CMakeLists.txt Source JUCE_framework installer/windows Scripts/build_windows.ps1

scp "${ssh_options[@]}" "$SOURCE_ARCHIVE" "$REMOTE:ScratchPadBuild-source.tar.gz"

BUILD_SCRIPT=$(cat <<'POWERSHELL'
$ErrorActionPreference = "Stop"
$workspace = Join-Path $env:USERPROFILE "Coding Projects\\ScratchPadBuild\\source"
$archive = Join-Path $env:USERPROFILE "ScratchPadBuild-source.tar.gz"
New-Item -ItemType Directory -Path $workspace -Force | Out-Null
& tar.exe -xzf $archive -C $workspace
if ($LASTEXITCODE -ne 0) { throw "Could not extract the project source archive." }
Remove-Item -LiteralPath $archive -Force
$buildScript = Join-Path $workspace "Scripts\\build_windows.ps1"
& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $buildScript
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$versionMatch = Select-String -Path (Join-Path $workspace "CMakeLists.txt") -Pattern '^project\(ScratchpadPlugin VERSION ([0-9]+\.[0-9]+\.[0-9]+)'
if (-not $versionMatch) { throw "Could not read project version after the Windows build." }
$version = $versionMatch.Matches[0].Groups[1].Value
$artifacts = Join-Path $workspace "builds\Windows"
$installer = Join-Path $artifacts "ScratchPad_v${version}_Windows.exe"
$plugin = Join-Path $artifacts "nla_scratchpad.vst3"
if (-not (Test-Path -LiteralPath $plugin -PathType Container) -or
    -not (Test-Path -LiteralPath $installer -PathType Leaf)) {
    throw "Expected Windows plugin and installer were not produced."
}
$transfer = Join-Path $env:USERPROFILE "__REMOTE_TRANSFER__"
if (Test-Path -LiteralPath $transfer) {
    throw "Temporary artifact transfer path already exists: $transfer"
}
New-Item -ItemType Directory -Path $transfer | Out-Null
Copy-Item -LiteralPath $plugin -Destination $transfer -Recurse
Copy-Item -LiteralPath $installer -Destination $transfer
Write-Output "Windows artifacts staged for transfer."
POWERSHELL
)
BUILD_SCRIPT="${BUILD_SCRIPT//__REMOTE_TRANSFER__/$REMOTE_TRANSFER}"
BUILD_ENCODED="$(encode_powershell "$BUILD_SCRIPT")"
REMOTE_TRANSFER_CREATED=1
ssh "${ssh_options[@]}" "$REMOTE" powershell.exe -NoProfile -EncodedCommand "$BUILD_ENCODED"

VERSION=$(sed -nE 's/^project\(ScratchpadPlugin VERSION ([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' "$PROJECT_ROOT/CMakeLists.txt")
if [[ -z "$VERSION" ]]; then
    echo "Error: Could not read the project version from CMakeLists.txt." >&2
    exit 1
fi

INSTALLER="$BUILDS_DIR/ScratchPad_v${VERSION}_Windows.exe"

if [[ -d "$BUILDS_DIR/nla_scratchpad.vst3" ]]; then
    rm -rf "$BUILDS_DIR/nla_scratchpad.vst3"
fi
scp "${ssh_options[@]}" \
    -r "$REMOTE:$REMOTE_TRANSFER/nla_scratchpad.vst3" \
    "$REMOTE:$REMOTE_TRANSFER/ScratchPad_v${VERSION}_Windows.exe" \
    "$BUILDS_DIR/"

if [[ ! -d "$BUILDS_DIR/nla_scratchpad.vst3" || ! -f "$INSTALLER" ]]; then
    echo "Error: Expected Windows artifacts were not retrieved." >&2
    exit 1
fi

echo "Windows artifacts built in the Parallels VM:"
echo "  VST3:      $BUILDS_DIR/nla_scratchpad.vst3"
echo "  Installer: $INSTALLER"
