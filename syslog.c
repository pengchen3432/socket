#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
int main()
{
    while ( 1 ) {
    	syslog(LOG_ERR, "hello world\n");
	sleep(1);
    }
}
