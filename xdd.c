#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_COLS 256

struct Args {
    char* filename;
    unsigned int columns;
};

enum ArgOption {
    OPT_NONE,
    OPT_COLUMNS,
};

int parse_args(struct Args* args, int argc, char** argv) {
    args->filename = NULL;
    args->columns = 16;

    enum ArgOption prev_opt = OPT_NONE;

    int i = 1;
    for (; i < argc; i++) {
        char* arg = argv[i];
        /* printf("[%s]\n", arg); */

        if (arg[0] == '-') {
            arg++;
            if (arg[0] == '\0') {
                return 1;
            }
            if (arg[0] == 'c') {
                prev_opt = OPT_COLUMNS;
            } else if (arg[0] == 'h') {
                return -1;
            } else {
                fprintf(stderr, "Unknown option `-%c`.\n", arg[0]);
                return 1;
            }
        } else {
            switch (prev_opt) {
                case OPT_COLUMNS: {
                    char* endptr = NULL;
                    int columns = strtol(arg, &endptr, 10);
                    if (endptr[0] != '\0') {
                        fprintf(stderr, "Not an integer: `%s`\n", arg);
                        return 1;
                    }
                    if (columns > 256 || columns < 1) {
                        fprintf(stderr,
                                "Column number must be between 0-256.\n");
                        return 1;
                    }
                    args->columns = columns;
                }; break;

                default: {
                    if (args->filename != NULL) {
                        fprintf(stderr, "Too many arguments.\n");
                        return 1;
                    }
                    args->filename = arg;
                }
            }
            prev_opt = OPT_NONE;
        }
    }

    if (prev_opt != OPT_NONE) {
        fprintf(stderr, "Missing value for option `%s`.\n", argv[i - 1]);
        return 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    char* prog = argv[0];

    struct Args args;
    int result = parse_args(&args, argc, argv);
    if (result > 0) {
        return result;
    } else if (result < 0) {
        printf("Usage: %s [FILENAME] ...\n", prog);
        return 0;
    }

    FILE* fp = stdin;
    if (args.filename != NULL) {
        char* filename = args.filename;
        fp = fopen(filename, "r");
        if (errno) {
            fprintf(stderr, "%s: ", prog);
            perror(filename);
            return errno;
        }
    }

    char buf[MAX_COLS] = "";
    int bytes_read = 0;
    int row = 0;

    while ((bytes_read = fread(buf, sizeof(char), args.columns, fp))) {
        // Index
        printf("%08x: ", row * args.columns);

        // Bytes
        printf("\x1b[1m");
        for (int i = 0; i < args.columns; i++) {
            if (i > 0 && i % 2 == 0) {
                printf(" ");
            }
            if (i >= bytes_read) {
                printf("  ");
                continue;
            }

            signed char ch = buf[i];
            if (ch == '\0') {
                printf("\x1b[37m");
            } else if (ch == '\xff') {
                printf("\x1b[34m");
            } else if (isspace(ch) && ch != ' ') {
                printf("\x1b[33m");
            } else if (!isprint(ch)) {
                printf("\x1b[31m");
            } else {
                printf("\x1b[32m");
            }
            printf("%02x", (unsigned char)ch);
        }
        printf("  ");

        // Characters
        for (int i = 0; i < bytes_read; i++) {
            signed char ch = buf[i];

            if (ch == '\0') {
                printf("\x1b[37m");
            } else if (ch == '\xff') {
                printf("\x1b[34m");
            } else if (ch == '\n') {
                printf("\x1b[33m");
            } else if (!isprint(ch)) {
                printf("\x1b[31m");
            } else {
                printf("\x1b[32m");
                printf("\x1b[32m");
                printf("%c", buf[i]);
                continue;
            }
            printf(".");
        }
        printf("\x1b[0m");
        printf("\n");

        row++;
    };
}
