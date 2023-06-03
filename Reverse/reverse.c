#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

bool is_same_file(FILE* file1, FILE* file2);

void init(int argc, char** argv, FILE** input, FILE** output)
{
    if (argc == 1) {
        *input = stdin;
        *output = stdout;
    } else if (argc == 2) {
        *input = fopen(argv[1], "r");
        if (*input == NULL) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
        *output = stdout;
    } else if (argc == 3) {
        *input = fopen(argv[1], "r");
        if (*input == NULL) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
        *output = fopen(argv[2], "w");
        if (*output == NULL) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[2]);
            exit(1);
        }

        if (is_same_file(*input, *output)) {
            fprintf(stderr, "reverse: input and output file must differ\n");
            exit(1);
        }
    } else {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }
}

bool is_same_file(FILE* file1, FILE* file2)
{
    struct stat stat1, stat2;
    fstat(fileno(file1), &stat1);
    fstat(fileno(file2), &stat2);
    return stat1.st_dev == stat2.st_dev && stat1.st_ino == stat2.st_ino;
}

int main(int argc, char** argv)
{
    FILE *input, *output;
    init(argc, argv, &input, &output);

    int num_of_lines = 40, current = 0;
    char** lines = malloc(num_of_lines * sizeof(char*));
    if (lines == NULL) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    size_t len = 0;

    while (getline(&lines[current], &len, input) != -1) {
        current++;
        if (current == num_of_lines) {
            num_of_lines *= 2;
            lines = realloc(lines, num_of_lines * sizeof(char*));
            if (lines == NULL) {
                fprintf(stderr, "malloc failed\n");
                exit(1);
            }
        }
    }

    for (int i = current - 1; i >= 0; i--) {
        fprintf(output, "%s", lines[i]);
    }

    for (int i = 0; i < current; i++) {
        free(lines[i]);
    }
    free(lines);

    return 0;
}