#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For sleep()
#include <dirent.h> // For directory traversal
#include <ctype.h>  // For isdigit()
#include <time.h>   // Included for context, but not strictly needed here
#include <signal.h> // For continuousMonitor/Signal handling

#define BUFFER_SIZE 1024

// Forward declarations (required because your functions call other students' functions)
void clearScreen();
void getCPUUsage();
void getMemoryUsage();
void writeLog(const char *message);

// Global flag (used in continuousMonitor)
volatile sig_atomic_t keep_running = 1;

// Data structure for process sorting (crucial for listTopProcesses)
struct Process {
    int pid;
    char name[256];
    unsigned long time; // Total CPU time (utime + stime)
};

// Comparison function for qsort() (crucial for listTopProcesses)
int compareProcesses(const void *a, const void *b) {
    struct Process *p1 = (struct Process *)a;
    struct Process *p2 = (struct Process *)b;
    // Sort in descending order based on CPU time
    return (p2->time - p1->time); 
}