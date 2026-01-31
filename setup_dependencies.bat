@echo off
REM Setup script for function-plotter dependencies (Windows)

echo Setting up function-plotter dependencies...

REM Create external directory if it doesn't exist
if not exist external mkdir external
cd external

REM Download ImGui
if not exist imgui (
    echo Downloading ImGui...
    git clone https://github.com/ocornut/imgui.git
    echo ImGui downloaded
) else (
    echo ImGui already exists
)

REM Download GLFW
if not exist glfw (
    echo Downloading GLFW...
    git clone https://github.com/glfw/glfw.git
    echo GLFW downloaded
) else (
    echo GLFW already exists
)

REM Download ExprTk
if not exist exprtk (
    echo Downloading ExprTk...
    git clone https://github.com/ArashPartow/exprtk.git
    echo ExprTk downloaded
) else (
    echo ExprTk already exists
)

cd ..

echo.
echo All dependencies downloaded successfully!
echo.
echo Next steps:
echo 1. mkdir build
echo 2. cd build
echo 3. cmake ..
echo 4. cmake --build . --config Release
echo.
pause
