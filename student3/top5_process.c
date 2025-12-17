// --- 3. Top 5 Processes (reads /proc/[pid]/stat) ---
struct Process {
    int pid;
    char name[256];
    unsigned long time;
};

int compareProcesses(const void *a, const void *b) {
    struct Process *p1 = (struct Process *)a;
    struct Process *p2 = (struct Process *)b;
    return (p2->time - p1->time); // Descending order
}

void listTopProcesses() {
    DIR *dir;
    struct dirent *ent;
    struct Process procs[1024]; // Simplified buffer for processes
    int count = 0;

    if ((dir = opendir("/proc")) == NULL) {
        perror("Cannot open /proc");
        return;
    }

    while ((ent = readdir(dir)) != NULL) {
        if (!isdigit(*ent->d_name)) continue; // Skip non-PID directories

        int pid = atoi(ent->d_name);
        char path[256], buffer[BUFFER_SIZE];
        unsigned long utime, stime;
        char comm[256];

        snprintf(path, sizeof(path), "/proc/%d/stat", pid);
        FILE *fp = fopen(path, "r");
        if (fp) {
            // Parse stat file. Format is complex, extracting comm and times.
            // Simplified for project: reading PID, Comm, state, ppid... utime(14), stime(15)
            fscanf(fp, "%d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", 
                   &pid, comm, &utime, &stime);
            
            // Remove parentheses from comm name
            if(comm[0] == '(') {
                memmove(comm, comm+1, strlen(comm));
                comm[strlen(comm)-1] = '\0';
            }

            procs[count].pid = pid;
            strcpy(procs[count].name, comm);
            procs[count].time = utime + stime;
            count++;
            fclose(fp);
            
            if(count >= 1024) break; // Safety limit
        }
    }
    closedir(dir);

    // Sort by time
    qsort(procs, count, sizeof(struct Process), compareProcesses);

    printf("\n--- Top 5 Processes (by CPU Time) ---\n");
    printf("%-8s %-20s %-10s\n", "PID", "Name", "Time(ticks)");
    printf("----------------------------------------\n");
    
    char logBuffer[512] = "Top 5 Processes: ";
    
    for (int i = 0; i < 5 && i < count; i++) {
        printf("%-8d %-20s %-10lu\n", procs[i].pid, procs[i].name, procs[i].time);
        
        char tmp[50];
        snprintf(tmp, sizeof(tmp), "[%s:%d] ", procs[i].name, procs[i].pid);
        strcat(logBuffer, tmp);
    }
    writeLog(logBuffer);
}