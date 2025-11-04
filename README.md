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

### Run a Specific Test
```bash
make test test_identifiers.eac
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

## Features

### Core Language Features
- ✅ **Dynamic Variables** - `flex` keyword for mutable variables
- ✅ **Constants** - `fixed` keyword for immutable values
- ✅ **Type Hints** - Optional type annotations (int, float, str, bool, char)
- ✅ **Control Flow** - `when`/`else` conditionals, `while`/`for` loops
- ✅ **Functions** - First-class function support
- ✅ **Comments** - Single-line (#) and multi-line (/* */) comments
- ✅ **Noise Words** - Polite keywords (please, kindly, maybe) for readability

### Operators
- **Arithmetic**: `+`, `-`, `*`, `/`, `%`, `^` (exponent), `|` (absolute)
- **Relational**: `<`, `>`, `<=`, `>=`, `==`, `!=`
- **Logical**: `and`, `or`, `not`
- **Assignment**: `=`, `+=`, `-=`, `*=`, `/=`, `%=`

### Example Code
```eac
flex age: int = 25
fixed PI: float = 3.14159
flex name: str = "Alice"

when age > 18:
    output("Adult")
else:
    output("Minor")

flex counter = 0
while counter < 5:
    output(counter)
    counter += 1

function greet(person):
    output("Hello, ", person)
    return

please greet("World")
```

---

## Test Suite

### Comprehensive Test Coverage (494+ Test Cases)

The compiler includes a comprehensive test suite covering all 10 criteria:

| # | Criterion | File | Cases |
|---|-----------|------|-------|
| 1 | File Type Validation | test_file.{py,txt,c} | 3 files |
| 2 | Identifiers | test_identifiers.eac | 10 |
| 3 | Keywords | test_all_keywords.eac | 190 |
| 4 | Reserved Words | test_reserved_words.eac | 50 |
| 5 | Constant Values | test_constant_values.eac | 50 |
| 6 | Noise Words | test_noise_words.eac | 10 |
| 7 | Comments | test_all_comments.eac | 10 |
| 8a | Arithmetic Operators | test_arithmetic_operators.eac | 70 |
| 8b | Boolean Operators | test_boolean_operators.eac | 90 |
| 9 | Delimiters | test_delimiters.eac | 10 |
| 10 | Invalid Tokens | test_all_invalid.eac | 10 |

### Run All Tests
```bash
make test-all
```

This command will:
1. Test file type validation (reject non-.eac files)
2. Run all 10 criterion tests
3. Generate token tables in `output/` directory
4. Display test progress and results

---

## Project Structure

```
eac/
├── src/
│   ├── main.c              # Test harness
│   ├── common/
│   │   └── token.h         # Token definitions
│   ├── lexer/
│   │   ├── lexer.h         # Lexer interface
│   │   └── lexer.c         # Lexer implementation
│   ├── parser/             # Parser (future)
│   └── semantic/           # Semantic analyzer (future)
├── tests/
│   ├── test_identifiers.eac
│   ├── test_all_keywords.eac
│   ├── test_reserved_words.eac
│   ├── test_constant_values.eac
│   ├── test_noise_words.eac
│   ├── test_all_comments.eac
│   ├── test_arithmetic_operators.eac
│   ├── test_boolean_operators.eac
│   ├── test_delimiters.eac
│   ├── test_all_invalid.eac
│   ├── test_indentation.eac
│   ├── test_comprehensive_all.eac
│   ├── test_file.py
│   ├── test_file.txt
│   └── test_file.c
├── output/                 # Generated token tables
├── docs/
│   ├── DOCUMENTATION.md    # Language specification
│   ├── DEV_GUIDE.md        # Development guide
│   └── QUICK_START.md      # Quick start guide
├── Makefile                # Build system
└── README.md               # This file
```

---

## Output

### Token Tables

All test results are saved in `output/` directory:

```
output/symbol_table_<test_name>.txt
```

Each file contains a formatted table:
```
Line   Lexeme              Token               Token Special
==========================================================================

1      flex                KEYWORD             FLEX
1      age                 IDENTIFIER          IDENTIFIER
1      :                   DELIMITER           COLON
1      int                 HINT_KEYWORD        HINT_INT
1      =                   ASSIGNMENT          EQUAL
1      25                  INTEGER             INTEGER
...
```

---

## Documentation

- **[DOCUMENTATION.md](docs/DOCUMENTATION.md)** - Complete language specification
- **[DEV_GUIDE.md](docs/DEV_GUIDE.md)** - Development guide
- **[QUICK_START.md](docs/QUICK_START.md)** - Quick start guide
- **[RUN_ALL_TESTS.md](RUN_ALL_TESTS.md)** - Test execution guide
- **[TEST_SUITE_COMPLETION_REPORT.md](TEST_SUITE_COMPLETION_REPORT.md)** - Test coverage report
- **[CLEANUP_SUMMARY.md](CLEANUP_SUMMARY.md)** - Recent cleanup changes

---

## Development

### Requirements
- GCC compiler
- Make
- Windows, Linux, or macOS

### Build
```bash
make        # Build the compiler
make clean  # Clean build artifacts
```

### Testing
```bash
make test-all                    # Run all tests
make test test_identifiers.eac   # Run specific test
```

### Adding New Tests
1. Create a new `.eac` file in `tests/`
2. Add test cases following the existing patterns
3. Run `make test <your_test>.eac`
4. Check `output/symbol_table_<your_test>.txt` for results

---

## Current Status

✅ **Lexical Analyzer** - Complete and fully tested  
🔄 **Parser** - In development  
⏳ **Semantic Analyzer** - Planned  
⏳ **Code Generator** - Planned

---

## Keywords (19 total)

`flex`, `fixed`, `when`, `else`, `output`, `while`, `for`, `in`, `break`, `continue`, `return`, `function`, `import`, `from`, `true`, `false`, `and`, `or`, `not`

## Reserved Words (5 total)

`int`, `float`, `str`, `bool`, `char`

## Noise Words (3 total)

`please`, `kindly`, `maybe`

---

## License

Educational project for compiler design coursework.

---

## Authors

EaC Compiler Development Team

---

**Version:** 1.0  
**Last Updated:** November 4, 2025  
**Status:** ✅ Lexical Analysis Phase Complete
