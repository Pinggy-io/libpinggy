
@REM This script installs visual studio
@echo on
setlocal enabledelayedexpansion

echo =================================================
echo Visual Studio Installer for C++ Development
echo =================================================
echo This script will download and install Visual Studio with C++ components
echo for all major architectures (x86, x64, ARM, ARM64)
echo.

:: Check for admin privileges
NET SESSION >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script requires administrator privileges.
    echo Please right-click and select "Run as administrator".
    pause
    exit /b 1
)

:: Set installation directory (default or custom)
set "INSTALL_DIR=C:\Program Files\Microsoft Visual Studio\2022\Community"
set /p "CUSTOM_DIR=Enter installation directory or press Enter for default [%INSTALL_DIR%]: "
if not "!CUSTOM_DIR!"=="" set "INSTALL_DIR=!CUSTOM_DIR!"

:: Create temp folder for the installer
set "TEMP_DIR=%TEMP%\vs_installer"
if not exist "%TEMP_DIR%" mkdir "%TEMP_DIR%"

echo.
echo Downloading Visual Studio Installer...
echo.

:: Download VS bootstrapper
powershell -Command "& {Invoke-WebRequest -Uri 'https://aka.ms/vs/17/release/vs_community.exe' -OutFile '%TEMP_DIR%\vs_community.exe'}"

if not exist "%TEMP_DIR%\vs_community.exe" (
    echo ERROR: Failed to download Visual Studio installer.
    echo Please check your internet connection and try again.
    pause
    exit /b 1
)

echo.
echo Starting Visual Studio installation with C++ components...
echo.

:: Install VS with C++ components for all architectures
"%TEMP_DIR%\vs_community.exe" ^
    --installPath "%INSTALL_DIR%" ^
    --add Microsoft.VisualStudio.Workload.NativeDesktop ^
    --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ^
    --add Microsoft.VisualStudio.Component.VC.Tools.ARM ^
    --add Microsoft.VisualStudio.Component.VC.Tools.ARM64 ^
    --add Microsoft.VisualStudio.Component.VC.ATL ^
    --add Microsoft.VisualStudio.Component.VC.ATL.ARM ^
    --add Microsoft.VisualStudio.Component.VC.ATL.ARM64 ^
    --add Microsoft.VisualStudio.Component.Windows10SDK.19041 ^
    --add Microsoft.VisualStudio.Component.VC.CMake.Project ^
    --add Microsoft.VisualStudio.Component.TestTools.BuildTools ^
    --add Microsoft.VisualStudio.Component.VC.ASAN ^
    --includeRecommended ^
    --quiet --norestart --wait

echo.
echo Installation completed!
echo.
echo If the installation was successful, you can now build C++ projects for:
echo - x86 (32-bit)
echo - x64 (64-bit)
echo - ARM
echo - ARM64
echo.
echo Visual Studio is installed at: %INSTALL_DIR%
echo.

:: Cleanup
rmdir /s /q "%TEMP_DIR%"

pause
exit /b 0