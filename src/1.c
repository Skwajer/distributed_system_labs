#include <stdio.h>
#define DATA_SIZ 11

void print_file_attrs(FILE *f);

int main(int argc, char *argv[]) 
{
    int i;
    FILE *file = NULL;
    unsigned char byte = 0;
    unsigned char buf[4];
    unsigned char const bytes[DATA_SIZ] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    if (argc != 2)
    {
        printf(stderr, "incorrect args count");
        return -1;
    }

    file = fopen(argv[1], "wb");
    if (file == NULL)
    {
        printf(stderr, "file not open!");
        return -2;
    }

    fwrite(bytes, sizeof(unsigned char), DATA_SIZ, file);
    fclose(file);
    file = NULL;

    file = fopen(argv[1], "rb");
    if (file == NULL)
    {
        printf(stderr, "file not open!");
        return -2;
    }
    while (fread(&byte, sizeof(unsigned char), 1, file) == 1)
    {
        printf("%u\n", byte);
        print_file_attrs(file);
    }
    fclose(file);
    file = NULL;
    
    file = fopen(argv[1], "rb");
    if (file == NULL)
    {
        printf(stderr, "file not open!");
        return -2;
    }

    fseek(file, 3, SEEK_SET);

    fread(buf, sizeof(unsigned char), 4, file);
    printf("buffer contains: [ ");
    for(i = 0; i < 4; ++i)
    {
        printf("%u ", buf[i]);
    }
    printf("]\n");

    return 0;
}

void print_file_attrs(FILE *f)
{
    printf("_IO_write_base:  %s\n", f->_IO_write_base);
    printf("_IO_write_ptr:   %p\n", f->_IO_write_ptr);
    printf("_IO_write_end:   %s\n", f->_IO_write_end);

    printf("_IO_read_base:   %s\n", f->_IO_read_base);
    printf("_IO_read_ptr:    %p\n", f->_IO_read_ptr);
    printf("_IO_read_end:    %s\n", f->_IO_read_end);

    printf("_IO_buf_base:    %s\n", f->_IO_buf_base);
    printf("_IO_buf_end:     %s\n", f->_IO_buf_end);

    printf("_IO_save_base:   %s\n", f->_IO_save_base);
    printf("_IO_save_end:    %s\n", f->_IO_save_end);
    printf("_IO_backup_base: %s\n", f->_IO_backup_base);

    printf("_offset:         %ld\n", f->_offset);
    printf("_old_offset:     %ld\n", f->_old_offset);

    printf("_fileno:         %d\n", f->_fileno);
    printf("_mode:           %d\n", f->_mode);
    printf("_flags:          %d\n", f->_flags);
    printf("_flags2:         %d\n", f->_flags2);

    printf("_shortbuf:       %d\n", *f->_shortbuf);
    printf("_short_backupbuf:%d\n", *f->_short_backupbuf);

    printf("_total_written:  %lu\n", f->_total_written);
    printf("_cur_column:     %hu\n", f->_cur_column);

    printf("_lock:           %p\n", f->_lock);
    printf("_markers:        %p\n", f->_markers);
    printf("_codecvt:        %p\n", f->_codecvt);
    printf("_wide_data:      %p\n", f->_wide_data);

    printf("_chain:          %p\n", f->_chain);
    printf("_prevchain:      %p\n", f->_prevchain);
    printf("_freeres_buf:    %p\n", f->_freeres_buf);
    printf("_freeres_list:   %p\n", f->_freeres_list);

    printf("_vtable_offset:  %hhd\n", f->_vtable_offset);
    printf("_unused2:        %d, %d\n", f->_unused2[0], f->_unused2[1]);
    printf("_unused3:        %d\n", f->_unused3);
    printf("\n");
}