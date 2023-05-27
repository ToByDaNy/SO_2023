#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "a2_helper.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <signal.h>

#define O_CREAT 0100

typedef struct fire_
{
    int id;
    int nrThread;
    sem_t *sem;
    sem_t *sem2;
    pthread_mutex_t *lock;
    pthread_cond_t *cond;
} fire;

sem_t *logSem = NULL;
sem_t *logSem2 = NULL;

int k = 0;
void *thrd2(void *arg)
{
    fire *s = (fire *)arg;
    sem_wait(s->sem);
    info(BEGIN, 9, s->id);
    pthread_mutex_lock(s->lock);
    if (s->id != 12)
    {
        k++;
        pthread_cond_broadcast(s->cond);
    }
    while (k < 6)
    {
        if (s->id == 12)
        {
            while (k < 5)
            {
                pthread_cond_wait(s->cond, s->lock);
            }
            break;
        }
        pthread_cond_wait(s->cond, s->lock);
    }

    if (s->id == 12)
    {
        k = 100;
        pthread_cond_broadcast(s->cond);
        info(END, 9, s->id);
    }
    else
    {
        k -= 1;
        pthread_cond_signal(s->cond);
        info(END, 9, s->id);
    }
    pthread_mutex_unlock(s->lock);

    sem_post(s->sem);
    return NULL;
}

void *thrd(void *arg)
{
    fire *s = (fire *)arg;
    if (s->id == 1 && s->nrThread == 2)
    {
        sem_wait(s->sem);
    }

    if (s->id == 2 && s->nrThread == 2)
    {
        sem_wait(logSem);
    }
    if (s->id == 2 && s->nrThread == 4)
    {
        sem_wait(logSem2);
    }

    info(BEGIN, s->nrThread, s->id);



    if (s->id == 4 && s->nrThread == 2)
    {
        sem_post(s->sem);
    }

    if (s->id == 4 && s->nrThread == 2) 
    {
        sem_wait(s->sem2);
    }

    if (s->id == 2 && s->nrThread == 2)
    {
        sem_post(logSem2);
    }

    info(END, s->nrThread, s->id);

    if (s->id == 1 && s->nrThread == 4)
    {
        sem_post(logSem);
    }

    if (s->id == 1 && s->nrThread == 2)
    {
        sem_post(s->sem2);
    }

    return NULL;
}

void *thrd3(void *arg)
{
    fire *s = (fire *)arg;

    if (s->id == 2)
    {
        sem_wait(logSem2);
    }


    info(BEGIN, 4, s->id);

    info(END, 4, s->id);

    if (s->id == 1)
    {
        sem_post(logSem);
    }
    return NULL;
}

int main()
{
    init();
    int nrProcese = 8;
    int nrThreads = 46;
    pid_t *procese = (pid_t *)calloc(nrProcese, sizeof(pid_t));
    procese[0] = getpid();
    pthread_t *threads = (pthread_t *)calloc(nrThreads, sizeof(pthread_t));
    fire *threads_st = (fire *)calloc(nrThreads, sizeof(fire));
    sem_t second;
    sem_t second2;
    sem_t first;
    sem_t first2;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

    int status = 0;
    logSem = sem_open("/1", O_CREAT, 0644, 0);
    logSem2 = sem_open("/2", O_CREAT, 0644, 0);
    if (logSem == NULL)
    {
        return -1;
    }
    if (logSem2 == NULL)
    {
        return -1;
    }
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        return -1;
    }
    if (sem_init(&first, 0, 0) != 0)
    {
        return -1;
    }
    if (sem_init(&first2, 0, 0) != 0)
    {
        return -1;
    }
    if (sem_init(&second, 0, 6) != 0)
    {
        return -1;
    }
    if (sem_init(&second2, 0, 1) != 0)
    {
        return -1;
    }

    info(BEGIN, 1, 0);
    procese[1] = fork();
    if (procese[1] == -1)
    {
        return -1;
    }
    else if (procese[1] == 0)
    {
        info(BEGIN, 2, 0);
        for (int i = 0; i < 4; i++)
        {
            threads_st[i].id = i + 1;
            threads_st[i].sem = &first;
            threads_st[i].sem2 = &first2;
            threads_st[i].nrThread = 2;
            pthread_create(&threads[i], NULL, thrd, &(threads_st[i]));
        }

        procese[2] = fork();
        if (procese[2] == -1)
        {
            return -1;
        }
        else if (procese[2] == 0)
        {
            info(BEGIN, 3, 0);
            info(END, 3, 0);
            exit(status);
        }
        else
        {
            procese[3] = fork();
            
            if (procese[3] == -1)
            {
                return -1;
            }
            else if (procese[3] == 0)
            {
                info(BEGIN, 4, 0);

                for (int i = 42; i < 46; i++)
                {
                    threads_st[i].id = i - 41;
                    threads_st[i].nrThread = 4;
                    pthread_create(&threads[i], NULL, thrd3, &(threads_st[i]));
                }
                for (int i = 45; i >= 42; i--)
                {
                    pthread_join(threads[i], NULL);
                }

                procese[5] = fork();
                if (procese[5] == -1)
                {
                    return -1;
                }
                else if (procese[5] == 0)
                {
                    info(BEGIN, 6, 0);
                    info(END, 6, 0);
                    exit(status);
                }
                else
                {
                    procese[6] = fork();
                    if (procese[6] == -1)
                    {
                        return -1;
                    }
                    else if (procese[6] == 0)
                    {
                        info(BEGIN, 7, 0);
                        procese[7] = fork();
                        if (procese[7] == -1)
                        {
                            return -1;
                        }
                        else if (procese[7] == 0)
                        {
                            info(BEGIN, 8, 0);
                            info(END, 8, 0);
                            exit(status);
                        }
                        else
                        {
                            waitpid(procese[7], NULL, 0);
                            info(END, 7, 0);
                            exit(status);
                        }
                    }
                    else
                    {
                        
                        waitpid(procese[5], NULL, 0);
                        waitpid(procese[6], NULL, 0);
                        info(END, 4, 0);
                        exit(status);
                    }
                }
            }
            else
            {
                for (int i = 0; i < 4; i++)
                {
                    pthread_join(threads[i], NULL);
                }

                
                procese[4] = fork();
                if (procese[4] == -1)
                {
                    return -1;
                }
                else if (procese[4] == 0)
                {
                    info(BEGIN, 5, 0);
                    info(END, 5, 0);
                    exit(status);
                }
                else
                {
                    procese[8] = fork();
                    if (procese[8] == -1)
                    {
                        return -1;
                    }
                    else if (procese[8] == 0)
                    {
                        info(BEGIN, 9, 0);
                        for (int i = 4; i < 42; i++)
                        {
                            threads_st[i].id = i - 3;
                            threads_st[i].sem = &second;
                            threads_st[i].sem2 = &second2;
                            threads_st[i].lock = &lock;
                            threads_st[i].cond = &cond;
                            pthread_create(&threads[i], NULL, thrd2, &(threads_st[i]));
                        }
                        for (int i = 4; i < 42; i++)
                        {
                            pthread_join(threads[i], NULL);
                        }
                        info(END, 9, 0);
                        exit(status);
                    }
                    else
                    {
                        
                        waitpid(procese[2], NULL, 0);
                        waitpid(procese[3], NULL, 0);
                        waitpid(procese[4], NULL, 0);
                        waitpid(procese[8], NULL, 0);
                        info(END, 2, 0);
                        exit(status);
                    }
                }
            }
        }
    }
    else
    {

        waitpid(procese[1], NULL, 0);

        info(END, 1, 0);
        free(procese);
        free(threads);
        free(threads_st);
        sem_destroy(&first);
        sem_destroy(&first2);
        sem_destroy(&second);
        sem_destroy(&second2);
        sem_close(logSem);
        sem_close(logSem2);
        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&cond);
        sem_unlink("/1");
        sem_unlink("/2");
    }

    return 0;
}
