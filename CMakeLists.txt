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

add_executable(GALM main.cpp launcher.cpp resources.qrc)
target_link_libraries(GALM
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

include(CTest)
if(BUILD_TESTING)
    add_executable(launcher_serialization_tests tests/launcher_serialization_tests.cpp launcher.cpp resources.qrc)
    target_include_directories(launcher_serialization_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(launcher_serialization_tests PRIVATE Qt::Core)
    add_test(NAME launcher_serialization COMMAND launcher_serialization_tests)
endif()
