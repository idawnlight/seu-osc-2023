#include <stdio.h>
#include <stdlib.h>

void try_fopen(char* filename, FILE** file)
{
    if (*file != NULL) {
        fclose(*file);
    }

    *file = fopen(filename, "r");
    if (*file == NULL) {
        printf("cannot open file\n");
        exit(1);
    }
}

void write_part(int* count, char* current_char)
{
    fwrite(count, sizeof(int), 1, stdout);
    fwrite(current_char, sizeof(char), 1, stdout);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("seuzip: file1 [file2 ...]\n");
        return 1;
    }

    char current_char = 255;
    int count = 1, current_file = 0;

    FILE* file = NULL;

    // find first non-empty file
    while (current_char == 255) {
        current_file++;
        if (current_file == argc) {
            return 0;
        }

        try_fopen(argv[current_file], &file);
        current_char = fgetc(file);
    }

    while (1) {
        char temp = fgetc(file);
        if (temp == 255) {
            current_file++;
            if (current_file == argc) {
                write_part(&count, &current_char);
                return 0;
            }

            try_fopen(argv[current_file], &file);
        } else {
            if (temp == current_char) {
                count++;
            } else {
                write_part(&count, &current_char);
                count = 1;
                current_char = temp;
            }
        }
    }

    fclose(file);

    return 0;
}