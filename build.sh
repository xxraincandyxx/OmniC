#!/bin/bash

# --- Script Configuration ---
# Exit immediately if a command exits with a non-zero status.
set -e
# Treat unset variables as an error.
set -u

# --- 1. Environment Sanity Check ---
# Ensure the script is being run inside an active Conda environment.
if [[ -z "${CONDA_PREFIX-}" ]]; then
  echo "ERROR: No Conda environment is active."
  echo "Please activate your omnic-dev environment with 'conda activate omnic-dev' and try again."
  exit 1
fi

echo "--- Using Conda environment at: $CONDA_PREFIX"

# --- Cleaning the Build Directory ---
# The most reliable way to clean is to remove the directory entirely.

# echo "--- Cleaning previous build..."
# rm -rf build
# mkdir -p build

# Clean Cache to avoid unexpected behavior
make clean

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
  mkdir build
fi

# --- Configuring the Project with CMake ---
# Let CMake detect the compilers from the Conda environment.
# Hardcoding /usr/bin/gcc defeats the purpose of using Conda for your toolchain.
echo "--- Configuring project with CMake..."
cmake -S . -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH="$CONDA_PREFIX"
      # -DCMAKE_C_COMPILER=/usr/bin/gcc \
      # -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
      # -DCUDAHOSTCXX=/usr/bin/g++

# --- Building the Library and Examples ---
# Detect number of CPU cores for parallel build.
# CORES=$(getconf _NPROCESSORS_ONLN 2>/dev/null || getconf NPROCESSORS_ONLN 2>/dev/null || echo 1)
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  CORES=$(nproc)
elif [[ "$OSTYPE" == "darwin"* ]]; then
  CORES=$(sysctl -n hw.ncpu)
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
  CORES=$NUMBER_OF_PROCESSORS
else
  CORES=1  # Fallback
fi
echo "--- Building project with $CORES cores..."
cmake --build build --parallel "$CORES"

# --- Generating Documentation ---
# Use the modern cmake command to build a specific target.
# This works even if the generator isn't 'make' (e.g., Ninja).
echo "--- Generating documentation..."
cmake --build build --target doc

# --- Success ---
echo ""
echo "✅ Build and documentation generation complete!"
echo "   - Library is in:      build/lib/"
echo "   - Examples are in:    build/bin/"
echo "   - Docs are in:        build/docs/html/index.html"
