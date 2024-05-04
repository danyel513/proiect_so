# Task 4: 
File Analysis and Isolation

# Objective:

Identify and isolate potentially dangerous or corrupted files within a specified directory to enhance system security.

# Description:

This task focuses on implementing a file analysis and isolation mechanism to mitigate security risks posed by malicious or corrupted files. Attackers often exploit vulnerabilities by injecting harmful content into files, necessitating proactive measures to detect and neutralize such threats.

# Implementation Steps:

Rights Checking:

Verify file permissions to identify files lacking necessary access rights.
Create a dedicated process to perform syntactic analysis using a script (e.g., verify_for_malicious.sh) to detect corruption or malicious intent.
Isolate suspicious files in a separate, secure environment upon confirmation of potential threats.

File Syntactic Analysis:

Conduct syntactic analysis of file contents to detect signs of maliciousness or corruption.
Check line, word, and character counts within files.
Search for keywords associated with corrupt or malicious files, such as "corrupt," "dangerous," "risk," "attack," "malware," "malicious," or non-ASCII characters.
If any indicators are found, classify the file as dangerous and proceed with isolation.

Isolation of Dangerous Files:

Move identified dangerous files to a designated directory specified as an argument in the command line, named "isolated_space_dir."
Prevent potential threats from affecting the system by isolating suspicious files.
Enable further investigation of isolated files to assess the extent of potential damage.

Program Invocation:

bash
/program_exe output_directory isolated_space_dir dir1 dir2 dir3

Additional Information:

Introduce a new flag, "-s" (for safe), in the command line before the "isolated_space_dir" argument to signify the directory for isolating potentially dangerous files.
No dedicated snapshot is required for the isolation directory.

 # Outcome:
By implementing this task, we aim to enhance system security by proactively identifying and isolating potentially harmful files, thereby safeguarding critical data and infrastructure against security threats.