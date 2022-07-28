#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#define SEM_PATH "local_sem"
#define SHM_PATH "local_shm"
int sem_init();
int init_sem_id(char *path, int count);
int semaphort_p(int sem_id);
int semaphort_v(int sem_id);
int sem_wait(int sem_id);
int sem_post(int sem_id);
int sem_deinit(int sem_id);

int main()
{
    int sem_id;
    sem_id = sem_init();
    if ( sem_id < 0 ) {
        syslog(LOG_ERR, "sem_id failed\n");
        return -1;
    }
    printf("sem_id = %d\n", sem_id);
    sem_wait(sem_id);
    printf("1111111111\n");
    sleep(10);
    printf("2222222222\n");
    sem_post(sem_id);

    sem_post( sem_id );
    return 0;
}
    

int sem_init()
{
    int fd, key, sem_id;
    fd = open( SEM_PATH, O_RDWR | O_CREAT | O_EXCL, 0666 );
    if ( fd < 0 ) {
        if ( errno == EEXIST ) {
            syslog(LOG_NOTICE, "file exist\n");
        }
        else {
            syslog(LOG_ERR, "creat failed\n");
            return -1;
        }
    }
    else {
        close(fd);
    }

    sem_id = init_sem_id( SEM_PATH, 1 );
}

int init_sem_id(char *path, int count) 
{
    int key, ret, id;
    union semun {
        int              val;    /* Value for SETVAL */
        struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
        unsigned short  *array;  /* Array for GETALL, SETALL */
        struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                    (Linux-specific) */
    } sem_union;

    key = ftok( path, 'a' );
    if ( key < 0 ) {
        syslog(LOG_ERR, "key failed\n");
        return key;
    }
    // 得到一个信号量集
    id = semget( key, 1, IPC_EXCL );
    if ( id < 0 ) {
        id = semget(key, 1, 0666 | IPC_CREAT );
    }
    else {
        printf("1111111\n");
        return id;
    }
    if ( id < 0 ) {
        printf("id failed\n");
        return -1;
    }

    // 初始化一个信号量集
    sem_union.val = count;
    ret = semctl( id, 0, SETVAL, sem_union );
    if ( ret < 0 ) {
        syslog(LOG_ERR, "init failed\n");
        return ret;
    }
    return id;
}

int semaphort_p(int sem_id)
{
    struct sembuf sem_b;
    memset( &sem_b, 0, sizeof( sem_b ) );
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;

    return semop( sem_id, &sem_b, 1 );
}
int semaphort_v(int sem_id)
{
    struct sembuf sem_b;
    memset( &sem_b, 0, sizeof( sem_b ) );
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;

    return semop( sem_id, &sem_b, 1 );
}
int sem_wait(int sem_id)
{
    int ret;
    ret = semaphort_p( sem_id );
    if ( ret < 0 ) {
        syslog(LOG_ERR, "p operate failed\n");
        return ret;
    }
}
int sem_post(int sem_id)
{
    int ret;
    ret = semaphort_v( sem_id );
    if ( ret < 0 ) {
        syslog(LOG_ERR, "p operate failed\n");
        return ret;
    }
}
int sem_deinit(int sem_id)
{
    if ( semctl(sem_id, 0, IPC_RMID ) == -1 ) {
        syslog(LOG_ERR, "delete failed\n");
        return -1;
    }
    return 1;
}