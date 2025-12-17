//Monitor CPU and Memory usage
// Need to be altered before combining all ok
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  
#include <fcntl.h>   
#include <time.h>	//timestamp in logs
#include <signal.h>  //for signal handling

#define MAX_READ_SIZE 1024

void writeToLog(char *message)
	{
	//get the current date and time
	time_t now = time(NULL);
	char *time_string = ctime(&now);
    
	// Remove the "enter" key (newline) from the end of the time string
	time_string[strlen(time_string) - 1] = '\0';

	//format the final log line
	char final_log_entry[MAX_READ_SIZE];
	sprintf(final_log_entry, "[%s] %s\n", time_string, message);

   
	int file_descriptor = open("syslog.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);

	if (file_descriptor == -1) {
    	perror("Error opening log file"); // Print error if file fails
    	return;
	}

	//to write the message to the file
	write(file_descriptor, final_log_entry, strlen(final_log_entry));

	//close the file to save changes
	close(file_descriptor);
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

static volatile sig_atomic_t keep_running = 1;
static void handle_sigint(int signo) {
	(void)signo;
	keep_running = 0;
}

int main(int argc, char **argv) {
	signal(SIGINT, handle_sigint);

	if (argc == 1) {
    	//user menu
    	while (keep_running) {
        	printf("\nSelect option:\n");
        	printf("1) CPU Usage\n");
        	printf("2) Memory Usage\n");
        	printf("3) Both\n");
        	printf("4) Exit\n");
        	printf("Choice: ");
        	int choice = 0;
        	if (scanf("%d", &choice) != 1) {
            	int c;
            	while ((c = getchar()) != '\n' && c != EOF) {}
            	continue;
        	}
        	if (!keep_running) break;
        	if (choice == 1) {
            	getCPUUsage();
        	} else if (choice == 2) {
            	getMemoryUsage();
        	} else if (choice == 3) {
            	getCPUUsage();
            	getMemoryUsage();
        	} else if (choice == 4) {
            	break;
        	} else {
            	printf("Invalid choice\n");
        	}
    	}
    	return 0;
	}

	int do_cpu = 0, do_mem = 0;
	int interval = 0;
	for (int i = 1; i < argc; ++i) {
    	if (strcmp(argv[i], "-c") == 0) do_cpu = 1;
    	else if (strcmp(argv[i], "-m") == 0) do_mem = 1;
    	else if (strcmp(argv[i], "-a") == 0) { do_cpu = do_mem = 1; }
    	else if (strcmp(argv[i], "-i") == 0) {
        	if (i + 1 < argc) {
            	interval = atoi(argv[++i]);
            	if (interval < 1) interval = 1;
        	} else {
            	fprintf(stderr, "Missing value for -i\n");
            	return 1;
        	}
    	} else {
        	fprintf(stderr, "Unknown argument: %s\n", argv[i]);
        	fprintf(stderr, "Usage: %s [-c] [-m] [-a] [-i seconds]\n", argv[0]);
        	return 1;
    	}
	}

	if (!do_cpu && !do_mem) {
    	fprintf(stderr, "Nothing requested. Use -c, -m or -a\n");
    	return 1;
	}

	if (interval <= 0) {
    	if (do_cpu) getCPUUsage();
    	if (do_mem) getMemoryUsage();
    	return 0;
	}

	//loop so user want to repeat monitoring
	while (keep_running) {
    	if (do_cpu) getCPUUsage();
    	if (do_mem) getMemoryUsage();
    	for (int s = 0; s < interval && keep_running; ++s) sleep(1);
	}

	printf("Stopped by user\n");
	return 0;
}
