cmake_minimum_required(VERSION 3.10)
project(src)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)
include_directories(libs/inih)
include_directories(libs/inih/cpp)
include_directories(/usr/include/pcap)

add_executable(src
        libs/inih/cpp/INIReader.cpp
        libs/inih/cpp/INIReader.h
        libs/inih/ini.c
        libs/inih/ini.h
        libs/csv.h
        control_pipe.cc
        control_pipe.h
        datastruct.h
        files.cc
        files.h
        mpradio.cc
        player.cc
        player.h
        settings_provider.cc
        settings_provider.h)
