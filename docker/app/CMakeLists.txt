cmake_minimum_required(VERSION 3.22)
project(Twelf)
enable_language(ASM)
add_application(Twelf
    twelf.c
    twelf.r
	 clikloop.s
    CREATOR "TWLF"
    )

add_library(TwelfLib STATIC IMPORTED)
set_target_properties(TwelfLib PROPERTIES
  IMPORTED_LOCATION "/root/app/libtwelf.a"
  INTERFACE_INCLUDE_DIRECTORIES "/root/app"
)

target_link_libraries(Twelf TwelfLib)


set_target_properties(Twelf PROPERTIES COMPILE_OPTIONS "-ffunction-sections")
set_target_properties(Twelf PROPERTIES LINK_FLAGS "-Wl,-gc-sections /root/app/libtwelf.a")
