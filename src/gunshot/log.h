#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* #define GUNSHOT_LOG_FILE "gunshot.log" */
/* #define GUNSHOT_LOG_FILE "/home/soren/Desktop/gunshot.log" */
/* #define GUNSHOT_LOG_FILE "C:/Users/Christine/Desktop/gunshot.log" */

int log_init(void);
int log_write(const char *s);

#ifdef __cplusplus
}
#endif
#endif
