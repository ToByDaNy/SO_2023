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

void ok(const char *dataMap, bool *flagSUCCES, unsigned int sectionNr, unsigned int offset, unsigned int nr_bytes, char *result)
{
    unsigned int dataOffset = 0;
    *flagSUCCES = false;
    char MAGIC = dataMap[dataOffset++], NR_OF_SECTIONS = dataMap[7];
    dataOffset += 2;
    int VERSION = (int)((unsigned char)(dataMap[dataOffset + 3]) * 16 * 16 * 16 * 16 * 16 * 16 + (unsigned char)dataMap[dataOffset + 2] * 16 * 16 * 16 * 16 + (unsigned char)dataMap[dataOffset + 1] * 16 * 16 + (unsigned char)dataMap[dataOffset + 0]);

    dataOffset += 4;
    dataOffset++;
    
    bool flagMagic = false, flagVersion = false, flagSectNr = false, flagSectTypes = false;
    flagMagic = (MAGIC == 'S');
    flagVersion = (61 <= VERSION && 127 >= VERSION);
    flagSectNr = (5 <= NR_OF_SECTIONS && 17 >= NR_OF_SECTIONS);
    if (!flagMagic)
    {
        return;
    }
    if (!flagVersion)
    {
        return;
    }
    if (!flagSectNr)
    {
        return;
    }
    if (sectionNr > (unsigned int)NR_OF_SECTIONS)
    {
        return;
    }

    int **hed = calloc(NR_OF_SECTIONS, sizeof(int *)); // [0] pentru offset [1] pentru size
    for (int i = 0; i < NR_OF_SECTIONS; i++)
    {
        hed[i] = calloc(2, sizeof(int));
        dataOffset += 13;

        char type = 0;
        type = dataMap[dataOffset];
        dataOffset++;
        hed[i][0] = (int)((unsigned char)(dataMap[dataOffset + 3]) * 16 * 16 * 16 * 16 * 16 * 16 + (unsigned char)dataMap[dataOffset + 2] * 16 * 16 * 16 * 16 + (unsigned char)dataMap[dataOffset + 1] * 16 * 16 + (unsigned char)dataMap[dataOffset + 0]);

        dataOffset += 4;
        hed[i][1] = (int)((unsigned char)(dataMap[dataOffset + 3]) * 16 * 16 * 16 * 16 * 16 * 16 + (unsigned char)dataMap[dataOffset + 2] * 16 * 16 * 16 * 16 + (unsigned char)dataMap[dataOffset + 1] * 16 * 16 + (unsigned char)dataMap[dataOffset + 0]);
        dataOffset += 4;
        flagSectTypes = ((int)type == 73 || (int)type == 16 || (int)type == 97 || (int)type == 29);
        if (!flagSectTypes)
        {
            break;
        }
    }

    if (!flagSectTypes || ((unsigned int)hed[sectionNr - 1][1] < offset + nr_bytes))
    {
        for (int i = 0; i < (int)NR_OF_SECTIONS; i++)
        {
            free(hed[i]);
        }
        free(hed);
        return;
    }
    dataOffset = hed[sectionNr - 1][0]+offset;
    char *strr = calloc(hed[sectionNr - 1][1] + 1, sizeof(char));
    strncpy(strr, dataMap + dataOffset, hed[sectionNr - 1][1]);
    strr[hed[sectionNr - 1][1]] = '\0';
    *flagSUCCES = true;

    strncpy(result, strr, nr_bytes);
    free(strr);
    strr = NULL;
    for (int i = 0; i < (int)NR_OF_SECTIONS; i++)
    {
        free(hed[i]);
    }
    free(hed);
    return;
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
    char *error = calloc(7, sizeof(char));
    error[0] = 5;
    strcpy(error + 1, "ERROR");
    char *success = calloc(9, sizeof(char));
    strcpy(success + 1, "SUCCESS");
    success[0] = 7;
    char *TextP = calloc(7, sizeof(char));
    TextP[0] = 5;

    strcpy(TextP + 1, "BEGIN");
    if (write(fd2, TextP, strlen(TextP)) == -1)
    {
        perror("BEGIN");
        return (1);
    }
    int fd, fdMap;
    off_t sizeMap;
    unsigned int memorySize = 0;
    char *dataMap;
    char *path;
    char *memoryCreation;
    // puts("SUCCESS");
    while (1)
    {

        int nr = 0;
        char *req_name = calloc(length, sizeof(char));
        read(fd1, &nr, 1);
        read(fd1, req_name, (int)nr);
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
        else if (strcmp(s, "CREATE_SHM") == 0)
        {
            char *str2 = calloc(12, sizeof(char));
            str2[0] = 10;
            strcpy(str2 + 1, s);
            write(fd2, str2, 11);

            read(fd1, &memorySize, sizeof(int));
            fd = shm_open("/dYHej3", O_CREAT | O_RDWR, 0664);
            if (fd == -1)
            {
                write(fd2, error, 6);
                perror("OPEN");
                break;
            }
            ftruncate(fd, memorySize);
            memoryCreation = (char *)mmap(NULL, memorySize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (memoryCreation == (void *)-1)
            {
                write(fd2, error, 6);
                perror("MEMORY");
            }
            else
            {
                write(fd2, success, 8);
            }
        }
        else if (strcmp(s, "WRITE_TO_SHM") == 0)
        {
            char *str2 = calloc(14, sizeof(char));
            str2[0] = 12;
            strcpy(str2 + 1, s);
            write(fd2, str2, 13);
            unsigned int offset = 0;
            read(fd1, &offset, 4);
            char *value = calloc(4, sizeof(char));
            read(fd1, value, 4);
            if (offset < memorySize - 4)
            {
                memoryCreation[offset] = value[0];
                memoryCreation[offset + 1] = value[1];
                memoryCreation[offset + 2] = value[2];
                memoryCreation[offset + 3] = value[3];
                write(fd2, success, 8);
            }
            else
            {
                strcpy(error + 1, "ERROR");
                write(fd2, error, 6);
            }
        }
        else if (strcmp(s, "MAP_FILE") == 0)
        {
            char *str2 = calloc(10, sizeof(char));
            str2[0] = 8;
            strcpy(str2 + 1, s);
            write(fd2, str2, 9);
            int charNr = 0;
            read(fd1, &charNr, 1);
            path = calloc((charNr) + 1, sizeof(char));
            read(fd1, path, charNr);
            fdMap = open(path, O_RDONLY);
            if (fdMap == -1)
            {
                write(fd2, error, 6);
                perror("Could not open input file");
                break;
            }
            sizeMap = lseek(fdMap, 0, SEEK_END);
            lseek(fdMap, 0, SEEK_SET);
            dataMap = (char *)mmap(NULL, sizeMap, PROT_READ, MAP_SHARED, fdMap, 0);
            if (dataMap == (void *)-1)
            {
                write(fd2, error, 6);
                perror("Could not map file");
            }
            else
            {

                write(fd2, success, 8);
            }
        }
        else if (strcmp(s, "READ_FROM_FILE_OFFSET") == 0)
        {
            char *str2 = calloc(23, sizeof(char));
            str2[0] = 21;
            strcpy(str2 + 1, s);
            unsigned int offset = 0;
            read(fd1, &offset, 4);
            unsigned int nr_bytes = 0;
            read(fd1, &nr_bytes, 4);
            if (((offset + nr_bytes) <= memorySize) && ((offset + nr_bytes) <= sizeMap) && (memoryCreation != (void *)-1) && (dataMap != (void *)-1))
            {
                memcpy(memoryCreation, dataMap + offset, nr_bytes);
                write(fd2, str2, 22);
                write(fd2, success, 8);
            }
            else
            {
                write(fd2, str2, 22);
                write(fd2, error, 6);
            }
        }
        else if (strcmp(s, "READ_FROM_FILE_SECTION") == 0)
        {
            char *str2 = calloc(24, sizeof(char));
            str2[0] = 22;
            strcpy(str2 + 1, s);
            unsigned int sectionNr = 0;
            read(fd1, &sectionNr, 4);
            unsigned int offset = 0;
            read(fd1, &offset, 4);
            unsigned int nr_bytes = 0;
            read(fd1, &nr_bytes, 4);
            bool flag = false;
            char *fileRead = calloc((nr_bytes + 2), sizeof(char));
            ok(dataMap, &flag, sectionNr, offset, nr_bytes, fileRead);
            if (flag)
            {
                memcpy(memoryCreation, fileRead, (size_t)nr_bytes);
                write(fd2, str2, 23);
                write(fd2, success, 8);
            }
            else
            {
                write(fd2, str2, 23);
                write(fd2, error, 6);
            }
        }
        else if (strcmp(s, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0)
        {
            char *str2 = calloc(32, sizeof(char));
            str2[0] = 30;
            strcpy(str2 + 1, s);
            unsigned int lOffset = 0;
            read(fd1, &lOffset, 4);
            unsigned int nr_bytes = 0;
            read(fd1, &nr_bytes, 4);
            if (((lOffset + nr_bytes) <= memorySize) && ((lOffset + nr_bytes) <= sizeMap) && (memoryCreation != (void *)-1) && (dataMap != (void *)-1))
            {
                memcpy(memoryCreation, dataMap + lOffset, nr_bytes);
                write(fd2, str2, 31);
                write(fd2, success, 8);
            }
            else
            {
                write(fd2, str2, 31);
                write(fd2, error, 6);
            }
        }
        else if (strcmp(s, "EXIT") == 0)
        {
            close(fd1);
            close(fd2);
            unlink(FIFO_RESP);
            close(fd);
            shm_unlink("/dYHej3");
            break;
        }
    }
    return 0;
}