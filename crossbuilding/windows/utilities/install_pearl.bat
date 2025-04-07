@echo off
setlocal enabledelayedexpansion

echo =======================================
echo Perl Installation Script
echo =======================================
echo This script will download and install Strawberry Perl
echo.

:: Check for admin privileges
NET SESSION >nul 2>&1
if %errorLevel% neq 0 (
    echo WARNING: This script may require administrator privileges.
    echo Consider running as administrator if installation fails.
    echo.
    choice /C YN /M "Continue anyway? (Y/N)"
    if !errorlevel! equ 2 exit /b 1
    echo.
)

:: Set installation directory (default or custom)
set "INSTALL_DIR=C:\Strawberry"
set /p "CUSTOM_DIR=Enter installation directory or press Enter for default [%INSTALL_DIR%]: "
if not "!CUSTOM_DIR!"=="" set "INSTALL_DIR=!CUSTOM_DIR!"

:: Create temp folder for the installer
set "TEMP_DIR=%TEMP%\perl_installer"
if not exist "%TEMP_DIR%" mkdir "%TEMP_DIR%"

:: Set Perl version and architecture
set "PERL_VERSION=5.32.1.1"
set "ARCH=64bit"

echo.
echo Which Perl architecture would you like to install?
echo 1. 64-bit (recommended for modern systems)
echo 2. 32-bit (for older systems or compatibility)
choice /C 12 /M "Select option"
if !errorlevel! equ 2 set "ARCH=32bit"

echo.
echo Downloading Strawberry Perl %PERL_VERSION% (%ARCH%)...
echo.

:: Determine correct download URL based on architecture
if "%ARCH%"=="64bit" (
    set "DOWNLOAD_URL=https://strawberryperl.com/download/%PERL_VERSION%/strawberry-perl-%PERL_VERSION%-64bit.msi"
) else (
    set "DOWNLOAD_URL=https://strawberryperl.com/download/%PERL_VERSION%/strawberry-perl-%PERL_VERSION%-32bit.msi"
)

:: Download Perl installer
powershell -Command "& {Invoke-WebRequest -Uri '%DOWNLOAD_URL%' -OutFile '%TEMP_DIR%\strawberry-perl.msi'}"

if not exist "%TEMP_DIR%\strawberry-perl.msi" (
    echo ERROR: Failed to download Perl installer.
    echo Please check your internet connection and try again.
    pause
    exit /b 1
)

echo.
echo Installing Perl to %INSTALL_DIR%...
echo This may take several minutes. Please wait...
echo.

:: Install Perl
msiexec /i "%TEMP_DIR%\strawberry-perl.msi" INSTALLDIR="%INSTALL_DIR%" /qb

:: Check if installation was successful
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Installation failed with error code %errorlevel%.
    echo Try running this script as administrator.
    pause
    exit /b 1
)

:: Update PATH environment variable
echo.
echo Adding Perl to PATH environment variable...
setx PATH "%PATH%;%INSTALL_DIR%\perl\bin;%INSTALL_DIR%\perl\site\bin;%INSTALL_DIR%\c\bin" /M

echo.
echo Installation completed!
echo.
echo To verify your installation, open a new command prompt and type:
echo   perl -v
echo.
echo You may need to restart your computer for PATH changes to take effect.
echo.

:: Cleanup
rmdir /s /q "%TEMP_DIR%"

pause
exit /b 0