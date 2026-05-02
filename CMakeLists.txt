cmake_minimum_required(VERSION 3.20)
project(NetworkMonitor)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

add_executable(NetworkMonitor
    main.cpp
    mainwindow.h
    mainwindow.cpp
    worker.h
    worker.cpp
)

target_link_libraries(NetworkMonitor
    Qt6::Widgets
    Qt6::Charts
    iphlpapi
    wlanapi
    ws2_32
)