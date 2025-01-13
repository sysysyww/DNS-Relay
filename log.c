#include <stdio.h>
#include <time.h>

void log_print_current_time() {
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[80];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, 80, "[%Y-%m-%d %H:%M:%S]", timeinfo);
  printf("%s ", buffer);
}