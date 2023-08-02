#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "disk.h"

char *month_to_string(short m) {
    switch(m) {
        case 1: return "Jan";
        case 2: return "Feb";
        case 3: return "Mar";
        case 4: return "Apr";
        case 5: return "May";
        case 6: return "Jun";
        case 7: return "Jul";
        case 8: return "Aug";
        case 9: return "Sep";
        case 10: return "Oct";
        case 11: return "Nov";
        case 12: return "Dec";
        default: return "?!?";
    }
}


void unpack_datetime(unsigned char *time, short *year, short *month,
                     short *day, short *hour, short *minute, short *second)
{
    assert(time != NULL);

    memcpy(year, time, 2);
    *year = htons(*year);

    *month = (unsigned short)(time[2]);
    *day = (unsigned short)(time[3]);
    *hour = (unsigned short)(time[4]);
    *minute = (unsigned short)(time[5]);
    *second = (unsigned short)(time[6]);
}


int main(int argc, char *argv[]) {
    superblock_entry_t sb;
    int  i;
    char *imagename = NULL;
    FILE *f;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL)
    {
        fprintf(stderr, "usage: ls360fs --image <imagename>\n");
        exit(1);
    }

    //Pseudo Code Starts Here

    //step 1

    if ((f = fopen(imagename, "r")) == NULL) {
        fprintf(stderr, "unable to open %s\n", imagename);
        exit(1);
    }

    // TODO???
    if (fread(&sb, sizeof(sb), 1, f) != 1) {
        fprintf(stderr, "problems reading superblock\n");
        exit(1);
    }

    if (strncmp(sb.magic, FILE_SYSTEM_ID, FILE_SYSTEM_ID_LEN) != 0) {
        fprintf(stderr, "%s is not in the proper format\n", imagename);
        exit(1);
    }

    //step 2

    /* Convert from big-endian to host's byte orde uisng ntohl for all block variables you using */
    sb.block_size = ntohs(sb.block_size);
    sb.num_blocks = ntohl(sb.num_blocks);
    sb.fat_start = ntohl(sb.fat_start);
    sb.fat_blocks = ntohl(sb.fat_blocks);
    sb.dir_start = ntohl(sb.dir_start);
    sb.dir_blocks = ntohl(sb.dir_blocks);

    //step 3
    /* At this point we have what we need from the superblock.
     * We can now "fast-forward" to the area of the disk
     * holding directory entries. As a simplifying assumptions,
     * we can read all of the entries at once.
     */
    int entry_per_block = sb.block_size / SIZE_DIR_ENTRY;  // use block size and directory size
    int num_entries = entry_per_block * sb.dir_blocks;     // use size entries_per_block and dir_blocks

    directory_entry_t *dir = malloc(num_entries * sizeof(directory_entry_t));   // you can use malloc here

    if (dir == NULL) {
        fprintf(stderr, "cat360fs: problems malloc memory for dir\n");
        exit(1);
    }

    fseek(f, sb.dir_start * sb.block_size, SEEK_SET);
    if (fread(dir, sizeof(directory_entry_t), num_entries, f) != num_entries) {
        fprintf(stderr, "cat360fs: problems reading directory from image\n");
        exit(1);
    }

    // step 4
    short date[6];

    for(i = 0; i < num_entries; i++) {
        if (dir[i].status != DIR_ENTRY_AVAILABLE) {
            unpack_datetime(dir[i].modify_time, &date[0], &date[1], &date[2], &date[3], &date[4], &date[5]);

            printf("%-2u %4d-%s-%02d %02d:%02d:%02d %s\n",
                   ntohl(dir[i].file_size),
                   date[0],
                   month_to_string(date[1]),
                   date[2],
                   date[3],
                   date[4],
                   date[5],
                   dir[i].filename);
        }
    }

    free(dir);
    fclose(f);
    return 0;
}