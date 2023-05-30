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

void print(char* str);

void print(char* str) {
    write(1, str, strlen(str));
    write(1, "\n", 1);
}

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
        print("Enter strings:");
        bytesRead = read(STDIN_FILENO, input, sizeof(input));

        // exit if input is mere an enter
        if (bytesRead == 0 || bytesRead == 1) {
            print("Exit");
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

    execve(cmd2[0], cmd2, envp);

    return 0;
//    char *args[] = { "/usr/bin/ls", "-l -a -h", 0 };
      execve(cmd1[0], cmd1, envp);
}
