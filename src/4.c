#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int is_byte_prime(unsigned char const byte);

int main(int argc, char *argv[])
{
    int flag_is_prime = 0, i;
    unsigned char byte, result = 0;
    FILE *file = NULL;
    if (argc <= 3)
    {
        printf(stderr, "incorrect args count");
        return -1;
    }

    file = fopen(argv[1], "rb");
    if (file == NULL)
    {
        printf(stderr, "file not open!");
        return -2;
    }

    if (strcmp(argv[2], "xor8") == 0)
    {
        if (fread(&result, sizeof(unsigned char), 1, file) == 0)
        {
            printf("file is empty\n");
            return 2;
        }
        while (fread(&byte, sizeof(unsigned char), 1, file) == 1)
            {
                result ^= byte;
            }
        printf("result of xor8 = %u\n", result);
    }

    else if ((strcmp(argv[2], "xorodd") == 0))
    {
        unsigned int _4bytes_var;
        while(fread(&_4bytes_var, sizeof(unsigned int), 1, file) == 1)
        {
            unsigned char *_4bytes_arr = (unsigned char *)&_4bytes_var;
            for (i = 0; i < 4; ++i)
            {
                if (is_byte_prime(_4bytes_arr[i]) == 1)
                {
                    flag_is_prime = 1;
                    break;
                }
            }
            if (flag_is_prime == 1)
                {
                    result ^= _4bytes_var;
                }
        }
        printf("result of xorodd = %u", result);
    }

    else if(strcmp(argv[2], "mask") == 0)
    {
        printf("\n1111111111111111111\n");
    
        if (argc < 4)
        {
            printf(stderr, "4 mode of file processing require 4 arguments with hex mask");
        }
        unsigned int mask = strtol(argv[3], NULL, 16);
        unsigned int digit;
        while(fread(&digit, sizeof(unsigned int), 1, file))
        {
            if ((digit & mask) == mask)
            ++result;
        }
        printf("count of 4bytes words matching (%s) = %u",argv[3], result);
    }
    return 0;
}

int is_byte_prime(unsigned char const byte)
{
    int i = 0;
    if (byte <= 1)
    {
        return 0;
    }

    if (byte == 2)
    {
        return 1;
    }

    if (byte % 2 == 0)
    {
        return 0;
    }

    for (i = 3; i * i <= byte; i += 2)
    {
        if (byte % i == 0)
        {
            return 0;
        }
    }
    return 1;
}