// --- 3. Top 5 Processes (reads /proc/[pid]/stat) ---
void listTopProcesses() {
    DIR *dir;
    struct dirent *ent;
    struct Process procs[1024]; // Buffer for processes
    int count = 0;

    if ((dir = opendir("/proc")) == NULL) {
        perror("Cannot open /proc");
        writeLog("ERROR: Cannot open /proc directory.");
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
            // Parsing /proc/[PID]/stat: reading PID, Comm, and fields 14 (utime) & 15 (stime)
            fscanf(fp, "%d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", 
                    &pid, comm, &utime, &stime);
            
            // Remove parentheses from comm name (e.g., "(bash)" -> "bash")
            if(comm[0] == '(') {
                memmove(comm, comm+1, strlen(comm));
                comm[strlen(comm)-1] = '\0';
            }

            procs[count].pid = pid;
            strcpy(procs[count].name, comm);
            procs[count].time = utime + stime; // Total CPU time in clock ticks
            count++;
            fclose(fp);
            
            if(count >= 1024) break; // Safety limit for array
        }
    }
    closedir(dir);

    // Sort the list of processes
    qsort(procs, count, sizeof(struct Process), compareProcesses);

    printf("\n--- Top 5 Processes (by CPU Time) ---\n");
    printf("%-8s %-20s %-10s\n", "PID", "Name", "Time(ticks)");
    printf("----------------------------------------\n");
    
    char logBuffer[512] = "Top 5 Processes: ";
    
    // Print and log the top 5
    for (int i = 0; i < 5 && i < count; i++) {
        printf("%-8d %-20s %-10lu\n", procs[i].pid, procs[i].name, procs[i].time);
        
        char tmp[50];
        snprintf(tmp, sizeof(tmp), "[%s:%d] ", procs[i].name, procs[i].pid);
        strcat(logBuffer, tmp);
    }
    writeLog(logBuffer);
}