#include <stdio.h>

int main(int argc, char *argv[])
{
    int count_to_write;
    FILE *file1 = NULL, *file2 = NULL;
    unsigned char buffer[BUFSIZ];

    if (argc != 3)
    {
        printf(stderr, "incorrect args count");
        return -1;
    }

    file1 = fopen(argv[1], "rb");
    if (file1 == NULL)
    {
        printf(stderr, "file1 not open!");
        return -2;
    }
    file2 = fopen(argv[2], "wb");
    if (file2 == NULL)
    {
        fclose(file1);
        printf(stderr, "file2 not open!");
        return -2;
    }

    while ((count_to_write = fread(buffer, sizeof(unsigned char), BUFSIZ, file1)) != 0)
    {
        fwrite(buffer, sizeof(unsigned char), count_to_write, file2);
    }

    fclose(file1);
    fclose(file2);
    return 0;
}