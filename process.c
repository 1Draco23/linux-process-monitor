#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
struct Process
{
    int pid;
    char name[100];
    int ppid;
    char uid[100];
    char gid[100];
};
struct Uid
{
    int uid;
    int count;
};
int check(char *a)
{
    if (*a == '\n')
    {
        return 0;
    }
    for (char *b = a; *b != '\0'; b++)
    {
        if (isdigit(*b))
        {
            continue;
        }
        else
        {
            return 0;
        }
    }
    return 1;
}

int main()
{
    DIR *dir = opendir("/proc");
    DIR *dire;
    struct Process pro[100];
    struct dirent *r;
    struct Uid u[100];
    int count = 0;
    while ((r = readdir(dir)) != NULL)
    {
        char path[512];
        if (check(r->d_name))
        {
            snprintf(path, sizeof(path), "/proc/%s/status", r->d_name);
            FILE *f = fopen(path, "r");
            if (f == NULL)
            {
                perror("fopen");
            }
            else
            {
                char buffer[256];
                int b = atoi(r->d_name);
                pro[count].pid = b;
                while (fgets(buffer, sizeof(buffer), f) != NULL)
                {

                    if (strncmp(buffer, "Name:", 5) == 0)
                    {
                        char result[50];
                        if (sscanf(buffer + 5, "%49s", result) == 1)
                        {
                            strcpy(pro[count].name, result);
                        }
                    }
                    else if (strncmp(buffer, "Uid:", 4) == 0)
                    {
                        char result[50];
                        if (sscanf(buffer + 4, "%49s", result) == 1)
                        {
                            strcpy(pro[count].uid, result);
                        }
                    }
                    else if (strncmp(buffer, "Gid:", 4) == 0)
                    {
                        char result[50];
                        if (sscanf(buffer + 4, "%49s", result) == 1)
                        {
                            strcpy(pro[count].gid, result);
                        }
                    }
                    else if (strncmp(buffer, "PPid:", 5) == 0)
                    {
                        char result[50];
                        if (sscanf(buffer + 5, "%49s", result) == 1)
                        {
                            int a = atoi(result);
                            pro[count].ppid = a;
                        }
                    }
                }
                fclose(f);
                count++;
            }
        }
    }
    int ucount = 0;
    for (int i = 0; i < count; i++)
    {
        printf("PID: %d\n", pro[i].pid);
        printf("Name: %s\n", pro[i].name);
        printf("Ppid: %d\n", pro[i].ppid);
        printf("Uid: %s\n", pro[i].uid);
        printf("Gid: %s\n", pro[i].gid);
        int c = atoi(pro[i].uid);
        int found = 0;
        for (int i = 0; i < ucount; i++)
        {
            if (u[i].uid == c)
            {
                u[i].count++;
                found = 1;
            }
        }
        if (!found)
        {
            u[ucount].uid = c;
            u[ucount].count = 1;
            ucount++;
        }
    }
    for (int i = 0; i < ucount; i++)
    {
        printf("UID: %d Count: %d\n", u[i].uid, u[i].count);
    }

    return 0;
}