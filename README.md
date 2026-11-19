
# Azy Programming Language

<div align="center">

**Azy** — компилируемый язык программирования общего назначения, разрабатываемый с нуля на C++.

*Простой. Быстрый. Понятный.*

[![Language](https://img.shields.io/badge/language-C%2B%2B17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![Build](https://img.shields.io/badge/build-CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey?style=for-the-badge&logo=linux&logoColor=white)](#)
[![License](https://img.shields.io/badge/license-MIT-green?style=for-the-badge)](#)

[![Version](https://img.shields.io/badge/version-0.2-blue?style=flat-square)](#)
[![Lexer](https://img.shields.io/badge/lexer-complete-success?style=flat-square)](#)
[![Parser](https://img.shields.io/badge/parser-complete-success?style=flat-square)](#)
[![Semantic](https://img.shields.io/badge/semantic-complete-success?style=flat-square)](#)
[![Interpreter](https://img.shields.io/badge/interpreter-complete-success?style=flat-square)](#)
[![Codegen](https://img.shields.io/badge/codegen-planned-lightgrey?style=flat-square)](#)

</div>

---

## 📚 Документация

Подробное описание языка, синтаксиса, конструкций и грамматики:

- 📖 [Документация языка Azy (v0.2)](docs/language_v0.2.md)

---

## 📖 О языке

**Azy** — молодой язык программирования, создаваемый с нуля. Его цель — быть чистым, лаконичным и предсказуемым: таким, в котором приятно писать код, а компилятор легко расширять и сопровождать.

Проект задуман как учебно-практический, но с прицелом на реальное применение. **Лексер, парсер, AST, семантический анализатор и интерпретатор написаны вручную** — без сторонних фреймворков для построения языков. Это даёт полный контроль над архитектурой и поведением каждой части компилятора.

### 🎯 Философия

- **Простота важнее умности** — синтаксис должен читаться как псевдокод.
- **Явное лучше неявного** — никакой магии и скрытых преобразований.
- **Ошибки — часть языка** — понятные сообщения с кодом, позицией и подсказкой.
- **Открытость** — исходный код доступен, а грамматика развивается открыто.

---

## ✨ Ключевые идеи

| Принцип | Что это значит |
|---|---|
| 🧩 **Чистый синтаксис** | Минимум символов — максимум читаемости |
| ⚙️ **Полный конвейер** | Лексер → Парсер → Семантика → Интерпретатор |
| 🔍 **Точная диагностика** | Код ошибки, строка, колонка и подсказка по исправлению |
| 🧱 **Модульность** | Каждый этап — отдельный класс/файл, легко расширять |
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

### Сборка

```bash
cmake -S . -B build
cmake --build build
# Бинарник появится в Application/AZY_compiler(.exe)
```

### Запуск

```bash
./Application/AZY_compiler path/to/program.azy
./Application/AZY_compiler path/to/program.azy --ast    # + печать AST
```

---

## 🚦 Текущее состояние (v0.2)

| Этап компилятора | Статус | Примечание |
|---|---|---|
| 🔤 **Лексический анализ** | ✅ Завершён | Все токены, комментарии, строки, числа, операторы, BOM |
| 🌳 **Синтаксический анализ** | ✅ Завершён | `var`/`fn`/`def`/`return`, вызовы, все конструкции v0.2 |
| 🧠 Семантический анализ | ✅ Завершён | Области видимости, `S001`–`S005` |
| ⚙️ Интерпретация AST | ✅ Завершён | Tree-walk, рекурсия, builtin'ы, runtime-ошибки |
| 📦 Кодогенерация | ⏳ Запланирована | — |

### Что уже работает

**Лексер:**
- Идентификаторы и ключевые слова: `var`, `fn`/`def`, `if`, `else`, `while`, `do`, `for`, `in`, `return`, `break`, `continue`, `not`, `true`, `false`, `print`, `input`
- **Зарезервированные типы:** `int`, `float`, `char`, `string` (нельзя использовать как имена переменных)
- Числовые литералы: целые (`42`) и с плавающей точкой (`3.14`, `.5`, `10.`)
- Строковые литералы с поддержкой многострочных строк (с предупреждением `W001`)
- Комментарии трёх видов: `#`, `//`, `/* … */`
- Все операторы: арифметические, сравнения, логические, присваивания, `++`/`--`
- Разделители: `( ) { } [ ] ; , : .`
- Точные координаты каждого токена (строка + столбец)
- Обработка ошибок: `L001`–`L004`, `W001`
- Пропуск UTF-8 BOM

**Парсер:**
- Полностью рабочий AST с `std::unique_ptr`
- Объявления переменных: `var x;`, `var x = 5;`, `var a, b, c = 1;`
- Определения функций: `fn add(a, b) { … }`, `def` как синоним
- `return;` и `return expr;`
- Вызовы функций: `add(1, 2)`, вложенные `add(add(1,2), 3)`
- Присваивания: `=`, `+=`, `-=`, `*=`, `/=`
- Арифметика с правильным приоритетом и ассоциативностью
- Унарные операторы `-`, `!`, `not`
- Постфиксные `++` / `--` (как statement и внутри выражений)
- Логические `&&`, `||` и сравнения `==`, `!=`, `<`, `>`, `<=`, `>=`
- Условные: `if`, `else`, `else if`, вложенные
- Циклы: `while`, `do…while`, `for` (три формы)
- Управление: `break`, `continue`
- Вывод: `print(…)`, `print.ln(…)`
- Ввод: `input(x)`, `input(x, y, z)`, `input.line(x)`, `x = input()`
- Выражение как оператор: `greet("World");`
- Печать AST для отладки (`--ast`)

**Семантика:**
- Лексические области видимости (block scope, for-scope, function scope)
- Таблица символов + builtin-функции в глобальном scope
- Ошибки: `S001` (не объявлена), `S002` (дубль), `S003` (`return` вне `fn`), `S004` (`break` вне цикла), `S005` (`continue` вне цикла)

**Интерпретатор:**
- Древовидный (tree-walk) обход AST
- Динамическая типизация: `Int`, `Float`, `String`, `Bool`, `Null`, `Function`
- Рекурсия (`fib`, `factorial` — работают)
- `return`/`break`/`continue` через C++-исключения
- Builtin-функции: `len(s)`, `str(x)`, `int(x)`, `float(x)`
- Runtime-ошибки: `R001` (деление на 0), `R002` (input), `R003` (типы), `R004` (функция не найдена)
- `input`/`input.line` через `std::cin` / `std::getline`
- `input(x)` неявно объявляет `x` (как out-параметр)

### Примеры рабочих программ

**Рекурсия — числа Фибоначчи:**

```azy
fn fib(n) {
    if (n < 2) { return n; }
    return fib(n - 1) + fib(n - 2);
}

for (i = 0; i == 10; i++) {
    print(fib(i));
    print(" ");
}
print.ln("");
# Вывод: 0 1 1 2 3 5 8 13 21 34
```

**Полная программа — факториал, ввод, циклы:**

```azy
var counter = 0;

fn greet(name) {
    print("Hello, ");
    print.ln(name);
}

fn factorial(n) {
    if (n <= 1) { return 1; }
    return n * factorial(n - 1);
}

greet("World");
print("5! = ");
print.ln(factorial(5));

while (counter < 3) {
    print("counter = ");
    print.ln(counter);
    counter++;
}

for (ch in "abc") {
    print(ch);
    print(" ");
}
print.ln("");

var x = 10;
var y = 3;
print.ln(x / y);            # 3   (int-деление)
print.ln(x / float(y));     # 3.33333
```

**Ввод и вывод:**

```azy
print("Enter name: ");
input.line(name);
print.ln("Hello, " + name + "!");

print("Enter two numbers: ");
input(a, b);
print.ln(int(a) + int(b));
```

### Как выглядят ошибки

Azy использует **таблицу ошибок** с кодами, категорией и подсказкой:

```
[Ошибка лексера] L002: Незакрытая строка
  Подсказка: Добавьте закрывающую '"' в конце строки
  (строка: 25, символ: 12, токен: '"Alice;')
```

```
[Ошибка парсера] P006: Ожидался идентификатор после 'var'
  (строка: 1, символ: 5, токен: 'int')
```

```
[Ошибка семантики] S001: Использование необъявленной переменной 'y'
  Подсказка: Объявите её через var до первого использования
  (строка: 10, символ: 1)
```

```
[Ошибка выполнения] R001: Деление на ноль
```

### Что в работе (v0.3)

- **Массивы**: литералы `[1, 2, 3]`, индексация `arr[i]`, `arr[i] = x`
- **`for-in` по массивам** — сейчас только по строкам
- **`range(n)`** — потребует массивов
- **`print(a, b, c)`** — сейчас только один аргумент
- **Escape-последовательности** `\n`, `\t`, `\"`, `\\` в строках
- **Forward-ссылки на функции** — сейчас `fn a()` должна быть объявлена до вызова
- **Префиксные `++i` / `--i`**
- **Тернарный оператор** `? :`
- **`switch / case / default`**
- **Замыкания** и функции как значения

---

## 📂 Структура проекта

```text
AZY/
├── Application/                # Собранные бинарники и тестовые *.azy
├── docs/
│   └── language.md             # Спецификация языка v0.2
├── src/                        # Исходный код компилятора
│   ├── Token.h                 # Token и TokenType
│   ├── Lexer.h / Lexer.cpp     # Лексер (готов)
│   ├── Parser.h / Parser.cpp   # Парсер и AST (готов)
│   ├── Errors.h / Errors.cpp   # Таблица всех ошибок (L/P/S/R/W)
│   ├── Semantic.h / Semantic.cpp  # Семантический анализатор
│   ├── Value.h / Value.cpp     # Runtime-значения (Int/Float/String/Bool/Null/Function)
│   ├── Environment.h           # Scope chain (header-only)
│   ├── Interpreter.h / .cpp    # Tree-walk интерпретатор
│   └── main.cpp                # Точка входа
├── tests/
│   ├── test_all.azy            # Полный регресс-тест v0.2
│   ├── test_full.azy           # Факториал + циклы + builtin
│   ├── test_fib.azy            # Рекурсия
│   ├── test_input.azy          # Ввод/вывод
│   ├── test_runtime_err.azy    # R001
│   └── test_semantic_errors.azy # S001–S005
├── .gitignore
├── CMakeLists.txt
└── README.md
```

---

## 🗺 Планы развития

- [x] **v0.1** — Лексический анализатор + базовый парсер
- [x] **v0.2** — `var`/`fn`/`def`/`return`, вызовы функций, семантика, интерпретатор, builtin'ы, таблица ошибок
- [ ] **v0.3** — Массивы, `range()`, `for-in` по массивам, `print(a, b, c)`, escape-последовательности
- [ ] **v0.4** — Forward-ссылки на функции, префиксные `++/--`, тернарник, `switch`
- [ ] **v0.5** — Замыкания, функции как значения
- [ ] **v0.6** — Стандартная библиотека
- [ ] **v1.0** — Компиляция в байт-код / нативный код
- [ ] Пакетный менеджер

---

## 📐 Грамматика (фрагмент, v0.2)

```ebnf
program        → statement* EOF ;

statement      → varDecl
               | functionDecl
               | returnStmt
               | ifStmt
               | whileStmt
               | doWhileStmt
               | forStmt
               | breakStmt
               | continueStmt
               | printStmt
               | inputStmt
               | assignment
               | postfixStmt
               | expressionStmt
               | block
               | ";" ;

varDecl        → "var" varItem ( "," varItem )* ";" ;
varItem        → IDENTIFIER ( "=" expression )? ;

functionDecl   → ( "fn" | "def" ) IDENTIFIER
                 "(" ( IDENTIFIER ( "," IDENTIFIER )* )? ")"
                 block ;

returnStmt     → "return" expression? ";" ;

assignment     → IDENTIFIER ( "=" | "+=" | "-=" | "*=" | "/=" ) expression ";" ;
postfixStmt    → IDENTIFIER ( "++" | "--" ) ";" ;
expressionStmt → expression ";" ;

ifStmt         → "if" "(" expression ")" block
                 ( "else" ( ifStmt | block ) )? ;
whileStmt      → "while" "(" expression ")" block ;
doWhileStmt    → "do" block "while" "(" expression ")" ";" ;

forStmt        → "for" "(" IDENTIFIER
                   ( "in" expression
                   | "==" expression
                   | "=" expression ";" expression ";" forStep )
                 ")" block ;
forStep        → IDENTIFIER ( "++" | "--" )
               | IDENTIFIER "=" expression ;

printStmt      → "print" ( ".ln" )? "(" expression ")" ";" ;
inputStmt      → "input" ( ".line" )?
                 "(" IDENTIFIER ( "," IDENTIFIER )* ")" ";" ;

block          → "{" statement* "}" ;

expression     → logicalOr ;
logicalOr      → logicalAnd ( "||" logicalAnd )* ;
logicalAnd     → equality ( "&&" equality )* ;
equality       → relational ( ( "==" | "!=" ) relational )* ;
relational     → additive ( ( "<" | ">" | "<=" | ">=" ) additive )* ;
additive       → multiplicative ( ( "+" | "-" ) multiplicative )* ;
multiplicative → unary ( ( "*" | "/" ) unary )* ;
unary          → ( "-" | "!" | "not" ) unary | primary ;

primary        → INT | FLOAT | STRING | BOOL
               | IDENTIFIER
               | IDENTIFIER "(" ( expression ( "," expression )* )? ")"   (* вызов *)
               | "int"     "(" expression ")"                             (* builtin *)
               | "float"   "(" expression ")"                             (* builtin *)
               | "input"   ( ".line" )? "(" ")"                           (* input как выражение *)
               | "(" expression ")"
               | IDENTIFIER ( "++" | "--" ) ;
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
```
