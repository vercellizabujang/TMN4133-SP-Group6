#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOG_FILE "syslog.txt"
#define BUFFER_SIZE 1024

// --- Function Prototypes ---
void getCPUUsage();
void getMemoryUsage();
void listTopProcesses();
void continuousMonitor(int interval);
void handleSignal(int sig);
void writeLog(const char *message);
void displayMenu();
void clearScreen();
void parseArguments(int argc, char *argv[]);

// Global flag for the main loop
volatile sig_atomic_t keep_running = 1;

// --- Helper: Write to Log with Timestamp ---
void writeLog(const char *message) {
    FILE *fp = fopen(LOG_FILE, "a");
    if (fp == NULL) {
        perror("Error opening log file");
        return;
    }
    
    time_t now;
    time(&now);
    char *date_str = ctime(&now);
    date_str[strlen(date_str) - 1] = '\0'; // Remove newline
    
    fprintf(fp, "[%s] %s\n", date_str, message);
    fclose(fp);
}

// --- Helper: Clear Screen ---
void clearScreen() {
    printf("\033[H\033[J"); // ANSI escape code to clear screen
}

// --- Signal Handler ---
void handleSignal(int sig) {
    if (sig == SIGINT) {
        printf("\nExiting... Saving log.\n");
        writeLog("Session ended (Signal SIGINT received).");
        exit(0);
    }
}

// --- 1. CPU Usage (reads /proc/stat) ---
void getCPUUsage() {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    long double a[4], b[4], loadavg;
    
    // Read 1
    fp = fopen("/proc/stat", "r");
    if (!fp) { perror("Error reading /proc/stat"); return; }
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
    fclose(fp);
    
    sleep(1); // Wait 1 second to calculate delta
    
    // Read 2
    fp = fopen("/proc/stat", "r");
    if (!fp) { perror("Error reading /proc/stat"); return; }
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
    fclose(fp);

    // Calculate Load
    loadavg = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / 
              ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));
    
    float cpu_percent = loadavg * 100;
    
    printf("\n--- CPU Usage ---\n");
    printf("CPU Usage: %.2f%%\n", cpu_percent);
    
    char logMsg[100];
    snprintf(logMsg, sizeof(logMsg), "Checked CPU Usage: %.2f%%", cpu_percent);
    writeLog(logMsg);
}

// --- 2. Memory Usage (reads /proc/meminfo) ---
void getMemoryUsage() {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) { perror("Error reading /proc/meminfo"); return; }
    
    char buffer[BUFFER_SIZE];
    long total_mem = 0, free_mem = 0;
    
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strncmp(buffer, "MemTotal:", 9) == 0) {
            sscanf(buffer, "MemTotal: %ld kB", &total_mem);
        }
        if (strncmp(buffer, "MemFree:", 8) == 0) {
            sscanf(buffer, "MemFree: %ld kB", &free_mem);
        }
    }
    fclose(fp);
    
    long used_mem = total_mem - free_mem;
    printf("\n--- Memory Usage ---\n");
    printf("Total: %ld kB\n", total_mem);
    printf("Used:  %ld kB\n", used_mem);
    printf("Free:  %ld kB\n", free_mem);
    
    char logMsg[100];
    snprintf(logMsg, sizeof(logMsg), "Checked Memory: Used %ld kB / Total %ld kB", used_mem, total_mem);
    writeLog(logMsg);
}

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
        char path[256];
        unsigned long utime, stime;
        char comm[256];

        snprintf(path, sizeof(path), "/proc/%d/stat", pid);
        FILE *fp = fopen(path, "r");
        if (fp) {
            // Parse stat file
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
    
    // FIX APPLIED HERE: Increased buffer size
    char logBuffer[2048] = "Top 5 Processes: ";
    
    for (int i = 0; i < 5 && i < count; i++) {
        printf("%-8d %-20s %-10lu\n", procs[i].pid, procs[i].name, procs[i].time);
        
        // FIX APPLIED HERE: Increased tmp size to handle 256 char names
        char tmp[320]; 
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
        printf("    SysMonitor++ Main Menu     \n");
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
        printf("  ./sysmonitor             (Menu Mode)\n");
        printf("  ./sysmonitor -m cpu      (CPU Usage)\n");
        printf("  ./sysmonitor -m mem      (Memory Usage)\n");
        printf("  ./sysmonitor -m proc     (Top Processes)\n");
        printf("  ./sysmonitor -c [sec]    (Continuous Mode)\n");
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
