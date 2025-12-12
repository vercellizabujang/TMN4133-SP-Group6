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

// ...

// --- Signal Handler ---
void handleSignal(int sig) {
    if (sig == SIGINT) {
        printf("\nExiting... Saving log.\n");
        writeLog("Session ended (Signal SIGINT received).");
        exit(0);
    }
}

// ... inside main() ...
signal(SIGINT, handleSignal);
// --- Menu System ---
void displayMenu() {
    int choice;
    do {
        // ... (Print statements for the menu UI) ...
        
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
        // ... (Help message print statements) ...
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

