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
