add_executable(nes_tests test.cpp)
target_link_libraries(nes_tests PUBLIC nuvola_event_system)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_PLATFORM_ID STREQUAL "Darwin")
    target_link_libraries(nes_tests PRIVATE -lstdc++)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(nes_tests PRIVATE -lc++)
endif()