#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "disk.h"


int main(int argc, char *argv[]) {
    superblock_entry_t sb;
    int  i;
    char *imagename = NULL;
    FILE  *f;
    int   *fat_data;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL)
    {
        fprintf(stderr, "usage: stat360fs --image <imagename>\n");
        exit(1);
    }
    //Pseudo Code Starts Here

    //step 1

    if ((f = fopen(imagename, "r")) == NULL) {
        fprintf(stderr, "unable to open %s\n", imagename);
        exit(1);
    }

    if (fread(&sb, sizeof(sb), 1, f) != 1) {
        fprintf(stderr, "problems reading superblock\n");
    }

    // TODO: Double check this
    if (strncmp(sb.magic, FILE_SYSTEM_ID, FILE_SYSTEM_ID_LEN) != 0) {
        fprintf(stderr, "%s is not in the proper format\n",
                imagename);
    }

    //step 2

    //Convert from big-endian to host's byte orde uisng ntohl for all block variables you using
    sb.block_size = ntohs(sb.block_size);
    sb.num_blocks = ntohl(sb.num_blocks);
    sb.fat_start = ntohl(sb.fat_start);
    sb.fat_blocks = ntohl(sb.fat_blocks);
    sb.dir_start = ntohl(sb.dir_start);
    sb.dir_blocks = ntohl(sb.dir_blocks);

    char *filename = strrchr(imagename, '/');

    if (filename != NULL) {
        filename++; // remove '/'
    } else {
        filename = imagename;
    }

    printf("%s (%s)\n\n", sb.magic, filename);
    printf("------------------------------------------------\n");
    printf("Bsz   Bcnt   FATcnt    DIRst    DIRcnt\n");
    printf("%-5d %-9d %-8d %-10d %-10d\n", sb.block_size, sb.num_blocks, sb.fat_blocks, sb.dir_start, sb.dir_blocks);
    printf("\n");
    printf("\n");
    // Step 3

    /* Read in the FAT as an array of integers */
    fseek(f, sb.fat_start * sb.block_size, SEEK_SET);

    int num_fat = sb.fat_blocks * sb.block_size / SIZE_FAT_ENTRY;

    fat_data = malloc(num_fat * sizeof(int));

    if (fat_data == NULL) {
        fprintf(stderr, "problems allocating memory for fat\n");
        exit(1);
    }

    //declare and initialize some importatn variables to zero
    int num_available = 0;
    int num_reserved = 0;
    int num_allocated = 0;

    if (fread(fat_data, SIZE_FAT_ENTRY, num_fat, f) != num_fat) {
        fprintf(stderr, "Error reading FAT from image -- number " \
            "of FAT entries not what expected.\n");
        exit(1);
    }

    //step 4

    for (i = 0; i < num_fat; i++) {
        fat_data[i] = ntohl(fat_data[i]);
        if (fat_data[i] == FAT_AVAILABLE) {
            num_available++;
        } else if (fat_data[i] == FAT_RESERVED) {
            num_reserved++;
        } else {
            num_allocated++;
        }
    }

    printf("------------------------------------------------\n");
    printf("Free    Resv    Alloc\n");
    printf("%-9d %-8d %-5d\n", num_available, num_reserved, num_allocated);

    fclose(f);
    free(fat_data);
    return 0;
}