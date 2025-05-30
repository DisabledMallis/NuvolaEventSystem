add_executable(nes_tests test.cpp)
target_link_libraries(nes_tests PUBLIC nuvola_event_system)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_libraries(nes_tests PRIVATE -lstdc++)
endif()