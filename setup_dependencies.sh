#!/bin/bash

# Setup script for function-plotter dependencies

echo "Setting up function-plotter dependencies..."

# Create external directory if it doesn't exist
mkdir -p external
cd external

# Download ImGui
if [ ! -d "imgui" ]; then
    echo "Downloading ImGui..."
    git clone https://github.com/ocornut/imgui.git
    echo "✓ ImGui downloaded"
else
    echo "✓ ImGui already exists"
fi

# Download GLFW
if [ ! -d "glfw" ]; then
    echo "Downloading GLFW..."
    git clone https://github.com/glfw/glfw.git
    echo "✓ GLFW downloaded"
else
    echo "✓ GLFW already exists"
fi

# Download ExprTk
if [ ! -d "exprtk" ]; then
    echo "Downloading ExprTk..."
    git clone https://github.com/ArashPartow/exprtk.git
    echo "✓ ExprTk downloaded"
else
    echo "✓ ExprTk already exists"
fi

cd ..

echo ""
echo "All dependencies downloaded successfully!"
echo ""
echo "Next steps:"
echo "1. mkdir build && cd build"
echo "2. cmake .."
echo "3. cmake --build ."
echo ""
echo "Or use the provided build script for your platform."
