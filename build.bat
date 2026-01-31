@echo off
REM Build script for Windows

echo Building function-plotter...

REM Create build directory
if not exist build mkdir build
cd build

REM Run CMake
echo Configuring with CMake...
cmake ..

if errorlevel 1 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build
echo Building...
cmake --build . --config Release

if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Build successful!
echo.
echo Run the application with:
echo   .\build\bin\Release\function-plotter.exe
echo.
pause
