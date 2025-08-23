cmake_minimum_required(VERSION 3.18)
project(py_cr LANGUAGES CXX)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message(STATUS "CMAKE_BUILD_TYPE='${CMAKE_BUILD_TYPE}'")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
find_package(pybind11 CONFIG QUIET)
if(NOT pybind11_FOUND)
  message(STATUS "pybind11 CMake package not found; fetching...")
  FetchContent_Declare(pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.12.0)
  FetchContent_MakeAvailable(pybind11)
endif()

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/bindings/*.cpp
)

# Compiled module will be py_cr/pycrlib.cpython-*.so
pybind11_add_module(pycrlib ${SOURCE_FILES})

# Unconditional optimization for the extension (avoid config drift)
target_compile_definitions(pycrlib PRIVATE NDEBUG)
if(MSVC)
  target_compile_options(pycrlib PRIVATE /O2)
else()
  target_compile_options(pycrlib PRIVATE -O3 -fno-semantic-interposition)
endif()

# Fail CI if our target isn't actually optimized
get_target_property(_opts pycrlib COMPILE_OPTIONS)
if(NOT _opts)  # handle empty property
  set(_opts "")
endif()
string(JOIN " " _opts_joined ${_opts})

if(MSVC)
  if(NOT _opts_joined MATCHES "(/O2)( |$)")
    message(FATAL_ERROR "pycrlib: expected /O2 in COMPILE_OPTIONS")
  endif()
else()
  if(NOT _opts_joined MATCHES "(^| )-O3( |$)")
    message(FATAL_ERROR "pycrlib: expected -O3 in COMPILE_OPTIONS")
  endif()
endif()

# Enable source-level checks only in wheel/CI builds
if(DEFINED ENV{CIBUILDWHEEL})
  target_compile_definitions(pycrlib PRIVATE PYCRLIB_ENFORCE_OPT=1)
endif()


target_include_directories(pycrlib PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Extra debug diagnostics only in Debug
target_compile_definitions(pycrlib PRIVATE
  $<$<CONFIG:Debug>:PYBIND11_DETAILED_ERROR_MESSAGES>
)

# Enable IPO/LTO project-wide when supported
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_ok OUTPUT _ipo_msg)
if(ipo_ok)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

# Put the extension inside the py_cr package in the wheel
install(TARGETS pycrlib
  LIBRARY DESTINATION py_cr
  RUNTIME DESTINATION py_cr
)
