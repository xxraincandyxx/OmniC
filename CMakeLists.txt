# --- Project Definition ---
cmake_minimum_required(VERSION 3.10)
project(OmniC C)

# --- Configuration ---
# Set the C standard to C11.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# This is the crucial line that tells CMake to generate the file clangd needs.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Organize the build output. Libraries will go into build/lib, executables into
# build/bin.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- Define the OmniC Library ---
# Create a static library target named "omnic".
# NOTE: For larger projects, it's better to list sources explicitly.
# file(GLOB omnic_SOURCES "src/*.c")
# add_library(omnic STATIC ${omnic_SOURCES})
add_library(omnic STATIC
  src/vector.c
  src/binarytree.c
  src/huffmantree.c
)

# Tell CMake that the "omnic" target needs to look for headers in the "include"
# directory. `PUBLIC` means that any target linking against "omnic" will also
# automatically inherit this include directory. This is exactly what we want for
# our example program.
target_include_directories(omnic PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---------------------------------------------------------------------------- #

# --- Define the Vector Example Executable ---
# Add the example program as an executable target.
add_executable(test_vector
  examples/test_vector.c
)

target_link_libraries(test_vector PRIVATE omnic)

# --- Compiler Warnings ---
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(omnic PRIVATE -Wall -O2 -Wextra -Wpedantic)
  target_compile_options(test_vector PRIVATE -Wall -O2 -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------- #

# --- Define the Dynarray Example Executable ---
add_executable(test_dynarray
  examples/test_dynarray.c
)

target_link_libraries(test_dynarray PRIVATE omnic)

# Add compiler warnings
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(test_dynarray PRIVATE -Wall -O2 -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------- #

# --- Define the List Example Executable ---
add_executable(test_list
  examples/test_list.c
)

target_link_libraries(test_list PRIVATE omnic)

# Add compiler warnings
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(test_list PRIVATE -Wall -O2 -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------- #

# --- Define the List-based Polynomial Example Executable ---
add_executable(test_poly
  examples/demo_list_polynomial.c
)

target_link_libraries(test_poly PRIVATE omnic)

# Add compiler warnings
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(test_poly PRIVATE -Wall -O2 -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------- #

# --- Define the Binary Tree Example Executable ---
add_executable(test_btree
  examples/test_binarytree.c
)

target_link_libraries(test_btree PRIVATE omnic)

# Add compiler warnings
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(test_btree PRIVATE -Wall -O2 -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------- #

# --- Define the Huffman Tree Example Executable ---
add_executable(test_htree
  examples/test_huffmantree.c
)

target_link_libraries(test_htree PRIVATE omnic)

# Add compiler warnings
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(test_htree PRIVATE -Wall -O2 -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------- #

# --- Define the Graph Example Executable ---
add_executable(test_graph
  examples/test_graph.c
)

target_link_libraries(test_graph PRIVATE omnic)

# Add compiler warnings
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(test_graph PRIVATE -Wall -O2 -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------- #

# --- Define the Graph Example Executable ---
add_executable(demo_graph
  examples/demo_graph_flights.c
)

target_link_libraries(demo_graph PRIVATE omnic)

# Add compiler warnings
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  target_compile_options(demo_graph PRIVATE -Wall -O2 -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------- #

# Print a message after configuration is done.
message(STATUS "OmniC project configured. Build with 'make' or your chosen generator.")

# --- Doxygen Documentation Target ---

# Find the Doxygen executable and enable the CMake module.
find_package(Doxygen)

if(DOXYGEN_FOUND)
  # Define a custom target named "doc" to generate the documentation.
  # It will run the 'doxygen' command using the Doxyfile in the project's root.
  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen..."
    VERBATIM
  )
  message(STATUS
    "Doxygen found. You can build the 'doc' target to generate documentation.")
else()
  message(WARNING "Doxygen not found. The 'doc' target will not be available.")
endif()
