#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGUMENTS 100
#define MAX_HISTORY_SIZE 100

void printPrompt();
void setEnvVariables(char *argv[], int argc);
void addToHistory(char *command,char **arguments, char *history[MAX_HISTORY_SIZE], int *historySize, int pid)
;
void printHistory(char *history[MAX_HISTORY_SIZE], int historySize) {
    for (int i = 0; i < historySize; i++) {
        // skip the first word in the history entry
        char *command = strchr(history[i], ' ');
        if (command != NULL) {
            command++;  // move past the space
            printf("%s\n", command);
        }
    }
}
int main(int argc, char *argv[]) {
    char command[MAX_COMMAND_LENGTH];
    char *arguments[MAX_ARGUMENTS];
    char* history[MAX_HISTORY_SIZE];
    int historySize = 0;
    setEnvVariables(argv, argc);
    while (1) {
        printPrompt();
        fgets(command, MAX_COMMAND_LENGTH, stdin);
        command[strcspn(command, "\n")] = '\0';

        int i = 0;
        arguments[i] = strtok(command, " ");

        while (arguments[i] != NULL && i < MAX_ARGUMENTS - 1) {
            i++;
            arguments[i] = strtok(NULL, " ");
        }
        arguments[i] = NULL;

        if (strcmp(command, "exit") == 0) {
            // close all alive process and exit
            for (int j = 0; j < historySize; j++) {
                free(history[j]);
            }
            exit(0);
            break;

        } else if (strcmp(command, "cd") == 0) {
            addToHistory(command,arguments, history, &historySize,getpid());
            if (chdir(arguments[1]) == -1) {
                perror("chdir failed");
            }
            continue;
        } else if (strcmp(command, "history") == 0) {
            addToHistory(command,arguments, history, &historySize,getpid());
            printHistory(history, historySize);
            continue;
        }

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork failed");
        } else if (pid == 0) {
            if (execvp(arguments[0], arguments) == -1) {
                perror("execvp failed");
                exit(1);
            }
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
        addToHistory(command,arguments, history, &historySize, pid);

    }
    return 0;
}
/*
 * printPrompt - print the prompt
 * @return: void
 */
void printPrompt() {
    printf("$ ");
    fflush(stdout);
}
/*
 * setEnvVariables - set the PATH environment variable
 * @argv: the command line arguments
 * @argc: the number of command line arguments
 * @return: void
 */

void setEnvVariables(char *argv[], int argc) {
    char *new_path = NULL;
    size_t new_path_len = 0;
    char *current_path = getenv("PATH");

    if (current_path != NULL) {
        new_path_len = strlen(current_path);
    }
    for (int i = 1; i < argc; i++) {
        new_path_len += strlen(argv[i]) + 1;
    }

    new_path = malloc(new_path_len + 1);
    if (new_path == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    new_path[0] = '\0';

    if (current_path != NULL) {
        strcat(new_path, current_path);
        strcat(new_path, ":");
    }

    for (int i = 1; i < argc; i++) {
        strcat(new_path, argv[i]);
        strcat(new_path, ":");
    }

    if (strlen(new_path) > 0 && new_path[strlen(new_path) - 1] == ':') {
        new_path[strlen(new_path) - 1] = '\0';
    }

    setenv("PATH", new_path, 1);

    free(new_path);

}

/*
 * addToHistory - add the command to the history array
 * @command: the command to add to the history array
 * @arguments: the arguments to add to the history array
 * @history: the history array
 * @historySize: the size of the history array
 * @pid: the pid of the process
 * @return: void
 */
void addToHistory(char *command, char **arguments, char *history[MAX_HISTORY_SIZE], int *historySize, int pid){
    if (*historySize == MAX_HISTORY_SIZE) {
        for (int i = 0; i < MAX_HISTORY_SIZE - 1; i++) {
            history[i] = history[i + 1];
        }
        *historySize = MAX_HISTORY_SIZE - 1;
    }
    history[*historySize] = malloc(MAX_COMMAND_LENGTH);
    // concatenate the command and arguments into a single string
    char fullCommand[MAX_COMMAND_LENGTH];
    sprintf(fullCommand, "%s", command);
    for (int i = 0; arguments[i] != NULL; i++) {
        strcat(fullCommand, " ");
        strcat(fullCommand, arguments[i]);
    }
    sprintf(history[*historySize],"%s %d", fullCommand, pid);
    (*historySize)++;
}


