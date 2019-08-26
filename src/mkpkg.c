#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "readcfg.h"
#include "crc32.h"
#include "checksum.h"

struct
{                      // length 1092
    int32_t len;       // length 4
    int32_t unknow_0;  // length 4
    int32_t ver;       // length 4      dword_804B768
    int32_t unknow_1;  // length 4
    int32_t fstype;    // length 4      dword_804B770
    int32_t include;   // length 4      dword_804B774
    char file[0x400u]; // length 1024   byte_804B778
    char dev[0x2Cu];   // length 44     dword_804BB78
} pkg_item_table[256]; // length 0x44400 = 1092 * 256

struct
{
    char *name;
    int32_t id;
} pkg_fstype_tbl[] = {
    {"none", 0},
    {"fat", 1},
    {"yaffs", 2},
    {"yaffs2", 3},
    {"ext2", 4},
    {"ram", 5},
    {"raw", 6},
    {"nor", 7},
    {"ubifs", 8},
    {NULL, 0},
};

struct
{                           // length 2048 0x800u
    int64_t tag;            // length 8
    int32_t ver;            // length 4
    char unset[64 - 8 - 4]; // length 52
    struct
    {                      // length 64
        uint32_t len;      // length 4
        uint32_t offset;   // length 4
        int32_t ver;       // length 4
        int32_t fstype;    // length 4
        uint32_t checksum; // length 4
        char dev[12];      // length 12
        char unset[32];    // length 32
    } item[31];            // length 64 * 31 = 1984
} pkg_file_header;

int32_t seed = 0x55;

unsigned int myrand()
{
    seed = 0x41C64E6D * seed + 12345;
    return ((unsigned int)seed >> 16) & 0x7FFF;
}

int ora_buf(char *buffer, int size)
{
    int i;
    for (i = 0; i < size; ++i)
    {
        buffer[i] = ((buffer[i] & 0x55) << 1) | ((buffer[i] & 0xAA) >> 1);
    }
    return i;
}

int main(int argc, const char **argv)
{
    char tmpstr[0x400u];
    char out_file[0x400u];
    int pkg_num = 0;
    int idx = 0;
    int32_t ver = 0;
    int64_t tag = 0;

    if (argc <= 1)
    {
        printf("\n Used : %s config.ini \n", argv[0]);
        exit(1);
    }

    FILE *stream = fopen(argv[1], "r");
    if (!stream)
        return -1;

    memset(tmpstr, 0, 0x400u);
    if (get_key(stream, "Header", "tag", tmpstr))
    {
        memcpy((void *)&tag, tmpstr, 7);

        memset(tmpstr, 0, 0x400u);
        if (!get_key(stream, NULL, "ver", tmpstr))
        {
            puts("\n can't find header ver item");
            fclose(stream);
            return -1;
        }
        ver = strtol(tmpstr, 0, 0);

        sprintf(out_file, "%s_%08x.bin", (char *)&tag, ver);
        while (!feof(stream))
        {
            if (!get_key(stream, "pkg", "name", tmpstr))
                break;
            memset(tmpstr, 0, 0x400u);
            if (!get_key(stream, NULL, "idx", tmpstr))
                break;
            idx = strtol(tmpstr, 0, 0);
            if (idx)
                --idx;
            memset(tmpstr, 0, 0x400u);
            if (!get_key(stream, NULL, "include", tmpstr))
                break;
            pkg_item_table[idx].include = strtol(tmpstr, 0, 0); // dword_804B774
            memset(tmpstr, 0, 0x400u);
            if (!get_key(stream, NULL, "file", tmpstr))
                break;
            memset((void *)(pkg_item_table[idx].file), 0, 0x400u - 1);      // 0x804B778
            memcpy((void *)(pkg_item_table[idx].file), tmpstr, 0x400u - 1); // 0x804B778
            struct stat statbuff;
            if (!stat(pkg_item_table[idx].file, &statbuff)) // 0x804B778
                pkg_item_table[idx].len = statbuff.st_size; // pkg_item_table 0
            memset(tmpstr, 0, 0x400u);
            if (!get_key(stream, NULL, "ver", tmpstr))
                break;
            pkg_item_table[idx].ver = strtol(tmpstr, 0, 0); // dword_804B768
            memset(tmpstr, 0, 0x400u);
            if (!get_key(stream, NULL, "dev", tmpstr))
                break;
            memset((void *)pkg_item_table[idx].dev, 0, 0x2Cu - 1);      // 0x804BB78
            memcpy((void *)pkg_item_table[idx].dev, tmpstr, 0x2Cu - 1); // 0x804BB78

            pkg_item_table[idx].fstype = 0;
            memset(tmpstr, 0, 0x400u);
            if (!get_key(stream, NULL, "fstype", tmpstr))
                break;
            for (int i = 0; pkg_fstype_tbl[i].name; ++i)
            {
                if (!strcmp(pkg_fstype_tbl[i].name, tmpstr))
                {
                    pkg_item_table[idx].fstype = pkg_fstype_tbl[i].id;
                    break;
                }
            }
            ++pkg_num;
        }
    }
    printf("\n pkg_num = %d \n", pkg_num);

    if (pkg_num)
    {
        for (int i = 0; i < 1024; ++i)
        {
            *(((uint16_t *)&pkg_file_header) + i) = myrand();
        }
        pkg_file_header.tag = tag;
        pkg_file_header.ver = ver;
        FILE *s = fopen(out_file, "wb");
        if (s)
        {
            fwrite((char *)&pkg_file_header, 0x800u, 1u, s);
            int offset = 2048;
            for (int i = 0; i < 31; ++i)
            {
                if (pkg_item_table[i].include && pkg_item_table[i].len && pkg_item_table[i].file && pkg_item_table[i].file != 10)
                {
                    pkg_file_header.item[i].len = pkg_item_table[i].len;
                    pkg_file_header.item[i].offset = offset;
                    offset += pkg_item_table[i].len;
                    pkg_file_header.item[i].ver = pkg_item_table[i].ver;
                    pkg_file_header.item[i].fstype = pkg_item_table[i].fstype;
                    pkg_file_header.item[i].checksum = checksum(pkg_item_table[i].file, pkg_item_table[i].fstype);
                    memset(pkg_file_header.item[i].unset, 0, sizeof(pkg_file_header.item[i].unset));
                    memcpy(pkg_file_header.item[i].dev, pkg_item_table[i].dev, 12);
                    printf("\n include = %d ", pkg_item_table[i].include);
                    printf("\n file = %s ", pkg_item_table[i].file);
                    printf("\n pkg_item_len = %d ", pkg_item_table[i].len);
                    printf("\n ver = %d ", pkg_item_table[i].ver);
                    printf("\n dev = %s ", pkg_item_table[i].dev);
                    printf("\n fstype = %d ", pkg_item_table[i].fstype);
                    printf("\n ");
                }
                else
                {
                    pkg_file_header.item[i].len = 0;
                }
            }
            rewind(s);
            ora_buf((void *)&pkg_file_header, 2048);
            fwrite(&pkg_file_header, 0x800u, 1u, s);
            fclose(s);
            for (int i = 0; i < 31; ++i)
            {
                if (pkg_item_table[i].include && pkg_item_table[i].len && pkg_item_table[i].file && pkg_item_table[i].file != 10)
                {
                    char command[0x400u];
                    memset(command, 0, 0x400u);
                    sprintf(command, "cat %s >> %s", pkg_item_table[i].file, out_file);
                    printf("\n %s ", command);
                    system(command);
                }
            }
            putchar(10);
        }
        else
        {
            printf("\n Open file %s Error !\n", out_file);
        }
    }
    fclose(stream);
    return 0;
}