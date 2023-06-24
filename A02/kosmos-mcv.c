/*
 * kosmos-mcv.c (mutexes & condition variables)
 *
 * UVic CSC 360, Summer 2023
 *
 * Here is some code from which to start.
 *
 * PLEASE FOLLOW THE INSTRUCTIONS REGARDING WHERE YOU ARE PERMITTED
 * TO ADD OR CHANGE THIS CODE. Read from line 133 onwards for
 * this information.
 */

#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "logging.h"


/* Random # below threshold indicates H; otherwise C. */
#define ATOM_THRESHOLD 0.55
#define DEFAULT_NUM_ATOMS 40

#define MAX_ATOM_NAME_LEN 10
#define MAX_KOSMOS_SECONDS 5

/* Global / shared variables */
int  cNum = 0, hNum = 0;
long numAtoms;


/* Function prototypes */
void kosmos_init(void);
void *c_ready(void *);
void *h_ready(void *);
void make_radical(int, int, int, char *);
void wait_to_terminate(int);


/* Needed to pass legit copy of an integer argument to a pthread */
int *dupInt( int i )
{
    int *pi = (int *)malloc(sizeof(int));
    assert( pi != NULL);
    *pi = i;
    return pi;
}


int main(int argc, char *argv[])
{
    long seed;
    numAtoms = DEFAULT_NUM_ATOMS;
    pthread_t **atom;
    int i;
    int status;

    if ( argc < 2 ) {
        fprintf(stderr, "usage: %s <seed> [<num atoms>]\n", argv[0]);
        exit(1);
    }

    if ( argc >= 2) {
        seed = atoi(argv[1]);
    }

    if (argc == 3) {
        numAtoms = atoi(argv[2]);
        if (numAtoms < 0) {
            fprintf(stderr, "%ld is not a valid number of atoms\n",
                    numAtoms);
            exit(1);
        }
    }

    kosmos_log_init();
    kosmos_init();

    srand(seed);
    atom = (pthread_t **)malloc(numAtoms * sizeof(pthread_t *));
    assert (atom != NULL);
    for (i = 0; i < numAtoms; i++) {
        atom[i] = (pthread_t *)malloc(sizeof(pthread_t));
        if ( (double)rand()/(double)RAND_MAX < ATOM_THRESHOLD ) {
            hNum++;
            status = pthread_create (
                    atom[i], NULL, h_ready,
                    (void *)dupInt(hNum)
            );
        } else {
            cNum++;
            status = pthread_create (
                    atom[i], NULL, c_ready,
                    (void *)dupInt(cNum)
            );
        }
        if (status != 0) {
            fprintf(stderr, "Error creating atom thread\n");
            exit(1);
        }
    }

    /* Determining the maximum number of ethynyl radicals is fairly
     * straightforward -- it will be the minimum of the number of
     * hNum and cNum/2.
     */

    int max_radicals = (hNum < cNum/2 ? hNum : (int)(cNum/2));
#ifdef VERBOSE
    printf("Maximum # of radicals expected: %d\n", max_radicals);
#endif

    wait_to_terminate(max_radicals);
}


/*
* Now the tricky bit begins....  All the atoms are allowed
* to go their own way, but how does the Kosmos ethynyl-radical
* problem terminate? There is a non-zero probability that
* some atoms will not become part of a radical; that is,
* many atoms may be blocked on some condition variable of
* our own devising. How do we ensure the program ends when
* (a) all possible radicals have been created and (b) all
* remaining atoms are blocked (i.e., not on the ready queue)?
*/


/*
 * ^^^^^^^
 * DO NOT MODIFY CODE ABOVE THIS POINT.
 *
 *************************************
 *************************************
 *
 * ALL STUDENT WORK MUST APPEAR BELOW.
 * vvvvvvvv
 */


/* 
 * DECLARE / DEFINE NEEDED VARIABLES IMMEDIATELY BELOW.
 */
int radicals;
int num_free_c;
int num_free_h;

int combining_c1;
int combining_c2;
int combining_h;
char combiner[MAX_ATOM_NAME_LEN];

pthread_mutex_t mutex;
pthread_mutex_t wait_c;
pthread_mutex_t wait_h;
pthread_mutex_t staging_area;



/*
 * FUNCTIONS YOU MAY/MUST MODIFY.
 */

void kosmos_init() {
    radicals = 0;
    num_free_c = cNum;
    num_free_h = hNum;
    combining_c1 = 0;
    combining_c2 = 0;
    combining_h = 0;
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&wait_c, NULL);
    pthread_mutex_init(&wait_h, NULL);
    pthread_mutex_init(&staging_area, NULL);
}

void setCombinder(int id, char* name, char* status) {
    if (combining_h == 0 && combining_c1 != 0 && combining_c2 != 0) {
        sprintf(combiner, "h%03d", id);
    } else if (combining_h != 0 && combining_c1 == 0 && combining_c2 != 0) {
        sprintf(combiner, "c%03d", id);
    } else if (combining_h != 0 && combining_c1 != 0 && combining_c2 == 0) {
        sprintf(combiner, "c%03d", id);
    }

    // tester
    printf("After Name: %s, H: %d, C1: %d, C2: %d, countH: %d, countC: %d, id: %d, combiner: %s, status: %s\n", name, combining_h, combining_c1, combining_c2, num_free_h, num_free_c, id, combiner, status);
}

void tryToCreateRadical() {
    // Check if a radical is ready to be made
    pthread_mutex_lock(&staging_area);
    if (num_free_c >= 2 && num_free_h >= 1) {
        // Make a radical
        num_free_c -= 2;
        num_free_h -= 1;
        radicals += 1;
        kosmos_log_add_entry(radicals, combining_c1, combining_c2, combining_h, combiner);

        combining_c1 = 0;
        combining_c2 = 0;
        combining_h = 0;

        // Unlock mutexes to prevent race condition
        pthread_mutex_unlock(&wait_c);
        pthread_mutex_unlock(&wait_h);
    }
    pthread_mutex_unlock(&staging_area);
}

void *h_ready( void *arg )
{
    int id = *((int *)arg);
    char name[MAX_ATOM_NAME_LEN];

    sprintf(name, "h%03d", id);

    pthread_mutex_lock(&wait_h);

    // Lock the mutex for critical section
    pthread_mutex_lock(&mutex);

    // Check for free space
    if (combining_h == 0) {
        setCombinder(id, name, "Non-wait");
        num_free_h += 1;
        combining_h = id;
        tryToCreateRadical();
    }

    pthread_mutex_unlock(&mutex);

#ifdef VERBOSE
    printf("%s now exists\n", name);
#endif

    return NULL;
}


void *c_ready( void *arg )
{
    int id = *((int *)arg);
    char name[MAX_ATOM_NAME_LEN];

    sprintf(name, "c%03d", id);

    pthread_mutex_lock(&wait_c);

    // Lock the mutex for critical section
    pthread_mutex_lock(&mutex);

    // Check for free c1 space
    if (combining_c1 == 0) {
        setCombinder(id, name, "Non-wait");
        num_free_c += 1;
        combining_c1 = id;
        tryToCreateRadical();
        pthread_mutex_unlock(&wait_c);
        pthread_mutex_unlock(&mutex);
    } else if (combining_c2 == 0) {
        setCombinder(id, name, "Non-wait");
        num_free_c += 1;
        combining_c2 = id;
        tryToCreateRadical();
        pthread_mutex_unlock(&mutex);
    }

#ifdef VERBOSE
    printf("%s now exists\n", name);
#endif

    return NULL;
}


void wait_to_terminate(int expected_num_radicals) {
    /* A rather lazy way of doing it, but good enough for this assignment. */
    sleep(MAX_KOSMOS_SECONDS);
    kosmos_log_dump();
    exit(0);
}
