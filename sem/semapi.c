#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
void fun(int id) {
    if (id) {
        printf("11111111\n");
    }
    else {
        printf("00000000\n");
    }
}
int main()
{
    sem_t sem;
    int ret, pid, i;
    ret = sem_init( &sem, 1, 1);
    pid = fork();
    if ( pid > 0 ) {
        sem_wait(&sem);
        for(i = 0; i < 1000; i++) {
            fun(0);
        }
        sem_post(&sem);
    }
    else {
        sem_wait(&sem);
        for(i = 0; i < 1000; i++) {
            fun(1);
        }
        sem_post(&sem);
        
    }
    return 0;
}