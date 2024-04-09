// COTOC DANIEL BENIAMIN
// PROJECT SO

// task 1

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
    int file_type; // = 0 for directory and = 1 for files
    char file_name[FILE_NAME_SIZE];
    long file_id;
    long file_size;

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
        perror("An error has occurred while opening the directory!");
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
        perror("An error occurred while opening the file!");
        exit(-1);
    }

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
        perror("An error has occurred while reading directory/file");
        exit(-1);
    }

    retFile.file_id = file_stat.st_ino;
    retFile.file_size = file_stat.st_size;                    // -> add data to the array
    retFile.file_type = (S_ISDIR(file_stat.st_mode) ? 0 : 1);

    return retFile;
}

void initializeDirectory(int *returnCount, char *dirName, FileMetadata_t files[ARR_SIZE]) // initialize the directory to monitor changes - read the initial information
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

    if(fstat(fin, &file_stat) == -1) // we check if the file status was properly created
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
        perror("An error has occurred while reading file");
        exit(-1);
    }
    if(file_stat.st_size == 0) // if the file is empty => initialize the data about the directory
    {
        lseek(fin, 0, SEEK_SET);
        initializeDirectory(count, name, files);
        if(write(fin, files, sizeof(FileMetadata_t) * (*count)) != sizeof(FileMetadata_t) * (*count))
        {
            perror("Writing file error!");
            exit(-1);
        }
    }
    else // otherwise we read the initial data about the directory
    {
        lseek(fin, 0, SEEK_SET);
        (*count)=(int) (file_stat.st_size/sizeof(FileMetadata_t));
        read(fin, files, sizeof(FileMetadata_t) * (*count));

    }
    close(fin);
}


void printFilesArray(FileMetadata_t *files, int count) // prints on the std output the content of the FilesArray
{
    for(int i=0; i<count; i++)
    {
        printf("%s - %s - %ld - %ld \n", files[i].file_name, files[i].file_type ? "file": "directory", files[i].file_id, files[i].file_size);
    }
    printf("\n");
}

void printSnapshot(int count, FileMetadata_t files[ARR_SIZE]) // write a snapshot of the file list to a text file
{
    int fout = open("snapshot.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    checkFile(fout);

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

int main(int argc, char **argv)
{
    if(argc > 11) // verify the arguments format and number
    {
        perror("The arguments provided in the command line do not meet the requirements OR there are too many arguments!");
        exit(-1);
    }

    // for each directory given as argument -> realize a snapshot

    for (int i = 1; i < argc; ++i)
    {

        FileMetadata_t initialFiles[ARR_SIZE], updateFiles[ARR_SIZE];

        int count = 0;

        emptyResourceFile(&count, initialFiles, argv[i]); // set the initial stats

        // dont forget to delete >>>>>>>>>>>>>>>>>
        initialFiles[0].file_id = 1;
        initialFiles[1].file_id = 2;
        initialFiles[2].file_id = 3;
        initialFiles[3].file_id = 4;
        initialFiles[4].file_id = 5;
        initialFiles[5].file_id = 6;
        // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

        int count2 = 0;
        initializeDirectory(&count2, argv[1], updateFiles); // save the new data

        // dont forget to delete >>>>>>>>>>>>>>>>>
        updateFiles[0].file_id = 1;
        updateFiles[1].file_id = 2;
        updateFiles[2].file_id = 3;
        updateFiles[3].file_id = 4;
        updateFiles[4].file_id = 5;
        updateFiles[5].file_id = 6;
        //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

        if (updateSnapshot(initialFiles, updateFiles, count, count2))
        {
            printf("Changes were found in the directory %s \n", argv[1]);
            printSnapshot(count2, updateFiles);
        }
    }
    return 0;
}

