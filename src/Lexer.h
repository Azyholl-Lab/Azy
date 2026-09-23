#pragma once
#include <string>
#include <cstdio>
#include "Token.h"
#include <cctype>

// Функция, которая возвращает следующий токен из потока
Token gettok(FILE *stream);