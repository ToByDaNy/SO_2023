#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int list(const char *path, int nr)
{
    puts(path);
    return nr;
}

int main(int argc, char **argv)
{
    if (argc >= 2)
    {
        if (strcmp(argv[1], "variant") == 0)
        {
            printf("84112\n");
        }
        else if (strcmp(argv[1],"list") == 0)
        {
            char *c = calloc(128, sizeof(char));
            if(sscanf(argv[argc - 1], "path=%s", c) == 1)
                list(c, argc - 2);
            else
                puts("Path gresit !");
        }
    }
    return 0;
}