#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>


typedef struct Head
{
    char name[13];
    char type;
    int offset, size;
}head;

void parse(const char *path, int nr, char **option)
{


    int fp = open(path, O_RDONLY);
    if(fp == -1)
    {
        perror("Could not open output file!");
        close(fp);
        return;
    }

    char MAGIC,NR_OF_SECTIONS;
    short int HEADER_SIZE; 
    int VERSION;
    head* hed = calloc(1,sizeof(head));
    read(fp,&MAGIC,1);
    read(fp,&HEADER_SIZE,2);
    read(fp,&VERSION,4);
    read(fp,&NR_OF_SECTIONS,1);
    read(fp,(hed->name),13);
    read(fp,&(hed->type),1);
    read(fp,&(hed->offset),4);
    read(fp,&(hed->size),4);
    bool flagMagic = false, flagVersion = false, flagSectNr = false, flagSectTypes = false;
    flagMagic = (MAGIC =='S');
    flagVersion = (61<=VERSION && 127>=VERSION);
    flagSectNr = (5<=NR_OF_SECTIONS && 17>=NR_OF_SECTIONS);
    flagSectTypes = ((int)(hed->type)) == 73 || ((int)(hed->type)) == 16 ;
    if(!flagMagic)
    {
        puts("ERROR");
        puts("wrong magic");
        return;
        
    }
    if(!flagVersion)
    {
        puts("ERROR");
        puts("wrong version");
        return;
    }
    if(!flagSectNr)
    {
        puts("ERROR");
        puts("wrong sect_nr");
        return;
    }
    if(!flagSectTypes)
    {
        puts("ERROR");
        puts("wrong sect_types");
        return;
    }
    puts("SUCCESS");

    //read(fp,header,1000);
    //puts(header);
    // sscanf(header,"%c%hd%d%c%s%c%d%d",&MAGIC,&HEADER_SIZE,&VERSION,&NR_OF_SECTIONS,hed->name,&(hed->type),&(hed->offset),&(hed->size));
    // // MAGIC(char),HEADER_SIZE(short int),VERSION(int),NR_OF_SECTIONS(char),HEAD(struct)
    //int SECTION_HEADERS = (int)NR_OF_SECTIONS*sizeof(head);     //NR_OF_SECTIONS*22
    //printf("%c: %hd: %d\n",MAGIC,HEADER_SIZE,VERSION);
    //printf("%c: %hd: %d: %d: %s: %c: %d: %d: %d\n",MAGIC,HEADER_SIZE,VERSION,(int)NR_OF_SECTIONS,hed->name,(hed->type),(hed->offset),(hed->size),SECTION_HEADERS);
    close(fp);
    //hed->name = NULL;
    free(hed);
}

void list(const char *path, int nr, char **option)
{

    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;
    bool flagRecursiv = false, flagSize = false, flagPerm = false;
    long sizeConstrain = 0;
    for (int i = 0; i < nr; i++)
    {
        if (strcmp(option[i], "recursive") == 0)
        {
            flagRecursiv = true;
        }
        if (strstr(option[i], "size_smaller=") == option[i])
        {
            sscanf(option[i], "size_smaller=%ld", &sizeConstrain);
            flagSize = true;
        }
        if (strstr(option[i], "has_perm_write") == option[i])
        {
            flagPerm = true;
        }
    }
    dir = opendir(path);
    if (dir == NULL)
    {
        perror("Could not open directory");
        return;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if (lstat(fullPath, &statbuf) == 0)
            {

                if (flagSize)
                {
                    if (S_ISREG(statbuf.st_mode))
                    {
                        if (sizeConstrain > statbuf.st_size)
                        {
                            if (!flagPerm)
                            {
                                printf("%s\n", fullPath);
                            }
                            else
                            {
                                if (statbuf.st_mode & __S_IWRITE)
                                {
                                    printf("%s\n", fullPath);
                                }
                            }
                        }
                    }
                }
                else if (flagPerm)
                {
                    if (statbuf.st_mode & __S_IWRITE)
                    {
                        printf("%s\n", fullPath);
                    }
                }
                else
                {
                    printf("%s\n", fullPath);
                }
                if (flagRecursiv)
                {
                    if (S_ISDIR(statbuf.st_mode))
                    {
                        list(fullPath, nr, option);
                    }
                }
            }
        }
    }
    closedir(dir);
}

int main(int argc, char **argv)
{
    if (argc >= 2)
    {
        if (strcmp(argv[1], "variant") == 0)
        {
            printf("84112\n");
        }
        else if (strcmp(argv[1], "list") == 0)
        {
            char *c = calloc(128, sizeof(char));
            if (sscanf(argv[argc - 1], "path=%s", c) == 1)
            {
                printf("SUCCESS\n");
                list(c, argc - 3, (argv + 2));
            }
            else
                puts("Invalid directory path!");
        }
        else if (strcmp(argv[1], "parse") == 0)
        {
            char *c = calloc(128, sizeof(char));
            if (sscanf(argv[argc - 1], "path=%s", c) == 1)
            {
                
                parse(c, argc - 3, (argv + 2));
            }
            else
                puts("Invalid directory path!");
        }
        else if (strcmp(argv[1], "corrupted") == 0)
        {
            char *c = calloc(128, sizeof(char));
            if (sscanf(argv[argc - 1], "path=%s", c) == 1)
            {
                printf("SUCCESS\n");
                list(c, argc - 3, (argv + 2));
            }
            else
                puts("Invalid directory path!");
        }
    }
    return 0;
}