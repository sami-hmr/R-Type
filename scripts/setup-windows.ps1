# R-Type Windows Setup Script
# Run this in Admin PowerShell: .\setup-windows.ps1

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "R-Type Windows Build Setup" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# Step 1: Install Visual Studio Build Tools
Write-Host "[1/4] Downloading Visual Studio Build Tools..." -ForegroundColor Cyan
$vsInstaller = "$env:TEMP\vs_buildtools.exe"
Invoke-WebRequest -Uri "https://aka.ms/vs/17/release/vs_buildtools.exe" -OutFile $vsInstaller

Write-Host "[1/4] Installing Visual Studio Build Tools..." -ForegroundColor Cyan
Write-Host "       This will take 5-15 minutes. Please wait..." -ForegroundColor Yellow
Start-Process -FilePath $vsInstaller -ArgumentList "--quiet","--wait","--norestart","--nocache","--add","Microsoft.VisualStudio.Workload.VCTools","--add","Microsoft.VisualStudio.Component.VC.Tools.x86.x64","--add","Microsoft.VisualStudio.Component.Windows11SDK.22621","--includeRecommended" -Wait
Write-Host "[1/4] Visual Studio Build Tools installed!" -ForegroundColor Green
Write-Host ""

# Step 2: Install CMake
Write-Host "[2/4] Downloading CMake..." -ForegroundColor Cyan
$cmakeInstaller = "$env:TEMP\cmake-installer.msi"
Invoke-WebRequest -Uri "https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-windows-x86_64.msi" -OutFile $cmakeInstaller

Write-Host "[2/4] Installing CMake..." -ForegroundColor Cyan
Start-Process msiexec.exe -ArgumentList "/i",$cmakeInstaller,"/quiet","/norestart","ADD_CMAKE_TO_PATH=System" -Wait
Write-Host "[2/4] CMake installed!" -ForegroundColor Green
Write-Host ""

# Step 3: Setup vcpkg
Write-Host "[3/4] Setting up vcpkg..." -ForegroundColor Cyan
Set-Location C:\Users\fromt\R-Type

if (Test-Path "vcpkg") {
    Write-Host "       Removing old vcpkg installation..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force vcpkg
}

Write-Host "       Cloning vcpkg repository..." -ForegroundColor Cyan
git clone https://github.com/microsoft/vcpkg.git
Set-Location vcpkg

Write-Host "       Bootstrapping vcpkg..." -ForegroundColor Cyan
.\bootstrap-vcpkg.bat

Write-Host "[3/4] vcpkg installed!" -ForegroundColor Green
Write-Host ""

# Step 4: Set environment variables
Write-Host "[4/4] Setting VCPKG_ROOT environment variable..." -ForegroundColor Cyan
[System.Environment]::SetEnvironmentVariable('VCPKG_ROOT','C:\Users\fromt\R-Type\vcpkg','User')
$env:VCPKG_ROOT = 'C:\Users\fromt\R-Type\vcpkg'
Write-Host "[4/4] Environment variable set!" -ForegroundColor Green
Write-Host ""

# Complete
Write-Host "=====================================" -ForegroundColor Green
Write-Host "Installation Complete!" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Green
Write-Host ""
Write-Host "NEXT STEPS:" -ForegroundColor Yellow
Write-Host "1. Close this PowerShell window" -ForegroundColor White
Write-Host "2. Open a NEW Admin PowerShell" -ForegroundColor White
Write-Host "3. Navigate to: cd C:\Users\fromt\R-Type" -ForegroundColor White
Write-Host "4. Run: .\build-windows.ps1" -ForegroundColor White
Write-Host ""
Write-Host "The build script will be ready for you!" -ForegroundColor Cyan
Write-Host ""
