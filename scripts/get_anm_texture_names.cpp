#include "../src/AnmManager.hpp"
#include "../src/utils.hpp"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *file;

    if (argc < 2)
    {
        printf("usage: %s ANM\n", argv[0]);
        exit(-1);
    }

    file = fopen(argv[1], "rb");
    if (file == NULL)
    {
        perror("error: cannot open anm file");
        exit(-1);
    }

    th06::AnmRawEntry entry = {};

    fread(&entry, sizeof(entry), 1, file);

    char color[64] = {}, alpha[64] = {};

    fseek(file, entry.nameOffset, SEEK_SET);
    fread(color, sizeof(char), ARRAY_SIZE(color), file);
    if (entry.alphaNameOffset != 0)
    {
        fseek(file, entry.alphaNameOffset, SEEK_SET);
        fread(alpha, sizeof(char), ARRAY_SIZE(alpha), file);
    }

    fclose(file);

    printf("color: %s\n", color);
    printf("alpha: %s\n", alpha);

    return 0;
}
