#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Check if Qt is installed
if command -v qmake &> /dev/null; then
    echo "Qt development tools found, attempting to build GUI version"
    cmake -DUSE_QT_GUI=ON ..

    # Build the application
    make

    echo "Build complete."
    echo "Run the main application with: ./system_info"
    echo "Run the simple menu example with: ./simple_menu"
else
    echo "Qt development tools not found, building console version only"
    echo "To install Qt5 on Ubuntu/Debian: sudo apt-get install qtbase5-dev"
    echo "On Fedora: sudo dnf install qt5-qtbase-devel"
    echo "On macOS with Homebrew: brew install qt5"
    cmake -DUSE_QT_GUI=OFF ..

    # Build the application
    make

    echo "Build complete. Run with: ./system_info"
fi