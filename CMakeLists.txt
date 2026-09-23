cmake_minimum_required(VERSION 3.10)
project(Azy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Установка папки для сохранения исполняемого файла
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Application")

add_executable(AZY_compiler
    src/main.cpp

    src/Token.h

    src/Lexer.cpp
    src/Lexer.h
    src/Parser.cpp

    # src/parser.cpp
    # src/parser.h

    
)