# OPERATING SYSTEMS - PROJECT 

Project topic: Monitoring changes occurring in directories over time by taking snapshots upon user request.

This project was carried out as part of the "Operating Systems" course at the Faculty of Automation and Computers. 
The program requirements consist of weekly programming tasks in advanced C, aiming to achieve general objectives 
such as working with directories and files, using system calls, processes, and managing these processes. 
The difficulty increases from week to week. I chose to implement a separate branch for each weekly task to be able to compare progress.

Weekly tasks:

# TASK 1: 

The user will be able to specify the monitoring directory as a command-line argument, and the program will track changes occurring in it and its subdirectories.
Each time the program is run, it will update the snapshot of the directory, storing the metadata of each entry.

# TASK 2:

The program's functionality will be updated to accept an unspecified number of command-line arguments, but no more than 10, 
with the specification that no argument will be repeated. The program will process only directories, ignoring other types of arguments. 
The logic for capturing metadata will now apply to all valid received arguments, meaning the program will update snapshots for all directories specified by the user.

In the event of changes at the directory level, the user will be able to compare the previous snapshot of the specified directory with the current one. 
If there are differences between the two snapshots, the old snapshot will be updated with the new information from the current snapshot.

The code's functionality will be extended so that the program receives an additional argument representing the output directory where all 
snapshots of entries from the directories specified on the command line will be stored. This output directory will be specified using the -o option. 
For example, the command to run the program will be: ./program_exe -o output input1 input2 ....

# TASK 3:

Utilize a system call to create a new process for each parsed directory. Develop logic within each child process to create snapshots for the assigned directory. ALL CREATED PROCESSES SHOULD RUN IN PARALLEL. At the end of each child process, the parent will retrieve its status and display a message in the form of:

The process with PID has ended with code where represents the PID of the terminated child process, and represents the exit code with which it terminated. Example of how the program should work: ./program_exe -o output_dir dir1 dir2 dir3

Assuming the program prints a message after each snapshot creation process is completed, the output might look something like this:

Snapshot for Directory 1 created successfully. Snapshot for Directory 2 created successfully. Snapshot for Directory 3 created successfully.

Child Process 1 terminated with PID 123 and exit code 0. Child Process 2 terminated with PID 124 and exit code 0. Child Process 3 terminated with PID 125 and exit code 0. Each message indicates that the snapshot creation process for each directory has been completed without any errors.

# TASK 4:

Identify and isolate potentially dangerous or corrupted files within a specified directory to enhance system security.
This task focuses on implementing a file analysis and isolation mechanism to mitigate security risks posed by malicious or corrupted files. Attackers often exploit vulnerabilities by injecting harmful content into files, necessitating proactive measures to detect and neutralize such threats.

# TASK 5

## File Corruption Detection Script

A corrupted file may contain a sequence of characters without adhering to a specific line structure. Additionally, the number of so-called words may be reduced since this sequence of characters can be fragmented unexpectedly. To evaluate the nature of file corruption, the script will count the number of lines, words, and characters it contains. This aspect can serve as a relevant filter for identifying a potentially dangerous file.

For file evaluation, the following criteria will be applied to the script designed for syntax analysis:

- **Number of lines, words, and characters:** Limits will be established for each of these aspects.
- **Verification of the number of lines and words:** If the file contains fewer than 3 lines and the number of words exceeds 1000 and the number of characters exceeds 2000, it will be considered suspicious.
- **Checking for non-ASCII characters and keywords:** In the case of suspicious behavior, it will be examined whether the character sequence also contains non-ASCII characters or keywords associated with dangerous files ("corrupted", "dangerous", "risk", "attack", "malware", "malicious").

## Classification and Display

If dangerous characteristics are detected, the file will be classified as dangerous, and the script will display only its name on stdout. Otherwise, the script will print "SAFE" to stdout.

## Implementation

This week, we continue analyzing files in the directory provided as an argument to identify and isolate potentially dangerous or corrupted files. Each suspected file will be subjected to a verification by executing the dedicated script. The script will provide relevant information about the file via stdout.

The child process that executes the script will communicate with the parent process through a pipe, transmitting information such as the file name if it is dangerous or the word "SAFE" otherwise. The parent process will retrieve this information and subsequently decide on moving the file to the isolated directory based on the obtained results. Note: if the logic for moving to the isolated directory has been previously executed through different methods, this time only the process receiving the file name through the pipe will perform this move.

Multiple corrupted files may be encountered in the same directory. Thus, we will need pipes to communicate information about these corrupted files between child and parent processes. Each corrupted file will undergo the same verification and script execution procedure as in the previous week, and the resulting information will be transmitted via pipe to the creating process. The creating process will retrieve this information and decide how to handle each corrupted file accordingly, including moving them to the isolated directory.

# Process Overview

1. Identify and isolate potentially dangerous or corrupted files.
2. Execute a script to verify the files' integrity.
3. Communicate results between child and parent processes using pipes.
4. Move files to an isolated directory based on verification results.
