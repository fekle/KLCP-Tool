cmake_minimum_required(VERSION 2.8.4)
project(bif_ws14_vsys_server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    bif-ws14-vsys-server/client.hpp
    bif-ws14-vsys-server/common.hpp
    bif-ws14-vsys-server/connection.hpp
    bif-ws14-vsys-server/klcp.hpp
    bif-ws14-vsys-server/main.cpp
    bif-ws14-vsys-server/server.hpp
    bif-ws14-vsys-server/ldap_connect.hpp)

add_executable(bif_ws14_vsys_server ${SOURCE_FILES})