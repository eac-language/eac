# EaC Lexer - Quick Start Guide

## Installation

1. **Clone or download the repository**

2. **Ensure you have GCC installed**
   ```bash
   gcc --version  # Should show GCC version
   ```

## Building

```bash
# Build the lexer
make all

# This creates the 'eac' executable
```

## Running Tests

### Quick Test (Comprehensive Suite)
```bash
make test
```

### All Tests
```bash
make test-all
```

### Individual Test File
```bash
./eac tests/test_indentation.eac
./eac tests/test_operators.eac
./eac tests/test_keywords.eac
# etc.
```

### Test Your Own Code
```bash
./eac your_file.eac
```

## Understanding Output

The lexer outputs tokens in this format:
```
Line <num> | <TOKEN_TYPE>       | '<lexeme>'
```

**Example:**
```
Line    1 | FLEX               | 'flex'
Line    1 | IDENTIFIER         | 'x'
Line    1 | COLON              | ':'
Line    1 | HINT_INT           | 'int'
Line    1 | EQUAL              | '='
Line    1 | INTEGER            | '10'
Line    1 | NEWLINE            
```

## Special Tokens

### Structural Tokens
- **NEWLINE**: End of statement
- **INDENT**: Indentation level increased
- **DEDENT**: Indentation level decreased

### Example with Indentation:
```eac
when x > 5:
    output("yes")
```

**Output:**
```
Line    1 | WHEN               | 'when'
Line    1 | IDENTIFIER         | 'x'
Line    1 | GREATER            | '>'
Line    1 | INTEGER            | '5'
Line    1 | COLON              | ':'
Line    1 | NEWLINE            
Line    2 | INDENT             
Line    2 | OUTPUT             | 'output'
Line    2 | LPAREN             | '('
Line    2 | STRING             | '"yes"'
Line    2 | RPAREN             | ')'
Line    2 | NEWLINE            
Line    3 | DEDENT             
```

## Writing EaC Code

### Variable Declaration
```eac
flex variable = 10          # Mutable variable
fixed constant = 20         # Immutable constant

flex typed: int = 42        # With type hint
```

### Control Flow
```eac
when condition:
    # Indented block
    output("true")
else:
    output("false")
```

### Loops
```eac
while x < 10:
    x += 1

for i in range(10):
    output(i)
```

### Operators
```eac
# Arithmetic
result = 10 + 5 - 3 * 2 / 4 % 3

# Relational
test = x < 10 and y > 5 or not z

# Assignment
counter += 1
```

### Comments
```eac
# Single-line comment

/*
 * Multi-line
 * comment
 */
```

## Common Issues

### Indentation Errors
**Problem:** `Indentation error: dedent to invalid level`

**Cause:** Dedenting to a level that was never indented to

**Solution:** Ensure consistent indentation (use spaces, not tabs)

**Bad:**
```eac
when x > 5:
    output("a")
  output("b")  # Wrong indentation level!
```

**Good:**
```eac
when x > 5:
    output("a")
output("b")  # Correct: back to base level
```

### Unterminated String
**Problem:** `Unterminated string literal`

**Cause:** Missing closing quote

**Solution:** Add closing quote
```eac
flex name = "Alice"  # Correct
```

### Unterminated Comment
**Problem:** `Unterminated block comment`

**Cause:** Missing `*/`

**Solution:** Close the comment
```eac
/* This is a comment */  # Correct
```

## Test File Examples

All test files are in the `tests/` directory:

- **test.eac** - Comprehensive test of all features
- **test_indentation.eac** - Indentation tracking
- **test_operators.eac** - All operators
- **test_keywords.eac** - All keywords
- **test_comments.eac** - Comment handling
- **test_literals.eac** - All literal types

## Next Steps

1. **Explore test files**: See examples of all EaC features
2. **Write your own code**: Create `.eac` files and test them
3. **Check the documentation**: See `docs/documentation.md` for full spec
4. **View sample output**: See `docs/SAMPLE_OUTPUT.md` for expected token output
5. **Understand architecture**: See `docs/ARCHITECTURE.md` for implementation details

## Getting Help

If you encounter issues:

1. Check that source file has correct syntax
2. Verify indentation is consistent (use spaces)
3. Check that all strings and comments are closed
4. Review test files for correct usage examples
5. Check error messages - they indicate the line and problem

## Summary of Commands

```bash
# Build
make all

# Clean build artifacts
make clean

# Run tests
make test           # Main test
make test-all       # All tests

# Test your file
./eac myfile.eac
```

Happy coding with EaC!

