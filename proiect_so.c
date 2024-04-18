// COTOC DANIEL BENIAMIN
// PROJECT SO

//task 3

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME_SIZE 100
#define ARR_SIZE 200
#define PATH_SIZE 100
#define BUFFER_SIZE 250

typedef struct File
{
    int file_type;
    char file_name[FILE_NAME_SIZE];
    long file_id;
    long file_size;
    time_t file_last_modified;

} FileMetadata_t;  // save in a structure all the data about a file or a directory

void checkDirectory(DIR *directory) // verify the directory -> if it was properly open or can be used
{
    if(directory == NULL)
    {
        if(errno == ENOENT) // if opendir() returned NULL and errno was set as ENOENT => there is no such directory in the current file path
        {
            perror("Directory does not exist!");
            exit(-1);
        }
        if(errno == ENOTDIR) // if opendir() returned an error (NULL) and errno was set as ENOTDIR => a file was selected instead of a directory
        {
            perror("Then name given as argument is not a directory!");
            exit(-1);
        }
        perror(strerror(errno));
        exit(-1);
    }
}

void checkFile(int f)  // verify that the file was properly opened and can be used
{
    if (f == -1)
    {
        if(errno == EACCES)
        {
            perror("Access to file denied!");
            exit(-1);
        }
        if(errno == EEXIST || errno == EFAULT)
        {
            perror("Path to file error!");
            exit(-1);
        }
        perror(strerror(errno));
        exit(-1);
    }

}

int validateDirectory(char *name) // validates if the name/path given as argument is a directory or not
{
    struct stat file_stat;
    if(stat(name, &file_stat) == -1)
    {
        perror(strerror(errno));
        exit(-1);
    }
    return S_ISDIR(file_stat.st_mode);
}

int compareByID(const void* a, const void* b) // auxiliary function used for qsort() - sort the data by the id of the files
{
    const FileMetadata_t *el1 = (const FileMetadata_t *)a;
    const FileMetadata_t *el2 = (const FileMetadata_t *)b;  //  -> casting the data to FileMetadata_t

    if (el1->file_id < el2->file_id)
    {
        return -1;
    }
    else if (el1->file_id > el2->file_id)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void printFilesArray(FileMetadata_t *files, int count) // prints on the std output the content of the FilesArray
{
    for(int i=0; i<count; i++)
    {
        printf("%s - %s - %ld - %ld - %lld \n", files[i].file_name, files[i].file_type ? "file": "directory", files[i].file_id, files[i].file_size, files[i].file_last_modified);
    }
    printf("\n");
}

FileMetadata_t addData(char *name) // uses file stat to set all the data that we need to know about a file or directory and returns the obtained data
{
    FileMetadata_t retFile;
    struct stat file_stat;
    if (stat(name, &file_stat) == -1) // if the file status returns an error code => stop the program, print a proper message
    {
        if (errno == ENOENT || errno == EFAULT)
        {
            perror("Nonexistent directory or file");
            exit(-1);
        }
        if (errno == EACCES)
        {
            perror("Can't access the directory/file, permission denied");
            exit(-1);
        }
        perror(strerror(errno));
        exit(-1);
    }

    retFile.file_id = file_stat.st_ino;
    retFile.file_size = file_stat.st_size;                    // -> add data to the array
    retFile.file_type = (S_ISDIR(file_stat.st_mode) ? 0 : 1);
    retFile.file_last_modified = file_stat.st_mtime;

    return retFile;
}

void initializeDirectory(int *returnCount, FileMetadata_t files[ARR_SIZE], char *dirName) // initialize the directory to monitor changes - read the initial information
{
    DIR *directory = opendir(dirName);
    checkDirectory(directory); // check that we can read the directory

    int count = *returnCount; // save the files number

    struct dirent *aux;
    while((aux = readdir(directory)) != NULL)
    {
        if(aux->d_name[0] == '.') // skip the . - current directory and .. - parent directory links
        {
            continue;
        }

        char path[PATH_SIZE] = "./"; // create path to directory files
        strcat(path, dirName);
        strcat(path, "/");
        strcat(path, aux->d_name);

        files[count] = addData(path);
        strcpy(files[count].file_name, aux->d_name);
        count++; // -> increment first so if we have a subdirectory, the count will point to the next free spot in the array (no overwriting)

        // if the current file is a subdirectory - read the information about its files and directories
        if(files[count-1].file_type == 0)
        {
            initializeDirectory(&count, path, files);// add the subdirectory files
        }
    }
    closedir(directory);
    *returnCount = count;
}


void emptyResourceFile(int *count, FileMetadata_t files[ARR_SIZE], char *name) // verify if the program was initialized before and save all the data
{
    int fin = open("resource.bin", O_RDWR);// open the resource file
    checkFile(fin);

    struct stat file_stat;

    if(fstat(fin, &file_stat) == -1) // check if the file status was properly created
    {
        if (errno == ENOENT || errno == EFAULT)
        {
            perror("Nonexistent file");
            exit(-1);
        }
        if (errno == EACCES)
        {
            perror("Can't access the file, permission denied");
            exit(-1);
        }
        perror(strerror(errno));
        exit(-1);
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
        initializeDirectory(count, name, files);

        if(write(fin, name, sizeof(char) * FILE_NAME_SIZE) != sizeof(char) * FILE_NAME_SIZE)  // write the directory name
        {
            perror("Writing name in the file - error!");
            exit(-1);
        }

        long size = (long) (sizeof(FileMetadata_t) * (*count));
        if(write(fin, &size, sizeof(long)) != sizeof(long)) // write the data size
        {
            perror("Writing the size in the file - error!");
            exit(-1);
        }

        if(write(fin, files, sizeof(FileMetadata_t) * (*count)) != sizeof(FileMetadata_t) * (*count)) // write all the data
        {
            perror("Writing data in the file - error!");
            exit(-1);
        }
    }

    close(fin);
}


void printSnapshot(char *outDirName, int count, FileMetadata_t files[ARR_SIZE], char *dirName) // write a snapshot of the file list in a text file
{
    int fout;

    char outPath[PATH_SIZE] = "./";
    strcat(outPath, outDirName);
    strcat(outPath, "/");
    strcat(outPath, dirName);
    strcat(outPath, "_snapshot.txt");

    // Set the file access permissions to 0644 (read and write permissions for the owner, and read-only permissions for the group and other users)

    fout = open(outPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    checkFile(fout);


    char dirString[BUFFER_SIZE];
    int dirStringSize = snprintf(dirString, sizeof(dirString), "..............................\n\n DIRECTORY: %s \n\n", dirName);
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

int updateSnapshot(FileMetadata_t initialFiles[ARR_SIZE], FileMetadata_t updateFiles[ARR_SIZE], int count1, int count2) // returns 1 is there were any changes made and 0 otherwise
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

    for(int i=0; i<count1; i++)
    {
        if(initialFiles[i].file_id == updateFiles[i].file_id)
        {
            if(strcmp(initialFiles[i].file_name, updateFiles[i].file_name) != 0)
            {
                printf("Name changed from %s to %s \n", initialFiles[i].file_name, updateFiles[i].file_name);
                return 1; // name changed
            }
            if(initialFiles[i].file_size != updateFiles[i].file_size)
            {
                printf("Size modified for %s from %ld to %ld \n", updateFiles[i].file_name, initialFiles[i].file_size, updateFiles[i].file_size);
                return 1; // size of the file changed
            }
        }
        else
        {
            return 1;
        }
    }
    return 0;
}

pid_t startChildProcess(char *dirName, char *outputDir) // starts a process to make a snapshot about a directory
{
    pid_t pid = fork();
    if(pid < 0)
    {
        perror(strerror(errno));
        exit(-1);
    }
    else if(pid == 0)
    {
        FileMetadata_t initialFiles[ARR_SIZE], updateFiles[ARR_SIZE];

        int count = 0;

        emptyResourceFile(&count, initialFiles, dirName); // set the initial stats

        // don't forget to delete >>>>>>>>>>>>>>>>>
        initialFiles[0].file_id = 1;
        initialFiles[1].file_id = 2;
        initialFiles[2].file_id = 3;
        initialFiles[3].file_id = 4;
        initialFiles[4].file_id = 5;
        initialFiles[5].file_id = 6;
        // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        //printFilesArray(initialFiles, count);


        int count2 = 0;
        initializeDirectory(&count2, updateFiles, dirName); // save the new data

        // don't forget to delete >>>>>>>>>>>>>>>>>
        updateFiles[0].file_id = 1;
        updateFiles[1].file_id = 2;
        updateFiles[2].file_id = 3;
        updateFiles[3].file_id = 4;
        updateFiles[4].file_id = 5;
        updateFiles[5].file_id = 6;
        //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        //printFilesArray(initialFiles, count);

        if (updateSnapshot(initialFiles, updateFiles, count, count2))
        {
            printf(" => Changes were found in the directory %s \n", dirName);
            printSnapshot(outputDir, count2, updateFiles, dirName);
        }

        printf("Print snapshot of directory %s successfully", dirName);
        exit(0);
    }
    else
    {
        return pid;
    }
}

int found(pid_t val, const pid_t* array, int size) // binary search for the pid val in the array
{
    int left = 0, right = size - 1;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        if (array[mid] == val)
            return 1;

        if (array[mid] < val)
            left = mid + 1;
        else
            right = mid - 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    if(argc > 13 || argc < 4) // verify the arguments format and number
    {
        perror("The arguments provided in the command line do not meet the requirements OR there are too many arguments!");
        exit(-1);
    }

    if(strcmp(argv[1], "-o") != 0)
    {
        perror("Missing the output directory argument: -o output_dir_name");
        exit(-1);
    }

    pid_t retPid[11];  // save all the process ids that were started
    int pidCount = 0;

    for (int i = 3; i < argc; ++i) // for each directory given as argument -> start a new process and run the child code
    {
        if(!validateDirectory(argv[i]))
        {
            continue;
        }
        retPid[i] = startChildProcess(argv[i], argv[2]); // -> parallelism = starting all the processes and wait for the one that comes first
        pidCount++;
    }
    for(int i=0; i<argc; i++)
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
            if(found(waitPid, retPid, pidCount))
            {
                printf("Print snapshot of directory %s was successfully done - process ended.", argv[i]);
            }
        }
    }
    return 0;
}