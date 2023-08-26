# Works with 3.14 and tested through 3.26
cmake_minimum_required(VERSION 3.14...3.26)

project(
  advanced_cpp
  VERSION 0.1
  DESCRIPTION "Learning Advanced C++"
  LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  option(GMSAM_ENABLE_TESTS "Build tests" ON)
endif()

set(HEADERS_LIST
  )

set(TARGET_NAME advanced_cpp)

add_library(${TARGET_NAME} INTERFACE)
target_sources(${TARGET_NAME} INTERFACE HEADERS_LIST)
target_include_directories(${TARGET_NAME} INTERFACE include)
target_compile_features(${TARGET_NAME} INTERFACE cxx_std_17)
target_compile_options(${TARGET_NAME} INTERFACE -Wall -Wextra -Wpedantic -Werror)

add_subdirectory(cpp_templates_the_complete_guide)
add_subdirectory(exceptional_cpp)
add_subdirectory(modern_cpp_design)

