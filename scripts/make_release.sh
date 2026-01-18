#!/bin/bash

# Exit on error
set -e

echo "Building release-tek binaries..."
cmake --preset=release-tek
cmake --build build --preset=release-tek-build -j 5

# Get the project root directory (parent of scripts/)
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# Create release package name with timestamp
RELEASE_NAME="rtype-release-$(date +%Y%m%d-%H%M%S)"
RELEASE_DIR="releases/$RELEASE_NAME"

echo "Creating release directory: $RELEASE_DIR"
mkdir -p "$RELEASE_DIR"

# Copy binaries
echo "Copying binaries..."
if [ -f "r-type_client" ]; then
    cp "r-type_client" "$RELEASE_DIR/"
else
    echo "Warning: r-type_client not found"
fi

if [ -f "r-type_server" ]; then
    cp "r-type_server" "$RELEASE_DIR/"
else
    echo "Warning: r-type_server not found"
fi

# Copy plugin_release folder
echo "Copying plugins..."
if [ -d "plugin_release" ]; then
    cp -r "plugin_release" "$RELEASE_DIR/"
else
    echo "Warning: plugin_release directory not found"
fi

# Copy config directories
echo "Copying configurations..."
if [ -d "server_config" ]; then
    cp -r "server_config" "$RELEASE_DIR/"
else
    echo "Warning: server_config directory not found"
fi

if [ -d "game_config" ]; then
    cp -r "game_config" "$RELEASE_DIR/"
else
    echo "Warning: game_config directory not found"
fi

if [ -d "client_config" ]; then
    cp -r "client_config" "$RELEASE_DIR/"
else
    echo "Warning: client_config directory not found"
fi

# Copy assets if they exist
if [ -d "assets" ]; then
    echo "Copying assets..."
    cp -r "assets" "$RELEASE_DIR/"
fi

# Create the zip archive
echo "Creating zip archive..."
cd "releases"
zip -r "${RELEASE_NAME}.zip" "$RELEASE_NAME"

echo ""
echo "========================================="
echo "Release package created successfully!"
echo "Location: releases/${RELEASE_NAME}.zip"
echo "========================================="
echo ""
echo "Contents:"
echo "  - r-type_client (executable)"
echo "  - r-type_server (executable)"
echo "  - plugin_release/ (all plugins)"
echo "  - server_config/ (server configuration)"
echo "  - game_config/ (game configuration)"
echo "  - client_config/ (client configuration)"
if [ -d "../assets" ]; then
    echo "  - assets/ (game assets)"
fi
echo ""

# Clean up the unzipped directory
rm -rf "$RELEASE_NAME"

echo "Cleaned up temporary directory"
echo "Done!"
