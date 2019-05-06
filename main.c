#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "encoder.h"

int main (int argc, char **argv)
{
    if (argc < 3)
        error_func ("executive data");

    struct args inp = args_analysis (argc, argv);

    FILE *ffr = stdin;
    FILE *fto = stdout;
    if (inp.fout != NULL)
    {
        fto = fopen (inp.fout, "w");
        assert (fto);
    }

    if (inp.fin == NULL)
    {
        if (inp.act == 1)                                        // If the given instruction is "-d"
        {
            unsigned char cmd = 0;
            char *tmp = argv[inp.st];

            if (strlen (argv[inp.st]) == BITES_IN_CODE)           // If the given code is in binary form
            {
                for (int j = BITES_IN_CODE - 1; j >= 0; j--)
                {
                    if (*tmp == '1')
                        cmd += pow2 (2, j);

                    tmp++;
                }

                decode (fto, cmd);
            }

            else if ((strlen (argv[inp.st]) >= 1) && (strlen (argv[inp.st]) <= 4))     // If the given code is in hex form (with "0x" or without)
            {
                for (int i = strlen (argv[inp.st]) - 1; i >= 0; i--)
                {
                    if (*tmp == 'x')
                    {
                        tmp++;
                        continue;
                    }

                    if (isalpha (*tmp))
                    {
                        if (((*tmp - 87) < 10) || ((*tmp - 87) > 15))
                            error_func ("given hex code 1\n");

                        cmd+= (*tmp - 87) * pow2 (16, i);
                        tmp++;

                        continue;
                    }

                    cmd += (*tmp - '0') * pow2 (16, i);
                    tmp++;
                }

                if (cmd >= pow2 (2, BITES_IN_CODE))
                    error_func ("given hex code 2\n");

                decode (fto, cmd);
            }

            else
                error_func ("given hex code 3\n");
        }

        if (inp.act == 0)                  // If the given instruction is "-c"
        {
            struct lex *st = lex_brake (argv[inp.st]);

            encode (fto, st);

            struct lex *tmp = st->next;
            for (;;)
            {
                free (st);
                st = tmp;
                if (st == NULL)
                    break;

                tmp = tmp->next;
            }
        }
    }

    else
    {
        ffr = fopen (inp.fin, "r");
        assert (ffr);

        if (inp.act == 1)                       // If the given instruction is "-d"
        {
            unsigned command = 0;

            while (fscanf (ffr, "%x", &command) == 1)
            {
                unsigned char cmd = command & 0xff;
                assert (cmd == command);

                decode (fto, cmd);
            }
        }

        if (inp.act == 0)                       // If the given instruction is "-c"
        {
            char *buff = (char *) calloc (MAX_CHARS_IN_COMM, sizeof(char));
            int res = fscanf (ffr, "%s", buff);
            assert (res == 1);

            struct lex *st = lex_brake (buff);
            struct lex *tmp = st;
            while (res == 1)
            {
                res = fscanf (ffr, "%s", buff);
                tmp->next = lex_brake (buff);
                tmp = tmp->next;
            }

            encode (fto, st);

            tmp = st->next;
            for (;;)
            {
                free (st);
                st = tmp;
                if (st == NULL)
                    break;

                tmp = tmp->next;
            }
        }

        fclose (ffr);
    }

    if (inp.fout != NULL)
        fclose (fto);

    return 0;
}
