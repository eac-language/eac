CC = gcc
CFLAGS = -Isrc -Wall -Wextra -std=c11 -g
LDFLAGS =

SRC = src/main.c src/lexer/lexer.c src/parser/ast.c src/parser/parser.c
OBJ = $(SRC:.c=.o)

TARGET = eac
TEST_FILE = tests/demo/test_main.eac

.PHONY: all clean test-lexer test-parser test-all

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test-lexer: $(TARGET)
	@echo "Running Lexer Test..."
	@./$(TARGET) $(TEST_FILE)

test-parser: $(TARGET)
	@echo "Running Parser Test..."
	@./$(TARGET) --parse $(TEST_FILE)

test-all: test-lexer test-parser
	@echo "All tests completed!"

clean:
ifeq ($(OS),Windows_NT)
	@if exist $(TARGET).exe del /Q $(TARGET).exe 2>nul
	@if exist src\main.o del /Q src\main.o 2>nul
	@if exist src\lexer\lexer.o del /Q src\lexer\lexer.o 2>nul
	@if exist src\parser\ast.o del /Q src\parser\ast.o 2>nul
	@if exist src\parser\parser.o del /Q src\parser\parser.o 2>nul
	@if exist output rmdir /S /Q output 2>nul
	@echo Clean completed.
else
	rm -f $(TARGET) $(TARGET).exe $(OBJ)
	rm -rf output
	@echo Clean completed.
endif