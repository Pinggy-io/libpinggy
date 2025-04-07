@echo off
setlocal enabledelayedexpansion

echo ===================================
echo NASM (Netwide Assembler) Installer
echo ===================================
echo This script will automatically download and install NASM 64-bit
echo.

:: Set variables - no prompts
set "NASM_VERSION=2.16.01"
set "INSTALL_DIR=C:\NASM"
set "TEMP_DIR=%TEMP%\nasm_installer"
set "ARCH=win64"

:: Check for admin privileges
NET SESSION >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script requires administrator privileges.
    echo Please right-click and select "Run as administrator".
    pause
    exit /b 1
)

:: Create temp folder for the installer
if not exist "%TEMP_DIR%" mkdir "%TEMP_DIR%"

echo Downloading NASM %NASM_VERSION% 64-bit...

:: Download NASM
set "DOWNLOAD_URL=https://www.nasm.us/pub/nasm/releasebuilds/%NASM_VERSION%/win64/nasm-%NASM_VERSION%-win64.zip"
powershell -Command "& {Invoke-WebRequest -Uri '%DOWNLOAD_URL%' -OutFile '%TEMP_DIR%\nasm.zip'}"

if not exist "%TEMP_DIR%\nasm.zip" (
    echo ERROR: Failed to download NASM.
    echo Please check your internet connection and try again.
    pause
    exit /b 1
)

echo Extracting NASM to %INSTALL_DIR%...

:: Create installation directory if it doesn't exist
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

:: Extract NASM
powershell -Command "& {Expand-Archive -Path '%TEMP_DIR%\nasm.zip' -DestinationPath '%TEMP_DIR%' -Force}"

:: Move files to installation directory
xcopy /E /I /Y "%TEMP_DIR%\nasm-%NASM_VERSION%\*" "%INSTALL_DIR%"

:: Update PATH environment variable
echo Adding NASM to PATH environment variable...
setx PATH "%PATH%;%INSTALL_DIR%" /M

:: Create documentation directory
if not exist "%INSTALL_DIR%\doc" mkdir "%INSTALL_DIR%\doc"

:: Download NASM documentation
echo Downloading NASM documentation...
powershell -Command "& {Invoke-WebRequest -Uri 'https://www.nasm.us/pub/nasm/releasebuilds/%NASM_VERSION%/doc/nasmdoc.pdf' -OutFile '%INSTALL_DIR%\doc\nasmdoc.pdf'}"

echo.
echo Installation completed!
echo NASM 64-bit has been installed to: %INSTALL_DIR%
echo.
echo To verify your installation, open a new command prompt and type:
echo   nasm -v
echo.
echo You may need to restart your computer for PATH changes to take effect.

:: Cleanup
rmdir /s /q "%TEMP_DIR%"

pause
exit /b 0