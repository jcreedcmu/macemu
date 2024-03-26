cmake_minimum_required(VERSION 3.22)
project(Twelf)

add_application(Twelf
    twelf.c
    twelf.r
    CREATOR "TWLF"
    )

set_target_properties(Twelf PROPERTIES COMPILE_OPTIONS -ffunction-sections)
set_target_properties(Twelf PROPERTIES LINK_FLAGS "-Wl,-gc-sections")
