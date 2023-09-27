#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include "fcntl.h"
#include "sys/stat.h"
#include <sys/wait.h>
#include <stdlib.h>
// Created by arie1 on 5/2/2023.

#define BUFFER_SIZE 300
#define FAILURE (-1)

void forkAndCompile(char path[300], char *executableName, int errorsFd);

void executeProgram(char *executableName, char *inputPath, char *outputPath, int errorsFd);

void printError(char *error) {
    write(2, error, strlen(error));
}

int isDirectory(const char *path) {
    struct stat pathStat;
    if (stat(path, &pathStat) != 0) {
        // failed to stat the path
        return 0;
    }
    return S_ISDIR(pathStat.st_mode);
}


int iterateInnerDir(char *fullpath, char *inputPath, char *outputPath, char *dirPath, int errorsFd) {
    // Iterate over the directory
    struct dirent *dirEntry = NULL;
    DIR *dir = opendir(fullpath);
    if (dir == NULL) {
        printError("Error in: opendir");
    }
    struct stat info;
    // Iterate over the files in the directory - readdir is null when there are no more files
    while ((dirEntry = readdir(dir)) != NULL) {
        char filePath[BUFFER_SIZE];
        /*
         * strcpy(filePath, fullpath) copies the contents of the fullpath string into the filePath string.
         * strcat(filePath, "/") appends a forward slash character to the filePath string.
         * strcat(filePath, dirEntry->d_name) appends the name of the current file being processed (stored in
         *  dirEntry->d_name) to the filePath string, creating the full path to the file.
         */
        strcpy(filePath, fullpath);
        strcat(filePath, "/");
        strcat(filePath, dirEntry->d_name);

        int len = strlen(dirEntry->d_name);
        // check if the file is a c file
        if (len >= 2 && strcmp(&dirEntry->d_name[len - 2], ".c") == 0) {
            // the file has a ".c" extension
            // now we need to fork and exec the program with the input and output files
            forkAndCompile(filePath, strtok(dirEntry->d_name, "."), errorsFd);
            // execute the program that was compiled
            executeProgram(strtok(dirEntry->d_name, "."), inputPath, outputPath, errorsFd);


        }
    }
    return 0;
}

void forkAndCompile(char path[300], char *executableName, int errorsFd) {
    // search for empty slot in the array of fd and copy 2 to it
    int stdError = dup(2);
    if (stdError < 0) {
        printError("Error in: dup");
        return;
    }
    // close the std error and redirect it to the errors file
    if (dup2(errorsFd, 2) < 0) {
        printError("Error in: dup2");
        return;
    }
    // fork
    pid_t pid = fork();
    if (pid < 0) {
        printError("Error in: fork");
        return;
    }
    // child process
    if (pid == 0) {
        // compile the file
        char *args[] = {"gcc", path, "-o", executableName, NULL};
        // exec with premmision to run
        execvp(args[0], args);
        printError("Error in: execvp");
        return;
    }
    // parent procces
    int status;
    wait(&status);
    // restore the std error
    dup2(stdError, 2);
    // check if the compilation was successful
    if (status != 0) {
        printError("Error in gcc\n");
        return;
    }
}

void executeProgram(char *executableName, char *inputPath, char *outputPath, int errorsFd) {

    // fork
    pid_t pid = fork();
    if (pid < 0) {
        printError("Error in: fork");
        return;
    }
    // child process
    if (pid == 0) {
        // search for empty slot in the array of fd and copy 2 to it
        int stdError = dup(2);
        if (stdError < 0) {
            printError("Error in: dup");
            return;
        }
        // close the std error and redirect it to the errors file
        if (dup2(errorsFd, 2) < 0) {
            printError("Error in: dup2");
            return;
        }
        // close the standard input
        if (close(0) < 0) {
            printError("Error in: close");
            return;
        }
        int inputFileFD = open(inputPath, O_RDONLY);
        // redirect the standard input
        if (inputFileFD < 0) {
            printError("Error in: open");
            return;
        }
        // close the standard output
        if (close(1) < 0) {
            printError("Error in: close");
            return;
        }
        // redirect the standard output
        int outputFileFD = open(outputPath, O_RDWR | O_APPEND , 0666);
        if (outputFileFD < 0) {
            printError("Error in: open");
            return;
        }
        // execute the program
        // add ./ to the executable name
        strcpy(executableName, "./");
        strcat(executableName, executableName);
        // give access to the executable to run
        chmod(executableName, 0777);

        // add executable permissions to the file
        if (chmod(executableName, 0777) < 0) {
            printError("Error in: chmod");
            return;
        }
        char *args[] = {executableName, inputPath, outputPath, NULL};
        // print the current directory
        execvp(args[0], args);

        // restore the standard error
        dup2(stdError, 2);
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Current working dir: %s\n", cwd);
        } else {
            perror("getcwd() error");
            return;
        }
        perror("Error in: execvp - executeProgram") ;
        close(inputFileFD);
        return;
    }
    // parent procces
    int status;
    wait(&status);
    // restore the std error
    // check if the execution was successful
    if (status != 0) {
        printError("Error in execution\n");
        return;
    }
}

int isFile(const char *path) {
    struct stat pathStat;
    if (stat(path, &pathStat) != 0) {
        // failed to stat the path
        return 0;
    }
    return S_ISREG(pathStat.st_mode);
}

int main(int argc, char *argv[]) {
    // second argument is the config file
    if (argc != 2) {
        printError("Error in: args");
        return -1;
    }
    // open the config file
    int configFile = open(argv[1], O_RDONLY);
    // chdeck if the file is opened successfully
    if (configFile < 0) {
        printError("Error in: open");
        return -1;
    }
    // read the config file - first line needs to be a path to a directory .
    char *dirPath = NULL;
    char *inputPath = NULL;
    char *outputPath = NULL;

    char buffer[450] = {0};
    int readBytes = read(configFile, buffer, 450);
    if (readBytes < 0) {
        printError("Error in: read");
        return -1;
    }
    // get the first line
    dirPath = strtok(buffer, "\n");
    inputPath = strtok(NULL, "\n");
    outputPath = strtok(NULL, "\n");
    // REMOVE THE LAST \r
    dirPath[strlen(dirPath) - 1] = '\0';
    inputPath[strlen(inputPath) - 1] = '\0';
//    outputPath[strlen(outputPath) - 1] = '\0';


    if (!isDirectory(dirPath)) {
        printError("Not a valid directory\n");
        return -1;
    }
    if (!isFile(inputPath)) {
        printError("Input file not exist\n");
        return -1;
    }
    if (!isFile(outputPath)) {
        printError("Output file not exist\n");
        return -1;
    }
    int errorsFile = open("errors.txt", O_CREAT | O_APPEND | O_RDWR, 0666);
    if (errorsFile < 0) {
        printError("Error in: open");
        return -1;
    }
//------------------------iterate over the dir------------------------------------//

    // iterate over the directory
    struct dirent *dirEntry;
    DIR *dir = opendir(dirPath);
    if (dir == NULL) {
        printError("Error in: opendir");
        return -1;
    }

    struct stat info;    // iterate over the files in the directory - readdir is null when there are no more files
    while ((dirEntry = readdir(dir)) != NULL) {
        char fullpath[BUFFER_SIZE];
        strcpy(fullpath, dirPath);
        strcat(fullpath, "/");
        strcat(fullpath, dirEntry->d_name);

        // check if the entry is a directory and not . or .. and is in depth 1 (not recursive) a
        if (dirEntry->d_type == DT_DIR && stat(fullpath, &info) == 0 && S_ISDIR(info.st_mode) &&
            strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0 &&
            strcmp(fullpath, dirPath) != 0) {
            // if entry is a directory and not . or .. and is in depth 1
            // go over the files in the directory entry  - fullpath is the path to the directory entry
            iterateInnerDir(fullpath, inputPath, outputPath, dirPath, errorsFile);
        }
    }

    return 0;
}