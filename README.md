SysMonitor++ - Linux System Resource Monitoring Tool

Course: TMN4133 System Programming 


Deadline: Friday, 19th December 2025 


Platform: Linux (Requires /proc filesystem) 


Language: C (using system calls) 

Description
SysMonitor++ is a lightweight command-line utility designed to monitor essential system resources in real-time without external dependencies like htop. It interacts directly with the Linux /proc pseudo-filesystem to provide accurate snapshots of CPU usage, Memory consumption, and active Process statistics.

It supports both an interactive Menu Mode for easy navigation and a Command-Line Interface (CLI) mode for quick execution and scripting.




Features

CPU Usage Monitoring: Calculates current CPU load percentage by reading /proc/stat.



Memory Analysis: Retrieves Total, Used, and Free memory statistics from /proc/meminfo.



Process Management: Lists the Top 5 active processes sorted by CPU consumption by traversing the /proc/ directory.




Continuous Monitoring: A live dashboard mode that refreshes at a user-defined interval.




Event Logging: Automatically logs actions and session termination timestamps to syslog.txt.




Graceful Exit: Catches SIGINT (Ctrl+C) signals to ensure logs are saved properly before exiting.

Prerequisites

Operating System: Linux (Ubuntu, Fedora, etc.) or WSL (Windows Subsystem for Linux).


Compiler: GNU GCC Compiler.

Permissions: Read access to the /proc filesystem.

Compilation
To compile the source code, open your terminal and run the following command:

Bash

gcc sysmonitor.c -o sysmonitor
Usage
1. Interactive Menu Mode
Run the program without arguments to enter the interactive menu:

Bash

./sysmonitor
Menu Options:

CPU Usage: Displays current CPU load.

Memory Usage: Displays memory statistics.

Top 5 Processes: Lists the most active processes.

Continuous Monitoring: Starts a live view (refreshing every 2 seconds).

Exit: Closes the application.

2. Command Line (CLI) Mode
You can run specific modules directly using arguments:

Check CPU Usage:

Bash

./sysmonitor -m cpu
Check Memory Usage:

Bash

./sysmonitor -m mem
List Top 5 Processes:

Bash

./sysmonitor -m proc
Start Continuous Monitoring (Custom Interval): Syntax: ./sysmonitor -c [seconds]

Bash

./sysmonitor -c 2

(Press Ctrl+C to stop continuous monitoring and save logs )

Files

sysmonitor.c: The main source code file.

sysmonitor: The compiled executable binary.


syslog.txt: Automatically generated log file tracking user activity and timestamps.



Logging
The program maintains an audit trail in syslog.txt. Every time a resource is checked or the session is terminated, a timestamped entry is added.

Example Log Output:

Plaintext

[Fri Dec 12 10:00:01 2025] Program started (Menu Mode).
[Fri Dec 12 10:00:05 2025] Checked CPU Usage: 12.50%
[Fri Dec 12 10:00:10 2025] Session ended (Signal SIGINT received).
Team Responsibilities

Student 1 (Architect): Menu System, Argument Parsing, Signal Handling, Documentation/Report.




Student 2 (Resource Monitor): CPU (/proc/stat) and Memory (/proc/meminfo) logic implementation.



Student 3 (Process Manager): Top 5 Processes logic, Continuous Monitoring loop, Presentation Video Editing.




Student 4 (QA & Logger): Logging system, Error handling (perror), Testing (T1-T4), GitHub Repository management.
