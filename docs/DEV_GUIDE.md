# EaC Compiler – Developer Guide

This document explains how the EaC lexer codebase is organised and how the key
modules interact. It is intended for new contributors, reviewers, and anyone
who needs to present or maintain the project.

---

## 1. Repository Structure

```
eac/
├── docs/                 # Documentation bundle
│   └── DEV_GUIDE.md      # (this file)
├── output/               # Generated symbol tables (git-ignored)
├── src/
│   ├── common/
│   │   └── token.h       # Token definitions shared by the lexer and harness
│   ├── lexer/
│   │   ├── lexer.c       # Full lexer implementation
│   │   └── lexer.h       # Public API for the lexer
│   └── main.c            # Command-line harness that exercises the lexer
├── tests/                # Curated .eac programs used as fixtures
├── Makefile              # Build, run, and clean targets
└── README.md             # Project overview for users
```

All generated artefacts live inside `output/`. Each lexer run writes a
`symbol_table_<test-name>.txt` file, which makes test results easy to inspect
side-by-side.

---

## 2. Build & Test Workflow

### make targets

- `make all` – builds the `eac` executable (no testing).
- `make test` – runs the lexer on a single test file. By default it chooses
  `tests/test.eac`, but you can override the input with
  `make test tests/test_keywords.eac` or with shorthand
  `make test test_keywords.eac`.
- `make test-all` – iterates through every curated test in `tests/` and
  writes a dedicated symbol table for each.
- `make clean` – Windows-friendly removal of the executable, object files,
  and the `output/` directory.

### Executables and artefacts

The compiled binary is `eac.exe` (on Windows). Each invocation expects a
single `.eac` source file and produces one symbol table. The harness validates
that the file extension is correct before attempting any work.

---

## 3. Core Modules Explained

### 3.1 `src/common/token.h`

| Function / Symbol | Purpose | Notes |
|-------------------|---------|-------|
| `typedef enum TokenType` | Enumerates every token category the lexer can emit. | Ordered by logical groups (lifecycle, structural, literals, keywords, operators, punctuation, noise). |
| `typedef struct Token` | Represents one lexeme with `type`, `lexeme`, `length`, `line`. | `lexeme` is a pointer into the original source string (no copies). |

Changes to token kinds should always update the table-driven sections of the
lexer and the CLI pretty-printers.

### 3.2 `src/lexer/lexer.h`

| Function | Purpose | Usage |
|----------|---------|-------|
| `Lexer* initLexer(const char* source);` | Allocates and initialises a lexer state machine for the provided source buffer. | Returns `NULL` on allocation failure; caller owns the returned pointer. |
| `Token getNextToken(Lexer* lexer);` | Produces the next token in sequence. | Always safe to call until `TOKEN_EOF` is returned. Errors are signalled via `TOKEN_ERROR`. |
| `void freeLexer(Lexer* lexer);` | Releases internal lexer storage. | Accepts `NULL` for convenience. |

### 3.3 `src/lexer/lexer.c`

This file contains all internal helpers plus the exported interface.

#### Keyword lookup helpers
| Function | Purpose |
|----------|---------|
| `identifierType` | Compares the current lexeme against `keywords[]` and `noiseWords[]`. Returns either a keyword token type, `TOKEN_NOISE`, or `TOKEN_IDENTIFIER`. |

#### Character / cursor utilities
| Function | Purpose |
|----------|---------|
| `isAlpha`, `isDigit`, `isAlphaNumeric` | Basic character classification wrappers. |
| `isAtEnd` | Checks if the cursor reached the terminating `\0`. |
| `peek`, `peekNext` | Lookahead without consuming characters. |
| `advance` | Consumes the current character and moves the cursor forward. |
| `match` | Conditionally consumes a character if it matches `expected`. |

#### Token construction
| Function | Purpose |
|----------|---------|
| `makeToken` | Builds a `Token` using the active lexeme range and supplied type. |
| `errorToken` | Returns a synthetic error token containing a message lexeme. |
| `syntheticToken` | Emits tokens that are not directly tied to source text (e.g., `INDENT`, `DEDENT`, `NEWLINE`). |

#### Lexeme scanners
| Function | Purpose |
|----------|---------|
| `scanString` | Consumes a double-quoted string literal; handles unterminated errors. |
| `scanChar` | Parses single-quoted characters, including escapes, enforcing single-char length. |
| `scanNumber` | Recognises integers and decimals (`FLOAT` when digits follow a dot). |
| `scanIdentifier` | Reads identifier characters and delegates to `identifierType`. |
| `scanLineComment` | Consumes `# ...` up to the newline and returns `TOKEN_COMMENT_LINE`. |
| `scanBlockComment` | Walks through `/* ... */`, tracking newlines and returning `TOKEN_COMMENT_BLOCK` or `TOKEN_ERROR` if unterminated. |

#### Indentation and whitespace
| Function | Purpose |
|----------|---------|
| `skipWhitespace` | Skips spaces, tabs, carriage returns. Does not skip newlines. |
| `measureIndentation` | Counts indentation width at the start of a logical line (tabs treated as 4 spaces). |
| `handleIndentation` | Compares the measured indent with the stack, emitting `TOKEN_INDENT`/`TOKEN_DEDENT` or reporting indentation errors. |

#### Main scanning loop
| Function | Purpose |
|----------|---------|
| `scanToken` | Coordinates pending dedents/newlines, indentation handling, literal dispatch, operator recognition, and error emission. This is the function repeatedly invoked by `getNextToken`. |

#### Public API wrappers
| Function | Purpose |
|----------|---------|
| `initLexer` | Allocates the `Lexer` struct, resets state flags (`atLineStart`, `needsNewline`), initialises the indentation stack, and stores the source string. |
| `getNextToken` | Thin wrapper over `scanToken` that guards against `NULL` lexers by returning `TOKEN_ERROR`. |
| `freeLexer` | Frees the struct allocated by `initLexer`. |

### 3.4 `src/main.c`

#### Token pretty printers
| Function | Purpose |
|----------|---------|
| `getTokenTypeName` | Converts `TokenType` to the middle-column label in the symbol table (e.g., `KEYWORD`, `ARITHMETIC`). |
| `getTokenSpecial` | Converts `TokenType` to a more specific descriptor (e.g., `FLEX`, `LESS_EQUAL`). |

#### File & path helpers
| Function | Purpose |
|----------|---------|
| `readFile` | Reads a file into a NUL-terminated buffer (`malloc` + `fread`). Caller must `free` the buffer. |
| `createDirectory` | Creates a directory if missing (portable implementation for Windows/POSIX). |
| `extractFilename` | Returns the basename from a path (supports `\` and `/`). |
| `generateOutputFilename` | Returns a new string `output/symbol_table_<stem>.txt`; ensures `output/` exists. |

#### Output formatting
| Function | Purpose |
|----------|---------|
| `printToken` | Renders one row of the symbol table, writing the line number, lexeme (sanitised for comments), token type, and token special identifier. |

#### CLI entry point
| Function | Purpose |
|----------|---------|
| `main` | Validates arguments, enforces `.eac` extension, generates output filename, reads source, initialises the lexer, iterates through tokens (printing and counting them), reports status, and frees all resources. |

Every error path provides a console message and returns a non-zero exit code to flag CI/build failures.

### 3.5 `tests/` directory

Each `.eac` file targets a specific rubric requirement. Running
`make test-all` executes them in the order below and leaves a matching symbol
table in `output/`.

| Test file              | Purpose |
|------------------------|---------|
| `test.eac`             | Comprehensive coverage: variables, loops, functions, imports, noise words, arithmetic, logical, and delimiter usage. |
| `test_indentation.eac` | Exercises nested `when` blocks and loops to validate `INDENT/DEDENT`. |
| `test_operators.eac`   | All arithmetic, relational, logical, and assignment operators. |
| `test_keywords.eac`    | Every keyword and reserved word, plus logical operators. |
| `test_comments.eac`    | Line and block comments, inline comments, nested comment content, noise words, and delimiter interactions. |
| `test_literals.eac`    | Every literal type (int, float, string, char, bool) and identifier shapes. |
| `test_invalid.eac`     | Purposely malformed tokens to trigger `TOKEN_ERROR`. |

Feel free to add new fixtures; they will automatically be exercised if you
append them to the `test-all` target.

### 3.6 `.gitignore`

Ensures build artefacts (`output/`, `*.o`, `eac.exe`) and temporary files are
not committed. Refer to the file for the full set of patterns.

---

## 4. Extending the Lexer

When adding new syntax features, follow these steps:

1. **Token type** – add an entry in `TokenType` inside `token.h`.
2. **Keyword/operator table** – update the appropriate table in `lexer.c`.
3. **Scanner logic** – modify `scanToken` or helper functions to recognise the
   new lexeme.
4. **CLI reporting** – extend `getTokenTypeName` and `getTokenSpecial` in
   `main.c` so the symbol table displays meaningful text.
5. **Tests** – include a `.eac` fixture that demonstrates the new behaviour.
6. **Documentation** – capture the change in `README.md` and this developer
   guide if external behaviour changes.

Always run `make clean`, `make all`, and `make test-all` before proposing a
patch.

---

## 5. Presentation Checklist

When presenting the project, consider highlighting:

- The lexer’s table-driven keyword recognition and zero-copy token design.
- Comment handling that preserves comment lexemes for later tooling.
- Indentation-aware scanning mirroring Python’s semantics.
- Noise-word support for “friendly” synctactic filler without affecting the
  AST.
- Automated test suite with per-test symbol tables for easy visual inspection.

This repository is intentionally modular so future stages (parser, semantic
analysis, code generation) can build on the same token infrastructure.

---

## 6. Support & Contribution

If you encounter issues:

1. Open a ticket describing the scenario, steps to reproduce, and lexer output.
2. Mention relevant `.eac` fixtures or add new ones under `tests/`.
3. Provide diffs for token tables or scanner changes when submitting patches.

All changes should pass `make test-all` and leave the workspace clean (`make clean`).

Happy hacking!

