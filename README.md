# EaC (easy) Programming Language Compiler

A modern, Python-inspired procedural programming language with C-based syntax.

## Quick Start

### Build the Compiler
```bash
make
```

### Run All Tests
```bash
make test-all
```

### Run Parser Tests
```bash
make test-parser
```

### Run Lexer Tests
```bash
make test-lexer
```

### Clean Build
```bash
make clean
```

---

## What is EaC?

**EaC (easy)** is a dynamically-typed programming language that combines:
- **C-based operators** (+, -, *, /, %, ^, |, ==, !=, etc.)
- **Python-inspired syntax** (clean, minimal, with significant indentation)
- **Developer-friendly features** (flexible variables, intuitive keywords)

---

## Core Features

- ✅ **Dynamic Variables** - `flex` keyword for mutable variables
- ✅ **Constants** - `fixed` keyword for immutable values
- ✅ **Type Hints** - Optional type annotations (int, float, str, bool, char)
- ✅ **Control Flow** - `when`/`else` conditionals, `while`/`for` loops
- ✅ **Functions** - First-class function support
- ✅ **Comments** - Single-line (#) and multi-line (/* */) comments
- ✅ **Noise Words** - Readability keywords (`to`, `of`, `then`, `each`, `as`) which are ignored by the parser.

---

## Technical Architecture & Code Tracing

### 1. Lexical Analysis (`src/lexer/`)
The **Lexer** transforms raw source code into a stream of tokens.

**Step-by-Step Execution:**
1.  **Initialization**: `initLexer(source)` sets up the lexer state, pointing to the start of the source string.
2.  **Scanning Loop**: The main loop calls `scanToken()` repeatedly.
    *   **Whitespace Handling**: `scanToken` skips spaces. If it encounters a newline, it calculates indentation.
    *   **Indentation Tracking**: The lexer maintains a stack of indentation levels. If indentation increases, it emits `TOKEN_INDENT`. If it decreases, it emits one or more `TOKEN_DEDENT` tokens.
    *   **State Machine**: Characters are processed through a finite state machine (FSM) to identify tokens:
        *   `"` triggers string state.
        *   Digits trigger number state.
        *   Letters trigger identifier/keyword state.
    *   **Keyword Matching**: Identifiers are checked against a trie or hash map to determine if they are reserved keywords (e.g., `flex`, `when`).
3.  **Token Generation**: valid lexemes are packaged into `Token` structs containing the type, lexeme string, line number, and column number.

### 2. Parsing (`src/parser/`)
The **Parser** consumes tokens to build an Abstract Syntax Tree (AST).

**Step-by-Step Execution:**
1.  **Initialization**: `initParser(lexer)` prepares the parser.
2.  **Recursive Descent**: The `parse()` function calls `statements()`, which recursively calls functions for specific grammar rules:
    *   `statements()` -> `statement()`
    *   `statement()` parses specific constructs like `impl_varDecl`, `impl_ifStmt`, etc.
3.  **Expression Parsing**: Uses a precedence climbing algorithm (or similar) to handle operator precedence (e.g., `*` binds tighter than `+`).
4.  **Error Handling**:
    *   If a syntax error occurs (e.g., missing expected token), `errorAt()` is called.
    *   The parser enters "panic mode" to suppress cascading errors.
    *   `synchronize()` skips tokens until a statement boundary (newline/semicolon) is found to recover.
    *   **Statement Termination**: `checkStatementEnd()` ensures statements end with a newline or EOF, preventing malformed constructs from entering the AST.
5.  **AST Construction**: Successful parses create `ASTNode` structures (e.g., `AST_BINARY_OP`, `AST_VAR_DECL`).

### 3. Testing (`tests/`)
The project includes a robust test suite.

*   `tests/demo/`: Contains demonstration files (e.g., `test_main.eac`, `test_criteria_compliance.eac`).
*   `tests/`: Root tests for specific language features.

---

## Project Structure

```
eac/
├── src/
│   ├── main.c              # Entry point
│   ├── common/
│   │   └── token.h         # Token definitions
│   ├── lexer/
│   │   ├── lexer.h         # Lexer interface
│   │   └── lexer.c         # Lexer implementation & FSM
│   ├── parser/             # Parser module
│   │   ├── parser.c        # Recursive descent parser
│   │   ├── parser.h        # Parser interface
│   │   ├── ast.c           # AST node creation & printing
│   │   └── ast.h           # AST structure definitions
├── tests/
│   ├── demo/               # Demo & compliance tests
│   │   ├── test_main.eac
│   │   └── test_criteria_compliance.eac
│   ├── parser/             # Parser-specific tests
│   └── ...                 # Other feature tests
├── output/                 # Generated artifacts (symbol tables, AST dumps)
├── docs/                   # Documentation
├── Makefile                # Build system
└── README.md               # This file
```

---

## Current Status

*   ✅ **Lexical Analyzer**: Complete. accurate tokenization including complex indentation handling.
*   ✅ **Parser**: Functional. Generates AST for declarations, expressions, control flow, and functions. Robust error handling implemented.
*   ⏳ **Semantic Analyzer**: Planned.
*   ⏳ **Code Generator**: Planned.

---

## Authors

EaC Compiler Development Team
