#include <stdio.h>
#include <pthread.h>
#include <string.h>
void *fun(void *data)
{
    printf("data = %d\n", *(int *)data);
}
int main()
{
    int a = 1;
    int b = 2;
    int c = 3;
    pthread_t tid1, tid2, tid3;
    pthread_create(&tid1, NULL, fun, (void *)&a);
    pthread_join(tid1, NULL);
    pthread_create(&tid2, NULL, fun, (void *)&b);
    pthread_join(tid2, NULL);
    pthread_create(&tid3, NULL, fun, (void *)&c);
    pthread_join(tid3, NULL);
}