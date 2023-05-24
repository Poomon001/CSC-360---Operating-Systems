/* getstats.c 
 *
 * CSC 360, Summer 2023
 *
 * - If run without an argument, dumps information about the PC to STDOUT.
 *
 * - If run with a process number created by the current user, 
 *   dumps information about that process to STDOUT.
 *
 * Please change the following before submission:
 *
 * Author: 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Note: You are permitted, and even encouraged, to add other
 * support functions in order to reduce duplication of code, or
 * to increase the clarity of your solution, or both.
 */

void printInfo(char* fileName, char* target, int isTimeValue);
void printTimeFromInfo(char* time);
void printSumSwitches(char* filePath, char* switches1, char* switches2);

void printSumSwitches(char* filePath, char* switches1, char* switches2) {
    FILE* file = fopen(filePath, "r");
    char str[255];
    int sum = 0;
    if(file == NULL) {
        printf("Failed to open: %s\n", filePath);
        return;
    }

    while (fgets(str, 255, file) != NULL) {
        if (strncmp(str, switches1, strlen(switches1)) == 0 || strncmp(str, switches2, strlen(switches2)) == 0) {
            if (strrchr(str, '\t') == NULL) {
                sum += atoi(strrchr(str, ' '));
            } else {
                sum += atoi(strrchr(str, '\t'));
            }
        }
    }
    printf("Total context switches: %d\n", sum);
    fclose(file);
}

void print_process_info(char* process_num) {
    // TODO: Can we assumne 255?
    char path[255] = "/proc/";
    strcat(path, process_num);

    FILE* file = fopen(path, "r");

    // TODO: Does this check work?
    if(file == NULL) {
        printf("Process number %s not found\n", process_num);
        return;
    }

    char statusPath[255];
    strcpy(statusPath, path);
    strcat(statusPath, "/status");

    char cmdlinePath[255];
    strcpy(cmdlinePath, path);
    strcat(cmdlinePath, "/comm");

    printf("Process number: %s\n", process_num);
    printInfo(statusPath, "Name", 0);
    printf("Filename (if any): ");
    printInfo(cmdlinePath, "", 0); // TODO: Is this correct?
    printInfo(statusPath, "Threads", 0);
    // TODO: Is these switches correct?
    printSumSwitches(statusPath, "voluntary_ctxt_switches", "nonvoluntary_ctxt_switches");
}

void printTimeFromInfo(char* time) {
    char* tokens = strtok(time, " ");
    double uptime = strtod(tokens, NULL);
    long days, hours, minutes;

    days = uptime / (24 * 3600);
    uptime -= days * (24 * 3600);
    hours = uptime / (3600);
    uptime -= hours * (3600);
    minutes = uptime / 60;
    uptime -= minutes * 60;

    // TODO: Do we need to calculate them manully?
    printf("Uptime: %ld days, %ld hours, %ld minutes, %ld seconds\n", days, hours, minutes, (long)uptime);
}

void printInfo(char* filePath, char* target, int isTimeValue) {
    FILE* file = fopen(filePath, "r");
    char str[255];

    if(file == NULL) {
        printf("Failed to open: %s\n", filePath);
        return;
    }

    while (fgets(str, 255, file) != NULL) {
        if(strncmp(target, str, strlen(target)) == 0) {
            (isTimeValue == 0) ? printf("%s", str) : printTimeFromInfo(str);
            fclose(file);
            return;
        }
    }
    fclose(file);
}

void print_full_info() {
    char cpuinfo[] = "/proc/cpuinfo";
    char version[] = "/proc/version";
    char meminfo[] = "/proc/meminfo";
    char uptime[] = "/proc/uptime";

    printInfo(cpuinfo, "model name", 0);
    printInfo(cpuinfo, "cpu cores", 0);
    printInfo(version, "", 0);
    printInfo(meminfo, "MemTotal", 0);
    printInfo(uptime, "", 1);

    // TODO: What does it for?
//     if(strncmp(substring, str, strlen(substring)) == 0) {
//        fclose(0);
//        return;
//     }
}


int main(int argc, char ** argv) {  
    if (argc == 1) {
        print_full_info();
    } else {
        print_process_info(argv[1]);
    }
}
