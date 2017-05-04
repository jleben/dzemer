cmake_minimum_required(VERSION 3.0)

project(dzemer)

include_directories(
    /usr/include/lilv-0
    /usr/lib/lv2
)

add_executable(dzemer
    dzemer.cpp
    lv2-util/Plugin.cpp
)

target_link_libraries(dzemer lilv-0)
