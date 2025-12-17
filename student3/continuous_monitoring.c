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