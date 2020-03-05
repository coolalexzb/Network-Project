cmake_minimum_required(VERSION 3.15)
project(Pro2)

set(CMAKE_CXX_STANDARD 14)

add_executable(Pro2 sendfile.cpp
                    recvfile.cpp PacketSendHandler.h PacketSendHandler.cpp PacketRecvHandler.cpp PacketRecvHandler.h helper.h)