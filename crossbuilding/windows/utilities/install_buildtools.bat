@REM This script installs build tools (visual studio build tools)
@echo off
setlocal

echo Downloading Visual Studio Build Tools...
set INSTALLER_URL=https://aka.ms/vs/17/release/vs_buildtools.exe
set INSTALLER_PATH=%TEMP%\vs_buildtools.exe

powershell -Command "(New-Object Net.WebClient).DownloadFile('%INSTALLER_URL%', '%INSTALLER_PATH%')" >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to download Visual Studio Build Tools installer.
    exit /b 1
)

echo Installing MSBuild and Build Tools silently...
%INSTALLER_PATH% --quiet --norestart --nocache --installPath "C:\BuildTools" ^
    --add Microsoft.VisualStudio.Workload.MSBuildTools >nul 2>&1

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build Tools installation failed.
    exit /b 1
)

echo Cleaning up...
del /f /q %INSTALLER_PATH%

echo Build Tools installed successfully.
endlocal
exit /b 0
