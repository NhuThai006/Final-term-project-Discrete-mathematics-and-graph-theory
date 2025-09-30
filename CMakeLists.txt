cmake_minimum_required(VERSION 3.20)

project(TrafficPatrolEulerCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.9 COMPONENTS Widgets Gui Core PrintSupport REQUIRED)

set(SRC
    src/main.cpp
    src/MainWindow.cpp
    src/GraphCanvas.cpp
    src/Graph.cpp
    src/Algorithms.cpp
)

set(HDR
    src/MainWindow.h
    src/GraphCanvas.h
    src/Graph.h
    src/Algorithms.h
)

add_executable(${PROJECT_NAME}
    ${SRC}
    ${HDR}
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME}
    Qt6::Widgets
    Qt6::Gui
    Qt6::Core
    Qt6::PrintSupport
)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()


