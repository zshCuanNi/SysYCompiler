#ifndef TOKEN_H
#define TOKEN_H
#define YYSTYPE char *
typedef enum {
    T_EQ=256, T_LEQ, T_GEQ, T_NEQ, T_AND, T_OR, T_VOID, T_INT, T_IF, T_ELSE, T_WHILE, T_BREAK, T_CONTINUE, T_RETURN, T_CONST, T_IDENTIFIER, T_DECIMAL_CONST, T_OCTAL_CONST, T_HEXADECIMAL_CONST
} TokenType;

static void print_token(int token) {
    static char* token_strs[] = {
        "T_EQ", "T_LEQ", "T_GEQ", "T_NEQ", "T_AND", "T_OR", "T_VOID", "T_INT", "T_IF", "T_ELSE", "T_WHILE", "T_BREAK", "T_CONTINUE", "T_RETURN", "T_CONST", "T_IDENTIFIER", "T_DECIMAL_CONST", "T_OCTAL_CONST", "T_HEXADECIMAL_CONST"
    };

    if (token < 256) {
        printf("%-20c", token);
    } else {
        printf("%-20s", token_strs[token-256]);
    }
}

#endif