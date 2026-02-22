cmake_minimum_required(VERSION 3.16)
project(LFSInstallerBeta LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(lfs_installer_beta
    src/main.cpp
    src/installerwindow.h
    src/installerwindow.cpp
)

target_link_libraries(lfs_installer_beta PRIVATE Qt6::Widgets)
