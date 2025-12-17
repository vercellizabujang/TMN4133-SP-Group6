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

// --- 4. Continuous Monitoring ---
void continuousMonitor(int interval) {
    printf("Starting continuous monitoring (Interval: %ds). Press Ctrl+C to stop.\n", interval);
    writeLog("Started continuous monitoring mode.");
    
    while (keep_running) {
        clearScreen();
        getCPUUsage();
        getMemoryUsage();
        listTopProcesses();
        printf("\nRefreshing in %d seconds...\n", interval);
        sleep(interval);
    }
}

// --- Menu System ---
void displayMenu() {
    int choice;
    do {
        printf("\n=============================\n");
        printf("   SysMonitor++ Main Menu    \n");
        printf("=============================\n");
        printf("1. CPU Usage\n");
        printf("2. Memory Usage\n");
        printf("3. Top 5 Processes\n");
        printf("4. Continuous Monitoring\n");
        printf("5. Exit\n");
        printf("Enter choice: ");
        
        if (scanf("%d", &choice) != 1) {
            // Handle non-integer input
            while(getchar() != '\n'); 
            choice = 0;
        }

        switch (choice) {
            case 1: getCPUUsage(); break;
            case 2: getMemoryUsage(); break;
            case 3: listTopProcesses(); break;
            case 4: 
                continuousMonitor(2); // Default 2 seconds for menu
                break;
            case 5: 
                printf("Exiting...\n"); 
                writeLog("User exited from menu.");
                break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 5);
}

// --- Argument Parsing (Command Line Mode) ---
void parseArguments(int argc, char *argv[]) {
    if (strcmp(argv[1], "-m") == 0) {
        if (argc < 3) {
            printf("Error: missing parameter. Use -m [cpu/mem/proc]\n");
            return;
        }
        if (strcmp(argv[2], "cpu") == 0) getCPUUsage();
        else if (strcmp(argv[2], "mem") == 0) getMemoryUsage();
        else if (strcmp(argv[2], "proc") == 0) listTopProcesses();
        else printf("Invalid option. Use -h for help.\n");
    } 
    else if (strcmp(argv[1], "-c") == 0) {
        int interval = (argc >= 3) ? atoi(argv[2]) : 2; // Default to 2 if not provided
        continuousMonitor(interval);
    }
    else {
        printf("Invalid option. Usage:\n");
        printf("  ./sysmonitor            (Menu Mode)\n");
        printf("  ./sysmonitor -m cpu     (CPU Usage)\n");
        printf("  ./sysmonitor -m mem     (Memory Usage)\n");
        printf("  ./sysmonitor -m proc    (Top Processes)\n");
        printf("  ./sysmonitor -c [sec]   (Continuous Mode)\n");
    }
}

int main(int argc, char *argv[]) {
    // Register Signal Handler
    signal(SIGINT, handleSignal);

    // Check if arguments are provided
    if (argc > 1) {
        parseArguments(argc, argv);
    } else {
        writeLog("Program started (Menu Mode).");
        displayMenu();
    }

    return 0;
}