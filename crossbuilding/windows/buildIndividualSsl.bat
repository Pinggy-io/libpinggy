@echo on
set ARCH=%~1
set OPENSSL_SOURCE_DIR=%~2
if "%ARCH%"=="" (
    echo Please specify the architecture: x86, x64, arm, or arm64
    exit /b 1
)

if "%ARCH%"=="i686" (
    set CONFIG=VC-WIN32
    set VAR_TYPE=x86
) else if "%ARCH%"=="x86_64" (
    set CONFIG=VC-WIN64A
    set VAR_TYPE=x64
) else if "%ARCH%"=="armv7" (
    set CONFIG=VC-WIN32-ARM
    set VAR_TYPE=x64_arm
) else if "%ARCH%"=="aarch64" (
    set CONFIG=VC-WIN64-ARM
    set VAR_TYPE=x64_arm64
) else (
    echo Invalid architecture specified. %ARCH%
    exit /b 1
)

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" %VAR_TYPE%


if errorlevel 1 (
    echo environment failed
    exit /b 1
)

cd %OPENSSL_SOURCE_DIR%

set RELEASE_DIR=%HOMEDRIVE%\OpenSSL\%ARCH%

perl Configure %CONFIG% ^
        shared no-unit-test no-tests ^
        --prefix="%RELEASE_DIR%" ^
        --openssldir="%RELEASE_DIR%"

if errorlevel 1 (
    echo configuration failed
    exit /b 1
)

nmake
if errorlevel 1 (
    echo faile nmake
    exit /b 1
)

nmake install_sw

if errorlevel 1 (
    echo faile nmake install_sw
    exit /b 1
)

@REM nmake clean
