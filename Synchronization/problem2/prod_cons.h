/* CSci5103 Fall 2016
 * Assignment# 4
 * name: Ajay Sundar Karuppasamy
 * student id: 5298653
 * x500 id: karup002
 * CSELABS machine: 
 *
 * */

#ifndef PROD_CONS_H
#define PROD_CONS_H

#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <time.h>

#define NUM_THREADS 4
#define NUM_ITEMS 1000
#define BUF_SIZE 2
#define SBUF_MAX 100
#define SHM_PPERM 0666

typedef enum {
    red,
    black,
    white
} Color_t;

typedef struct {
    char buffer[BUF_SIZE][SBUF_MAX];
    int pfill;
    int pget;
    int available;

    //locks and conditions
    pthread_mutex_t lock;
    pthread_cond_t item_available;
    pthread_cond_t space_available;
} shmem_obj_t;

#endif
