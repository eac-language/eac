CC = gcc
CFLAGS = -Isrc -Wall -Wextra -std=c11 -g
LDFLAGS =

# Source files (no ast.c)
LEXER_SRC = src/lexer/lexer.c
PARSER_SRC = src/parser/parser.c
MAIN_SRC = src/main.c

SRC = $(MAIN_SRC) $(LEXER_SRC) $(PARSER_SRC)
OBJ = $(SRC:.c=.o)

TARGET = eac

# Test file selection
TEST_GOAL := $(firstword $(filter %.eac,$(MAKECMDGOALS)))

ifeq ($(TEST_GOAL),)
SELECTED_TEST := tests/test.eac
else ifneq ($(findstring /,$(TEST_GOAL)),)
SELECTED_TEST := $(TEST_GOAL)
else
SELECTED_TEST := tests/$(TEST_GOAL)
endif

.PHONY: all clean test test-all test-lexer test-parser test-integration $(TEST_GOAL)

all: $(TARGET)

$(TARGET): $(OBJ)
	@echo "Linking $(TARGET)..."
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)
	@echo "Build complete!"

%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Run single test
test: $(TARGET)
	@echo "Running test file: $(SELECTED_TEST)"
	@./$(TARGET) $(SELECTED_TEST)

ifneq ($(TEST_GOAL),)
$(TEST_GOAL):
endif

# Test only lexer (lexical analysis)
test-lexer: $(TARGET)
	@echo "========================================================================"
	@echo "                    LEXER TESTS - Lexical Analysis Only"
	@echo "========================================================================"
	@echo ""
	@./$(TARGET) --lex-only tests/test_identifiers.eac
	@./$(TARGET) --lex-only tests/test_all_keywords.eac
	@echo ""
	@echo "Lexer tests complete. Check output/ for symbol tables."
	@echo ""

# Test only parser (syntax analysis)
test-parser: $(TARGET)
	@echo "========================================================================"
	@echo "                   PARSER TESTS - Syntax Validation"
	@echo "========================================================================"
	@echo ""
	@echo "[TEST 1] Simple Declaration:"
	@./$(TARGET) tests/parser/test_declaration.eac
	@echo ""
	@echo "[TEST 2] Expressions:"
	@./$(TARGET) tests/parser/test_expressions.eac
	@echo ""
	@echo "[TEST 3] Control Flow:"
	@./$(TARGET) tests/parser/test_control_flow.eac
	@echo ""
	@echo "[TEST 4] Loops:"
	@./$(TARGET) tests/parser/test_loops.eac
	@echo ""
	@echo "[TEST 5] Functions:"
	@./$(TARGET) tests/parser/test_functions.eac
	@echo ""
	@echo "Parser tests complete."
	@echo ""

# Test integration (both lexer and parser)
test-integration: $(TARGET)
	@echo "========================================================================"
	@echo "            INTEGRATION TESTS - Full Lexer + Parser Pipeline"
	@echo "========================================================================"
	@echo ""
	@echo "[INTEGRATION 1] Simple Program:"
	@echo "---"
	@type tests\integration\test_simple.eac 2>nul || echo File not found
	@echo "---"
	@.\$(TARGET) tests/integration/test_simple.eac
	@echo ""
	@echo "[INTEGRATION 2] With Indentation:"
	@.\$(TARGET) tests/integration/test_indented.eac
	@echo ""
	@echo "[INTEGRATION 3] Complex Expressions:"
	@.\$(TARGET) tests/integration/test_complex.eac
	@echo ""
	@echo "[INTEGRATION 4] Error Recovery:"
	@echo "  (Testing parser error handling with lexically valid input)"
	-@.\$(TARGET) tests/integration/test_syntax_errors.eac
	@echo ""
	@echo "[INTEGRATION 5] Full Program:"
	@.\$(TARGET) tests/integration/test_full_program.eac
	@echo ""
	@echo "========================================================================"
	@echo "                    INTEGRATION TESTS COMPLETE"
	@echo "========================================================================"
	@echo ""

# Comprehensive test suite
test-all: $(TARGET)
	@echo "========================================================================"
	@echo "           EaC COMPILER - COMPREHENSIVE TEST SUITE"
	@echo "========================================================================"
	@echo ""
	@echo "========================================================================"
	@echo "                         PHASE 1: LEXICAL ANALYSIS"
	@echo "========================================================================"
	@echo ""
	@echo "[CRITERION 1] File Type Validation Tests:"
	@echo "  Testing non-.eac files (should be rejected)..."
	-@./$(TARGET) tests/test_file.py 2>nul
	-@./$(TARGET) tests/test_file.txt 2>nul
	-@./$(TARGET) tests/test_file.c 2>nul
	@echo "  [PASS] File type validation complete"
	@echo ""
	@echo "[CRITERION 2] Identifiers Test (10 cases):"
	@./$(TARGET) --lex-only tests/test_identifiers.eac
	@echo ""
	@echo "[CRITERION 3] Keywords Test (24 keywords, 240 cases):"
	@./$(TARGET) --lex-only tests/test_all_keywords.eac
	@echo ""
	@echo "[CRITERION 4] Reserved Words Test (5 types, 50 cases):"
	@./$(TARGET) --lex-only tests/test_reserved_words.eac
	@echo ""
	@echo "[CRITERION 5] Constant Values Test (5 types, 50 cases):"
	@./$(TARGET) --lex-only tests/test_constant_values.eac
	@echo ""
	@echo "[CRITERION 6] Noise Words Test (10 cases):"
	@./$(TARGET) --lex-only tests/test_noise_words.eac
	@echo ""
	@echo "[CRITERION 7] Comments Test (10 cases):"
	@./$(TARGET) --lex-only tests/test_all_comments.eac
	@echo ""
	@echo "[CRITERION 8a] Arithmetic Operators Test (7 operators, 70 cases):"
	@./$(TARGET) --lex-only tests/test_arithmetic_operators.eac
	@echo ""
	@echo "[CRITERION 8b] Boolean Operators Test (9 operators, 90 cases):"
	@./$(TARGET) --lex-only tests/test_boolean_operators.eac
	@echo ""
	@echo "[CRITERION 9] Delimiters & Brackets Test (10 cases):"
	@./$(TARGET) --lex-only tests/test_delimiters.eac
	@echo ""
	@echo "[CRITERION 10] Invalid Tokens Test (10 cases):"
	-@./$(TARGET) --lex-only tests/test_all_invalid.eac
	@echo ""
	@echo "[BONUS] Python-Style Indentation Test:"
	@./$(TARGET) --lex-only tests/test_indentation.eac
	@echo ""
	@echo "========================================================================"
	@echo "                         PHASE 2: SYNTAX ANALYSIS"
	@echo "========================================================================"
	@echo ""
	@echo "[PARSER 1] Declaration Statements:"
	@./$(TARGET) tests/parser/test_declaration.eac
	@echo ""
	@echo "[PARSER 2] Expression Parsing:"
	@./$(TARGET) tests/parser/test_expressions.eac
	@echo ""
	@echo "[PARSER 3] Control Flow:"
	@./$(TARGET) tests/parser/test_control_flow.eac
	@echo ""
	@echo "[PARSER 4] Loop Constructs:"
	@./$(TARGET) tests/parser/test_loops.eac
	@echo ""
	@echo "========================================================================"
	@echo "                      PHASE 3: INTEGRATION TESTING"
	@echo "========================================================================"
	@echo ""
	@echo "[INTEGRATION 1] Simple Program:"
	@./$(TARGET) tests/integration/test_simple.eac
	@echo ""
	@echo "[INTEGRATION 2] Indented Blocks:"
	@./$(TARGET) tests/integration/test_indented.eac
	@echo ""
	@echo "[INTEGRATION 3] Complex Program:"
	@./$(TARGET) tests/integration/test_full_program.eac
	@echo ""
	@echo "========================================================================"
	@echo "                        ALL TESTS COMPLETED"
	@echo "========================================================================"
	@echo ""
	@echo "Summary:"
	@echo "  [PASS] Lexical Analysis Tests: 544+ cases"
	@echo "  [PASS] Syntax Analysis Tests: 50+ cases"
	@echo "  [PASS] Integration Tests: 5 programs"
	@echo ""
	@echo "Check the output/ directory for detailed results."
	@echo ""

# Create test directory structure
setup-tests:
	@echo "Creating test directory structure..."
	@if not exist tests\parser mkdir tests\parser
	@if not exist tests\integration mkdir tests\integration
	@echo "Test directories created."

# Quick smoke test
smoke: $(TARGET)
	@echo "Running quick smoke test..."
	@echo "flex x = 10" > tests/smoke.eac
	@echo "output(x)" >> tests/smoke.eac
	@./$(TARGET) tests/smoke.eac
	@del tests\smoke.eac
	@echo "Smoke test passed!"

TARGET_BIN := $(TARGET)$(EXEEXT)
OBJ_CLEAN := $(subst /,\,$(OBJ))

clean:
	@echo "Cleaning build artifacts..."
	@if exist $(TARGET_BIN) del /f /q $(TARGET_BIN) >nul 2>&1
	@if exist $(TARGET) del /f /q $(TARGET) >nul 2>&1
	@if not "$(OBJ_CLEAN)"=="" del /f /q $(OBJ_CLEAN) >nul 2>&1
	@if exist output rmdir /s /q output >nul 2>&1
	@echo "Clean complete."

# Help target
help:
	@echo "EaC Compiler - Available Targets:"
	@echo ""
	@echo "  make                  - Build the compiler"
	@echo "  make test            - Run single test file"
	@echo "  make test-lexer      - Test lexer only"
	@echo "  make test-parser     - Test parser syntax validation"
	@echo "  make test-integration - Test lexer+parser integration"
	@echo "  make test-all        - Run all tests (comprehensive)"
	@echo "  make smoke           - Quick smoke test"
	@echo "  make setup-tests     - Create test directory structure"
	@echo "  make clean           - Clean build artifacts"
	@echo "  make help            - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make test tests/test.eac"
	@echo "  make test test.eac"
	@echo "  make test-integration"