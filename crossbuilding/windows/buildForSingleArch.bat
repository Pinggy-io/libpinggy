@echo off

set ARCH=%~1

if "%~3"=="" (
    set buildDir=build
) else (
    set buildDir=%~3
)

if "%~2"=="" (
    set releaseDir=releases
) else (
    set releaseDir=%~2
)

if "%~4"=="" (
    set opensslDir="%HOMEDRIVE%\OpenSSL"
) else (
    set opensslDir=%~4
)

if "%ARCH%"=="i686" (
    set VAR_TYPE=x86
    set CMAKE_ARCH=Win32
) else if "%ARCH%"=="x86_64" (
    set VAR_TYPE=x64
    set CMAKE_ARCH=x64
) else if "%ARCH%"=="armv7" (
    set VAR_TYPE=x64_arm
    set CMAKE_ARCH=ARM
) else if "%ARCH%"=="aarch64" (
    set VAR_TYPE=x64_arm64
    set CMAKE_ARCH=ARM64
) else (
    echo Invalid architecture specified. %ARCH%
    exit /b 1
)

set PROJECT_ROOT=%cd%
set OPENSSL_ROOT_PATH="%opensslDir%\%ARCH%"




set BUILD_PATH=%PROJECT_ROOT%\%buildDir%\%ARCH%

set RELEASE_PATH=%PROJECT_ROOT%\%releaseDir%\windows\%ARCH%
set RELEASE_HEADER_PATH=%PROJECT_ROOT%\%releaseDir%
set GENERATOR=Visual Studio 17 2022
mkdir "%RELEASE_PATH%"

set CMAKE_EXE=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe

"%CMAKE_EXE%" ^
    "-S%PROJECT_ROOT%" ^
    "-B%BUILD_PATH%/pinggy" ^
    "-G%GENERATOR%" ^
    -A %CMAKE_ARCH% ^
    -DOPENSSL_ROOT_DIR="%OPENSSL_ROOT_PATH%" ^
    -DCMAKE_BUILD_SERVER=no ^
    -DPINGGY_RELEASE_DIR="%RELEASE_PATH%" ^
    -DPINGGY_HEADER_RELEASE_DIR="%RELEASE_HEADER_PATH%" ^
    -DCMAKE_INSTALL_PREFIX="%RELEASE_PATH%" ^
    -DCMAKE_BUILD_TYPE=Release

if errorlevel 1 (
    echo configuration failed
    exit /b 1
)

"%CMAKE_EXE%" --build "%BUILD_PATH%/pinggy" --config Release

if errorlevel 1 (
    echo build failed
    exit /b 1
)
