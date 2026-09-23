# Azy Programming Language

<div align="center">

**Azy** — компилируемый язык программирования общего назначения, разрабатываемый с нуля на C++.

*Простой. Быстрый. Понятный.*

[![Language](https://img.shields.io/badge/language-C%2B%2B17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![Build](https://img.shields.io/badge/build-CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey?style=for-the-badge&logo=linux&logoColor=white)](#)
[![License](https://img.shields.io/badge/license-MIT-green?style=for-the-badge)](#)

[![Lexer](https://img.shields.io/badge/lexer-complete-success?style=flat-square)](#)
[![Parser](https://img.shields.io/badge/parser-in%20progress-yellow?style=flat-square)](#)
[![Semantic](https://img.shields.io/badge/semantic%20analysis-planned-lightgrey?style=flat-square)](#)
[![Interpreter](https://img.shields.io/badge/interpreter-planned-lightgrey?style=flat-square)](#)

</div>


---

## 📚 Документация

Подробное описание языка, синтаксиса, конструкций и грамматики доступно здесь:

- [Документация языка Azy](docs/language.md)

---

## 📖 О языке

**Azy** — молодой компилируемый язык программирования, создаваемый с нуля. Его цель — быть чистым, лаконичным и предсказуемым: таким, в котором приятно писать код, а компилятор легко расширять и сопровождать.

Проект задуман как учебно-практический, но с прицелом на реальное применение. Лексер, парсер, AST, семантический анализ и интерпретатор/кодогенератор пишутся вручную — без сторонних фреймворков для построения языков. Это даёт полный контроль над архитектурой и поведением каждой части компилятора.

### 🎯 Философия

- **Простота важнее умности** — синтаксис должен читаться как псевдокод.
- **Явное лучше неявного** — никакой магии и скрытых преобразований.
- **Ошибки — часть языка** — понятные сообщения с точной позицией в коде.
- **Открытость** — исходный код доступен, а грамматика развивается открыто.

---

## ✨ Ключевые идеи

| Принцип | Что это значит |
|---|---|
| 🧩 **Чистый синтаксис** | Минимум символов — максимум читаемости |
| ⚙️ **Нативная компиляция** | Компилятор — программа под конкретную ОС и архитектуру |
| 🔍 **Точная диагностика** | Номер строки, колонка и контекст ошибки |
| 🧱 **Модульность** | Лексер, парсер, семантик и интерпретатор — раздельные этапы |
| 🚀 **C++17 под капотом** | Современный стандарт, RAII, `std::unique_ptr`, безопасность памяти |

---

## 🛠 Технологии

<div align="center">

![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.10%2B-064F8C?style=flat-square&logo=cmake&logoColor=white)
![GCC](https://img.shields.io/badge/GCC-7%2B-A42E2B?style=flat-square&logo=gnu&logoColor=white)
![Clang](https://img.shields.io/badge/Clang-5%2B-262D3A?style=flat-square&logo=llvm&logoColor=white)
![MSVC](https://img.shields.io/badge/MSVC-2017%2B-5C2D91?style=flat-square&logo=visualstudio&logoColor=white)
![Git](https://img.shields.io/badge/Git-F05032?style=flat-square&logo=git&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat-square&logo=linux&logoColor=black)
![Windows](https://img.shields.io/badge/Windows-0078D6?style=flat-square&logo=windows&logoColor=white)
![macOS](https://img.shields.io/badge/macOS-000000?style=flat-square&logo=apple&logoColor=white)

</div>

**Требования к сборке:**

- Компилятор с поддержкой **C++17** — GCC 7+, Clang 5+, MSVC 2017+
- Система сборки **CMake** версии **3.10** и выше

---

## 🚦 Текущее состояние

| Этап компилятора | Статус | Примечание |
|---|---|---|
| 🔤 **Лексический анализ** | ✅ Завершён | Все типы токенов, комментарии, строки, числа, операторы |
| 🌳 **Синтаксический анализ** | 🟡 В процессе | Основные конструкции разбираются, AST строится |
| 🧠 Семантический анализ | ⏳ Запланирован | — |
| ⚙️ Интерпретация AST | ⏳ Запланирован | — |
| 📦 Кодогенерация | ⏳ Запланирована | — |

### Что уже работает

**Лексер:**
- Идентификаторы и ключевые слова (`let`, `fn`/`def`, `if`, `else`, `while`, `do`, `for`, `in`, `return`, `break`, `continue`, `not`, `true`, `false`)
- Числовые литералы: целые (`42`) и с плавающей точкой (`3.14`, `.5`)
- Строковые литералы
- Комментарии трёх видов: `#`, `//`, `/* … */`
- Все операторы: арифметические, сравнения, логические, присваивания, `++`/`--`
- Разделители: `( ) { } [ ] ; , : .`
- Точные координаты каждого токена (строка + столбец)
- Обработка ошибок через `TokenType::Unknown`

**Парсер:**
- Полностью рабочий AST с `std::unique_ptr`
- Присваивания: `=`, `+=`, `-=`, `*=`, `/=`
- Арифметика с правильным приоритетом и ассоциативностью
- Унарные операторы `-`, `!`, `not`
- Постфиксные `++` / `--`
- Логические `&&`, `||` и сравнения `==`, `!=`, `<`, `>`, `<=`, `>=`
- Условные конструкции: `if`, `else`, `else if`, вложенные `if`
- Циклы: `while`, `do…while`
- Цикл `for` в трёх формах:
  - `for (i == N)` — сокращённая
  - `for (i = 0; i == N; i++)` — полная C-style
  - `for (x in iterable)` — foreach
- Управление циклом: `break`, `continue`
- Оператор вывода: `print(…)` и `print.ln(…)`
- Печать AST для отладки (`Parser::printAST`)
- Понятные сообщения об ошибках с координатами

### Пример исходного кода

```azy
# Полный тест парсера языка Azy

i = 42;
f = 3.14;
s = "Hello, Azy!";
t = true;

x = 2 + 3 * 4;              # 14 — приоритет соблюдён
y = (2 + 3) * 4;            # 20

if (i == 42) {
    print.ln("Ответ найден");
} else {
    print.ln("Мимо");
}

for (i = 0; i == 10; i++) {
    if (i == 5) {
        continue;
    }
    print(i);
}

text = "abc";
for (ch in text) {
    print(ch);
}
```

### Что в работе (TODO)

- Префиксные `++` / `--` (`++i`, `--i`) — в `parseUnary`
- Массивы: литералы `[1, 2, 3]`, индексация `arr[i]`, присваивание по индексу
- Функции: определение `def greet(name) { … }`, вызов `greet("мир")`, `return`
- Встроенные функции: `len`, `range`
- Тернарный оператор `? :`
- `switch / case / default`
- Приведение `KwLet` / `KwFn` к полноценному использованию в грамматике
- Семантический анализ (типы, области видимости)
- Интерпретатор AST

---

## 📂 Структура проекта

```text
AZY/
├── Application/              # Собранные бинарники и тестовые скрипты (*.azy)
├── docs/                     # Документация языка
├── src/                      # Исходный код компилятора
│   ├── Lexer.cpp                 # Лексический анализатор (готов)
│   ├── Lexer.h
│   ├── Parser.cpp                # Синтаксический анализатор (в работе)
│   ├── Parser.h                  # Интерфейс парсера и узлы AST
│   ├── Token.h                   # Определение Token и TokenType
│   └── main.cpp                  # Точка входа
├── tests/                    # Тестовые сценарии языка
│   └── test_all.txt              # Полный регресс-тест лексера и парсера
├── .gitignore
├── CMakeLists.txt
└── README.md
```

---

## 🗺 Планы развития

- [x] Лексический анализатор
- [ ] Синтаксический анализатор *(в процессе)*
- [ ] Семантический анализ
- [ ] Интерпретатор AST
- [ ] Компиляция в байт-код / нативный код
- [ ] Стандартная библиотека
- [ ] Пакетный менеджер

---

## 📐 Грамматика (фрагмент)

```ebnf
program        → statement* EOF ;

statement      → assignment
               | ifStatement
               | whileStatement
               | doWhileStatement
               | forStatement
               | breakStatement
               | continueStatement
               | printStatement ;

assignment     → IDENTIFIER ("=" | "+=" | "-=" | "*=" | "/=") expression ";" ;

ifStatement    → "if" "(" expression ")" block ( "else" ( ifStatement | block ) )? ;

whileStatement → "while" "(" expression ")" block ;

doWhileStatement → "do" block "while" "(" expression ")" ";" ;

forStatement   → "for" "(" IDENTIFIER
                   ( "in" expression
                   | "==" expression
                   | "=" expression ";" expression ";" forStep )
                 ")" block ;

printStatement → "print" ( ".ln" )? "(" expression ")" ";" ;

expression     → logicalOr ;
logicalOr      → logicalAnd ( "||" logicalAnd )* ;
logicalAnd     → equality ( "&&" equality )* ;
equality       → relational ( ( "==" | "!=" ) relational )* ;
relational     → additive ( ( "<" | ">" | "<=" | ">=" ) additive )* ;
additive       → multiplicative ( ( "+" | "-" ) multiplicative )* ;
multiplicative → unary ( ( "*" | "/" ) unary )* ;
unary          → ( "-" | "!" | "not" ) unary | primary ;
primary        → INT | FLOAT | STRING | BOOL | IDENTIFIER
               | "(" expression ")" ;
```

---

## 🤝 Участие в разработке

Проект молодой и открыт для идей. Если вы хотите:

- предложить синтаксическую конструкцию,
- обсудить дизайн языка,
- сообщить об ошибке,
- помочь с реализацией,

— создавайте **Issue** или присылайте **Pull Request**. Мы рады любой конструктивной помощи.

---

## 📄 Лицензия

Проект распространяется под лицензией **MIT** — свободно используйте, изменяйте и распространяйте.

---

<div align="center">

**Azy** — язык, который пишется с нуля и с любовью. 🚀

*Сделано на C++17 и CMake.*

</div>

---
