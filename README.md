# EaC (easy) Programming Language

**EaC (easy)** is a modern, general-purpose, procedural programming language designed for simplicity, readability, and ease of use. It is a **dynamically-typed** language with **Python-inspired syntax** and **C-based operators**.

## Key Features

- **Dynamic Typing**: Variable types inferred at runtime
- **Clean Syntax**: Python-inspired with significant whitespace
- **C-Based Operators**: Familiar operators from C
- **Simple Keywords**: 23 built-ins including `flex`, `fixed`, `when`, `output`, `function`, `import`, `from`
- **Noise Word Handling**: Recognizes and labels benign words like `please`, `kindly`, `maybe`
- **Extended Literals**: Integers, floats, strings, characters, and booleans
- **Type Hints**: Optional type annotations for clarity

## Project Status

- **Lexical Analyzer**: Complete with full indentation tracking  
- **Parser**: Coming soon  
- **Semantic Analyzer**: Coming soon  
- **Code Generator**: Coming soon

## Project Structure

```
eac/
├── src/
│   ├── common/
│   │   └── token.h          # Token definitions
│   ├── lexer/
│   │   ├── lexer.h          # Lexer interface
│   │   └── lexer.c          # Lexer implementation
│   ├── parser/              # Parser (coming soon)
│   ├── semantic/            # Semantic analyzer (coming soon)
│   └── main.c               # Test harness
├── tests/
│   ├── test.eac             # Comprehensive test suite
│   ├── test_indentation.eac # Indentation tracking tests
│   ├── test_operators.eac   # All operators tests
│   ├── test_keywords.eac    # All keywords tests
│   ├── test_comments.eac    # Comment handling tests
│   └── test_literals.eac    # All literal types tests
├── docs/
│   ├── documentation.md     # Complete language specification
│   ├── QUICK_START.md       # Getting started guide
│   ├── SAMPLE_OUTPUT.md     # Example lexer output
│   └── ARCHITECTURE.md      # System architecture
├── Makefile                 # Build configuration
└── README.md                # This file
```

## Installation & Setup (Windows)

### Step 1: Install MSYS2

1. Download and install MSYS2 from [https://www.msys2.org/](https://www.msys2.org/)
2. Follow the installation wizard
3. Open MSYS2 terminal and install GCC:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc
   ```
4. Add MSYS2 to your Windows PATH (typically `C:\msys64\mingw64\bin`)
5. Verify installation:
   ```bash
   gcc --version
   ```

### Step 2: Install Chocolatey and Make

1. **Open Windows PowerShell as Administrator**
2. Check execution policy:
   ```powershell
   Get-ExecutionPolicy
   ```
3. If it returns `Restricted`, run one of these:
   ```powershell
   Set-ExecutionPolicy AllSigned
   # OR
   Set-ExecutionPolicy Bypass -Scope Process
   ```
4. Install Chocolatey:
   ```powershell
   Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
   ```
5. Install Make:
   ```powershell
   choco install make
   ```
6. Verify installation:
   ```bash
   make --version
   ```

### Step 3: Clone and Build

```bash
# Clone the repository
git clone https://github.com/eac-language/eac.git

# Switch to development branch
cd eac
git checkout dev

```

### Troubleshooting Windows Setup

**GCC not found after MSYS2 install?**
- Make sure you added `C:\msys64\mingw64\bin` to your Windows PATH
- Restart your terminal after updating PATH

**Make command not found?**
- Ensure Chocolatey installation completed successfully
- Try reopening PowerShell as Administrator
- Run `choco --version` to verify Chocolatey is installed

**Permission errors?**
- Run PowerShell as Administrator
- Check your execution policy with `Get-ExecutionPolicy`

## Quick Start

### Compile
```bash
make all
```

### Run Tests

```bash
# Run comprehensive test
make test

# Run all test suites
make test-all

# Run individual tests
./eac tests/test_indentation.eac
./eac tests/test_operators.eac
```

### Test Your Own Code

- Only `.eac` source files are accepted by the lexer (validation happens before tokenization)

```bash
./eac your_file.eac
```

### See Example Output

Want to see what the lexer produces? Check out **[Sample Output](docs/SAMPLE_OUTPUT.md)** for detailed examples of token output for various EaC code snippets.

For complete instructions, see **[Quick Start Guide](docs/QUICK_START.md)**.

## Example Code

```eac
# Variable declarations
flex age: int = 25
fixed PI: float = 3.14159

# Conditional statements
when age >= 18:
    output("You are an adult")
else:
    output("You are a minor")

# Loops
flex counter = 0
while counter < 5:
    output(counter)
    counter += 1

# For loops
for i in range(10):
    output(i)
```

## Documentation

- **[Language Specification](docs/documentation.md)** - Complete language reference
- **[Quick Start Guide](docs/QUICK_START.md)** - Get started quickly
- **[Sample Output](docs/SAMPLE_OUTPUT.md)** - See what the lexer produces
- **[Architecture](docs/ARCHITECTURE.md)** - System design and implementation details


## Lexical Analyzer Features

The refactored lexer includes:

- **Full Indentation Tracking** - INDENT/DEDENT tokens for Python-style blocks  
- **Comprehensive Token Set** - All operators, keywords, and literals  
- **Comment Tokens** - Single-line (#) and multi-line (/* */) emitted as COMMENT tokens  
- **Error Detection** - Detailed error messages with line numbers  
- **DRY Architecture** - Keyword lookup table for maintainability  
- **Opaque Pointer API** - Clean, encapsulated interface  
- **Zero-Copy Design** - Efficient token processing  


### Using GCC Directly

```bash
gcc -o eac src/main.c src/lexer/lexer.c -Isrc -Wall -Wextra -std=c11
```

## Testing

The project includes comprehensive test suites:

| Test File | Description |
|-----------|-------------|
| `test.eac` | Comprehensive test of all features |
| `test_indentation.eac` | Indentation tracking |
| `test_operators.eac` | All operators |
| `test_keywords.eac` | All keywords |
| `test_comments.eac` | Comment recognition, delimiters, and noise words |
| `test_literals.eac` | All literal types |
| `test_invalid.eac` | Invalid lexemes and error handling |

## Language Highlights

### Keywords (23 total)
- **Variables & Flow**: `flex`, `fixed`, `when`, `else`, `while`, `for`, `in`
- **Loop Control**: `break`, `continue`, `return`
- **IO & Structure**: `output`, `function`
- **Modules**: `import`, `from`
- **Truth Values**: `true`, `false`
- **Logical Operators**: `and`, `or`, `not`

### Reserved Words (Type Hints)
- `int`, `float`, `str`, `bool`, `char`

### Noise Words
- Recognized but semantically ignored: `please`, `kindly`, `maybe`

### Operators
- **Arithmetic (7)**: `+`, `-`, `*`, `/`, `%`, `^`, `|`
- **Relational (6)**: `<`, `>`, `<=`, `>=`, `==`, `!=`
- **Logical (3)**: `and`, `or`, `not`
- **Assignment (6)**: `=`, `+=`, `-=`, `*=`, `/=`, `%=`
- **Delimiters & Punctuation (7)**: `(`, `)`, `[`, `]`, `:`, `,`, `.`

## Contributing

We welcome contributions! If you find bugs or have improvements to suggest:

### Contribution Workflow

1. **Create a feature branch**:
   ```bash
   git checkout -b feat/your-addition
   ```
   
2. **Make your changes**:
   - Write clean, well-documented code
   - Follow existing code style
   - Add tests if applicable

3. **Test your changes**:
   ```bash
   make clean
   make all
   make test-all
   ```

4. **Commit and push**:
   ```bash
   git add .
   git commit -m "Description of your changes"
   git push origin feat/your-addition
   ```

5. **Create a Pull Request**:
   - Go to [https://github.com/eac-language/eac](https://github.com/eac-language/eac)
   - Click "New Pull Request"
   - Select your branch
   - Describe your changes clearly
   - Submit for review

### Contribution Guidelines

- Follow compiler design best practices
- Write comprehensive documentation
- Include test cases for new features
- Ensure all existing tests pass
- Keep commits focused and well-described

This is an academic/educational project implementing a programming language from scratch. The focus is on clean, well-documented code following compiler design best practices.

## License

Educational project - See documentation for details.

## References

- **Language Specification**: `docs/documentation.md`
- **Quick Start Guide**: `docs/QUICK_START.md`
- **Sample Output**: `docs/SAMPLE_OUTPUT.md`
- **Architecture Details**: `docs/ARCHITECTURE.md`
- **MSYS2**: [https://www.msys2.org/](https://www.msys2.org/)

---

**Status**: Lexical Analysis Phase Complete  
**Next**: Parser Implementation
