cmake_minimum_required(VERSION 3.0)
project(chips_daq C CXX)

# Build Configuration
set(CMAKE_CXX_STANDARD 11)

set(BUILD_SHARED_LIBS YES CACHE BOOL "")
set(BUILD_STATIC_LIBS NO CACHE BOOL "")

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# This is just to have neat colors in Ninja
option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." OFF)
if(${FORCE_COLORED_OUTPUT})
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options(-fdiagnostics-color=always)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       add_compile_options(-fcolor-diagnostics)
    endif()
endif()

# Dependencies
find_package(Boost 1.60
    COMPONENTS thread program_options chrono system
    REQUIRED
)

find_package(ROOT 
    COMPONENTS Core Imt RIO Net Hist Graf Graf3d Gpad ROOTVecOps Tree TreePlayer Rint Postscript Matrix Physics MathCore Thread MultiProc ROOTDataFrame
    REQUIRED
)
include(${ROOT_USE_FILE})

# Subprojects
add_subdirectory(lib)

add_subdirectory(ops_cmd)
add_subdirectory(util)
add_subdirectory(relay)
add_subdirectory(fsm)
add_subdirectory(daqonite)
add_subdirectory(daqulator)
add_subdirectory(daqsitter)
add_subdirectory(daqontrol)
add_subdirectory(clb)
add_subdirectory(bbb)
