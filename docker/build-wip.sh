#!/usr/bin/env sh
set -eu

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REGISTRY="registry.rosetta.ericssondevops.com/ewasjon/supl-3gpp-lpp-client"

PLATFORM="${1:-}"
if [ -z "$PLATFORM" ]; then
    echo "Usage: $0 <platform>"
    echo ""
    echo "Available platforms:"
    echo "  x86_64-unknown-linux-gnu     (your laptop)"
    echo "  aarch64-rpi4-linux-gnu       (Raspberry Pi 4)"
    echo "  aarch64-unknown-linux-gnu    (generic ARM64)"
    echo "  armv8-rpi3-linux-gnueabihf   (Raspberry Pi 3)"
    echo "  arm-cortex_a8-linux-gnueabi  (Cortex-A8)"
    echo "  armv6-unknown-linux-gnueabihf"
    exit 1
fi

# Use branch name as image tag
BRANCH="$(git -C "$SCRIPT_DIR/.." rev-parse --abbrev-ref HEAD 2>/dev/null || echo "")"
if [ -z "$BRANCH" ]; then
    echo "ERROR: Could not determine current git branch." >&2
    exit 1
fi

if [ "$BRANCH" = "main" ] || [ "$BRANCH" = "master" ]; then
    echo "ERROR: Do not build WIP images from '$BRANCH'. Switch to your project branch first." >&2
    echo "  git checkout 2026-summer-yourproject" >&2
    exit 1
fi

# Fail fast if builder image is not available locally and cannot be pulled
BUILDER_IMAGE="s3lc-builder:${PLATFORM}"
if ! docker image inspect "$BUILDER_IMAGE" > /dev/null 2>&1; then
    echo "Builder image '$BUILDER_IMAGE' not found locally. Trying to pull..."
    REMOTE_BUILDER="${REGISTRY}/builder:${PLATFORM}"
    if ! docker pull "$REMOTE_BUILDER" 2>/dev/null; then
        echo ""
        echo "ERROR: Builder image for '$PLATFORM' is not available locally or in the registry." >&2
        echo "Contact your supervisor to get the builder images set up." >&2
        exit 1
    fi
    docker tag "$REMOTE_BUILDER" "$BUILDER_IMAGE"
fi

echo "Building for platform: $PLATFORM"
echo "Image tag:             $BRANCH"
echo "Registry:              $REGISTRY"
echo ""

python3 "$SCRIPT_DIR/build-images.py" \
    --platform "$PLATFORM" \
    --build-mode debug \
    --registry "$REGISTRY" \
    --tag "$BRANCH" \
    --force \
    --push
