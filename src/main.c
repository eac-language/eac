#include <stdio.h>
#include <stdlib.h>

#include "src/common/token.h"
#include "src/lexer/lexer.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source file>\n", argv[0]);
        return 1;
    }

    char *source_path = argv[1];
    FILE *file = fopen(source_path, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *source_code = (char *)malloc(length + 1);
    if (!source_code) {
        fprintf(stderr, "Error allocating memory for source code\n");
        fclose(file);
        return 1;
    }

    fread(source_code, 1, length, file);
    source_code[length] = '\0';
    fclose(file);

    initLexer(source_code);

    int line = -1;
    for (;;) {
        Token token = scanToken();
        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        } else {
            printf("   | ");
        }
        printf("%2d '%.*s'\n", token.type, token.length, token.start);

        if (token.type == TOKEN_EOF) {
            break;
        }
    }

    free(source_code);

    return 0;
}

