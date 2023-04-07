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
    short int HEADER_SIZE;
    int VERSION;
    read(fp, &MAGIC, 1);
    read(fp, &HEADER_SIZE, 2);
    read(fp, &VERSION, 4);
    read(fp, &NR_OF_SECTIONS, 1);
    // printf("MAGIC=%c HEADER_SIZE=%hd VERSION=%d NR_OF_SECTIONS=%d\n", MAGIC, HEADER_SIZE, VERSION, (int)NR_OF_SECTIONS);
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

    int **hed = calloc(NR_OF_SECTIONS, sizeof(int *));
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
        char *strr = calloc(hed[i][1]+1, sizeof(char));
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

void findAll(const char *path, bool *flagSUCCES,int* first)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;
    dir = opendir(path);
    
    if (dir == NULL)
    {
        return;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if (lstat(fullPath, &statbuf) == 0)
            {
                if (S_ISREG(statbuf.st_mode))
                {
                    bool flagLines = true;
                    ok(fullPath, flagSUCCES, &flagLines);
                    if (*flagSUCCES && *first == 0)
                    {
                        (*first)++;
                        puts("SUCCES");
                    }
                    if (flagLines)
                    {
                        printf("%s\n", fullPath);
                        *flagSUCCES = true;
                        flagLines = false;
                    }
                }
                if (S_ISDIR(statbuf.st_mode))
                {
                    findAll(fullPath, flagSUCCES,first);
                }
            }
        }
    }
    closedir(dir);
}

void extract(const char *path, int nr, char **option)
{
    int fp = open(path, O_RDONLY);
    if (fp == -1)
    {
        puts("ERROR");
        puts("invalid file");
        close(fp);
        return;
    }
    bool flagSection = false, flagLine = false;
    int nrSection = 0, nrLine = 0;
    for (int i = 0; i < nr; i++)
    {
        if (strstr(option[i], "section=") == option[0])
        {
            sscanf(option[i], "section=%d", &nrSection);
            flagSection = true;
        }
        else if (strstr(option[i], "line=") == option[i])
        {
            sscanf(option[i], "line=%d", &nrLine);
            flagLine = true;
        }
    }
    if (flagSection)
    {
        char NR_OF_SECTIONS;
        lseek(fp, 7, SEEK_CUR);
        read(fp, &NR_OF_SECTIONS, 1);
        if ((int)NR_OF_SECTIONS >= nrSection)
        {
            head htyp;
            int salt = 0;
            for (int i = 0; i < (int)NR_OF_SECTIONS; i++)
            {
                if (i + 1 < nrSection)
                {

                    lseek(fp, 18, SEEK_CUR);
                    int aux = 0;
                    read(fp, &aux, 4); // size
                    salt += aux;
                }
                else if (i + 1 == nrSection)
                {
                    read(fp, (htyp.name), 13);
                    htyp.name[13] = '\0';
                    read(fp, &(htyp.type), 1);
                    read(fp, &(htyp.offset), 4);
                    read(fp, &(htyp.size), 4);
                    salt += htyp.size;
                }
                else
                    break;
            }

            lseek(fp, htyp.offset, SEEK_SET);
            char *strr = calloc(htyp.size, sizeof(char));
            char *links = strr;
            read(fp, strr, htyp.size);
            if (flagLine)
            {

                for (int i = 1; i < nrLine && strlen(strr) > 0; i++)
                {
                    char s[] = "\n";

                    char *s1 = strstr(strr, s);
                    if (s1 == NULL)
                    {
                        flagLine = false;
                        break;
                    }
                    strr = s1;
                    if (strlen(strr) > 1)
                        strr += 1;
                    else
                        break;
                }
                if (flagLine)
                {
                    puts("SUCCESS");
                    char *s = calloc(strlen(strr), sizeof(char));
                    sscanf(strr, "%s\n", s);
                    printf("%s\n", s);
                    free(s);
                }
                else
                {
                    puts("ERROR");
                    puts("invalid line");
                }
            }
            else
            {
                printf("%s\n", links);
            }
            free(links);
        }
        else
        {
            puts("ERROR");
            puts("invalid section");
        }
    }
    close(fp);
    return;
}

void parse(const char *path, int nr, char **option)
{

    int fp = open(path, O_RDONLY);
    if (fp == -1)
    {
        perror("Could not open output file!");
        close(fp);
        return;
    }

    char MAGIC, NR_OF_SECTIONS;
    short int HEADER_SIZE;
    int VERSION;
    read(fp, &MAGIC, 1);
    read(fp, &HEADER_SIZE, 2);
    read(fp, &VERSION, 4);
    read(fp, &NR_OF_SECTIONS, 1);

    bool flagMagic = false, flagVersion = false, flagSectNr = false, flagSectTypes = false;
    flagMagic = (MAGIC == 'S');
    flagVersion = (61 <= VERSION && 127 >= VERSION);
    flagSectNr = (5 <= NR_OF_SECTIONS && 17 >= NR_OF_SECTIONS);
    if (!flagMagic)
    {
        puts("ERROR");
        puts("wrong magic");
        close(fp);

        return;
    }
    if (!flagVersion)
    {
        puts("ERROR");
        puts("wrong version");
        close(fp);

        return;
    }
    if (!flagSectNr)
    {
        puts("ERROR");
        puts("wrong sect_nr");
        close(fp);

        return;
    }
    char *str = calloc(55 * (NR_OF_SECTIONS), sizeof(char));
    char *strc = str;
    int pr = 0;
    for (int i = 0; i < NR_OF_SECTIONS; i++)
    {
        head hed;
        read(fp, (hed.name), 13);
        hed.name[13] = '\0';
        read(fp, &(hed.type), 1);
        read(fp, &(hed.offset), 4);
        read(fp, &(hed.size), 4);
        flagSectTypes = ((int)(hed.type)) == 73 || ((int)(hed.type)) == 16 || ((int)(hed.type)) == 97 || ((int)(hed.type)) == 29;
        if (!flagSectTypes)
        {
            break;
        }
        str = str + pr;
        pr = sprintf((str), "section%d: %s %d %d\n", i + 1, hed.name, (int)hed.type, hed.size);
    }
    str = NULL;
    if (!flagSectTypes)
    {
        close(fp);
        free(strc);
        strc = NULL;
        puts("ERROR");
        puts("wrong sect_types");

        return;
    }
    puts("SUCCESS");
    printf("version=%d\nnr_sections=%d\n%s", VERSION, NR_OF_SECTIONS, strc);

    free(strc);
    strc = NULL;
    close(fp);
    return;
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
        else if (strstr(option[i], "size_smaller=") == option[i])
        {
            sscanf(option[i], "size_smaller=%ld", &sizeConstrain);
            flagSize = true;
        }
        else if (strstr(option[i], "has_perm_write") == option[i])
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
    char *c = calloc(128, sizeof(char));
    if (argc >= 2)
    {
        if (strcmp(argv[1], "variant") == 0)
        {
            printf("84112\n");
        }
        else if (strcmp(argv[1], "list") == 0)
        {
            if (sscanf(argv[argc - 1], "path=%s", c) == 1)
            {
                printf("SUCCESS\n");
                list(c, argc - 3, (argv + 2));
            }
            else
                puts("Invalid directory path!(last argument)");
        }
        else if (strcmp(argv[1], "parse") == 0)
        {
            if (sscanf(argv[argc - 1], "path=%s", c) == 1)
            {

                parse(c, argc - 3, (argv + 2));
            }
            else
                puts("Invalid file path!(last argument)");
        }
        else if (strcmp(argv[1], "extract") == 0)
        {
            if (sscanf(argv[2], "path=%s", c) == 1)
            {

                extract(c, argc - 3, (argv + 3));
            }
            else
                puts("Invalid file path!(first argument)");
        }
        else if (strcmp(argv[1], "findall") == 0)
        {
            if (sscanf(argv[2], "path=%s", c) == 1)
            {
                bool flagSucces = false;
                int first = 0;
                findAll(c, &flagSucces,&first);
                if (!flagSucces)
                {
                    puts("ERROR");
                    puts("invalid directory path");
                }
            }
            else
                puts("Invalid file path!(first argument)");
        }
    }
    free(c);
    return 0;
}