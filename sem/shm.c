#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
 #include <sys/types.h>
 #include <sys/wait.h>
typedef struct local{
    int a;
    char b[12];
}local;
int main()
{
    local l;
    l.a = -1;
    strncpy(l.b, "!!", sizeof(l.b));
    int shm, fd, pid, ret, status;
    void *addr;
    shm = shmget(IPC_PRIVATE, sizeof(l), IPC_CREAT | 0666);
    if ( shm < 0 ) {
        printf("shm failed\n");
        return -1;
    }
    pid = fork();
    if ( pid > 0 ) {
        l.a = 5;
        strncpy(l.b, "hello world", sizeof(l.b));
        addr = shmat(shm, NULL, 0 );
        memcpy(addr, &l, sizeof(l));
        wait(NULL);
    }
    else {
        printf("is %d", getpid());
        local *tmp;
        sleep(3);
        printf("l.a = %d\n", l.a);
        printf("l.b = %s\n", l.b);
        sleep(3);
        addr = shmat(shm, NULL, 0);
        tmp = (local*) addr;
        printf("l.a = %d\n", tmp->a);
        printf("l.b = %s\n", tmp->b);
    }
}