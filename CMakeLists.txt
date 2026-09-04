cmake_minimum_required(VERSION 3.10)
project(Azy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Установка папки для сохранения исполняемого файла
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Application")

add_executable(AZY_compiler_Andrey
    src/main.cpp

    src/Token.h

    src/lexer.cpp
    src/lexer.h

    src/parser.cpp
    src/parser.h

)

add_executable(AZY_compiler_Dima
    scr2_for_dima/main.cpp

    scr2_for_dima/Token.h

    scr2_for_dima/lexer.cpp
    scr2_for_dima/lexer.h

    scr2_for_dima/parser.cpp
    scr2_for_dima/parser.h

    
)