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

// CPU USAGE
// Reads stat of CPU
void getCPUUsage() {
	char file_content[MAX_READ_SIZE];
    
	// Variables to hold the numbers from the file
	unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;

	// 1. Open the system file that stores CPU data
	int fd = open("/proc/stat", O_RDONLY);
	if (fd == -1) {
    	perror("Error opening /proc/stat");
    	return;
	}

	// 2. Read the file content into our variable 'file_content'
	int bytes_read = read(fd, file_content, MAX_READ_SIZE - 1);
	close(fd); // We are done reading, so close it.

	if (bytes_read == -1) {
    	perror("Error reading CPU data");
    	return;
	}
    
 
	sscanf(file_content, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
       	&user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);

	//calculate total work time vs idle time
	unsigned long long total_time = user + nice + system + idle + iowait + irq + softirq + steal;
    
	// Idle time is doing nothing
	unsigned long long idle_time = idle + iowait;

	//calculate percentage
   
	unsigned long long time_working = total_time - idle_time;
	double cpu_percentage = ((double)time_working / total_time) * 100.0;

	//print cpu usage
	printf("CPU Usage: %.2f%%\n", cpu_percentage);
   

	//to save result in log file
	char log_message[100];
	sprintf(log_message, "Checked CPU Usage: %.2f%%", cpu_percentage);
	writeToLog(log_message);
}

//MEMORY USAGE
// Reads RAM usage
void getMemoryUsage() {
	char file_content[MAX_READ_SIZE];
	long total_memory = 0;
	long free_memory = 0;

	// open the system file that stores Memory data
	int fd = open("/proc/meminfo", O_RDONLY);
	if (fd == -1) {
    	perror("Error opening /proc/meminfo");
    	return;
	}

	//read the file
	int bytes_read = read(fd, file_content, MAX_READ_SIZE - 1);
	close(fd);

	if (bytes_read == -1) {
    	perror("Error reading Memory data");
    	return;
	}
	file_content[bytes_read] = '\0'; //make sure the string ends properly

   
	//look for the text Memtotal and Memfree in file
	char *p_total = strstr(file_content, "MemTotal:");
	char *p_free  = strstr(file_content, "MemFree:");

	//if text found, value will be read
	if (p_total != NULL) {
    	sscanf(p_total, "MemTotal: %ld", &total_memory);
	}
	if (p_free != NULL) {
    	sscanf(p_free, "MemFree: %ld", &free_memory);
	}

	//calc used memory
	long used_memory = total_memory - free_memory;

	//print result to screen (/1024 to convert to MB)
	printf("\n--- MEMORY STATUS ---\n");
	printf("Total Memory: %ld MB\n", total_memory / 1024);
	printf("Used Memory : %ld MB\n", used_memory / 1024);
	printf("Free Memory : %ld MB\n", free_memory / 1024);
	printf("---------------------\n");

	//to save result in log file
	char log_message[100];
	sprintf(log_message, "Checked Memory: Used %ld MB / Free %ld MB",
        	used_memory / 1024, free_memory / 1024);
	writeToLog(log_message);
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
