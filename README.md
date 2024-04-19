# OPERATING SYSTEMS - PROJECT 

Project topic: Monitoring changes occurring in directories over time by taking snapshots upon user request.

This project was carried out as part of the "Operating Systems" course at the Faculty of Automation and Computers. 
The program requirements consist of weekly programming tasks in advanced C, aiming to achieve general objectives 
such as working with directories and files, using system calls, processes, and managing these processes. 
The difficulty increases from week to week. I chose to implement a separate branch for each weekly task to be able to compare progress.

Weekly tasks:

TASK 1: 

The user will be able to specify the monitoring directory as a command-line argument, and the program will track changes occurring in it and its subdirectories.
Each time the program is run, it will update the snapshot of the directory, storing the metadata of each entry.

TASK 2:

The program's functionality will be updated to accept an unspecified number of command-line arguments, but no more than 10, 
with the specification that no argument will be repeated. The program will process only directories, ignoring other types of arguments. 
The logic for capturing metadata will now apply to all valid received arguments, meaning the program will update snapshots for all directories specified by the user.

In the event of changes at the directory level, the user will be able to compare the previous snapshot of the specified directory with the current one. 
If there are differences between the two snapshots, the old snapshot will be updated with the new information from the current snapshot.

The code's functionality will be extended so that the program receives an additional argument representing the output directory where all 
snapshots of entries from the directories specified on the command line will be stored. This output directory will be specified using the -o option. 
For example, the command to run the program will be: ./program_exe -o output input1 input2 ....

TASK 3:

Utilize a system call to create a new process for each parsed directory. Develop logic within each child process to create snapshots for the assigned directory. ALL CREATED PROCESSES SHOULD RUN IN PARALLEL. At the end of each child process, the parent will retrieve its status and display a message in the form of:

The process with PID has ended with code where represents the PID of the terminated child process, and represents the exit code with which it terminated. Example of how the program should work: ./program_exe -o output_dir dir1 dir2 dir3

Assuming the program prints a message after each snapshot creation process is completed, the output might look something like this:

Snapshot for Directory 1 created successfully. Snapshot for Directory 2 created successfully. Snapshot for Directory 3 created successfully.

Child Process 1 terminated with PID 123 and exit code 0. Child Process 2 terminated with PID 124 and exit code 0. Child Process 3 terminated with PID 125 and exit code 0.
Each message indicates that the snapshot creation process for each directory has been completed without any errors.
