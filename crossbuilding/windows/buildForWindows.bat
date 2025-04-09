@echo off

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

for %%i in (i686 x86_64 armv7 aarch64) do (
    call "%~dp0buildForSingleArch.bat" %%i "%opensslDir%" "%buildDir%" "%releaseDir%"
    if errorlevel 1 (
        echo Failed while compiling
        exit /b 1
    )
)
