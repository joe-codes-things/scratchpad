#!/bin/bash
# Build and stage ScratchPad plugins for macOS and the Parallels Windows VM.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_TYPE="${1:-production}"

if [[ "$#" -gt 1 || ( "$BUILD_TYPE" != "development" && "$BUILD_TYPE" != "production" ) ]]; then
    echo "Usage: $0 [development|production]" >&2
    exit 2
fi

VERSION=$(sed -nE 's/^project\(ScratchpadPlugin VERSION ([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' "$SCRIPT_DIR/../CMakeLists.txt")
if [[ -z "$VERSION" ]]; then
    echo "Error: Could not read the project version from CMakeLists.txt." >&2
    exit 1
fi

for platform in macOS Windows; do
    output_dir="$SCRIPT_DIR/../exports/${BUILD_TYPE}-builds/$platform/v${VERSION}"
    if [[ -e "$output_dir" ]]; then
        echo "Error: Output already exists; refusing to overwrite: $output_dir" >&2
        exit 1
    fi
done

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "Error: This build entry point requires macOS with Xcode." >&2
    exit 1
fi

# Verify the VM is available before spending time building either platform.
"$SCRIPT_DIR/build_windows.sh" --check-only "$BUILD_TYPE"
"$SCRIPT_DIR/build_macos.sh" "$BUILD_TYPE"
"$SCRIPT_DIR/build_windows.sh" "$BUILD_TYPE"
