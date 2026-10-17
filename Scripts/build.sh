#!/bin/bash
# Build and stage ScratchPad plugins for macOS and the Parallels Windows VM.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "Error: This build entry point requires macOS with Xcode." >&2
    exit 1
fi

# Verify the VM is available before spending time building either platform.
"$SCRIPT_DIR/build_windows.sh" --check-only
"$SCRIPT_DIR/build_macos.sh"
"$SCRIPT_DIR/build_windows.sh"
