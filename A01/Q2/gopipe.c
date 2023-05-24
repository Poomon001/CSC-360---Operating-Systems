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


int main() {
    char *message = "Nothing working just yet... Stay tuned.\n";
    write(1, message, strlen(message));
}
