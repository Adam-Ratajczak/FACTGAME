#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

void log_clear(){
    remove(LOG_FILE);
}

void log_debug(const char* format, ...) {
    FILE* log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        time_t rawtime;
        struct tm* timeinfo;
        char time_buffer[20];

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);  // Format time

        fprintf(log_file, "[%s] ", time_buffer);

        va_list args;
        va_start(args, format);

        vfprintf(log_file, format, args);

        fprintf(log_file, "\n");

        va_end(args);

        fclose(log_file);
    }
}