#ifndef LOG_H
#define LOG_H
#define LOG_FILE "log.txt"

void log_clear();
void log_debug(const char* format, ...);

#endif // LOG_H