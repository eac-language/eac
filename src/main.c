#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>

#include "common/token.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

// ===== COMMAND LINE OPTIONS =====

typedef enum {
    MODE_FULL,          // Lexer + Parser (default)
    MODE_LEX_ONLY,      // Lexer only
    MODE_PARSE_ONLY     // Parser only (assumes lexer works)
} CompilerMode;

typedef struct {
    CompilerMode mode;
    bool showTokens;
    bool verbose;
    const char* inputFile;
} CompilerOptions;

// ===== UTILITY FUNCTIONS =====

const char* getTokenSpecial(TokenType type) {
    switch (type) {
        case TOKEN_EOF:              return "EOF";
        case TOKEN_ERROR:            return "ERROR";
        case TOKEN_NEWLINE:          return "NEWLINE";
        case TOKEN_INDENT:           return "INDENT";
        case TOKEN_DEDENT:           return "DEDENT";
        case TOKEN_IDENTIFIER:       return "IDENTIFIER";
        case TOKEN_INTEGER:          return "INTEGER";
        case TOKEN_FLOAT:            return "FLOAT";
        case TOKEN_STRING:           return "STRING";
        case TOKEN_CHAR:             return "CHAR";
        case TOKEN_COMMENT_LINE:     return "COMMENT_LINE";
        case TOKEN_COMMENT_BLOCK:    return "COMMENT_BLOCK";
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
        case TOKEN_HINT_INT:         return "INT_TYPE";
        case TOKEN_HINT_FLOAT:       return "FLOAT_TYPE";
        case TOKEN_HINT_STR:         return "STR_TYPE";
        case TOKEN_HINT_BOOL:        return "BOOL_TYPE";
        case TOKEN_HINT_CHAR:        return "CHAR_TYPE";
        case TOKEN_AND:              return "AND";
        case TOKEN_OR:               return "OR";
        case TOKEN_NOT:              return "NOT";
        case TOKEN_AS:               return "AS";
        case TOKEN_OF:               return "OF";
        case TOKEN_TO:               return "TO";
        case TOKEN_THEN:             return "THEN";
        case TOKEN_EACH:             return "EACH";
        case TOKEN_PLUS:             return "PLUS";
        case TOKEN_MINUS:            return "MINUS";
        case TOKEN_STAR:             return "STAR";
        case TOKEN_SLASH:            return "SLASH";
        case TOKEN_FLOOR_DIV:        return "FLOOR_DIV";
        case TOKEN_PERCENT:          return "PERCENT";
        case TOKEN_CARET:            return "CARET";
        case TOKEN_VBAR:             return "VBAR";
        case TOKEN_LESS:             return "LESS";
        case TOKEN_GREATER:          return "GREATER";
        case TOKEN_EQUAL_EQUAL:      return "EQUAL_EQUAL";
        case TOKEN_LESS_EQUAL:       return "LESS_EQUAL";
        case TOKEN_GREATER_EQUAL:    return "GREATER_EQUAL";
        case TOKEN_BANG_EQUAL:       return "BANG_EQUAL";
        case TOKEN_EQUAL:            return "EQUAL";
        case TOKEN_PLUS_EQUAL:       return "PLUS_EQUAL";
        case TOKEN_MINUS_EQUAL:      return "MINUS_EQUAL";
        case TOKEN_STAR_EQUAL:       return "STAR_EQUAL";
        case TOKEN_SLASH_EQUAL:      return "SLASH_EQUAL";
        case TOKEN_PERCENT_EQUAL:    return "PERCENT_EQUAL";
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
    if (len < 4) return false;
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
    if (filename == NULL) filename = strrchr(path, '\\');
    return filename ? filename + 1 : path;
}

char* generateOutputFilename(const char* inputPath) {
    const char* filename = extractFilename(inputPath);
    size_t len = strlen(filename);
    
    if (!createDirectory("output")) return NULL;
    
    const char* prefix = "output/symbol_table_";
    size_t prefixLen = strlen(prefix);
    const char* suffix = ".txt";
    size_t suffixLen = strlen(suffix);

    char* output;
    if (len > 4 && strcmp(filename + len - 4, ".eac") == 0) {
        size_t stemLen = len - 4;
        output = (char*)malloc(prefixLen + stemLen + suffixLen + 1);
        if (output == NULL) return NULL;
        memcpy(output, prefix, prefixLen);
        memcpy(output + prefixLen, filename, stemLen);
        memcpy(output + prefixLen + stemLen, suffix, suffixLen);
        output[prefixLen + stemLen + suffixLen] = '\0';
    } else {
        output = (char*)malloc(prefixLen + len + suffixLen + 1);
        if (output == NULL) return NULL;
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
    } else if (token.type == TOKEN_INDENT) {
        snprintf(lexeme, sizeof(lexeme), "<increase indentation>");
    } else if (token.type == TOKEN_DEDENT) {
        snprintf(lexeme, sizeof(lexeme), "<decrease indentation>");
    } else if (token.type == TOKEN_EOF) {
        snprintf(lexeme, sizeof(lexeme), "<end of file>");
    } else if (token.length > 0 && token.length < 255) {
        snprintf(lexeme, sizeof(lexeme), "%.*s", token.length, token.lexeme);
    }
    
    fprintf(outFile, "%s\n", lexeme);
}

// ===== LEXER-ONLY MODE =====

int runLexerOnly(const char* sourcePath, bool verbose) {
    if (verbose) {
        printf("\n=== LEXICAL ANALYSIS MODE ===\n");
        printf("Source file: %s\n\n", sourcePath);
    }
    
    char* outputPath = generateOutputFilename(sourcePath);
    if (outputPath == NULL) return 1;
    
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
    fprintf(outFile, "==========================================================================\n\n");
    
    int tokenCount = 0;
    bool hasErrors = false;
    
    for (;;) {
        Token token = getNextToken(lexer);
        
        if (token.type == TOKEN_ERROR) {
            hasErrors = true;
            fprintf(stderr, "Lexical error on line %d: %s\n", token.line, token.lexeme);
            printToken(outFile, token);
            break;
        }
        
        if (token.type != TOKEN_COMMENT_LINE && 
            token.type != TOKEN_COMMENT_BLOCK &&
            token.type != TOKEN_NEWLINE &&
            token.type != TOKEN_INDENT &&
            token.type != TOKEN_DEDENT &&
            token.type != TOKEN_EOF) {
            tokenCount++;
        }
        
        printToken(outFile, token);
        
        if (token.type == TOKEN_EOF) break;
    }
    
    fprintf(outFile, "\n==========================================================================\n");
    fprintf(outFile, "Total tokens: %d\n", tokenCount);
    fprintf(outFile, "Status: %s\n", hasErrors ? "ERROR" : "SUCCESS");
    
    if (verbose) {
        printf("Tokens: %d\n", tokenCount);
        printf("Output: %s\n", outputPath);
        printf("Status: %s\n\n", hasErrors ? "FAILED" : "SUCCESS");
    }
    
    fclose(outFile);
    freeLexer(lexer);
    free(source);
    free(outputPath);
    
    return hasErrors ? 1 : 0;
}

// ===== FULL MODE (Lexer + Parser) =====

int runFullAnalysis(const char* sourcePath, bool verbose) {
    if (verbose) {
        printf("\n=== FULL ANALYSIS MODE (Lexer + Parser) ===\n");
        printf("Source file: %s\n\n", sourcePath);
    }
    
    // Step 1: Lexical Analysis
    char* source = readFile(sourcePath);
    if (source == NULL) return 1;
    
    Lexer* lexer = initLexer(source);
    if (lexer == NULL) {
        fprintf(stderr, "Error: Failed to initialize lexer.\n");
        free(source);
        return 1;
    }
    
    // Step 2: Syntax Analysis
    Parser* parser = initParser(lexer);
    if (parser == NULL) {
        fprintf(stderr, "Error: Failed to initialize parser.\n");
        freeLexer(lexer);
        free(source);
        return 1;
    }
    
    printf("Phase 1: Lexical Analysis... ");
    printf("[OK]\n");
    
    printf("Phase 2: Syntax Analysis... ");
    bool parseSuccess = parse(parser);
    
    if (parseSuccess) {
        printf("[OK]\n\n");
        
        if (verbose) {
            printf("=== ANALYSIS COMPLETE ===\n");
            printf("[PASS] Lexical analysis: PASSED\n");
            printf("[PASS] Syntax analysis: PASSED\n");
        }
    } else {
        printf("[FAIL]\n\n");
        if (verbose) {
            printf("=== ANALYSIS FAILED ===\n");
            printf("[FAIL] Syntax errors detected\n");
        }
    }
    
    freeParser(parser);
    freeLexer(lexer);
    free(source);
    
    return parseSuccess ? 0 : 1;
}

// ===== COMMAND LINE PARSING =====

void printUsage(const char* programName) {
    printf("EaC Compiler - Usage:\n\n");
    printf("  %s <file.eac>               - Full analysis (lexer + parser)\n", programName);
    printf("  %s --lex-only <file.eac>    - Lexical analysis only\n", programName);
    printf("  %s -v <file.eac>            - Verbose output\n", programName);
    printf("  %s -h                       - Show this help\n\n", programName);
    printf("Examples:\n");
    printf("  %s tests/test.eac\n", programName);
    printf("  %s --lex-only tests/test.eac\n", programName);
}

CompilerOptions parseCommandLine(int argc, char* argv[]) {
    CompilerOptions opts = {
        .mode = MODE_FULL,
        .showTokens = false,
        .verbose = false,
        .inputFile = NULL
    };
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--lex-only") == 0) {
            opts.mode = MODE_LEX_ONLY;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            opts.verbose = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            return opts; // inputFile will be NULL
        } else if (argv[i][0] != '-') {
            opts.inputFile = argv[i];
        }
    }
    
    return opts;
}

// ===== MAIN =====

int main(int argc, char* argv[]) {
    printf("\n");
    printf("+======================================================================+\n");
    printf("|                    EaC Programming Language                        |\n");
    printf("|                   Compiler - Lexer + Parser                        |\n");
    printf("+======================================================================+\n");
    
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    CompilerOptions opts = parseCommandLine(argc, argv);
    
    if (opts.inputFile == NULL) {
        printUsage(argv[0]);
        return 0;
    }
    
    // Validate file extension
    if (!hasEacExtension(opts.inputFile)) {
        fprintf(stderr, "Error: Source file must have .eac extension.\n");
        return 1;
    }
    
    int result = 0;
    
    switch (opts.mode) {
        case MODE_LEX_ONLY:
            result = runLexerOnly(opts.inputFile, opts.verbose);
            break;
            
        case MODE_FULL:
        case MODE_PARSE_ONLY:
            result = runFullAnalysis(opts.inputFile, opts.verbose);
            break;
            
        default:
            fprintf(stderr, "Error: Invalid mode\n");
            return 1;
    }
    
    printf("\n");
    printf("+======================================================================+\n");
    if (result == 0) {
        printf("|                      [PASS] COMPILATION SUCCESS                    |\n");
    } else {
        printf("|                      [FAIL] COMPILATION FAILED                     |\n");
    }
    printf("+======================================================================+\n");
    printf("\n");
    
    return result;
}