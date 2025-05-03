@echo on

set ARCH=%~1
set VCRUNTIME=%~2

if "%~3"=="" (
    set opensslDir="%HOMEDRIVE%\OpenSSL"
) else (
    set opensslDir=%~3
)

if "%~4"=="" (
    set buildDir=build
) else (
    set buildDir=%~4
)

if "%~5"=="" (
    set releaseDir=releases
) else (
    set releaseDir=%~5
)

if "%ARCH%"=="i686" (
    set CMAKE_ARCH=Win32
) else if "%ARCH%"=="x86_64" (
    set CMAKE_ARCH=x64
) else if "%ARCH%"=="armv7" (
    set CMAKE_ARCH=ARM
) else if "%ARCH%"=="aarch64" (
    set CMAKE_ARCH=ARM64
) else (
    echo Invalid architecture specified. %ARCH%
    exit /b 1
)


@REM if "%VCRUNTIME%"=="MT" (
@REM     set "RTFLAG=MultiThreaded"
@REM ) else if "%VCRUNTIME%"=="MTd" (
@REM     set "RTFLAG=MultiThreadedDebug"
@REM ) else if "%VCRUNTIME%"=="MD" (
@REM     set "RTFLAG=MultiThreadedDLL"
@REM ) else if "%VCRUNTIME%"=="MDd" (
@REM     set "RTFLAG=MultiThreadedDebugDLL"
@REM ) else (
@REM     echo Invalid runtime specified. %VCRUNTIME%
@REM     exit /b 1
@REM )

set PROJECT_ROOT=%cd%
set OPENSSL_ROOT_PATH="%opensslDir%\%ARCH%\%VCRUNTIME%"




set BUILD_PATH=%PROJECT_ROOT%\%buildDir%\%ARCH%\%VCRUNTIME%

set RELEASE_PATH=%PROJECT_ROOT%\%releaseDir%\windows\%ARCH%\%VCRUNTIME%
set RELEASE_HEADER_PATH=%PROJECT_ROOT%\%releaseDir%
set RELEASE_ARCHIVE_PATH=%PROJECT_ROOT%\%releaseDir%
set GENERATOR=Visual Studio 17 2022
mkdir "%RELEASE_PATH%"



if "%CMAKE_EXE%"=="" (
  set CMAKE_EXE=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
)

if "%WINDOWS_SDK_VERSION%"=="" (
    set WINDOWS_SDK_VERSION=10.0.19041.0
)

"%CMAKE_EXE%" ^
    "-S%PROJECT_ROOT%" ^
    "-B%BUILD_PATH%\pinggy" ^
    "-G%GENERATOR%" ^
    -A "%CMAKE_ARCH%,version=%WINDOWS_SDK_VERSION%" ^
    -DPINGGY_BUILD_ARCH=%ARCH% ^
    -DOPENSSL_ROOT_DIR=%OPENSSL_ROOT_PATH% ^
    -DCMAKE_BUILD_SERVER=no ^
    -DPINGGY_RELEASE_DIR="%RELEASE_PATH%" ^
    -DPINGGY_HEADER_RELEASE_DIR="%RELEASE_HEADER_PATH%" ^
    -DPINGGY_ARCHIVE_RELEASE_DIR="%RELEASE_ARCHIVE_PATH%" ^
    -DCMAKE_INSTALL_PREFIX="%RELEASE_PATH%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DPINGGY_MSVC_RT="%VCRUNTIME%"

    @REM -DCMAKE_MSVC_RUNTIME_LIBRARY=%RTFLAG%

if errorlevel 1 (
    echo configuration failed
    exit /b 1
)

"%CMAKE_EXE%" --build "%BUILD_PATH%\pinggy" --config Release --parallel

if errorlevel 1 (
    echo build failed
    exit /b 1
)

"%CMAKE_EXE%" --build "%BUILD_PATH%\pinggy" --target distribute

if errorlevel 1 (
    echo build failed
    exit /b 1
)



if defined RELEASE_SO (
    "%CMAKE_EXE%" --build "%BUILD_PATH%/pinggy" --target releaselib
    if errorlevel 1 (
        echo build failed
        exit /b 1
    )
)

if defined RELEASE_SSL (
  "%CMAKE_EXE%" --build "%BUILD_PATH%/pinggy" --target releasessl
    if errorlevel 1 (
        echo build failed
        exit /b 1
    )
)
