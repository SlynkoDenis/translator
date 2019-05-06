#pragma once

enum lex_type
{
    OP, REG, NUM
};

enum opcodes
{
    MOVI = 0, ADD, SUB, MUL, DIV, IN, OUT, OPLAST
};

enum registers
{
    A = 0, B, C, D, RLAST
};

enum
{
    BITES_IN_CODE = 8, MAX_CHARS_IN_COMM = 5
};

struct args
{
    int act;          // An argument for defining the necessary operation (decoding -  "-d" or encoding - "-c")
    int st;           // A variable for indicating the position of translating command, if it comes from stdin
    char *fin;        // An argument for input stream, if information comes from a file (NULL if comes from stdin)
    char *fout;       // An argument for output stream (NULL if comes to stdout)
};

struct lex           // A structure of lexem, if the given information comes in an asm code
{
    struct lex *next;
    enum lex_type kind;
    union
    {
        enum opcodes op;
        enum registers reg;
        unsigned num;
    } inf;
};

void decode (FILE *fto, unsigned char cmd);           // A function that decodes hex code into asm code

int pow2 (int rt, int p);

void error_func (char *code);

void encode (FILE *fto, struct lex *head);            // A function that encodes asm code into hex code

struct args args_analysis (int argc, char **argv);    // A function that analyses the given instruction for the program

struct lex *lex_brake (char *ch);                     // A function that breaks given asm code into the list of lexems

unsigned put_bites_in_calc_comm (struct lex *head);
