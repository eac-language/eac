CC = gcc
CFLAGS = -Isrc -Wall -Wextra -std=c11 -g
LDFLAGS =

SRC = src/main.c src/lexer/lexer.c
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

.PHONY: all clean test test-all $(TEST_GOAL)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	@echo "Running test file: $(SELECTED_TEST)"
	@./$(TARGET) $(SELECTED_TEST)

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
	-@./$(TARGET) tests/test_file.py 2>nul || echo "  - test_file.py rejected correctly"
	-@./$(TARGET) tests/test_file.txt 2>nul || echo "  - test_file.txt rejected correctly"
	-@./$(TARGET) tests/test_file.c 2>nul || echo "  - test_file.c rejected correctly"
	@echo ""
	@echo "[CRITERION 2] Identifiers Test (10 cases):"
	@./$(TARGET) tests/test_identifiers.eac
	@echo ""
	@echo "[CRITERION 3] Keywords Test (19 keywords, 190 cases):"
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
	@./$(TARGET) tests/test_all_invalid.eac || true
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
	@echo "Total Test Cases: 494+"
	@echo ""
	@echo "Check the output/ directory for detailed token tables."
	@echo ""

TARGET_BIN := $(TARGET)$(EXEEXT)
OBJ_CLEAN := $(subst /,\,$(OBJ))

clean:
	@if exist $(TARGET_BIN) del /f /q $(TARGET_BIN) >nul 2>&1
	@if exist $(TARGET) del /f /q $(TARGET) >nul 2>&1
	@if not "$(OBJ_CLEAN)"=="" del /f /q $(OBJ_CLEAN) >nul 2>&1
	@if exist output rmdir /s /q output >nul 2>&1
