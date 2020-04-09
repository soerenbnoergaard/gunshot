#include "log.h"

#ifdef GUNSHOT_LOG_FILE
static char line[1024];
#endif

int log_init(void)
{
#ifdef GUNSHOT_LOG_FILE
    FILE *f = fopen(GUNSHOT_LOG_FILE, "w");
    if (f == NULL) {
        return 1;
    }
    fclose(f);
#endif
    return 0;
}

int log_write(const char *s)
{
#ifdef GUNSHOT_LOG_FILE
    FILE *f = fopen(GUNSHOT_LOG_FILE, "a");
    if (f == NULL) {
        return 1;
    }
    fprintf(f, "%s\n", s);
    fclose(f);
#endif
    return 0;
}
