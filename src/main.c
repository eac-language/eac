/**
 * main.c - Test Harness for EaC Lexer
 * 
 * This program reads an EaC source file and prints all tokens
 * produced by the lexer, demonstrating its functionality.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "common/token.h"
#include "lexer/lexer.h"

// ===== Token Type Name Mapping =====

/**
 * getTokenName - Returns a human-readable name for a token type
 */
const char* getTokenName(TokenType type) {
    switch (type) {
        // Lifecycle
        case TOKEN_EOF:              return "EOF";
        case TOKEN_ERROR:            return "ERROR";
        
        // Whitespace & Structural
        case TOKEN_NEWLINE:          return "NEWLINE";
        case TOKEN_INDENT:           return "INDENT";
        case TOKEN_DEDENT:           return "DEDENT";
        
        // Literals
        case TOKEN_IDENTIFIER:       return "IDENTIFIER";
        case TOKEN_INTEGER:          return "INTEGER";
        case TOKEN_FLOAT:            return "FLOAT";
        case TOKEN_STRING:           return "STRING";
        
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
        case TOKEN_TRUE:             return "TRUE";
        case TOKEN_FALSE:            return "FALSE";
        
        // Type Hint Keywords
        case TOKEN_HINT_INT:         return "HINT_INT";
        case TOKEN_HINT_FLOAT:       return "HINT_FLOAT";
        case TOKEN_HINT_STR:         return "HINT_STR";
        case TOKEN_HINT_BOOL:        return "HINT_BOOL";
        
        // Arithmetic Operators
        case TOKEN_PLUS:             return "PLUS";
        case TOKEN_MINUS:            return "MINUS";
        case TOKEN_STAR:             return "STAR";
        case TOKEN_SLASH:            return "SLASH";
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
        
        // Logical Operators
        case TOKEN_AND:              return "AND";
        case TOKEN_OR:               return "OR";
        case TOKEN_NOT:              return "NOT";
        
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

// ===== File Reading Utility =====

/**
 * readFile - Reads entire file contents into a dynamically allocated string
 * 
 * @param path      Path to the file
 * @return          Pointer to file contents (caller must free), or NULL on error
 */
char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s'.\n", path);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);
    
    // Allocate buffer
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Not enough memory to read '%s'.\n", path);
        fclose(file);
        return NULL;
    }
    
    // Read file
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

// ===== Token Printer =====

/**
 * printToken - Prints a token in a human-readable format
 */
void printToken(Token token) {
    printf("Line %4d | %-18s", token.line, getTokenName(token.type));
    
    if (token.length > 0) {
        // Print lexeme for tokens with actual text
        switch (token.type) {
            case TOKEN_NEWLINE:
            case TOKEN_INDENT:
            case TOKEN_DEDENT:
                // Don't print lexeme for structural tokens
                printf("\n");
                break;
            case TOKEN_ERROR:
                // For errors, lexeme is the error message
                printf(" | Error: %.*s\n", token.length, token.lexeme);
                break;
            default:
                printf(" | '%.*s'\n", token.length, token.lexeme);
                break;
        }
    } else {
        printf("\n");
    }
}

// ===== Main Program =====

int main(int argc, char* argv[]) {
    // Print header
    printf("===============================================\n");
    printf("    EaC Lexical Analyzer - Test Harness\n");
    printf("===============================================\n\n");
    
    // Check arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source-file.eac>\n", argv[0]);
        return 1;
    }
    
    const char* sourcePath = argv[1];
    printf("Reading source file: %s\n\n", sourcePath);
    
    // Read source file
    char* source = readFile(sourcePath);
    if (source == NULL) {
        return 1;
    }
    
    // Initialize lexer
    Lexer* lexer = initLexer(source);
    if (lexer == NULL) {
        fprintf(stderr, "Error: Failed to initialize lexer.\n");
        free(source);
        return 1;
    }
    
    // Print tokens header
    printf("Tokens:\n");
    printf("-----------------------------------------------\n");
    
    // Scan and print all tokens
    int tokenCount = 0;
    bool hasErrors = false;
    
    for (;;) {
        Token token = getNextToken(lexer);
        printToken(token);
        tokenCount++;
        
        if (token.type == TOKEN_ERROR) {
            hasErrors = true;
        }
        
        if (token.type == TOKEN_EOF) {
            break;
        }
    }
    
    // Print summary
    printf("-----------------------------------------------\n");
    printf("Total tokens: %d\n", tokenCount);
    
    if (hasErrors) {
        printf("Status: LEXICAL ERRORS DETECTED\n");
    } else {
        printf("Status: SUCCESS\n");
    }
    
    printf("===============================================\n");
    
    // Cleanup
    freeLexer(lexer);
    free(source);
    
    return hasErrors ? 1 : 0;
}
