#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

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
    long cpu_used;
    double cpu_usage;
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

void sorting(struct Process pro[], int count)
{
    for (int i = 0; i < count; i++)
    {
        int max_indx = i;
        for (int j = i + 1; j < count; j++)
        {
            if (pro[j].cpu_usage > pro[max_indx].cpu_usage)
            {
                max_indx = j;
            }
        }
        struct Process temp = pro[i];
        pro[i] = pro[max_indx];
        pro[max_indx] = temp;
    }
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
                    }

                    print_tree(pro[j].pid, pro, count, depth + 1, exists);
                }
            }
        }
    }
}

void filter_pid(int count, struct Process pro[], int pid)
{
    printf("%-8s %-20s %-8s %-10s %-10s\n",
           "PID", "NAME", "STATE", "CPU%", "RSS");

    for (int i = 0; i < count; i++)
    {
        if (pro[i].pid == pid)
        {
            printf("%-8d %-20s %-8s %-10.2f %-10d\n",
                   pro[i].pid,
                   pro[i].name,
                   pro[i].state,
                   pro[i].cpu_usage,
                   pro[i].VmRss);
        }
    }
}

void filter_process(int count, struct Process pro[], char name[])
{
    printf("%-8s %-20s %-8s %-10s %-10s\n",
           "PID", "NAME", "STATE", "CPU%", "RSS");
    for (int i = 0; i < count; i++)
    {
        if (strcmp(pro[i].name, name) == 0)
        {

            printf("%-8d %-20s %-8s %-10.2f %-10d\n",
                   pro[i].pid,
                   pro[i].name,
                   pro[i].state,
                   pro[i].cpu_usage,
                   pro[i].VmRss);
        }
    }
}
int main()
{
    DIR *dir = opendir("/proc");

    if (dir == NULL)
    {
        perror("opendir");
    }
    else
    {
        while (1)
        {

            struct Process *pro = malloc(10 * sizeof(struct Process));
            int capacity = 10;
            struct dirent *r;
            struct Uid u[100];
            int count = 0;

            // ==============================
            // FIRST PROCESS SNAPSHOT
            // ==============================

            rewinddir(dir);

            while ((r = readdir(dir)) != NULL)
            {
                char path[512];
                char path2[512];

                if (check(r->d_name))
                {
                    snprintf(path, sizeof(path), "/proc/%s/status", r->d_name);
                    snprintf(path2, sizeof(path2), "/proc/%s/stat", r->d_name);

                    FILE *f = fopen(path, "r");
                    FILE *f2 = fopen(path2, "r");

                    if (f == NULL || f2 == NULL)
                    {
                        if (f != NULL)
                            fclose(f);

                        if (f2 != NULL)
                            fclose(f2);

                        continue;
                    }

                    // Read CPU information
                    char stat_buffer[1024];

                    if (fgets(stat_buffer, sizeof(stat_buffer), f2) != NULL)
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
                                   " %c %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
                                   &state,
                                   &dummy, &dummy, &dummy, &dummy,
                                   &dummy, &dummy, &dummy, &dummy,
                                   &dummy, &dummy,
                                   &utime, &stime);

                            pro[count].utime = utime;
                            pro[count].stime = stime;
                        }
                    }

                    // Read process information
                    char buffer[256];

                    pro[count].pid = atoi(r->d_name);

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
                                pro[count].ppid = atoi(result);
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
                                pro[count].VmRss = atoi(result);
                            }
                        }
                    }

                    // Initially zero
                    pro[count].cpu_used = 0;
                    pro[count].cpu_usage = 0;

                    count++;

                    if (count == capacity)
                    {
                        capacity = 2 * capacity;
                        pro = realloc(pro, capacity * sizeof(struct Process));
                    }

                    fclose(f);
                    fclose(f2);
                }
            }

            // ==============================
            // FIRST SYSTEM CPU SNAPSHOT
            // ==============================

            long old_system_cpu = 0;

            FILE *system_file = fopen("/proc/stat", "r");

            if (system_file != NULL)
            {
                char buffer[1024];

                if (fgets(buffer, sizeof(buffer), system_file) != NULL)
                {
                    char cpu[10];
                    long user, nice, system, idle;
                    long iowait, irq, softirq, steal;

                    sscanf(buffer,
                           "%s %ld %ld %ld %ld %ld %ld %ld %ld",
                           cpu,
                           &user,
                           &nice,
                           &system,
                           &idle,
                           &iowait,
                           &irq,
                           &softirq,
                           &steal);

                    old_system_cpu =
                        user + nice + system + idle +
                        iowait + irq + softirq + steal;
                }

                fclose(system_file);
            }

            // Save first process snapshot
            struct Process *old_pro =
                malloc(count * sizeof(struct Process));

            for (int i = 0; i < count; i++)
            {
                old_pro[i] = pro[i];
            }

            // ==============================
            // WAIT 1 SECOND
            // ==============================

            sleep(1);

            // ==============================
            // SECOND PROCESS SNAPSHOT
            // ==============================

            rewinddir(dir);

            while ((r = readdir(dir)) != NULL)
            {
                if (check(r->d_name))
                {
                    char path2[512];

                    snprintf(path2,
                             sizeof(path2),
                             "/proc/%s/stat",
                             r->d_name);

                    FILE *f2 = fopen(path2, "r");

                    if (f2 != NULL)
                    {
                        char stat_buffer[1024];

                        if (fgets(stat_buffer,
                                  sizeof(stat_buffer),
                                  f2) != NULL)
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
                                       " %c %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
                                       &state,
                                       &dummy, &dummy, &dummy, &dummy,
                                       &dummy, &dummy, &dummy, &dummy,
                                       &dummy, &dummy,
                                       &utime, &stime);

                                int pid = atoi(r->d_name);

                                // Find same PID in old snapshot
                                for (int i = 0; i < count; i++)
                                {
                                    if (old_pro[i].pid == pid)
                                    {
                                        long old_cpu =
                                            old_pro[i].utime +
                                            old_pro[i].stime;

                                        long new_cpu =
                                            utime + stime;

                                        pro[i].cpu_used =
                                            new_cpu - old_cpu;

                                        break;
                                    }
                                }
                            }
                        }

                        fclose(f2);
                    }
                }
            }

            // ==============================
            // SECOND SYSTEM CPU SNAPSHOT
            // ==============================

            long new_system_cpu = 0;

            system_file = fopen("/proc/stat", "r");

            if (system_file != NULL)
            {
                char buffer[1024];

                if (fgets(buffer, sizeof(buffer), system_file) != NULL)
                {
                    char cpu[10];
                    long user, nice, system, idle;
                    long iowait, irq, softirq, steal;

                    sscanf(buffer,
                           "%s %ld %ld %ld %ld %ld %ld %ld %ld",
                           cpu,
                           &user,
                           &nice,
                           &system,
                           &idle,
                           &iowait,
                           &irq,
                           &softirq,
                           &steal);

                    new_system_cpu =
                        user + nice + system + idle +
                        iowait + irq + softirq + steal;
                }

                fclose(system_file);
            }

            // ==============================
            // CALCULATE CPU %
            // ==============================

            long system_cpu_used =
                new_system_cpu - old_system_cpu;

            for (int i = 0; i < count; i++)
            {
                if (system_cpu_used > 0)
                {
                    pro[i].cpu_usage =
                        ((double)pro[i].cpu_used /
                         system_cpu_used) *
                        100.0;
                }
                else
                {
                    pro[i].cpu_usage = 0;
                }
            }
            sorting(pro, count);
            // ==============================
            // PRINT PROCESS INFORMATION
            // ==============================

            printf("\nSystem CPU ticks used: %ld\n\n", system_cpu_used);
            char name[100];
            printf("Enter name");
            scanf("%99s", name);
            filter_process(count, pro, name);
            int pid;
            printf("Enter PID: ");
            scanf("%d", &pid);

            filter_pid(count, pro, pid);
            int ucount = 0;
            printf("%-8s %-20s %-8s %-10s %-10s\n",
                   "PID", "NAME", "STATE", "CPU%", "RSS");
            for (int i = 0; i < count; i++)
            {
                printf("%-8d %-20s %-8s %-10.2f %-10d\n",
                       pro[i].pid,
                       pro[i].name,
                       pro[i].state,
                       pro[i].cpu_usage,
                       pro[i].VmRss);
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

            // Print UID counts
            for (int i = 0; i < ucount; i++)
            {
                printf("UID: %d Count: %d\n",
                       u[i].uid,
                       u[i].count);
            }

            // Print process tree
            print_tree(1, pro, count, 0, 1);
            free(old_pro);
            free(pro);
        }
    }

    return 0;
}