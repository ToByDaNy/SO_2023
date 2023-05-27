#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <sys/mman.h>

#define FIFO_RESP "RESP_PIPE_84112"
#define FIFO_REQ "REQ_PIPE_84112"
#define __S_IWRITE 0200
#define length 250

typedef struct Head
{
    char name[14];
    char type;
    int offset, size;
} head;

void ok(const char *path, bool *flagSUCCES, bool *flagLines)
{
    int fp = open(path, O_RDONLY);
    if (fp == -1)
    {
        close(fp);
        return;
    }
    char MAGIC, NR_OF_SECTIONS;
    int VERSION;
    read(fp, &MAGIC, 1);
    lseek(fp, 2, SEEK_CUR); //  nu am nevoie sa citesc size-ul headerului
    read(fp, &VERSION, 4);
    read(fp, &NR_OF_SECTIONS, 1);
    bool flagMagic = false, flagVersion = false, flagSectNr = false, flagSectTypes = false;
    flagMagic = (MAGIC == 'S');
    flagVersion = (61 <= VERSION && 127 >= VERSION);
    flagSectNr = (5 <= NR_OF_SECTIONS && 17 >= NR_OF_SECTIONS);
    if (!flagMagic)
    {
        close(fp);
        return;
    }
    if (!flagVersion)
    {
        close(fp);
        return;
    }
    if (!flagSectNr)
    {
        close(fp);
        return;
    }

    int **hed = calloc(NR_OF_SECTIONS, sizeof(int *)); // [0] pentru offset [1] pentru size
    for (int i = 0; i < NR_OF_SECTIONS; i++)
    {
        hed[i] = calloc(2, sizeof(int));
        lseek(fp, 13, SEEK_CUR);
        char type = 0;
        read(fp, &(type), 1);
        read(fp, &(hed[i][0]), 4);
        read(fp, &(hed[i][1]), 4);
        flagSectTypes = ((int)type == 73 || (int)type == 16 || (int)type == 97 || (int)type == 29);
        if (!flagSectTypes)
        {
            break;
        }
    }

    if (!flagSectTypes)
    {
        close(fp);
        for (int i = 0; i < (int)NR_OF_SECTIONS; i++)
        {
            free(hed[i]);
        }
        free(hed);
        return;
    }
    for (int i = 0; i < (int)NR_OF_SECTIONS; i++)
    {
        lseek(fp, hed[i][0], SEEK_SET);
        char *strr = calloc(hed[i][1] + 1, sizeof(char));
        read(fp, strr, hed[i][1]);
        strr[hed[i][1]] = '\0';
        int count = 0;
        char *s = strr;
        while (s != NULL)
        {
            char cs[] = "\n";
            s = strstr(s, cs);
            if (s != NULL)
            {
                count++;
                s = s + 1;
                if (count >= 14)
                {
                    *flagLines = true;
                    *flagSUCCES = true;
                    break;
                }
            }
        }
        free(strr);
        strr = NULL;
        s = NULL;
    }
    for (int i = 0; i < (int)NR_OF_SECTIONS; i++)
    {
        free(hed[i]);
    }
    free(hed);
    close(fp);
}

int main()
{
    int fd1 = -1;
    int fd2 = -1;

    if (mkfifo(FIFO_RESP, 0600) != 0)
    {
        perror("ERROR\ncannot create the response pipe");
        return (1);
    }
    fd1 = open(FIFO_REQ, O_RDONLY);
    if (fd1 == -1)
    {
        perror("ERROR\ncannot open the request pipe");
        return (1);
    }
    fd2 = open(FIFO_RESP, O_WRONLY);
    if (fd2 == -1)
    {
        perror("ERROR\ncannot open the response pipe");
        return (1);
    }

    char *TextP = calloc(7, sizeof(char));
    TextP[0] = 5;
    strcpy(TextP + 1, "BEGIN");
    // puts(TextP);

    if (write(fd2, TextP, strlen(TextP)) == -1)
    {
        perror("BEGIN");
        return (1);
    }
    int fd;
    unsigned int memorySize = 0;
    char *memoryCreation;
    // puts("SUCCESS");
     //while (1)
     //{

        int nr = 0;
        char *req_name = calloc(length, sizeof(char));
        read(fd1, &nr, 1);
        int i = 0;
        while (((int)nr) > i)
        {
            read(fd1, &req_name[i++], sizeof(char));
        }
        char *s = calloc(length, sizeof(char));
        strncpy(s, req_name, (int)nr);
        s[(int)nr] = '\0';
        if (strcmp(s, "VARIANT") == 0)
        {
            char *str = calloc(7, sizeof(char));
            char *str2 = calloc(9, sizeof(char));
            str2[0] = 7;
            strcpy(str2 + 1, s);
            write(fd2, str2, 8);
            int aux = 84112;
            str[0] = 5;
            strcpy(str + 1, "VALUE");
            write(fd2, str, 6);
            write(fd2, &aux, 4);
        }
        if (strcmp(s, "CREATE_SHM") == 0)
        {
            char *str2 = calloc(12, sizeof(char));
            str2[0] = 10;
            strcpy(str2 + 1, s);
            write(fd2, str2, 11);

            read(fd1, &memorySize, sizeof(int));
            fd = shm_open("/dYHej3", O_CREAT | O_RDWR, 0664);
            if (fd == -1)
            {
                char *error = calloc(7, sizeof(char));
                error[0] = 5;
                strcpy(error + 1, "ERROR");
                write(fd2, error, 6);
                close(fd);
                perror("OPEN");
                return 1;
            }
            memoryCreation = (char *)mmap(NULL, memorySize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (memoryCreation == (void *)-1)
            {
                char *error = calloc(7, sizeof(char));
                error[0] = 5;
                strcpy(error + 1, "ERROR");
                write(fd2, error, 6);
                close(fd);
                perror("MEMORY");
                return 1;
            }
            char *success = calloc(9, sizeof(char));
            strcpy(success + 1, "SUCCESS");
            success[0] = 7;
            write(fd2, success, 8);
        }
        if (strcmp(s, "WRITE_TO_SHM") == 0)
        {
            char *str2 = calloc(14, sizeof(char));
            str2[0] = 12;
            strcpy(str2 + 1, s);
            write(fd2, str2, 13);
            unsigned int offset = 0;
            read(fd1, &offset, 4);
            char* value = calloc(5,sizeof(char));
            for(int j = 0 ; j < 4; j ++)
                read(fd1, &value[j], 1);
            value[4] = '\0';
            if (offset >= 0 || offset <= memorySize - 4)
            {
                char* auxstr = calloc(memorySize,sizeof(char));
                strncpy(auxstr,memoryCreation,memorySize);
                printf("%c\n",value[0]);
                strncpy(auxstr+offset,value,4);
                strncpy(memoryCreation,auxstr,memorySize);
            }

            
            char *success = calloc(9, sizeof(char));
            strcpy(success + 1, "SUCCESS");
            success[0] = 7;
            write(fd2, success, 8);
        }
        if (strcmp(s, "EXIT") == 0)
        {
            close(fd1);
            close(fd2);
            unlink(FIFO_RESP);
            close(fd);
            shm_unlink("/dYHej3");
            return 0;
        }
    //}
    return 0;
}