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
	@echo "=== Running All Test Cases ==="
	@echo ""
	@echo "[1] Comprehensive Test:"
	@./$(TARGET) tests/test.eac
	@echo ""
	@echo "[2] Indentation Test:"
	@./$(TARGET) tests/test_indentation.eac
	@echo ""
	@echo "[3] Operators Test:"
	@./$(TARGET) tests/test_operators.eac
	@echo ""
	@echo "[4] Keywords Test:"
	@./$(TARGET) tests/test_keywords.eac
	@echo ""
	@echo "[5] Comments Test:"
@./$(TARGET) tests/test_comments.eac
	@echo ""
	@echo "[6] Literals Test:"
	@./$(TARGET) tests/test_literals.eac
@echo ""
@echo "[7] Invalid Tokens Test:"
@./$(TARGET) tests/test_invalid.eac || true

clean:
	rm -f $(TARGET) $(OBJ)
