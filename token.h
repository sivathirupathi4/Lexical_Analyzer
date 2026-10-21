#ifndef TOKEN_H
#define TOKEN_H

typedef enum
{
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_OPERATOR,
    TOKEN_STRING,
    TOKEN_CHARACTER,
    TOKEN_SPECIAL,
    TOKEN_COMMENT,
    TOKEN_PREPROCESSOR,
    TOKEN_UNKNOWN
} TokenType;

typedef struct
{
    TokenType type;
    char lexeme[100];
    int line_number;
} Token;

#endif