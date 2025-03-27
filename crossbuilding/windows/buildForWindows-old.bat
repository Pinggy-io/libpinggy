@echo on



REM Define project variables
set PROJECT_ROOT=%cd%
set GENERATOR=Visual Studio 17 2022
set CMAKE_EXE=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe

set BUILD_DIR=%PROJECT_ROOT%\build\win32
"%CMAKE_EXE%" ^
    "-S%PROJECT_ROOT%" ^
    "-B%BUILD_DIR%" ^
    "-G%GENERATOR%" ^
    -A win32 ^
    -DCMAKE_BUILD_TYPE=Release

if %errorlevel% neq 0 (
    echo Error: CMake configuration failed.
    cd %PROJECT_ROOT%
    exit /b %errorlevel%
)
"%CMAKE_EXE%" --build "%BUILD_DIR%"