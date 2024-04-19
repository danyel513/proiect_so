# TASK 2:

The program's functionality will be updated to accept an unspecified number of command-line arguments, but no more than 10, with the specification that no argument will be repeated. The program will process only directories, ignoring other types of arguments. The logic for capturing metadata will now apply to all valid received arguments, meaning the program will update snapshots for all directories specified by the user.

In the event of changes at the directory level, the user will be able to compare the previous snapshot of the specified directory with the current one. If there are differences between the two snapshots, the old snapshot will be updated with the new information from the current snapshot.

The code's functionality will be extended so that the program receives an additional argument representing the output directory where all snapshots of entries from the directories specified on the command line will be stored. This output directory will be specified using the -o option. For example, the command to run the program will be: ./program_exe -o output input1 input2 ....
