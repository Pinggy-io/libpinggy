@echo on

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

for %%i in (x86_64 i686 aarch64 armv7) do (
    for %%j in (MTd MT MDd MD) do (
    cmd /c call "%~dp0buildForSingleArch.bat" %%i %%j "%opensslDir%" "%buildDir%" "%releaseDir%"
        if errorlevel 1 (
            echo Failed while compiling
            exit /b 1
        )
    )
)
