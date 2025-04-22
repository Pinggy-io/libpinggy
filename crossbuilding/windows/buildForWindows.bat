@echo on

setlocal enabledelayedexpansion

REM Check if MY_ARRAY is set, else use default values
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

if "%~1"=="" (
    set opensslDir=""
) else (
    set opensslDir=%~1
)

if "%~2"=="" (
    set releaseDir=releases
) else (
    set releaseDir=%~2
)

if "%~3"=="" (
    set buildDir=build
) else (
    set buildDir=%~3
)

set PROJECT_ROOT=%cd%

echo %PROJECT_ROOT%

rmdir /s /q "%PROJECT_ROOT%\%buildDir%"
rmdir /s /q "%PROJECT_ROOT%\%releaseDir%"

for %%i in (%NATIVE_ARCHES%) do (
    for %%j in (%NATIVE_RUNTIMES%) do (
    cmd /c call "%~dp0buildForSingleArch.bat" %%i %%j "%opensslDir%" "%buildDir%" "%releaseDir%"
        if errorlevel 1 (
            echo Failed while compiling
            exit /b 1
        )
    )
)
