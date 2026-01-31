#!/bin/bash

# Build script for Unix-like systems (macOS, Linux)

echo "Building function-plotter..."

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Run CMake
echo "Configuring with CMake..."
cmake ..

if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed!"
    exit 1
fi

# Build
echo "Building..."
cmake --build . -- -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo ""
echo "✓ Build successful!"
echo ""
echo "Run the application with:"
echo "  ./build/bin/function-plotter"
echo ""
