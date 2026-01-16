#!/bin/bash
# R-Type Windows Setup Script for Git Bash
# Run this in Git Bash (as Administrator if possible)

echo "====================================="
echo "R-Type Windows Build Setup"
echo "====================================="
echo ""

# Step 1: Install Visual Studio Build Tools
echo "[1/4] Downloading Visual Studio Build Tools..."
VS_INSTALLER="/c/Users/fromt/AppData/Local/Temp/vs_buildtools.exe"
curl -L "https://aka.ms/vs/17/release/vs_buildtools.exe" -o "$VS_INSTALLER"

echo "[1/4] Installing Visual Studio Build Tools..."
echo "       This will take 5-15 minutes. Please wait..."
"$VS_INSTALLER" --quiet --wait --norestart --nocache \
    --add Microsoft.VisualStudio.Workload.VCTools \
    --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 \
    --add Microsoft.VisualStudio.Component.Windows11SDK.22621 \
    --includeRecommended
echo "[1/4] Visual Studio Build Tools installed!"
echo ""

# Step 2: Install CMake
echo "[2/4] Downloading CMake..."
CMAKE_INSTALLER="/c/Users/fromt/AppData/Local/Temp/cmake-installer.msi"
curl -L "https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-windows-x86_64.msi" -o "$CMAKE_INSTALLER"

echo "[2/4] Installing CMake..."
msiexec.exe //i "$CMAKE_INSTALLER" //quiet //norestart ADD_CMAKE_TO_PATH=System
echo "[2/4] CMake installed!"
echo ""

# Step 3: Setup vcpkg
echo "[3/4] Setting up vcpkg..."
cd /c/Users/fromt/R-Type

if [ -d "vcpkg" ]; then
    echo "       Removing old vcpkg installation..."
    rm -rf vcpkg
fi

echo "       Cloning vcpkg repository..."
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

echo "       Bootstrapping vcpkg..."
./bootstrap-vcpkg.bat

echo "[3/4] vcpkg installed!"
echo ""

# Step 4: Set environment variables
echo "[4/4] Setting VCPKG_ROOT environment variable..."
setx VCPKG_ROOT "C:\\Users\\fromt\\R-Type\\vcpkg"
export VCPKG_ROOT="/c/Users/fromt/R-Type/vcpkg"
echo "[4/4] Environment variable set!"
echo ""

# Complete
echo "====================================="
echo "Installation Complete!"
echo "====================================="
echo ""
echo "NEXT STEPS:"
echo "1. Close this Git Bash window"
echo "2. Open a NEW Git Bash window"
echo "3. Navigate to: cd /c/Users/fromt/R-Type"
echo "4. Run: ./build-windows.sh"
echo ""
echo "The build script will be ready for you!"
echo ""
