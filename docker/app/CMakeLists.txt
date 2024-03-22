cmake_minimum_required(VERSION 3.22)
project(Logic)

add_application(Logic
    logic.c
    logic.r
    )

set_target_properties(Logic PROPERTIES COMPILE_OPTIONS -ffunction-sections)
set_target_properties(Logic PROPERTIES LINK_FLAGS "-Wl,-gc-sections")
