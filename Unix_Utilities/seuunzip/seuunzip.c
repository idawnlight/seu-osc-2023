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

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("seuunzip: file1 [file2 ...]\n");
        return 1;
    }

    FILE *file = NULL;

    for (int i = 1; i < argc; i++) {
        try_fopen(argv[i], &file);

        while (1) {
            int count;
            char c;
            if (fread(&count, sizeof(int), 1, file) != 1) {
                break;
            }
            if (fread(&c, sizeof(char), 1, file) != 1) {
                break;
            }
            for (int j = 0; j < count; j++) {
                printf("%c", c);
            }
        }
    }

    fclose(file);

    return 0;
}