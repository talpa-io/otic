#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "otic.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("usage: %s <filename>", argv[0]);
        return -1;
    }
    otic_reader r = otic_reader_open_filename(argv[1]);
    otic_result res;

    int count = 0;
    while (true) {
        res = otic_reader_next(r);
        if (!res) {
            if (otic_reader_geterror(r)) {
                printf("error reading!\n");
                otic_reader_close(r);
                return 1;
            }
            break;
        }
        count += 1;
        int typ = otic_result_get_type(res);

        time_t epoch = otic_result_get_epoch(res);
        long nanoseconds = otic_result_get_nanoseconds(res);
        char* name;
        size_t size = otic_result_get_colname(res, &name);
        for (int i = 0; i < size; i++) {
            putchar(name[i]);
        }
        printf(" %li %li ", epoch, nanoseconds);
        if (typ == OTIC_TYPE_INT) {
            printf("%li\n", otic_result_get_long(res));
        } else if (typ == OTIC_TYPE_DOUBLE) {
            printf("%f\n", otic_result_get_double(res));
        } else if (typ == OTIC_TYPE_NULL) {
            printf("NULL\n");
        } else {
            size = otic_result_get_string(res, &name);
            for (int i = 0; i < size; i++) {
                putchar(name[i]);
            }
        }
    }
    otic_reader_close(r);

    return 0;
}
