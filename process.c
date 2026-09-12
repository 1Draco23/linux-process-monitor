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
    char state[100];
    int VmRss;
    long utime;
    long stime;
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
    if (dir == NULL)
    {
        perror("Error while opening while");
    }
    else
    {
        struct Process *pro = malloc(10 * sizeof(struct Process));
        int capacity = 10;
        struct dirent *r;
        struct Uid u[100];
        int count = 0;
        while ((r = readdir(dir)) != NULL)
        {
            char path[512];
            char path2[512];
            if (check(r->d_name))
            {
                snprintf(path, sizeof(path), "/proc/%s/status", r->d_name);
                snprintf(path2, sizeof(path2), "/proc/%s/stat", r->d_name);

                FILE *f = fopen(path, "r");
                FILE *f1 = fopen(path2, "r");
                if (f == NULL || f1 == NULL)
                {
                    perror("fopen");
                }
                else
                {
                    char stat_buffer[1024];

                    if (fgets(stat_buffer, sizeof(stat_buffer), f1) != NULL)
                    {
                        char *p = strrchr(stat_buffer, ')');

                        if (p != NULL)
                        {
                            p++;

                            char state;
                            long dummy;
                            long utime;
                            long stime;

                            sscanf(p,
                                   " %c %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
                                   &state,
                                   &dummy, &dummy, &dummy, &dummy,
                                   &dummy, &dummy, &dummy, &dummy,
                                   &dummy, &dummy, &dummy, &dummy,
                                   &utime, &stime);

                            pro[count].utime = utime;
                            pro[count].stime = stime;
                        }
                    }
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
                        else if (strncmp(buffer, "State: ", 6) == 0)
                        {
                            char result[50];
                            if (sscanf(buffer + 6, "%49s", result) == 1)
                            {
                                strcpy(pro[count].state, result);
                            }
                        }
                        else if (strncmp(buffer, "VmRSS: ", 6) == 0)
                        {
                            char result[50];
                            if (sscanf(buffer + 6, "%49s", result) == 1)
                            {
                                int a = atoi(result);
                                pro[count].VmRss = a;
                            }
                        }
                    }
                    count++;
                    if (count == capacity)
                    {
                        capacity = 2 * capacity;
                        pro = realloc(pro, capacity * sizeof(struct Process));
                    }
                    fclose(f);
                    fclose(f1);
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
            printf("State: %s\n", pro[i].state);
            printf("VmRSS: %d\n", pro[i].VmRss);
            printf("Utime: %ld\n", pro[i].utime);
            printf("Stime: %ld\n", pro[i].stime);
            int c = atoi(pro[i].uid);
            int found = 0;
            for (int j = 0; j < ucount; j++)
            {
                if (u[j].uid == c)
                {
                    u[j].count++;
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
        free(pro);
    }

    closedir(dir);
    return 0;
}