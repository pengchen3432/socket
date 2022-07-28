#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/un.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>   
#include <sys/stat.h>
#include <stdint.h>
#define path "./sem"


//#######################sem function start#######################
static int semaphore_p(int sid)
{
    struct sembuf sem_b;

    memset(&sem_b, 0, sizeof(struct sembuf));
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;

    return semop(sid, &sem_b, 1);
}

static int semaphore_v(int sid)
{
    struct sembuf sem_b;

    memset(&sem_b, 0, sizeof(struct sembuf));
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;

    return semop(sid, &sem_b, 1);
}


static int init_sem_id(char *sem_path, int count)
{
    int ret;
    int id;                                                                                                                                   
    key_t key;
    union semun {
        int              val;    /* Value for SETVAL */
        struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
        unsigned short  *array;  /* Array for GETALL, SETALL */
        struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                    (Linux-specific) */
    } sem_union;

    key = ftok(sem_path, 'a');
    if (key < 0) {
        printf("key failure\n");
        return key;
    }

    printf("key=%d\n", key);
    id = semget(key, 1, 0666 | IPC_CREAT);
    if (id < 0) {
        printf("id  failure, %s\n", strerror(errno));
        return id;
    }

    sem_union.val = count;
    ret = semctl(id, 0, SETVAL, sem_union);
    if (ret < 0) {
        printf( "fatal error: can not set sem\n");
        semctl(id, 0, IPC_RMID, sem_union);
        return ret;
    }

    return id;
}

void  sem_gswait(int sem_id)
{
//    GS_DBG("wait:sem_id=%d\n", sem_id);
    int ret = semaphore_p(sem_id);
    if (ret < 0)
        printf("fatal error: p semphore failed\n");
}

void  sem_gspost(int sem_id)
{
//    GS_DBG("post:sem_id=%d\n", sem_id);
    int ret = semaphore_v(sem_id);
    if (ret < 0)
        printf("fatal error: v semphore failed\n");
}




int gs_sem_init(char *sem_path, int count)
{
    int fd;
    int sem_id = -1;

    fd = open(sem_path, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (fd < 0) {
        if (errno == EEXIST) {
            printf( "Sem file for  exist already\n");
            /* continue to init sem */
        } else {
            printf( "fatal error: can not create sem file\n");
            return (-1);
        }
    } else
        close(fd);

    sem_id = init_sem_id(sem_path, count);
    if (sem_id < 0) {     
        printf("init sem id failure\n");
        return -1;
    }     

    return sem_id;

}


int  gs_sem_deinit(int sem_id)
{
    if (semctl(sem_id, IPC_RMID, 0) == -1) {
        return -1;
    }
    return 0;
}


//#######################sem function end#######################

//#######################shm function start#######################


static int create_shm(char *shm_path, int size, uint8_t *init_flag)
{
    int id;
    key_t key;
    uint8_t init = 0;

    key = ftok(shm_path, 'a');
    if (key < 0) 
    {
        printf("key failure,%s\n", strerror(errno));
        return key;
    }

    /* get shared memory */
    id = shmget(key, size, IPC_EXCL);
    if (id == -1) {
            /* first create a shared memory */
        id = shmget(key, size, IPC_CREAT | S_IRUSR | S_IWUSR);
        init = 1;
    }

    if (id < 0) {
        printf("id  failure, %s\n", strerror(errno));
        return id;
    }

    if (init_flag)
        *init_flag = init;

    return id;
}


void *get_shm_addr(int shm_id)
{
    void *addr = NULL;
    
    /* get the pointer to the shared mem */
    addr = shmat(shm_id, 0, 0);
    if (-1 == (long)(addr)) {    
        printf("shmat error, %s\n", strerror(errno));
        addr = NULL;
    }    

    return addr;
}


void *gs_shm_init(char *shm_path, int size, int *shmid, uint8_t *init_flag)
{
    int fd;
    int shm_id;
    void *shm_addr = NULL;

    fd = open(shm_path, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (fd < 0) {
        if (errno == EEXIST) {
            printf( "Shm file exist already\n");
            /* continue to get shm id */
        } else {
            printf( "fatal error: can not create shm file\n");
            return NULL;
        }
    } else
        close(fd);

    shm_id = create_shm(shm_path, size, init_flag);
    if (shm_id < 0) {     
        printf("init shm id failure\n");
        return NULL;
    }     

    shm_addr = get_shm_addr(shm_id);
    if (shm_addr == NULL)
        goto OUT;

    if (shmid != NULL) {
        *shmid = shm_id;
    }

    printf("shm_id=%d\n", shm_id);

OUT:
    return shm_addr;

}


int gs_shm_deinit(int shmid, void *addr)
{
    if (shmdt(addr) == -1) {
        return -1;
    }

    return shmctl(shmid, IPC_RMID, NULL);
}

int main()
{
    int id;
    id = gs_sem_init( path, 1 );
    sem_gswait( id );
    printf("in in in\n");
    sleep(10);
    printf("out out out\n");
    sem_gspost( id );
}

