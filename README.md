# eac
EaC (easy) is a modern, general-purpose, procedural programming language designed for simplicity, readability, and ease of use. It is a dynamically-typed language, meaning variable types are inferred at runtime, not through explicit mandatory declarations.

## Getting Started

### Prerequisites

To build and run the EaC compiler, you will need a C compiler, such as GCC, installed on your system.

### Building the Compiler

You can build the compiler using either `make` or `gcc` directly.

**Using `make`:**

If you have `make` installed, you can compile the project by running the following command in the root directory:

```bash
make
```

**Using `gcc`:**

If you do not have `make` installed, you can compile the source files directly using `gcc`:

```bash
gcc -o eac eac.c src/lexer/lexer.c -Isrc
```

### Running the Lexical Analyzer

Once the compiler is built, an executable file named `eac` will be created in the root directory.

To run the lexical analyzer on a sample source file, use the following command:

```bash
tests/test.eac
```

This will read the source code from `tests/test.eac`, process it through the lexical analyzer, and print the resulting tokens to the console.
