# Azy
The official implementation of Azy — a long-term programming language project. Part of the Azyholl-Labs ecosystem. Contributions, forks, and feedback are welcome!

## Project Structure

```
Azy/
├── CMakeLists.txt          # Build configuration (CMake)
├── LICENSE                 # License information
├── README.md               # Project documentation
├── .gitignore              # Git ignore rules
├── include/                # Public header files
│   ├── lexer.hpp           # Lexer interface (tokenization)
│   └── parser.hpp          # Parser interface (AST building)
└── src/                    # Source code
    ├── main.cpp            # Entry point — reads source file and runs the pipeline
    ├── lexer.cpp           # Lexer implementation
    └── parser.cpp          # Parser implementation
```

### Components

- **`src/main.cpp`** — Entry point. Reads a source file (`.ml`) from the command line and prepares the pipeline (lexer → parser → interpreter).
- **`src/lexer.cpp` / `include/lexer.hpp`** — Lexer: converts raw source code into a stream of tokens. *(In development)*
- **`src/parser.cpp` / `include/parser.hpp`** — Parser: builds an abstract syntax tree (AST) from tokens. *(In development)*
- **`CMakeLists.txt`** — Build configuration for CMake-based compilation.
