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
//    char *envp[] = { 0 };

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
        // TODO: Cannot use fflush(stdout)?
        if (bytesRead == 0 || bytesRead == 1) {
            print("Exit");
            break;
        }

        if (bytesRead > 0) {
            if (input[bytesRead - 1] == '\n') {
                input[bytesRead - 1] = '\0';
            }

            t = strtok(input, " ");

            while (t != NULL && numTokens < MAX_NUM_TOKENS) {
                strcpy(token[numLines][numTokens], t);
                numTokens++;
                t = strtok(NULL, " ");
            }
        }
        numLines++;
    }

    for (int i = 0; i < numLines; i++) {
        int j = 0;
        while (token[i][j][0] != '\0') {
            print(token[i][j++]);
        }
    }

//    char *args[] = { "/usr/bin/ls", "-1", 0 };
//    execve(args[0], args, envp);
}
