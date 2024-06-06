include(FetchContent)

if (NOT TARGET magic_enum)
    FetchContent_Declare(
        menum
        GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
        GIT_TAG v0.9.5
    )
    FetchContent_MakeAvailable(menum)
endif ()

if (NOT TARGET EnTT)
    FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.13.2
    )
    FetchContent_MakeAvailable(entt)
endif ()

add_executable(nes_tests test.cpp)
target_link_libraries(nes_tests PUBLIC nuvola_event_system magic_enum EnTT)