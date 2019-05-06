#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "encoder.h"

void decode (FILE *fto, unsigned char cmd)
{
    if ((cmd & (1 << (BITES_IN_CODE - 1))) == 0)
    {
        fprintf (fto, "MOVI %u\n", cmd);

        return ;
    }

    if ((cmd & (1 << (BITES_IN_CODE - 2))) == 0)
    {
        switch (cmd >> 4)
        {
            case 8:
                fprintf (fto, "ADD ");
                break;
            case 9:
                fprintf (fto, "SUB ");
                break;
            case 10:
                fprintf (fto, "MUL ");
                break;
            case 11:
                fprintf (fto, "DIV ");
                break;
        }

        fprintf (fto, "%c, %c\n", ((cmd >> 2) & 3) + 65, (cmd & 3) + 65);

        return ;
    }

    if ((cmd & 0xfc) == 0xc0)
    {
        fprintf (fto, "IN %c\n", (cmd & 3) + 65);

        return ;
    }
    if ((cmd & 0xfc) == 0xc4)
    {
        fprintf (fto, "OUT %c\n", (cmd & 3) + 65);

        return ;
    }

    error_func ("opcode");
}

//-----------------------------------------------------------------------------------------

int pow2 (int rt, int p)
{
    assert (p >= 0);

    int res = rt;
    if (p > 0)
    {
        for (int i = 1; i < p; i++)
            res *= rt;

        return res;
    }

    return 1;
}

//-----------------------------------------------------------------------------------------

void error_func (char *code)
{
    assert (code);

    fprintf (stderr, "Error: incorrect %s\n", code);
    abort ();
}

//-----------------------------------------------------------------------------------------

struct args args_analysis (int argc, char **argv)
{
    assert (argc >= 3);

    struct args res = {-1, -1, NULL, NULL};

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp (argv[i], "-d"))
            if ((res.act == -1) && (i + 1 < argc))
            {
                res.act = 1;
                res.st = i + 1;
            }
            else
                error_func ("commands using in arguments");

        else if (!strcmp (argv[i], "-c"))
            if ((res.act == -1) && (i + 1 < argc))
            {
                res.act = 0;
                res.st = i + 1;
            }
            else
                error_func ("commands using in arguments");

        else if (!strcmp (argv[i], "-f"))
            if ((res.fin == NULL) && (i + 1 < argc))
            {
                i++;
                res.fin = argv[i];
            }
            else
                error_func ("commands using in arguments");

        else if (!strcmp (argv[i], "-o"))
            if ((res.fout == NULL) && (i + 1 < argc))
            {
                i++;
                res.fout = argv[i];
            }
            else
                error_func ("commands using in arguments");
    }

    if ((res.act == -1) || (res.st == -1))
        error_func ("commands using in arguments");

    return res;
};

struct lex *lex_brake (char *ch)
{
    assert (ch);

    struct lex *res = (struct lex *) calloc (1, sizeof(struct lex));
    assert (res);

    res->kind = -1;
    res->next = NULL;

    char *tmp = ch;
    while (tmp)
    {
        if (isspace (*tmp))
        {
            tmp++;
            continue;
        }

        if (ispunct (*tmp))
        {
            tmp++;
            continue;
        }

        if (isalpha (*tmp))
        {
            char *buff = (char *) calloc (MAX_CHARS_IN_COMM + 1, sizeof(char));
            assert (buff);

            int marker = 0;

            for (int i = 0; isalpha (*tmp); i++)
            {
                if (i >= MAX_CHARS_IN_COMM)
                    error_func ("command");

                buff[i] = *tmp;
                tmp++;
            }

            if (!strcmp (buff, "MOVI"))
            {
                marker = 1;
                res->kind = OP;
                res->inf.op = MOVI;
            }

            else if (!strcmp (buff, "ADD"))
            {
                marker = 1;
                res->kind = OP;
                res->inf.op = ADD;
            }

            else if (!strcmp (buff, "SUB"))
            {
                marker = 1;
                res->kind = OP;
                res->inf.op = SUB;
            }

            else if (!strcmp (buff, "MUL"))
            {
                marker = 1;
                res->kind = OP;
                res->inf.op = MUL;
            }

            else if (!strcmp (buff, "DIV"))
            {
                marker = 1;
                res->kind = OP;
                res->inf.op = DIV;
            }

            else if (!strcmp (buff, "IN"))
            {
                marker = 1;
                res->kind = OP;
                res->inf.op = IN;
            }

            else if (!strcmp (buff, "OUT"))
            {
                marker = 1;
                res->kind = OP;
                res->inf.op = OUT;
            }

            else if (!strcmp (buff, "A"))
            {
                marker = 1;
                res->kind = REG;
                res->inf.reg = A;
            }

            else if (!strcmp (buff, "B"))
            {
                marker = 1;
                res->kind = REG;
                res->inf.reg = B;
            }

            else if (!strcmp (buff, "C"))
            {
                marker = 1;
                res->kind = REG;
                res->inf.reg = C;
            }

            else if (!strcmp (buff, "D"))
            {
                marker = 1;
                res->kind = REG;
                res->inf.reg = D;
            }

            else
                error_func ("opcode or register in command");

            free (buff);
            break;
        }

        if (isdigit (*tmp))
        {
            int number = 0;
            int counter = 0;

            while ((isdigit(*tmp)) && (tmp != NULL))
			{
				counter++;
				tmp++;
			}

			for (int i = 0; i < counter; i++)
			{
			    tmp--;
                number += (*tmp - '0') * pow2 (10, i);
			}

			tmp = tmp + counter;

			if (number >= pow2(2, BITES_IN_CODE - 1))
                error_func ("value in MOVI");

			res->kind = NUM;
			res->inf.num = (unsigned)number;

			break;
        }

        break;
    }

    if (res->kind == -1)
        return res;

    if (tmp)
        res->next = lex_brake (tmp);

    return res;
}

void encode (FILE *fto, struct lex *head)
{
    if (!head)
        return ;

    unsigned num = 0;

    if ((head->kind == OP) && (head->inf.op == MOVI))
    {
        if (head->next->kind == NUM)
            num = (unsigned char)head->next->inf.num;
        else
            error_func ("process in MOVI");

        fprintf (fto, "0x%x\n", num);

        encode (fto, head->next->next);
    }

    else if ((head->kind == OP) && (head->inf.op == ADD))
    {
        num = num | (1 << 7);

        num = num | (put_bites_in_calc_comm (head));

        fprintf (fto, "0x%x\n", num);

        encode (fto, head->next->next->next);
    }

    else if ((head->kind == OP) && (head->inf.op == SUB))
    {
        num = num | (1 << 7);
        num = num | (1 << 4);

        num = num | (put_bites_in_calc_comm (head));

        fprintf (fto, "0x%x\n", num);

        encode (fto, head->next->next->next);
    }

    else if ((head->kind == OP) && (head->inf.op == MUL))
    {
        num = num | (1 << 7);
        num = num | (1 << 5);

        num = num | (put_bites_in_calc_comm (head));

        fprintf (fto, "0x%x\n", num);

        encode (fto, head->next->next->next);
    }

    else if ((head->kind == OP) && (head->inf.op == DIV))
    {
        num = num | (1 << 7);
        num = num | (1 << 5);
        num = num | (1 << 4);

        num = num | (put_bites_in_calc_comm (head));

        fprintf (fto, "0x%x\n", num);

        encode (fto, head->next->next->next);
    }

    else if ((head->kind == OP) && (head->inf.op == IN))
    {
        num = num | (1 << 7);
        num = num | (1 << 6);

        if (head->next->kind == REG)
            num = num | (head->next->inf.reg);
        else
            error_func ("process in IN");

        fprintf (fto, "0x%x\n", num);

        encode (fto, head->next->next);
    }

    else if ((head->kind == OP) && (head->inf.op == OUT))
    {
        num = num | (1 << 7);
        num = num | (1 << 6);
        num = num | (1 << 2);

        if (head->next->kind == REG)
            num = num | (head->next->inf.reg);
        else
            error_func ("process in OUT");

        fprintf (fto, "0x%x\n", num);

        encode (fto, head->next->next);
    }
}

unsigned put_bites_in_calc_comm (struct lex *head)
{
    assert (head);

    unsigned num = 0;
    struct lex *tmp = head->next;

    if (tmp->kind == REG)
        num = num | ((tmp->inf.reg) << 2);
    else
        error_func ("process in calculation command");

    tmp = tmp->next;
    if (tmp->kind == REG)
        num = num | (tmp->inf.reg);
    else
        error_func ("process in calculation command");

    return num;
}
