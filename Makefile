CC = gcc
CFLAGS = -Isrc -Wall -Wextra -std=c11 -g
LDFLAGS =

SRC = src/main.c src/lexer/lexer.c src/parser/ast.c src/parser/parser.c
OBJ = $(SRC:.c=.o)

TARGET = eac

# Determine if a specific test file was provided on the command line
TEST_GOAL := $(firstword $(filter %.eac,$(MAKECMDGOALS)))

ifeq ($(TEST_GOAL),)
SELECTED_TEST := tests/test.eac
else ifneq ($(findstring /,$(TEST_GOAL)),)
SELECTED_TEST := $(TEST_GOAL)
else
SELECTED_TEST := tests/$(TEST_GOAL)
endif

.PHONY: all clean test test-all test-parser $(TEST_GOAL)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	@echo "Running test file: $(SELECTED_TEST)"
	@./$(TARGET) $(SELECTED_TEST)

test-parser: $(TARGET)
	@echo "========================================================================"
	@echo "                        EaC PARSER TESTS"
	@echo "========================================================================"
	@echo ""
	@echo "[TEST 1] Simple Variable Declaration:"
	@./$(TARGET) tests/parser/test_var_decl.eac
	@echo ""
	@echo "[TEST 2] Expressions and Operators:"
	@./$(TARGET) tests/parser/test_expressions.eac
	@echo ""
	@echo "[TEST 3] Control Flow (if/while/for):"
	@./$(TARGET) tests/parser/test_control_flow.eac
	@echo ""
	@echo "[TEST 4] Functions:"
	@./$(TARGET) tests/parser/test_functions.eac
	@echo ""
	@echo "[TEST 5] Complete Program:"
	@./$(TARGET) tests/parser/test_complete.eac
	@echo ""
	@echo "========================================================================"
	@echo "                      PARSER TESTS COMPLETED"
	@echo "========================================================================"

ifneq ($(TEST_GOAL),)
$(TEST_GOAL):
endif

test-all: $(TARGET)
	@echo "========================================================================"
	@echo "                  EaC LEXICAL ANALYZER - COMPREHENSIVE TEST SUITE"
	@echo "========================================================================"
	@echo ""
	@echo "[CRITERION 1] File Type Validation Tests:"
	@echo "  Testing non-.eac files (should be rejected)..."
	-@./$(TARGET) tests/test_file.py 2>nul
	-@./$(TARGET) tests/test_file.txt 2>nul
	-@./$(TARGET) tests/test_file.c 2>nul
	@echo "  File type validation complete (non-.eac files rejected)"
	@echo ""
	@echo "[CRITERION 2] Identifiers Test (10 cases):"
	@./$(TARGET) tests/test_identifiers.eac
	@echo ""
	@echo "[CRITERION 3] Keywords Test (24 keywords, 240 cases):"
	@./$(TARGET) tests/test_all_keywords.eac
	@echo ""
	@echo "[CRITERION 4] Reserved Words Test (5 types, 50 cases):"
	@./$(TARGET) tests/test_reserved_words.eac
	@echo ""
	@echo "[CRITERION 5] Constant Values Test (5 types, 50 cases):"
	@./$(TARGET) tests/test_constant_values.eac
	@echo ""
	@echo "[CRITERION 6] Noise Words Test (10 cases):"
	@./$(TARGET) tests/test_noise_words.eac
	@echo ""
	@echo "[CRITERION 7] Comments Test (10 cases):"
	@./$(TARGET) tests/test_all_comments.eac
	@echo ""
	@echo "[CRITERION 8a] Arithmetic Operators Test (7 operators, 70 cases):"
	@./$(TARGET) tests/test_arithmetic_operators.eac
	@echo ""
	@echo "[CRITERION 8b] Boolean Operators Test (9 operators, 90 cases):"
	@./$(TARGET) tests/test_boolean_operators.eac
	@echo ""
	@echo "[CRITERION 9] Delimiters & Brackets Test (10 cases):"
	@./$(TARGET) tests/test_delimiters.eac
	@echo ""
	@echo "[CRITERION 10] Invalid Tokens Test (10 cases):"
	-@./$(TARGET) tests/test_all_invalid.eac
	@echo ""
	@echo "[BONUS] Python-Style Indentation Test:"
	@./$(TARGET) tests/test_indentation.eac
	@echo ""
	@echo "[BONUS] Comprehensive All-in-One Test:"
	@./$(TARGET) tests/test_comprehensive_all.eac
	@echo ""
	@echo "========================================================================"
	@echo "                          ALL TESTS COMPLETED"
	@echo "========================================================================"
	@echo ""
	@echo "Total Test Files: 14"
	@echo "Total Test Cases: 544+"
	@echo ""
	@echo "Check the output/ directory for detailed token tables."
	@echo ""

clean:
	rm -f $(TARGET) $(TARGET).exe $(OBJ)
	rm -rf output