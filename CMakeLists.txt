cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(clang-models)

# Set C++ Standard
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detect platform & set compiler
if(APPLE)
  message(STATUS "Building on macOS")
  set(CMAKE_CXX_COMPILER clang++)
elseif(UNIX)
  message(STATUS "Building on Linux")
  set(CMAKE_CXX_COMPILER g++)
endif()

# Set debug mode
if (BUILD_TEST OR BUILD_TRAIN)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
  # For logging
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -g")
else()
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
  # For check
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DNDEBUG -g")
endif()

# Find the LibTorch/OpenCV package
set(CMAKE_PREFIX_PATH
  "$ENV{HOME}/Dev/lib/libtorch;
  $ENV{HOME}/Dev/lib/opencv/build"
)

find_package(Torch REQUIRED)
find_package(OpenCV REQUIRED)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# Check if CUDA is available and set flags accordingly
if(TORCH_CUDA_VERSION)
  message(STATUS "CUDA support is enabled in LibTorch.")
  add_definitions(-DUSE_CUDA)
else()
  message(STATUS "CUDA support is NOT enabled in LibTorch.")
endif()

set(EXECUTABLE_TRAIN_NAME "train")
set(EXECUTABLE_TST_NAME "test")

set(CINC_DIR
  # Core
  ${CMAKE_SOURCE_DIR}/src/ori/cinc
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/utils
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/datasets
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/functional

  # Test
  ${CMAKE_SOURCE_DIR}/tests/C/include
)
set(CINC_FILES
  # Core
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/vanilla_vision_transformer.h
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/universe.h
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/constants.h
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/hovers.h
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/origin.h
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/focus.h

  # Logger
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/utils/log.hpp
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/utils/check.hpp

  # Datasets
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/dataloaders/mini_imagenet.h
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/dataloaders/mini_imagenet_chunked.h

  # Functional
  ${CMAKE_SOURCE_DIR}/src/ori/cinc/functional/image_utils.h

  # Test
  ${CMAKE_SOURCE_DIR}/tests/C/include/tests.h
)

set(CSRC_FILES
  # Vision Transformers
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/vision_transformers/vanilla_vision_transformer.cpp
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/vision_transformers/focus/configuration_focus.cpp
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/vision_transformers/focus/modeling_focus.cpp

  # Hovers
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/hovers/pos_embs/relative_position_embedding.cpp
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/hovers/pos_embs/rotary_position_embedding.cpp
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/hovers/attns/vanilla_attention.cpp
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/hovers/attns/sdpa_attention.cpp
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/hovers/attns/relative_attention.cpp
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/hovers/utils/auxiliary_loss_free_load_balancing.cpp
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/hovers/norms/rms_normalization.cpp
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/hovers/mlps/multi_layer_perception.cpp

  # Core
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/origin/modules.cpp
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/origin/options.cpp

  # Tools
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/functional/functional.cpp
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/functional/image_utils.cpp
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/utils.cpp
)

set(CTRAIN_FILES
  # Core
  ${CSRC_FILES}

  # Trainers
  ${CMAKE_SOURCE_DIR}/src/ori/csrc/scripts/trainer_focus.cpp
)
set(CTST_FILES
  # Core
  ${CSRC_FILES}

  # Test
  ${CMAKE_SOURCE_DIR}/tests/C/source/test.cpp
  ${CMAKE_SOURCE_DIR}/tests/C/source/utils.cpp
  ${CMAKE_SOURCE_DIR}/tests/C/source/models/test_focus.cpp
  ${CMAKE_SOURCE_DIR}/tests/C/source/models/test_vision_transformers.cpp
  ${CMAKE_SOURCE_DIR}/tests/C/source/hovers/test_relative_attention.cpp
  ${CMAKE_SOURCE_DIR}/tests/C/source/hovers/test_rotary_position_embedding.cpp
  ${CMAKE_SOURCE_DIR}/tests/C/source/hovers/test_relative_position_embedding.cpp
)

message(STATUS "Project root: ${CMAKE_SOURCE_DIR}")
message(STATUS "Include files: ${CINC_FILES}")
message(STATUS "Source  files: ${CSRC_FILES}")

# Options
option(BUILD_TEST "Build with test only" OFF)
option(BUILD_ALL "Build all" OFF)

if(BUILD_TEST)
  ## Test
  message(STATUS "Test build enabled~ 💖")
  add_executable(${EXECUTABLE_TST_NAME} ${CTST_FILES})
  target_include_directories(${EXECUTABLE_TST_NAME} PUBLIC ${CINC_DIR})

  ## Link against Torch
  target_link_libraries(${EXECUTABLE_TST_NAME} "${TORCH_LIBRARIES}")
  ## Link against OpenCV
  target_link_libraries(${EXECUTABLE_TST_NAME} ${OpenCV_LIBS})

  ## Set C++ standard
  set_property(TARGET ${EXECUTABLE_TST_NAME} PROPERTY CXX_STANDARD 17)
  set_property(TARGET ${EXECUTABLE_TST_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
elseif(BUILD_ALL)
  ## All
  message(STATUS "All build enabled~ 💖")

  ## --- Runnable
  add_executable(${EXECUTABLE_TRAIN_NAME} ${CTST_FILES})
  target_include_directories(${EXECUTABLE_TRAIN_NAME} PUBLIC ${CINC_DIR})

  ## Link against Torch
  target_link_libraries(${EXECUTABLE_TRAIN_NAME} "${TORCH_LIBRARIES}")
  ## Link against OpenCV
  target_link_libraries(${EXECUTABLE_TRAIN_NAME} ${OpenCV_LIBS})

  ## Set C++ standard
  set_property(TARGET ${EXECUTABLE_TRAIN_NAME} PROPERTY CXX_STANDARD 17)
  set_property(TARGET ${EXECUTABLE_TRAIN_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

  ## --- Test
  add_executable(${EXECUTABLE_TST_NAME} ${CTST_FILES})
  target_include_directories(${EXECUTABLE_TST_NAME} PUBLIC ${CINC_DIR})

  ## Link against Torch
  target_link_libraries(${EXECUTABLE_TST_NAME} "${TORCH_LIBRARIES}")
  ## Link against OpenCV
  target_link_libraries(${EXECUTABLE_TST_NAME} ${OpenCV_LIBS})

  ## Set C++ standard
  set_property(TARGET ${EXECUTABLE_TST_NAME} PROPERTY CXX_STANDARD 17)
  set_property(TARGET ${EXECUTABLE_TST_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
else()
  ## Train
  message(STATUS "Train build enabled~ ✨")
  add_executable(${EXECUTABLE_TRAIN_NAME} ${CTRAIN_FILES})
  target_include_directories(${EXECUTABLE_TRAIN_NAME} PUBLIC ${CINC_DIR})

  ## Link against Torch
  target_link_libraries(${EXECUTABLE_TRAIN_NAME} "${TORCH_LIBRARIES}")
  ## Link against OpenCV
  target_link_libraries(${EXECUTABLE_TRAIN_NAME} ${OpenCV_LIBS})

  ## Set C++ standard
  set_property(TARGET ${EXECUTABLE_TRAIN_NAME} PROPERTY CXX_STANDARD 17)
  set_property(TARGET ${EXECUTABLE_TRAIN_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
endif()
