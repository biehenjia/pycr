message(STATUS "CMAKE_BUILD_TYPE='${CMAKE_BUILD_TYPE}'")

cmake_minimum_required(VERSION 3.18)
project(py_cr LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


message(STATUS "CMAKE_BUILD_TYPE='${CMAKE_BUILD_TYPE}'")
foreach(cfg IN ITEMS "" DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
  string(TOUPPER "${cfg}" U)
  message(STATUS "CMAKE_CXX_FLAGS${cfg}: '${CMAKE_CXX_FLAGS${U}}'")
endforeach()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

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

target_include_directories(pycrlib PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(pycrlib PRIVATE
  $<$<CONFIG:Debug>:PYBIND11_DETAILED_ERROR_MESSAGES>
)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_ok OUTPUT _ipo_msg)
if(ipo_ok)
  set_property(TARGET pycrlib PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

# Put the extension inside the py_cr package in the wheel
install(TARGETS pycrlib
  LIBRARY DESTINATION py_cr
  RUNTIME DESTINATION py_cr
)