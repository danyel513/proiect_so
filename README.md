# TASK 3
Utilize a system call to create a new process for each parsed directory. Develop logic within each child process to create snapshots for the assigned directory. 
ALL CREATED PROCESSES SHOULD RUN IN PARALLEL. At the end of each child process, the parent will retrieve its status and display a message in the form of:

The process with PID has ended with code
where represents the PID of the terminated child process, and represents the exit code with which it terminated.
Example of how the program should work: ./program_exe -o output_dir dir1 dir2 dir3

Assuming the program prints a message after each snapshot creation process is completed, the output might look something like this:

Snapshot for Directory 1 created successfully.
Snapshot for Directory 2 created successfully.
Snapshot for Directory 3 created successfully.

Child Process 1 terminated with PID 123 and exit code 0.
Child Process 2 terminated with PID 124 and exit code 0.
Child Process 3 terminated with PID 125 and exit code 0.

Each message indicates that the snapshot creation process for each directory has been completed without any errors.
