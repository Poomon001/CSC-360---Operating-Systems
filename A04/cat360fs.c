#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "disk.h"


int main(int argc, char *argv[]) {
    superblock_entry_t sb;
    int  i, ii;
    char *imagename = NULL;
    char *filename  = NULL;
    FILE *f;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "--file") == 0 && i+1 < argc) {
            filename = argv[i+1];
            i++;
        }
    }

    //Pseudo Code Starts

    if (imagename == NULL || filename == NULL) {
        fprintf(stderr, "usage: cat360fs --image <imagename> " \
            "--file <filename in image>");
        exit(1);
    }

    if ((f = fopen(imagename, "r")) == NULL) {
        fprintf(stderr, "unable to open %s\n", imagename);
        exit(1);
    }

    if (fread(&sb, sizeof(sb), 1, f) != 1) {
        fprintf(stderr, "problems reading superblock\n");
        exit(1);
    }

    if (strncmp(sb.magic, FILE_SYSTEM_ID, FILE_SYSTEM_ID_LEN) != 0) {
        fprintf(stderr, "%s is not in the proper format\n", imagename);
        exit(1);
    }

    // Step 1
    sb.block_size = ntohs(sb.block_size);
    sb.num_blocks = ntohl(sb.num_blocks);
    sb.fat_start = ntohl(sb.fat_start);
    sb.fat_blocks = ntohl(sb.fat_blocks);
    sb.dir_start = ntohl(sb.dir_start);
    sb.dir_blocks = ntohl(sb.dir_blocks);



    // Step 2

    /* At this point we have what we need from the superblock.
    * We can now "fast-forward" to the area of the disk
    * holding directory entries. As a simplifying assumptions,
    * we can read all of the entries at once.
    */
    // get the entries per block

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

    // step 3
    /* Now let's find that file... */
    ii = -1;
    for (i = 0; i < num_entries; i++) {
        if(dir[i].status == DIR_ENTRY_AVAILABLE) {
            continue;
        }

        if(strcmp(dir[i].filename, filename) == 0) {
            ii = i;
            break;
        }
    }

    // step 4
    /* Hmm. Maybe file doesn't even exist... */
    if (ii == -1) {
        fprintf(stderr, "cat360fs: file not found (%s)\n", filename);
        free(dir);
        fclose(f);
        exit(1);
    }

    //step 5

    /* If we've gotten this far, the file is definitely in the
     * image. So let's read in the FAT as an array of unsigned
     * integers. Don't forget the "big-endian to host" conversion.
     */
    unsigned int *fat;
    int  num_fat_entries = sb.fat_blocks * sb.block_size / SIZE_FAT_ENTRY; // use fat_blocks, block_size, SIZE_FAT_ENTRY.

    //allocate memory for FAT using malloc
    fat = malloc(num_fat_entries * sizeof(unsigned int));

    if (fat == NULL) {
        fprintf(stderr, "cat360fs: problems malloc memory for FAT\n");
        free(dir);
        fclose(f);
        exit(1);
    }

    fseek(f, sb.fat_start * sb.block_size, SEEK_SET);

    if (fread(fat, sizeof(unsigned int), num_fat_entries, f) != num_fat_entries) {
        fprintf(stderr, "cat360fs: fewer FAT entries than expected\n");
        free(fat);
        free(dir);
        fclose(f);
        exit(1);
    }

    for (i = 0; i < num_fat_entries; i++) {
        fat[i] = ntohl(fat[i]);
    }

    // declare and allocate memory for a buffer
    char *buffer = malloc(sb.block_size);
    if (buffer == NULL) {
        fprintf(stderr, "cat360fs: problems malloc memory during file read\n");
        free(fat);
        free(dir);
        fclose(f);
        exit(1);
    }

    // step 6
    /* Now let's read that puppy...  Assume that there is always
     * at least one block for a file (i.e., even if file_size = 0,
     * file will always have one allocated block.
     */
    int current_block = dir[ii].start_block;
    current_block = ntohl(current_block);
    // int current_byte = 0;
    int bytes_read;

    for (;;) {
        /* Position read head over current block using fseek */
        fseek(f, current_block * sb.block_size, SEEK_SET);

        /* Read in that block using memset */
        memset(buffer, 0, sb.block_size);
        bytes_read = fread(buffer, 1, sb.block_size, f); // use fread

        /* Are we past the end? This shouldn't be true... */
        if (bytes_read <= 0) {
            assert(0);
        }

        /* Print as many characters as are in the buffer -- or as
         * many are left in the file given its size. */
        for (i = 0; i < bytes_read; i++) {
            fputc(buffer[i], stdout);
            fflush(stdout);
        }

        /* Read in the next FAT entry. */
        current_block = fat[current_block];
        assert(current_block != FAT_AVAILABLE);
        assert(current_block != FAT_RESERVED);

        if (current_block == FAT_LASTBLOCK) {
            break;
        }
    }

    /* Tidy up */
    free(fat);
    free(dir);
    free(buffer);
    fclose(f);
    return 0;
}