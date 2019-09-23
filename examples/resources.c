#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "otic.h"

long get_free_system_memory()
{
    FILE* info = fopen("/proc/meminfo","r");
    long total = 0;
    long memfree = 0;
    fscanf(info, "MemTotal: %ld kB MemFree: %ld kB", &total, &memfree);
    fclose(info);
    return memfree;
}

int main(int argc, char* argv[]) {
    otic_writer w = otic_writer_open_filename("output.otic");
    otic_column mem, loadavg;
    otic_register_column(w, "mem_free", &mem);
    otic_register_column(w, "load_average", &loadavg);

    struct timespec t;
    struct timespec sleeptime;

    // sleep for 0.1s
    sleeptime.tv_sec = 0;
    sleeptime.tv_nsec = 100000000L;

    long mem_free = 0;
    double load_averages[3];

    for (int i = 0; i < 100; i++) {
        clock_gettime(CLOCK_REALTIME, &t);
        mem_free = get_free_system_memory();
        getloadavg(load_averages, 3);
        otic_write_long(mem, t.tv_sec, t.tv_nsec, mem_free);
        otic_write_double(loadavg, t.tv_sec, t.tv_nsec, load_averages[0]);
        nanosleep(&sleeptime, NULL);
    }
    otic_writer_close(w);

    return 0;
}
