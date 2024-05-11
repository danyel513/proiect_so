// COTOC DANIEL BENIAMIN
// PROJECT - SO

//task 5

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define FILE_NAME_SIZE 100
#define ARR_SIZE 200
#define PATH_SIZE 100
#define BUFFER_SIZE 250
#define START_ARG_INDEX 5
#define OUT_DIR 2
#define SAFE_DIR 4

typedef struct File
{
    int file_type; // 0 = subdirectory and 1 = file
    char file_name[FILE_NAME_SIZE]; // the name of the file or subdir
    long file_id; // the inode number
    long file_size; // size in bytes
    time_t file_last_modified; // last modified timestamp

} FileMetadata_t;  // save in a structure all the data about a file or a directory (metadata)

// Check if a directory is opened correctly
void checkDirectory(const DIR *directory)
{
    if(directory == NULL)
    {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
}

// Check if a file is opened correctly
void checkFile(const int f)
{
    if (f == -1)
    {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
}

// Validate if name is directory
int validateDirectory(const char *name)
{
    struct stat file_stat;
    if(stat(name, &file_stat) == -1)
    {
        perror(strerror(errno));
        return 0;
    }
    return S_ISDIR(file_stat.st_mode);
}

// Compare two elements (auxiliar for the qsort function)
int compareByID(const void* a, const void* b)
{
    const FileMetadata_t *el1 = (const FileMetadata_t *)a;
    const FileMetadata_t *el2 = (const FileMetadata_t *)b;  //  -> casting the data to FileMetadata_t

    return (el1->file_id - el2->file_id);
}

// Prints on the std output the content of the FilesArray
void printFilesArray(FileMetadata_t *files, const int count)
{
    for(int i=0; i<count; i++)
    {
        printf("%s - %s - %ld - %ld - %ld \n", files[i].file_name, files[i].file_type ? "file": "directory", files[i].file_id, files[i].file_size, files[i].file_last_modified);
    }
    printf("\n");
}

// Binary search for the pid val in the array
int searchPid(pid_t val, const pid_t* array, int size)
{
    int left = 0, right = size - 1;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        if (array[mid] == val)
            return mid;

        if (array[mid] < val)
            left = mid + 1;
        else
            right = mid - 1;
    }
    return -1;
}

// Move a malicious file to the safezone
void isolateFile(const char *path_to_file, const char *destination_folder)
{
    char new_path[PATH_SIZE];
    char *file_name = strrchr(path_to_file, '/');
    snprintf(new_path, sizeof(new_path), "./%s/%s", destination_folder, file_name);
    if (rename(path_to_file, new_path) != 0)
    {
        perror(strerror(errno));
    }
}

// Check if there are all te permisions missing
int hasNoPermissions(const char *path)
{
    struct stat file_stat;
    if (stat(path, &file_stat) == 0)
    {
        if ((file_stat.st_mode & S_IRUSR) ||
            (file_stat.st_mode & S_IWUSR) ||
            (file_stat.st_mode & S_IXUSR) ||
            (file_stat.st_mode & S_IRGRP) ||
            (file_stat.st_mode & S_IWGRP) ||
            (file_stat.st_mode & S_IXGRP) ||
            (file_stat.st_mode & S_IROTH) ||
            (file_stat.st_mode & S_IWOTH) ||
            (file_stat.st_mode & S_IXOTH))
            {
                return 0;
            }
    }
    return 1;
}

// Function to analyze the file syntactically
int analyzeFile(const char *path, const char *isolatedDir)
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == 0) // child code
    {
        printf("----Nephew process started, number of pid: %d---- \n", getpid());
        close(pipefd[0]);  // Close unused read end

        char command[BUFFER_SIZE];
        int status;

        if (chmod(path, S_IRUSR | S_IRGRP | S_IROTH) == -1) // give reading permisions
        {
            perror(strerror(errno));
        }

        snprintf(command, sizeof(command), "./verify_for_malicious.sh %s", path);
        if ((status = system(command)) == -1) // run the script and save the exit status
        {
            perror("Error executing script: /verify_for_malicious.sh");
            exit(EXIT_FAILURE);
        }

        if (chmod(path, 0) == -1) // eliminate the permisions
        {
            perror(strerror(errno));
        }

        if (status == 0)
        {
            // Send the filename through the pipe
            if (write(pipefd[1], path, sizeof(char) * 100) == -1)
            {
                close(pipefd[1]);
                perror("Error: writing the pipe failed");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // Send "SAFE" through the pipe
            if (write(pipefd[1], "SAFE", sizeof(char) * 100) == -1)
            {
                close(pipefd[1]);
                perror("Write failed");
                exit(EXIT_FAILURE);
            }
        }
        close(pipefd[1]);
        exit(EXIT_SUCCESS);
    }
    else if (pid < 0)
    {
        perror(strerror(errno));
    }
    else
    {
        // parent process
        close(pipefd[1]);  // Close unused write end
        pid_t waitPid = wait(NULL);
        printf(" ---end of nephew process pid: %d--- \n", waitPid);

        char result[BUFFER_SIZE];
        ssize_t bytes_read = read(pipefd[0], result, sizeof(char) * 100);
        if (bytes_read == -1)
        {
            perror("Read failed");
            exit(EXIT_FAILURE);
        }

        // Check if the result is a filename or "SAFE"
        if (strcmp(result, "SAFE") != 0)
        {
            isolateFile(result, isolatedDir);
            close(pipefd[0]);
            return 1;
        }
        close(pipefd[0]);
        return 0;
    }
    return 0;
}

// Add metadata for a file
FileMetadata_t addData(const char *name)
{
    FileMetadata_t retFile;
    struct stat file_stat;
    if (stat(name, &file_stat) == -1)
    {
        perror("Error obtaining file metadata");
        memset(&retFile, 0, sizeof(FileMetadata_t)); // return a vid file data
        return retFile;
    }

    retFile.file_id = file_stat.st_ino;
    retFile.file_size = file_stat.st_size;
    retFile.file_type = (S_ISDIR(file_stat.st_mode) ? 0 : 1);
    retFile.file_last_modified = file_stat.st_mtime;

    return retFile;
}

// Get new data for a directory
void getNewData(int *returnCount, FileMetadata_t files[ARR_SIZE], char *dirName, char *isolatedDir)
{
    DIR *directory = opendir(dirName);
    checkDirectory(directory); // check that we can read the directory

    int count = *returnCount; // save the files number

    struct dirent *aux;
    while((aux = readdir(directory)) != NULL)
    {
        // skip the . - current directory and .. - parent directory links
        if(aux->d_name[0] == '.')
        {
            continue;
        }

        // create path to directory files
        char path[PATH_SIZE];
        snprintf(path, PATH_SIZE, "./%s/%s", dirName, aux->d_name);

        // check for malicious file
        if(hasNoPermissions(path) && analyzeFile(path, isolatedDir))
        {
            continue;
        }

        //add metadata
        files[count] = addData(path);
        strcpy(files[count].file_name, aux->d_name);
        count++; // -> increment first so if we have a subdirectory, the count will point to the next free spot in the array (no overwriting)

        // if the current file is a subdirectory - read the information about its files and directories
        if(files[count-1].file_type == 0)
        {
            snprintf(path, PATH_SIZE, "%s/%s", dirName, aux->d_name);
            getNewData(&count, files, path, isolatedDir);// add the subdirectory files
        }
    }
    closedir(directory);
    *returnCount = count;
}

// Get previous data for a directory
void getLastData(int *count, FileMetadata_t files[ARR_SIZE], char *name, char *isolatedDir)
{
    int fin = open("resource.bin", O_RDWR);// open the resource file
    checkFile(fin);

    struct stat file_stat;

    if(fstat(fin, &file_stat) == -1) // check if the file status was properly created
    {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }

    // search for the directory name in the resource file
    lseek(fin, 0, SEEK_SET); // set the file cursor at the beginning of the file
    long i = 0;
    int found = 0;

    while(lseek(fin, i, SEEK_CUR) < file_stat.st_size)  // seek for the directory name until the end of the file is reached
    {
        char string[FILE_NAME_SIZE];
        read(fin, string, sizeof(char) * FILE_NAME_SIZE);
        long size;
        read(fin, &size, sizeof(long));

        if(strcmp(name, string) == 0) // if the directory was found -> read data
        {
            (*count)=(int) (size / sizeof(FileMetadata_t));
            read(fin, files, sizeof(FileMetadata_t) * (*count));
            found = 1;
            break;
        }
        else
        {
            i = size; // if the directory was not found -> add to the offset the size of the current's directory data
        }

    }

    if(!found) // if the directory is not in the file => initialize the data about the directory
    {
        lseek(fin, 0, SEEK_END);
        getNewData(count, files, name, isolatedDir);
        long size = (sizeof(FileMetadata_t) * (*count));

        if(write(fin, name, sizeof(char) * FILE_NAME_SIZE) != sizeof(char) * FILE_NAME_SIZE ||
           write(fin, &size, sizeof(long)) != sizeof(long) ||
           write(fin, files, sizeof(FileMetadata_t) * (*count)) != sizeof(FileMetadata_t) * (*count))
            {
            perror("Error writing data to file");
            exit(EXIT_FAILURE);
           }
    }
    close(fin);
}

// Write a snapshot of the directory to a text file
void printSnapshot(char *outDirName, int count, FileMetadata_t files[ARR_SIZE], char *dirName)
{
    char outPath[PATH_SIZE] = "./";
    snprintf(outPath, PATH_SIZE, "%s/%s_snapshot.txt", outDirName, dirName);

    // Set the file access permissions to 0644 (read and write permissions for the owner, and read-only permissions for the group and other users)

    int fout = open(outPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    checkFile(fout);

    char dirString[BUFFER_SIZE];
    int dirStringSize = snprintf(dirString, sizeof(dirString), "\n DIRECTORY: %s \n\n", dirName);
    write(fout, dirString, dirStringSize);

    // Iterate through the file list and write the details to the snapshot file
    for (int i = 0; i < count; i++)
    {
        char buffer[BUFFER_SIZE];
        int written_bytes = snprintf(buffer, sizeof(buffer), "*** FILE LOG *** \n File name: %s \n File size: %ld \n File type: %s \n File id: %ld \n\n", files[i].file_name, files[i].file_size, (files[i].file_type ? "file" : "directory"), files[i].file_id);
        write(fout, buffer, written_bytes);
    }

    close(fout);
}

// Search for modifications between two sets of metadata
int modificationSearch(FileMetadata_t initialFiles[ARR_SIZE], FileMetadata_t updateFiles[ARR_SIZE], int count1, int count2)
{
    if(count1 != count2) // if the number of files are different, either one file or more were deleted, or some new files were added / created
    {
        if(count1 < count2)
        {
            printf("New files were added! \n");
        }
        else
        {
            printf("Files were deleted! \n");
        }
        return 1;
    }

    qsort(initialFiles, count1, sizeof(FileMetadata_t), compareByID);
    qsort(updateFiles, count2, sizeof(FileMetadata_t), compareByID);

    int found = 0;

    for(int i=0; i<count1; i++)
    {
        if(initialFiles[i].file_id == updateFiles[i].file_id)
        {
            if(strcmp(initialFiles[i].file_name, updateFiles[i].file_name) != 0)
            {
                printf(" Name changed from %s to %s. ", initialFiles[i].file_name, updateFiles[i].file_name);
                found = 1;
            }
            if(initialFiles[i].file_size != updateFiles[i].file_size)
            {
                printf(" Size modified for %s from %ld to %ld. ", updateFiles[i].file_name, initialFiles[i].file_size, updateFiles[i].file_size);
                found = 1;
            }
            if(initialFiles[i].file_last_modified != updateFiles[i].file_last_modified && initialFiles[i].file_type == 1)
            {
                printf(" File %s modified.", updateFiles[i].file_name);
                found = 1;
            }
        }
        else
        {
            found = 1;
        }
    }
    return found;
}

// Start a child process that creates a snapshot for a directory
pid_t startChildProcess(char *dirName, char *outputDir, char *isolatedDir)
{
    pid_t pid = fork();

    if(pid < 0)
    {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }

    else if(pid == 0)
    {
        printf("----Child process started, number of pid: %d---- \n", getpid());
        FileMetadata_t initialFiles[ARR_SIZE], updateFiles[ARR_SIZE];

        int count1 = 0;
        getLastData(&count1, initialFiles, dirName, isolatedDir); // set the initial stats

        int count2 = 0;
        getNewData(&count2, updateFiles, dirName, isolatedDir); // save the new data

        if (modificationSearch(initialFiles, updateFiles, count1, count2))
        {
            printf(" => changes found in the directory %s \n", dirName);
            printSnapshot(outputDir, count2, updateFiles, dirName);
        }

        exit(EXIT_SUCCESS);
    }

    return pid;
}

// Update the resource.bin file with data for all directories - to save the changed directories
void updateResourceFile(int argc, char **argv)
{
    int fout = open("resource.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    checkFile(fout);

    // Iterate through the command line arguments starting from index 3
    for (int i = START_ARG_INDEX; i < argc; ++i)
    {
        if(validateDirectory(argv[i])) {
            FileMetadata_t files[ARR_SIZE];
            int count = 0;
            getNewData(&count, files, argv[i], argv[4]);
            long size = count * sizeof(FileMetadata_t);
            if (write(fout, argv[i], sizeof(char) * FILE_NAME_SIZE) != sizeof(char) * FILE_NAME_SIZE ||
                write(fout, &size, sizeof(long)) != sizeof(long) ||
                write(fout, files, sizeof(FileMetadata_t) * count) != sizeof(FileMetadata_t) * count)
            {
                perror("Error writing data to file");
                exit(EXIT_FAILURE);
            }
        }
    }
    close(fout);
}

int main(int argc, char **argv)
{
    if(argc > 15) // verify the arguments format and number
    {
        perror("The arguments provided in the command line do not meet the requirements OR there are too many arguments!");
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[OUT_DIR - 1], "-o") != 0 || !validateDirectory(argv[OUT_DIR]))
    {
        perror("Missing the output directory argument: -o output_dir_name");
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[SAFE_DIR - 1], "-s") != 0 || !validateDirectory(argv[SAFE_DIR]))
    {
        perror("Missing the safezone directory argument: -s safezone_dir_name");
        exit(EXIT_FAILURE);
    }

    pid_t retPid[11];  // save all the process IDs that were started
    int pidCount = 0;

    for (int i = START_ARG_INDEX; i < argc; ++i) // for each directory given as argument -> start a new process and run the child code
    {
        if(!validateDirectory(argv[i]))
        {
            continue;
        }
        retPid[pidCount] = startChildProcess(argv[i], argv[OUT_DIR], argv[SAFE_DIR]); // -> parallelism = starting all the processes and wait for the one that comes first
        pidCount++;
    }

    for(int i=START_ARG_INDEX; i<argc; i++)
    {
        if(!validateDirectory(argv[i]))
        {
            continue;
        }

        pid_t waitPid = wait(NULL);  // wait for the zombie processes to end and write the suitable message
        if(waitPid == -1 && errno != ECHILD)
        {
            perror(strerror(errno));
        }
        else
        {
            int poz = searchPid(waitPid, retPid, pidCount); // search for the pid in the array to see which process ended first
            if(poz != -1)
            {
                printf("Process started for directory %s was successfully done - process pid: %d ended.\n", argv[START_ARG_INDEX + poz], waitPid);
            }
        }
    }
    updateResourceFile(argc, argv);
    printf("---end of MAIN process--- \n");
    return 0;
}