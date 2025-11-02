CC = gcc
CFLAGS = -Isrc -Wall -Wextra -std=c11 -g
LDFLAGS =

SRC = src/main.c src/lexer/lexer.c
OBJ = $(SRC:.c=.o)

TARGET = eac

.PHONY: all clean test test-all

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	@echo "Running comprehensive test..."
	@./$(TARGET) tests/test.eac

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

clean:
	rm -f $(TARGET) $(OBJ)
