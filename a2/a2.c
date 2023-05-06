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
    sem_t *sem;
    sem_t *sem2;
    pthread_mutex_t *lock;
} fire;

void *thrd(void *arg)
{
    fire *s = (fire *)arg;
    if (s->id == 1)
    {
        sem_wait(s->sem);
    }
    info(BEGIN, 2, s->id);
    if (s->id == 4)
    {
        sem_post(s->sem);
    }

    if (s->id == 4)
    {
        sem_wait(s->sem2);
    }

    info(END, 2, s->id);

    if (s->id == 1)
    {
        sem_post(s->sem2);
    }

    return NULL;
}
int k = 0, ks = 0;
void *thrd2(void *arg)
{
    fire *s = (fire *)arg;
    sem_wait(s->sem);
    info(BEGIN, 9, s->id);

    k++;
    while (k < 6 && ks == 0)
    {
        sem_wait(s->sem2);
    }
    sem_post(s->sem2);
    if (s->id == 12)
    {
        info(END, 9, s->id);
        ks = 1;
    }
    else
    {
        k--;
        info(END, 9, s->id);
    }

    sem_post(s->sem);
    return NULL;
}
void *thrd3(void *arg)
{
    fire *s = (fire *)arg;
    info(BEGIN, 4, s->id);
    info(END, 4, s->id);
    return NULL;
}

int main()
{
    init();
    int nrProcese = 8;
    int nrThreads = 42;
    pid_t *procese = (pid_t *)calloc(nrProcese, sizeof(pid_t));
    procese[0] = getpid();
    pthread_t *threads = (pthread_t *)calloc(nrThreads, sizeof(pthread_t));
    fire *threads_st = (fire *)calloc(nrThreads, sizeof(fire));
    sem_t second;
    sem_t second2;
    sem_t first;
    sem_t first2;
    pthread_mutex_t lock;
    // sem_t third;
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
    if (sem_init(&second2, 0, 0) != 0)
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
                    pthread_create(&threads[i], NULL, thrd3, &(threads_st[i]));
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
                        }
                        else
                        {
                            waitpid(procese[7], NULL, 0);
                            info(END, 7, 0);
                        }
                    }
                    else
                    {
                        for (int i = 42; i < 46; i++)
                        {
                            pthread_join(threads[i], NULL);
                        }
                        waitpid(procese[5], NULL, 0);
                        waitpid(procese[6], NULL, 0);
                        info(END, 4, 0);
                    }
                }
            }
            else
            {
                procese[4] = fork();
                if (procese[4] == -1)
                {
                    return -1;
                }
                else if (procese[4] == 0)
                {
                    info(BEGIN, 5, 0);
                    info(END, 5, 0);
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
                            pthread_create(&threads[i], NULL, thrd2, &(threads_st[i]));
                        }
                        for (int i = 4; i < 42; i++)
                        {
                            pthread_join(threads[i], NULL);
                        }
                        info(END, 9, 0);
                    }
                    else
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            pthread_join(threads[i], NULL);
                        }
                        waitpid(procese[2], NULL, 0);
                        waitpid(procese[3], NULL, 0);
                        waitpid(procese[4], NULL, 0);
                        waitpid(procese[8], NULL, 0);
                        info(END, 2, 0);
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
    }

    return 0;
}
