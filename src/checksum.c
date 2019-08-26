#include <stdlib.h>
#include <stdio.h>
#include "checksum.h"

signed int is_unmap_block(int a1)
{
    signed int i; // [esp+10h] [ebp-4h]

    for (i = 0; i <= 258047; ++i)
    {
        if (*(char *)(a1 + i) != -1)
            return 0;
    }
    return 1;
}

uint32_t checksum(const char *filename, int32_t fstype)
{
    unsigned int v2; // ST34_4
    unsigned int v4; // [esp+14h] [ebp-34h]
    int size;        // [esp+28h] [ebp-20h]
    unsigned int j;  // [esp+2Ch] [ebp-1Ch]
    unsigned int i;  // [esp+2Ch] [ebp-1Ch]
    unsigned int v8; // [esp+30h] [ebp-18h]
    unsigned int v9; // [esp+34h] [ebp-14h]
    int v10;         // [esp+38h] [ebp-10h]
    char *s;         // [esp+3Ch] [ebp-Ch]
    char *sa;        // [esp+3Ch] [ebp-Ch]
    char *sb;        // [esp+3Ch] [ebp-Ch]
    FILE *stream;    // [esp+44h] [ebp-4h]

    v8 = 0;
    v9 = 0;
    v10 = 0;
    printf("\n %s : %s sum = ", "checksum", filename);
    stream = fopen(filename, "rb");
    if (stream)
    {
        if (!fseek(stream, 0, 2))
        {
            v10 = ftell(stream);
            fseek(stream, 0, 0);
        }
        if (fstype != 3 && fstype != 4 && fstype != 8)
        {
            sa = malloc(0x4000u);
            if (!sa)
            {
                puts("Out of memory!");
                exit(0);
            }
            memset(sa, 0, 0x4000u);
            for (i = 0; i < v10 / 0x4000; ++i)
            {
                fread(sa, 1u, 0x4000u, stream);
                v2 = crc32(v8, sa, 0x4000);
                memset(sa, 0, 0x4000u);
                v8 = v2;
            }
            free(sa);
            size = v10 % 0x4000;
            sb = malloc(v10 % 0x4000);
            if (!sb)
            {
                puts("Out of memory!");
                exit(0);
            }
            fread(sb, 1u, size, stream);
            v9 = crc32(v8, sb, size);
            free(sb);
            printf("%x \n", v9);
        }
        else
        {
            s = malloc(0x3F004u);
            if (!s)
            {
                puts("Out of memory!");
                exit(0);
            }
            memset(s, 0, 0x3F004u);
            for (j = 0; j < v10 / 0x3F004u; ++j)
            {
                fread(s, 1u, 0x3F004u, stream);
                if (!is_unmap_block(s + 4))
                {
                    v9 = crc32(v8, s + 4, 258048);
                    memset(s, 0, 0x3F004u);
                    v8 = v9;
                }
            }
            free(s);
            printf("%x \n", v9);
        }
        fclose(stream);
        v4 = v9;
    }
    else
    {
        printf("\nfile open failure%d\n", 0);
        v4 = 0;
    }
    return v4;
}
