#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>

#include "common/token.h"
#include "lexer/lexer.h"

const char* getTokenSpecial(TokenType type) {
    switch (type) {
        case TOKEN_EOF:              return "EOF";
        case TOKEN_ERROR:            return "ERROR";
        
        case TOKEN_NEWLINE:          return "NEWLINE";
        
        // Literals
        case TOKEN_IDENTIFIER:       return "IDENTIFIER";
        case TOKEN_INTEGER:          return "INTEGER";
        case TOKEN_FLOAT:            return "FLOAT";
        case TOKEN_STRING:           return "STRING";
        case TOKEN_CHAR:             return "CHAR";
        case TOKEN_COMMENT_LINE:     return "COMMENT_LINE";
        case TOKEN_COMMENT_BLOCK:    return "COMMENT_BLOCK";
        
        // Primary Keywords
        case TOKEN_FLEX:             return "FLEX";
        case TOKEN_FIXED:            return "FIXED";
        case TOKEN_WHEN:             return "WHEN";
        case TOKEN_ELSE:             return "ELSE";
        case TOKEN_OUTPUT:           return "OUTPUT";
        case TOKEN_WHILE:            return "WHILE";
        case TOKEN_FOR:              return "FOR";
        case TOKEN_IN:               return "IN";
        case TOKEN_BREAK:            return "BREAK";
        case TOKEN_CONTINUE:         return "CONTINUE";
        case TOKEN_RETURN:           return "RETURN";
        case TOKEN_FUNCTION:         return "FUNCTION";
        case TOKEN_IMPORT:           return "IMPORT";
        case TOKEN_FROM:             return "FROM";
        case TOKEN_TRUE:             return "TRUE";
        case TOKEN_FALSE:            return "FALSE";
        case TOKEN_INPUT:            return "INPUT";
        
        // Type Hint Keywords
        case TOKEN_HINT_INT:         return "INT_TYPE";
        case TOKEN_HINT_FLOAT:       return "FLOAT_TYPE";
        case TOKEN_HINT_STR:         return "STR_TYPE";
        case TOKEN_HINT_BOOL:        return "BOOL_TYPE";
        case TOKEN_HINT_CHAR:        return "CHAR_TYPE";
        
        // Logical Operators
        case TOKEN_AND:              return "AND";
        case TOKEN_OR:               return "OR";
        case TOKEN_NOT:              return "NOT";
        
        // Noise Words
        case TOKEN_AS:               return "NOISE";
        case TOKEN_OF:               return "NOISE";
        case TOKEN_TO:               return "NOISE";
        case TOKEN_THEN:             return "NOISE";
        case TOKEN_EACH:             return "NOISE";
        
        // Arithmetic Operators
        case TOKEN_PLUS:             return "PLUS";
        case TOKEN_MINUS:            return "MINUS";
        case TOKEN_STAR:             return "STAR";
        case TOKEN_SLASH:            return "SLASH";
        case TOKEN_FLOOR_DIV:        return "FLOOR_DIV";
        case TOKEN_PERCENT:          return "PERCENT";
        case TOKEN_CARET:            return "CARET";
        case TOKEN_VBAR:             return "VBAR";
        
        // Relational & Equality
        case TOKEN_LESS:             return "LESS";
        case TOKEN_GREATER:          return "GREATER";
        case TOKEN_EQUAL_EQUAL:      return "EQUAL_EQUAL";
        case TOKEN_LESS_EQUAL:       return "LESS_EQUAL";
        case TOKEN_GREATER_EQUAL:    return "GREATER_EQUAL";
        case TOKEN_BANG_EQUAL:       return "BANG_EQUAL";
        
        // Assignment Operators
        case TOKEN_EQUAL:            return "EQUAL";
        case TOKEN_PLUS_EQUAL:       return "PLUS_EQUAL";
        case TOKEN_MINUS_EQUAL:      return "MINUS_EQUAL";
        case TOKEN_STAR_EQUAL:       return "STAR_EQUAL";
        case TOKEN_SLASH_EQUAL:      return "SLASH_EQUAL";
        case TOKEN_PERCENT_EQUAL:    return "PERCENT_EQUAL";
        
        // Delimiters
        case TOKEN_LPAREN:           return "LPAREN";
        case TOKEN_RPAREN:           return "RPAREN";
        case TOKEN_LBRACKET:         return "LBRACKET";
        case TOKEN_RBRACKET:         return "RBRACKET";
        case TOKEN_COLON:            return "COLON";
        case TOKEN_COMMA:            return "COMMA";
        case TOKEN_DOT:              return "DOT";
        
        default:                     return "UNKNOWN";
    }
}

static bool hasEacExtension(const char* path) {
    size_t len = strlen(path);
    if (len < 4) {
        return false;
    }

    const char* ext = path + len - 4;
    return ext[0] == '.' &&
           tolower((unsigned char)ext[1]) == 'e' &&
           tolower((unsigned char)ext[2]) == 'a' &&
           tolower((unsigned char)ext[3]) == 'c';
}

char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s'.\n", path);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);
    
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Not enough memory to read '%s'.\n", path);
        fclose(file);
        return NULL;
    }
    
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < (size_t)fileSize) {
        fprintf(stderr, "Error: Could not read file '%s'.\n", path);
        free(buffer);
        fclose(file);
        return NULL;
    }
    
    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

bool createDirectory(const char* path) {
#ifdef _WIN32
    if (mkdir(path) != 0 && errno != EEXIST) {
#else
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
#endif
        fprintf(stderr, "Error: Could not create directory '%s'.\n", path);
        return false;
    }
    return true;
}


const char* extractFilename(const char* path) {
    const char* filename = strrchr(path, '/');
    if (filename == NULL) {
        filename = strrchr(path, '\\');
    }
    return filename ? filename + 1 : path;
}

char* generateOutputFilename(const char* inputPath) {
    const char* filename = extractFilename(inputPath);
    size_t len = strlen(filename);
    
    // Create output directory
    if (!createDirectory("output")) {
        return NULL;
    }
    
    const char* prefix = "output/symbol_table_";
    size_t prefixLen = strlen(prefix);
    const char* suffix = ".txt";
    size_t suffixLen = strlen(suffix);

    char* output;
    if (len > 4 && strcmp(filename + len - 4, ".eac") == 0) {
        size_t stemLen = len - 4; // exclude .eac
        output = (char*)malloc(prefixLen + stemLen + suffixLen + 1);
        if (output == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return NULL;
        }
        memcpy(output, prefix, prefixLen);
        memcpy(output + prefixLen, filename, stemLen);
        memcpy(output + prefixLen + stemLen, suffix, suffixLen);
        output[prefixLen + stemLen + suffixLen] = '\0';
    } else {
        output = (char*)malloc(prefixLen + len + suffixLen + 1);
        if (output == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return NULL;
        }
        memcpy(output, prefix, prefixLen);
        memcpy(output + prefixLen, filename, len);
        memcpy(output + prefixLen + len, suffix, suffixLen);
        output[prefixLen + len + suffixLen] = '\0';
    }

    return output;
}

void printToken(FILE* outFile, Token token) {
    const char* special = getTokenSpecial(token.type);
    fprintf(outFile, "%-30s", special);
    
    char lexeme[256] = {0};
    
    if (token.type == TOKEN_NEWLINE) {
        snprintf(lexeme, sizeof(lexeme), "\\n");
    } else if ((token.type == TOKEN_COMMENT_LINE || token.type == TOKEN_COMMENT_BLOCK) &&
               token.length > 0) {
        int maxCopy = token.length < (int)sizeof(lexeme) - 1 ? token.length : (int)sizeof(lexeme) - 1;
        int j = 0;
        for (int i = 0; i < maxCopy; i++) {
            char ch = token.lexeme[i];
            if (ch == '\r' || ch == '\n' || ch == '\t') {
                ch = ' ';
            }
            lexeme[j++] = ch;
        }
        lexeme[j] = '\0';
    } else if (token.type == TOKEN_STRING && token.length > 0) {
        int maxCopy = token.length < (int)sizeof(lexeme) - 1 ? token.length : (int)sizeof(lexeme) - 1;
        snprintf(lexeme, sizeof(lexeme), "%.*s", maxCopy, token.lexeme);
    } else if (token.type == TOKEN_CHAR && token.length > 0) {
        int maxCopy = token.length < (int)sizeof(lexeme) - 1 ? token.length : (int)sizeof(lexeme) - 1;
        snprintf(lexeme, sizeof(lexeme), "%.*s", maxCopy, token.lexeme);
    } else if (token.length > 0 && token.length < 255) {
        snprintf(lexeme, sizeof(lexeme), "%.*s", token.length, token.lexeme);
    } else if (token.type == TOKEN_ERROR) {
        snprintf(lexeme, sizeof(lexeme), "%s", token.lexeme);
    }
    
    fprintf(outFile, "%s\n", lexeme);
}

// ===== Main Program =====

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source-file.eac>\n", argv[0]);
        return 1;
    }
    
    const char* sourcePath = argv[1];

    if (!hasEacExtension(sourcePath)) {
        fprintf(stderr, "Error: Source file '%s' must have a .eac extension.\n", sourcePath);
        return 1;
    }
    
    if (!createDirectory("output")) {
        return 1;
    }
    
    char* outputPath = generateOutputFilename(sourcePath);
    if (outputPath == NULL) {
        return 1;
    }
    
    char* source = readFile(sourcePath);
    if (source == NULL) {
        free(outputPath);
        return 1;
    }
    
    Lexer* lexer = initLexer(source);
    if (lexer == NULL) {
        fprintf(stderr, "Error: Failed to initialize lexer.\n");
        free(source);
        free(outputPath);
        return 1;
    }
    
    FILE* outFile = fopen(outputPath, "w");
    if (outFile == NULL) {
        fprintf(stderr, "Error: Could not create output file '%s'.\n", outputPath);
        freeLexer(lexer);
        free(source);
        free(outputPath);
        return 1;
    }
    
    fprintf(outFile, "EaC Lexer Output\n");
    fprintf(outFile, "Source: %s\n", sourcePath);
    fprintf(outFile, "==========================================================================\n");
    fprintf(outFile, "Token                          Lexeme\n");
    fprintf(outFile, "==========================================================================\n");
    fprintf(outFile, "\n");
    
    int tokenCount = 0;
    bool hasErrors = false;
    Token lastErrorToken;
    
    for (;;) {
        Token token = getNextToken(lexer);
        
        if (token.type == TOKEN_ERROR) {
            hasErrors = true;
            lastErrorToken = token;
            fprintf(stderr, "Lexical error on line %d: %s\n", token.line, token.lexeme);
            printToken(outFile, token);
            continue;
        }
        
        if (token.type == TOKEN_EOF) {
            printToken(outFile, token);
            break;
        }
        
        printToken(outFile, token);
        tokenCount++;
    }
    
    fprintf(outFile, "\n");
    fprintf(outFile, "==========================================================================\n");
    fprintf(outFile, "Total tokens: %d\n", tokenCount);
    if (hasErrors) {
        fprintf(outFile, "Status: ERROR - Lexical analysis failed\n");
    } else {
        fprintf(outFile, "Status: SUCCESS - All tokens recognized\n");
    }
    
    printf("\n");
    printf("==========================================================================\n");
    printf("EaC Lexer\n");
    printf("==========================================================================\n");
    printf("Source file:  %s\n", sourcePath);
    printf("Output file:  %s\n", outputPath);
    printf("Total tokens: %d\n", tokenCount);
    
    if (hasErrors) {
        printf("Status:       FAILED\n");
        printf("Error:        Line %d - %s\n", lastErrorToken.line, lastErrorToken.lexeme);
    } else {
        printf("Status:       SUCCESS\n");
    }
    printf("==========================================================================\n");
    
    fclose(outFile);
    freeLexer(lexer);
    free(source);
    free(outputPath);
    
    return hasErrors ? 1 : 0;
}