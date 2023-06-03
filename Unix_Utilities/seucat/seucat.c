#include <stdio.h>

int main(int argc, char** argv) {
    if (argc == 1) return 0;

    for (int i = 1; i < argc; i++) {
        FILE* file = fopen(argv[i], "r");
        if (file == NULL) {
            printf("cannot open file\n");
            return 1;
        }

        char buffer[1024];
        while (fgets(buffer, 1024, file) != NULL) {
            printf("%s", buffer);
        }
    }

    return 0;
}