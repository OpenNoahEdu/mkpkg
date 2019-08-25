#include <stdlib.h>
#include <stdio.h>
#include "readcfg.h"

char *get_key(FILE *stream, const char *section, const char *key, char *a4)
{
    int i;            // eax
    unsigned int v5;  // kr04_4
    int j;            // [esp+4Ch] [ebp-41Ch]
    int v9;           // [esp+50h] [ebp-418h]
    signed int v10;   // [esp+54h] [ebp-414h]
    char *src;        // [esp+58h] [ebp-410h]
    char *srca;       // [esp+58h] [ebp-410h]
    char *v13;        // [esp+5Ch] [ebp-40Ch]
    char s[1024];     // [esp+60h] [ebp-408h]
    
    v10 = 0;
    for (i = feof(stream);; i = feof(stream))
    {
        if (i)
            return 0;
        memset(s, 0, 0x400u);
        if (!fgets(s, 1024, stream))
            return 0;
        v5 = strlen(s) + 1;
        v9 = v5 - 1;
        if (s[v5 - 2] == 10)
        {
            s[v5 - 2] = 0;
            v9 = v5 - 2;
        }
        for (j = 0; j < v9 && s[j] == 32; ++j)
            ;
        if (j < v9 && s[j] != 10 && s[j] && !strchr("#;", s[j]))
            break;
    LABEL_35:;
    }
    if (s[j] == 91)
    {
        src = 0;
        if (v10 == 1)
            return 0;
    }
    else
    {
        if (!section)
            v10 = 1;
        src = strchr(s, 61);
    }
    if (!src || v10 != 1)
    {
        if (section)
        {
            *(int16_t *)a4 = 91;
            strcat(a4, section);
            strcat(a4, "]");
            if (!strcmp(a4, s))
                v10 = 1;
        }
        else
        {
            v10 = 1;
        }
        goto LABEL_35;
    }
    *src = 0;
    v13 = strstr(s, key);
    if (!v13 || s < v13 && !strchr(" \t", *(v13 - 1)))
        goto LABEL_35;
    v13 += strlen(key);
    if (*v13)
    {
        if (!strchr(" \t", *v13))
            goto LABEL_35;
    }
    for (srca = src + 1; *srca == 32 || *srca == 9; ++srca)
        ;
    strcpy(a4, srca);
    return a4;
}
