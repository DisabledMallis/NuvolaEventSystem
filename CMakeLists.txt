cmake_minimum_required(VERSION 3.26)
project(nuvola_event_system)

set(CMAKE_CXX_STANDARD 23)

add_library(nuvola_event_system INTERFACE)
if (NOT TARGET NES)
    add_library(NES ALIAS nuvola_event_system)
endif ()
target_include_directories(nuvola_event_system INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

option(BUILD_NES_TESTS "Build tests" OFF)
if(BUILD_NES_TESTS)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test)
endif ()
