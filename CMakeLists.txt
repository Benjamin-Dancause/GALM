cmake_minimum_required(VERSION 4.3)
project(GnomeAppLauncherMaker)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

configure_file(Template.desktop Template.desktop COPYONLY)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(GnomeAppLauncherMaker main.cpp)
target_link_libraries(GnomeAppLauncherMaker
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

