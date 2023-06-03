#include <stdio.h>
#include <string.h>

void search_in_stream(FILE* stream, char* searchterm) {
    char* line;
    size_t len = 0;

    while (getline(&line, &len, stream) != -1) {
        if (strstr(line, searchterm) != NULL) {
            printf("%s", line);
        }
    }
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        printf("searchterm [file ...]\n");
        return 1;
    }

    char* searchterm = argv[1];
    if (strlen(searchterm) == 0) {
        return 0;
    }

    if (argc == 2) {
        search_in_stream(stdin, searchterm);
    } else {
        for (int i = 2; i < argc; i++) {
            FILE* stream = fopen(argv[i], "r");
            if (stream == NULL) {
                printf("cannot open file\n");
                return 1;
            }
            search_in_stream(stream, searchterm);
            fclose(stream);
        }
    }

    return 0;
}