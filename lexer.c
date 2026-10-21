#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"
#include "token.h"

const char *keywords[] =
{
    "auto",
    "break",
    "case",
    "char",
    "const",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "float",
    "for",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "register",
    "restrict",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while"
};

#define KEYWORD_COUNT (sizeof(keywords) / sizeof(keywords[0]))

int is_keyword(const char *word)
{
    int i;

    for (i = 0; i < KEYWORD_COUNT; i++)
    {
        if (strcmp(word, keywords[i]) == 0)
        {
            return 1;
        }
    }

    return 0;
}

const char *token_type_string(TokenType type)
{
    switch (type)
    {
        case TOKEN_KEYWORD:
            return "KEYWORD";

        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";

        case TOKEN_INTEGER:
            return "INTEGER";

        case TOKEN_FLOAT:
            return "FLOAT";

        case TOKEN_OPERATOR:
            return "OPERATOR";

        case TOKEN_STRING:
            return "STRING";

        case TOKEN_CHARACTER:
            return "CHARACTER";

        case TOKEN_SPECIAL:
            return "SPECIAL";

        case TOKEN_COMMENT:
            return "COMMENT";

        case TOKEN_PREPROCESSOR:
            return "PREPROCESSOR";

        case TOKEN_UNKNOWN:
            return "UNKNOWN";

        default:
            return "UNKNOWN";
    }
}

void print_token(TokenType type, const char *lexeme, int line)
{
    printf("%-5d %-15s %s\n",
           line,
           token_type_string(type),
           lexeme);
}

void process_identifier(FILE *fp, int first_char, int *line)
{
    char buffer[100];
    int index = 0;
    int ch;

    buffer[index++] = first_char;

    while ((ch = fgetc(fp)) != EOF)
    {
        if (isalnum(ch) || ch == '_')
        {
            if (index < 99)
            {
                buffer[index++] = ch;
            }
        }
        else
        {
            ungetc(ch, fp);
            break;
        }
    }

    buffer[index] = '\0';

    if (is_keyword(buffer))
    {
        print_token(TOKEN_KEYWORD, buffer, *line);
    }
    else
    {
        print_token(TOKEN_IDENTIFIER, buffer, *line);
    }
}

void process_number(FILE *fp, int first_char, int *line)
{
    char buffer[100];
    int index = 0;
    int ch;
    int is_float = 0;

    buffer[index++] = first_char;

    while ((ch = fgetc(fp)) != EOF)
    {
        if (isdigit(ch))
        {
            if (index < 99)
            {
                buffer[index++] = ch;
            }
        }
        else if (ch == '.')
        {
            if (is_float)
            {
                ungetc(ch, fp);
                break;
            }

            is_float = 1;

            if (index < 99)
            {
                buffer[index++] = ch;
            }
        }
        else
        {
            ungetc(ch, fp);
            break;
        }
    }

    buffer[index] = '\0';

    if (is_float)
    {
        print_token(TOKEN_FLOAT, buffer, *line);
    }
    else
    {
        print_token(TOKEN_INTEGER, buffer, *line);
    }
}

void process_string(FILE *fp, int *line)
{
    char buffer[100];
    int index = 0;
    int ch;

    buffer[index++] = '"';

    while ((ch = fgetc(fp)) != EOF)
    {
        if (index < 98)
        {
            buffer[index++] = ch;
        }

        if (ch == '\\')
        {
            ch = fgetc(fp);

            if (ch == EOF)
            {
                break;
            }

            if (index < 98)
            {
                buffer[index++] = ch;
            }
        }
        else if (ch == '"')
        {
            break;
        }
        else if (ch == '\n')
        {
            (*line)++;
        }
    }

    buffer[index] = '\0';

    print_token(TOKEN_STRING, buffer, *line);
}

void process_character(FILE *fp, int *line)
{
    char buffer[100];
    int index = 0;
    int ch;

    buffer[index++] = '\'';

    while ((ch = fgetc(fp)) != EOF)
    {
        if (index < 98)
        {
            buffer[index++] = ch;
        }

        if (ch == '\\')
        {
            ch = fgetc(fp);

            if (ch == EOF)
            {
                break;
            }

            if (index < 98)
            {
                buffer[index++] = ch;
            }
        }
        else if (ch == '\'')
        {
            break;
        }
        else if (ch == '\n')
        {
            (*line)++;
            break;
        }
    }

    buffer[index] = '\0';

    print_token(TOKEN_CHARACTER, buffer, *line);
}

void process_comment(FILE *fp, int *line)
{
    char buffer[100];
    int index = 0;
    int ch;
    int next;

    buffer[index++] = '/';

    next = fgetc(fp);

    if (next == '/')
    {
        buffer[index++] = '/';

        while ((ch = fgetc(fp)) != EOF)
        {
            if (ch == '\n')
            {
                (*line)++;
                break;
            }

            if (index < 99)
            {
                buffer[index++] = ch;
            }
        }
    }
    else if (next == '*')
    {
        buffer[index++] = '*';

        while ((ch = fgetc(fp)) != EOF)
        {
            if (ch == '\n')
            {
                (*line)++;
            }

            if (index < 98)
            {
                buffer[index++] = ch;
            }

            if (ch == '*')
            {
                next = fgetc(fp);

                if (next == '/')
                {
                    if (index < 99)
                    {
                        buffer[index++] = '/';
                    }

                    break;
                }
                else
                {
                    if (next != EOF)
                    {
                        ungetc(next, fp);
                    }
                }
            }
        }
    }
    else
    {
        if (next != EOF)
        {
            ungetc(next, fp);
        }

        buffer[index] = '\0';

        print_token(TOKEN_OPERATOR, buffer, *line);
        return;
    }

    buffer[index] = '\0';

    print_token(TOKEN_COMMENT, buffer, *line);
}

void process_operator(FILE *fp, int first_char, int *line)
{
    char buffer[3];
    int index = 0;
    int ch;

    buffer[index++] = first_char;

    ch = fgetc(fp);

    switch (first_char)
    {
        case '=':
            if (ch == '=')
            {
                buffer[index++] = ch;
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, fp);
            }
            break;

        case '!':
            if (ch == '=')
            {
                buffer[index++] = ch;
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, fp);
            }
            break;

        case '<':
            if (ch == '=' || ch == '<')
            {
                buffer[index++] = ch;
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, fp);
            }
            break;

        case '>':
            if (ch == '=' || ch == '>')
            {
                buffer[index++] = ch;
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, fp);
            }
            break;

        case '+':
            if (ch == '+' || ch == '=')
            {
                buffer[index++] = ch;
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, fp);
            }
            break;

        case '-':
            if (ch == '-' || ch == '=')
            {
                buffer[index++] = ch;
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, fp);
            }
            break;

        case '*':
            if (ch == '=')
            {
                buffer[index++] = ch;
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, fp);
            }
            break;

        case '%':
            if (ch == '=')
            {
                buffer[index++] = ch;
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, fp);
            }
            break;

        case '&':
            if (ch == '&' || ch == '=')
            {
                buffer[index++] = ch;
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, fp);
            }
            break;

        case '|':
            if (ch == '|' || ch == '=')
            {
                buffer[index++] = ch;
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, fp);
            }
            break;

        case '^':
            if (ch == '=')
            {
                buffer[index++] = ch;
            }
            else
            {
                if (ch != EOF)
                    ungetc(ch, fp);
            }
            break;

        default:
            if (ch != EOF)
            {
                ungetc(ch, fp);
            }
            break;
    }

    buffer[index] = '\0';

    print_token(TOKEN_OPERATOR, buffer, *line);
}

void process_preprocessor(FILE *fp, int *line)
{
    char buffer[100];
    int index = 0;
    int ch;

    buffer[index++] = '#';

    while ((ch = fgetc(fp)) != EOF)
    {
        if (ch == '\n')
        {
            (*line)++;
            break;
        }

        if (index < 99)
        {
            buffer[index++] = ch;
        }
    }

    buffer[index] = '\0';

    print_token(TOKEN_PREPROCESSOR, buffer, *line);
}

int is_special(int ch)
{
    switch (ch)
    {
        case ';':
        case ',':
        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case ':':
        case '?':
        case '.':
            return 1;

        default:
            return 0;
    }
}

int is_operator(int ch)
{
    switch (ch)
    {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '=':
        case '<':
        case '>':
        case '!':
        case '&':
        case '|':
        case '^':
        case '~':
            return 1;

        default:
            return 0;
    }
}

void lexical_analyzer(FILE *fp)
{
    int ch;
    int line = 1;

    printf("\n");
    printf("------------------------------------------------------------\n");
    printf("                    LEXICAL ANALYZER\n");
    printf("------------------------------------------------------------\n");
    printf("%-5s %-15s %s\n", "LINE", "TOKEN", "LEXEME");
    printf("------------------------------------------------------------\n");

    while ((ch = fgetc(fp)) != EOF)
    {
        if (ch == ' ' || ch == '\t' || ch == '\r')
        {
            continue;
        }
        if (ch == '\n')
        {
            line++;
            continue;
        }
        if (isalpha(ch) || ch == '_')
        {
            process_identifier(fp, ch, &line);
            continue;
        }
        if (isdigit(ch))
        {
            process_number(fp, ch, &line);
            continue;
        }
        if (ch == '"')
        {
            process_string(fp, &line);
            continue;
        }
        if (ch == '\'')
        {
            process_character(fp, &line);
            continue;
        }
        if (ch == '/')
        {
            process_comment(fp, &line);
            continue;
        }
        if (ch == '#')
        {
            process_preprocessor(fp, &line);
            continue;
        }
        if (is_operator(ch))
        {
            process_operator(fp, ch, &line);
            continue;
        }
        if (is_special(ch))
        {
            char buffer[2];

            buffer[0] = ch;
            buffer[1] = '\0';

            print_token(TOKEN_SPECIAL, buffer, line);
            continue;
        }
        {
            char buffer[2];

            buffer[0] = ch;
            buffer[1] = '\0';

            print_token(TOKEN_UNKNOWN, buffer, line);
        }
    }

    printf("------------------------------------------------------------\n");
}