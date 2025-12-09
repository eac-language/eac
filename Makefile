CC = gcc
CFLAGS = -Isrc -Wall -Wextra -std=c11 -g
LDFLAGS =

SRC = src/main.c src/lexer/lexer.c src/parser/ast.c src/parser/parser.c
OBJ = $(SRC:.c=.o)

TARGET = eac

.PHONY: all clean test-lexer test-parser

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test-lexer: $(TARGET)
	@echo "Running Lexer Test..."
	@./$(TARGET) --lexer tests/demo/test_lexer.eac

test-parser: $(TARGET)
	@echo "Running Parser Test..."
	@./$(TARGET) --parse tests/demo/test_main.eac

clean:
	rm -f $(TARGET) $(TARGET).exe $(OBJ)
	rm -rf output