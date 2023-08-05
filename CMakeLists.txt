# Works with 3.14 and tested through 3.26
cmake_minimum_required(VERSION 3.14...3.26)

project(
  cpp_templates
  VERSION 0.1
  DESCRIPTION "C++ Templates - The Complete Guide"
  LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  option(GMSAM_ENABLE_TESTS "Build tests" ON)
endif()

set(HEADERS_LIST
  include/VariadicTemplates.hpp
  )

set(TARGET_NAME cpp_templates)

add_library(${TARGET_NAME} INTERFACE)
target_sources(${TARGET_NAME} INTERFACE HEADERS_LIST)
target_include_directories(${TARGET_NAME} INTERFACE include)
target_compile_features(${TARGET_NAME} INTERFACE cxx_std_17)
target_compile_options(${TARGET_NAME} INTERFACE -Wall -Wextra -Wpedantic -Werror)

add_subdirectory(src)

