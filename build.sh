#!/bin/bash

# Clean Cache to avoid unexpected behavior
make clean

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
  mkdir build
fi

# Detect number of CPU cores
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  CORES=$(nproc)
elif [[ "$OSTYPE" == "darwin"* ]]; then
  CORES=$(sysctl -n hw.ncpu)
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
  CORES=$NUMBER_OF_PROCESSORS
else
  CORES=1  # Fallback
fi

echo "Building Method: Test~ 😘"

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
-DCMAKE_C_COMPILER=/usr/bin/gcc \
-DCMAKE_CXX_COMPILER=/usr/bin/g++ \
-DCUDAHOSTCXX=/usr/bin/g++


# Equivalent to >>> make -j for parallel processing
cmake --build build --parallel $CORES
