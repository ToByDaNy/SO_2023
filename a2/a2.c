#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sys/wait.h>

#include "a2_helper.h"

#define BEGIN 1
#define END 2

void init();
int info(int action, int processNr, int threadNr);
sem_t *first;

void *function(void *arg)
{
    return NULL;
}

int main()
{
    init();
    int nrProcese = 8;
    int nrThreads = 2;
    pid_t *procese = (pid_t *)calloc(nrProcese, sizeof(pid_t));
    procese[0] = getpid();
    pthread_t *threads = (pthread_t *)calloc(nrThreads, sizeof(pthread_t));
    info(BEGIN, 1, 0);
    procese[1] = fork();
    if (procese[1] == -1)
    {
        return -1;
    }
    else if (procese[1] == 0)
    {
        info(BEGIN, 2, 0);
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
                        info(END, 9, 0);
                    }
                    else
                    {
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
    }

    pthread_create(&(threads[0]), NULL, function, NULL);

    free(procese);
    free(threads);

    return 0;
}
