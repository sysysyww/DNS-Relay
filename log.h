#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <time.h>

extern int verbose_mode;  // 这个变量在别的地方定义，表示是否为详尽模式
void log_print_current_time();
// 如果为详尽模式，则打印详尽日志
#define LOG_VERBOSE(fmt, ...)                                    \
  do {                                                           \
    if (verbose_mode) {                                          \
      log_print_current_time();                                  \
      printf("[VERBOSE] (%s:%d): " fmt "\n", __FILE__, __LINE__, \
             ##__VA_ARGS__);                                     \
    }                                                            \
  } while (0);

// 如果不是详尽模式，打印简单日志
#define LOG_SIMPLE(fmt, ...)                                    \
  do {                                                          \
    if (!verbose_mode) {                                        \
      log_print_current_time();                                 \
      printf("[SIMPLE] (%s:%d): " fmt "\n", __FILE__, __LINE__, \
             ##__VA_ARGS__);                                    \
    }                                                           \
  } while (0);

#endif  // LOG_H
