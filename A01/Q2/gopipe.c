/* gopipe.c
 *
 * CSC 360, Summer 2023
 *
 * Execute up to four instructions, piping the output of each into the
 * input of the next.
 *
 * Please change the following before submission:
 *
 * Author:
 */


/* Note: The following are the **ONLY** header files you are
 * permitted to use for this assignment! */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>

#define MAX_INPUT_CHAR 80
#define MAX_NUM_TOKENS 8
#define MAX_INPUT_LINE 4

int main() {
    char token[MAX_INPUT_LINE][MAX_NUM_TOKENS][MAX_INPUT_CHAR];
    int numLines = 0;
    char *envp[] = { 0 };

    // Initialize the token array with '\0'
    memset(token, '\0', sizeof(token));

    for (int i = 0; i < MAX_INPUT_LINE; i++) {
        int numTokens = 0;
        char input[MAX_INPUT_CHAR];
        char* t;
        int bytesRead;

        // take input
        bytesRead = read(STDIN_FILENO, input, sizeof(input));

        // exit if input is mere an enter
        if (bytesRead == 0 || bytesRead == 1) {
            break;
        }

        // repace newline with '\0'
        if (bytesRead > 0) {
            if (input[bytesRead - 1] == '\n') {
                input[bytesRead - 1] = '\0';
            }

            // add each word to array
            t = strtok(input, " ");
            while (t != NULL && numTokens < MAX_NUM_TOKENS) {
                strcpy(token[numLines][numTokens], t);
                numTokens++;
                t = strtok(NULL, " ");
            }
        }
        numLines++;
    }

    // assign 4 possible command to 4 variables
    char* cmd1[MAX_NUM_TOKENS + 1];
    char* cmd2[MAX_NUM_TOKENS + 1];
    char* cmd3[MAX_NUM_TOKENS + 1];
    char* cmd4[MAX_NUM_TOKENS + 1];
    memset(cmd1, '\0', sizeof(cmd1));
    memset(cmd2, '\0', sizeof(cmd2));
    memset(cmd3, '\0', sizeof(cmd3));
    memset(cmd4, '\0', sizeof(cmd4));

    for (int i = 0; i < numLines; i++) {
        // format input for execve to { "/bin/ls", "-1", NULL }
        int j;
        switch (i) {
            case 0:
                for (j = 0; token[i][j][0] != '\0'; j++) {
                    cmd1[j] = token[i][j];
                }
                break;
            case 1:
                for (j = 0; token[i][j][0] != '\0'; j++) {
                    cmd2[j] = token[i][j];
                }
                break;
            case 2:
                for (j = 0; token[i][j][0] != '\0'; j++) {
                    cmd3[j] = token[i][j];
                }
                break;
            case 3:
                for (j = 0; token[i][j][0] != '\0'; j++) {
                    cmd4[j] = token[i][j];
                }
                break;
            default:
                break;
        }
    }

    // pipe commands
    int pid1, pid2, pid3, pid4;
    int fd1[2], fd2[2], fd3[2];
    int status;

    pipe(fd1);
    pipe(fd2);
    pipe(fd3);

    pid1 = fork();
    if (pid1 == 0) {
        if (cmd1[0] != NULL && cmd2[0] != NULL) {
            dup2(fd1[1], 1);
            close(fd1[0]);
            execve(cmd1[0], cmd1, envp);
        } else {
            execve(cmd1[0], cmd1, envp);
        }
    }

    pid2 = fork();
    if (pid2 == 0) {
        if (cmd2[0] != NULL && cmd3[0] != NULL) {
            dup2(fd1[0], 0);
            dup2(fd2[1], 1);
            close(fd1[1]);
            close(fd2[0]);
            execve(cmd2[0], cmd2, envp);
        } else {
            dup2(fd1[0], 0);
            close(fd1[1]);
            execve(cmd2[0], cmd2, envp);
        }
    }

    close(fd1[1]);
    close(fd1[0]);

    pid3 = fork();
    if (pid3 == 0) {
        if (cmd3[0] != NULL && cmd4[0] != NULL) {
            dup2(fd2[0], 0);
            dup2(fd3[1], 1);
            close(fd2[1]);
            close(fd3[0]);
            execve(cmd3[0], cmd3, envp);
        } else {
            dup2(fd2[0], 0);
            close(fd2[1]);
            execve(cmd3[0], cmd3, envp);
        }
    }

    close(fd2[0]);
    close(fd2[1]);

    pid4 = fork();
    if (pid4 == 0) {
        if (cmd4[0] != NULL) {
            dup2(fd3[0], 0);
            close(fd3[1]);
            execve(cmd4[0], cmd4, envp);
        }
    }

    close(fd3[0]);
    close(fd3[1]);

    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);
    waitpid(pid3, &status, 0);
    waitpid(pid4, &status, 0);
}
