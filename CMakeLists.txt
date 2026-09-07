cmake_minimum_required(VERSION 3.16)
project(GALM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(GALM main.cpp resources.qrc)
target_link_libraries(GALM
        Qt::Core
        Qt::Gui
        Qt::Widgets
)
