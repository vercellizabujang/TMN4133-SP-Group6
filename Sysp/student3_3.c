// --- 4. Continuous Monitoring ---
void continuousMonitor(int interval) {
    printf("Starting continuous monitoring (Interval: %ds). Press Ctrl+C to stop.\n", interval);
    writeLog("Started continuous monitoring mode.");
    
    // The main loop that continues until Ctrl+C is pressed (handled by Student 1's signal handler)
    while (keep_running) {
        clearScreen();         // (Student 1's or Helper function)
        getCPUUsage();         // (Student 2's function)
        getMemoryUsage();      // (Student 2's function)
        listTopProcesses();    // (Your function)
        printf("\nRefreshing in %d seconds...\n", interval);
        sleep(interval);       // Wait for the specified interval
    }
}