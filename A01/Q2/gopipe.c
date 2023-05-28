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

// TODO: remove this
#include <stdio.h>

#define MAX_INPUT_LINE 4
#define MAX_NUM_TOKENS 8

int main() {
    char input[MAX_INPUT_LINE];
    char* token[MAX_NUM_TOKENS];
    char* t;
    int i;
    int num_tokens;
    


    char *message = "Nothing working just yet... Stay tuned.\n";
    write(1, message, strlen(message));
}
