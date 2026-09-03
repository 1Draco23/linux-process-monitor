#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
// Defining Process
struct Process
{
    int pid;
    char name[100];
    int ppid;
    char uid[100];
    char gid[100];
};
// Defining Uid to count number of occurances
struct Uid
{
    int uid;
    int count;
};
// To check if its a number or not
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
// Defining Function to create Tree
void print_tree(int pid, struct Process pro[], int count, int depth, int exists)
{

    for (int i = 0; i < count; i++)
    {
        if (pro[i].pid == pid)
        {
            if (depth == 0)
            {
                printf("Node: %s depth: %d\n", pro[i].name, depth);
            }
            else if (exists)
            {
                printf("%*s", depth * 2, "");
                printf("|----");
                printf("Node: %s depth: %d\n", pro[i].name, depth);
            }
            else
            {
                printf("%*s", depth * 2, "");
                printf("|____");
                printf("Node: %s depth: %d\n", pro[i].name, depth);
            }
            for (int j = 0; j < count; j++)
            {
                if (pro[j].ppid == pro[i].pid)
                {
                    int exists = 0;
                    for (int k = j + 1; k < count; k++)
                    {
                        if (pro[k].ppid == pro[i].pid)
                        {
                            exists = 1;
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    print_tree(pro[j].pid, pro, count, depth + 1, exists);
                }
            }
        }
    }
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
    print_tree(1, pro, count, 0, 1);

    return 0;
}