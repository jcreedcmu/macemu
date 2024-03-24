cmake_minimum_required(VERSION 3.22)
project(Twelf)

add_application(Twelf
    logic.c
    logic.r
    )

set_target_properties(Twelf PROPERTIES COMPILE_OPTIONS -ffunction-sections)
set_target_properties(Twelf PROPERTIES LINK_FLAGS "-Wl,-gc-sections")
