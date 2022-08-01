#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <stdarg.h>
void my_syslog(
    int level,
    const char *fmt,
    ...
)
{
    va_list vl;
    va_start(vl, fmt);
    vsyslog(level, fmt, vl);
    va_end(vl);
}

#define PRT(LOV, fmt,...) do {printf( "dengji %s %s %d:" fmt, #LOV, __func__, __LINE__, ##__VA_ARGS__ ); }while(0)
#define SYS(LEVEL, fmt, ...) do {my_syslog(LEVEL, "%s %d:"fmt, __func__, __LINE__, ##__VA_ARGS__);} while(0)
int main(
    int argc,
    char *argv[]
)
{
    PRT(2, "%d\n", 1);
    SYS(1, "%s\n", "hello world");
    return 0;
}