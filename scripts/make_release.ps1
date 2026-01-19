#!/usr/bin/env pwsh

# Exit on error
$ErrorActionPreference = "Stop"

Write-Host "Building release-tek binaries..."
cmake --preset=release-tek
cmake --build build --preset=release-tek-build -j 5

# Get the project root directory (parent of scripts/)
$PROJECT_ROOT = Split-Path -Parent $PSScriptRoot
Set-Location $PROJECT_ROOT

# Create release package name with timestamp
$RELEASE_NAME = "rtype-release-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
$RELEASE_DIR = "releases\$RELEASE_NAME"

Write-Host "Creating release directory: $RELEASE_DIR"
New-Item -Path $RELEASE_DIR -ItemType Directory -Force | Out-Null

# Copy binaries
Write-Host "Copying binaries..."
if (Test-Path "r-type_client.exe") {
    Copy-Item "r-type_client.exe" "$RELEASE_DIR\"
} elseif (Test-Path "r-type_client") {
    Copy-Item "r-type_client" "$RELEASE_DIR\"
} else {
    Write-Host "Warning: r-type_client not found" -ForegroundColor Yellow
}

if (Test-Path "r-type_server.exe") {
    Copy-Item "r-type_server.exe" "$RELEASE_DIR\"
} elseif (Test-Path "r-type_server") {
    Copy-Item "r-type_server" "$RELEASE_DIR\"
} else {
    Write-Host "Warning: r-type_server not found" -ForegroundColor Yellow
}

# Copy plugin_release folder
Write-Host "Copying plugins..."
if (Test-Path "plugin_release" -PathType Container) {
    Copy-Item -Path "plugin_release" -Destination "$RELEASE_DIR\" -Recurse
    
    # Copy DLL dependencies to the root directory for Windows DLL loading
    Write-Host "Copying DLL dependencies to executable directory..."
    Get-ChildItem -Path "plugin_release" -Filter "*.dll" | ForEach-Object {
        Copy-Item $_.FullName -Destination "$RELEASE_DIR\" -Force
    }
} else {
    Write-Host "Warning: plugin_release directory not found" -ForegroundColor Yellow
}

# Copy config directories
Write-Host "Copying configurations..."
if (Test-Path "server_config" -PathType Container) {
    Copy-Item -Path "server_config" -Destination "$RELEASE_DIR\" -Recurse
} else {
    Write-Host "Warning: server_config directory not found" -ForegroundColor Yellow
}

if (Test-Path "game_config" -PathType Container) {
    Copy-Item -Path "game_config" -Destination "$RELEASE_DIR\" -Recurse
} else {
    Write-Host "Warning: game_config directory not found" -ForegroundColor Yellow
}

if (Test-Path "client_config" -PathType Container) {
    Copy-Item -Path "client_config" -Destination "$RELEASE_DIR\" -Recurse
} else {
    Write-Host "Warning: client_config directory not found" -ForegroundColor Yellow
}

# Copy assets if they exist
if (Test-Path "assets" -PathType Container) {
    Write-Host "Copying assets..."
    Copy-Item -Path "assets" -Destination "$RELEASE_DIR\" -Recurse
}

# Create the zip archive
Write-Host "Creating zip archive..."
Set-Location "releases"
Compress-Archive -Path $RELEASE_NAME -DestinationPath "${RELEASE_NAME}.zip" -Force

Write-Host ""
Write-Host "========================================="
Write-Host "Release package created successfully!"
Write-Host "Location: releases\${RELEASE_NAME}.zip"
Write-Host "========================================="
Write-Host ""
Write-Host "Contents:"
Write-Host "  - r-type_client (executable)"
Write-Host "  - r-type_server (executable)"
Write-Host "  - plugin_release/ (all plugins)"
Write-Host "  - server_config/ (server configuration)"
Write-Host "  - game_config/ (game configuration)"
Write-Host "  - client_config/ (client configuration)"
if (Test-Path "..\assets" -PathType Container) {
    Write-Host "  - assets/ (game assets)"
}
Write-Host ""

# Clean up the unzipped directory
Remove-Item -Path $RELEASE_NAME -Recurse -Force

Write-Host "Cleaned up temporary directory"
Write-Host "Done!"
