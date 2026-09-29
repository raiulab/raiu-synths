cmake_minimum_required(VERSION 3.24)
project(RAIU_SYNTHS VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES arm64)

include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.9 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.7.1 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(Catch2)

add_subdirectory(plugins/raiu1)
