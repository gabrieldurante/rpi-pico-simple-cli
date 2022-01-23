cmake_minimum_required(VERSION 3.12)

# submodules
find_package(Git QUIET)
execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
if(NOT GIT_SUBMOD_RESULT EQUAL "0")
    message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
endif()

# build type
if(DEFINED DEBUG AND DEBUG STREQUAL 1)
set(CMAKE_BUILD_TYPE Debug)
else()
set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "BUILDTYPE " ${CMAKE_BUILD_TYPE})

# sdk settings
set(PICO_PLATFORM "rp2040")
set(PICO_BOARD "pico")

# set sdk path
set(PICO_SDK_PATH ${CMAKE_SOURCE_DIR}/3pp/pico-sdk)

# import sdk
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(rpi-pico-simple-cli C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# initialize the sdk
pico_sdk_init()

# set compile options
add_compile_options(-Wall -Werror)

# add libraries
add_subdirectory(utils)

# add main app
add_subdirectory(app)
