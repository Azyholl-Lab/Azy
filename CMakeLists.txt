cmake_minimum_required(VERSION 3.10)
project(AzyLexer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lexer
    src/main.cpp
    src/lexer.cpp
    src/lexer.h
)