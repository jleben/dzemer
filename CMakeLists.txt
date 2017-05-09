cmake_minimum_required(VERSION 3.0)

project(dzemer)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)

include_directories(
    /usr/include/lilv-0
    /usr/lib/lv2
)

add_executable(dzemer
    dzemer.cpp
    engine/engine.cpp
    engine/score_recorder.cpp
    engine/score_player.cpp
    gui/main_window.cpp
    gui/lv2_plugin_browser.cpp
    lv2-util/Plugin.cpp
)

target_link_libraries(dzemer
    lilv-0
    jack
    Qt5::Widgets
)

add_subdirectory(test)
