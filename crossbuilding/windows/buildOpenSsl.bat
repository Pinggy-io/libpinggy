@echo off

set CURDIR=%cd%
set PROJECT_ROOT=%USERPROFILE%\pinggy
cd "%PROJECT_ROOT%"

REM Set the OpenSSL version and download URL
set OPENSSL_VERSION=3.3.1
set OPENSSL_URL=https://www.openssl.org/source/openssl-%OPENSSL_VERSION%.tar.gz

REM Set download and extraction paths
set DOWNLOAD_DIR=%PROJECT_ROOT%
set OPENSSL_SOURCE_DIR=%PROJECT_ROOT%\openssl-%OPENSSL_VERSION%

REM Create the necessary directories
if not exist "%DOWNLOAD_DIR%" mkdir "%DOWNLOAD_DIR%"

if not exist "%DOWNLOAD_DIR%\openssl-%OPENSSL_VERSION%.tar.gz" (
    REM Download OpenSSL tar.gz file
    echo Downloading OpenSSL version %OPENSSL_VERSION%...
    curl -L -o "%DOWNLOAD_DIR%\openssl-%OPENSSL_VERSION%.tar.gz" %OPENSSL_URL%
    if errorlevel 1 (
        echo Failed to download OpenSSL. Please check your internet connection or the URL.
        exit /b 1
    )

    REM Verify the file was downloaded
    if not exist "%DOWNLOAD_DIR%\openssl-%OPENSSL_VERSION%.tar.gz" (
        echo Download failed or file not found!
        exit /b 1
    )

    echo Download successful.
)

if exist "%OPENSSL_SOURCE_DIR%" (
    rmdir /s /q "%OPENSSL_SOURCE_DIR%"
)

REM Check if NATIVE_RUNTIMES is set, else use default values
if "%NATIVE_RUNTIMES%"=="" (
    set "NATIVE_RUNTIMES=MT,MTd,MDd,MD"
    echo No environment variable passed. Using default array: %NATIVE_RUNTIMES%
) else (
    echo Environment variable found: %NATIVE_RUNTIMES%
)

if "%NATIVE_ARCHES%"=="" (
    set "NATIVE_ARCHES=x86_64,i686,aarch64,armv7"
    echo No environment variable passed. Using default array: %NATIVE_RUNTIMES%
) else (
    echo Environment variable found: %NATIVE_ARCHES%
)


for %%i in (%NATIVE_ARCHES%) do (
    for %%j in (%NATIVE_RUNTIMES%) do (

        cd "%PROJECT_ROOT%"

        rmdir /s /q "%OPENSSL_SOURCE_DIR%"
        if not exist "%OPENSSL_SOURCE_DIR%" mkdir "%OPENSSL_SOURCE_DIR%"
        tar -xzf "%DOWNLOAD_DIR%\openssl-%OPENSSL_VERSION%.tar.gz" -C "%OPENSSL_SOURCE_DIR%" --strip-components=1
        if errorlevel 1 (
            echo Extraction failed. Please ensure tar is available in your PATH.
            exit /b 1
        )

        cmd /c call "%~dp0buildIndividualSsl.bat" %%i %%j "%OPENSSL_SOURCE_DIR%"
        if errorlevel 1 (
            echo Failed while compiling
            exit /b 1
        )
    )
)

:end

cd "%CURDIR%"
exit /b 0
