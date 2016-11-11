/* CSci5103 Fall 2016
 * Assignment# 4
 * name: Ajay Sundar Karuppasamy
 * student id: 5298653
 * x500 id: karup002
 * CSELABS machine: csel-kh4240-01.cselabs.umn.edu
 *
 * */


#include "prod_cons.h"

static const char colors[3][6] = {
    "RED",
    "BLACK",
    "WHITE"
};

static const char filename[3][40] = {
    "Producer_RED.txt",
    "Producer_BLACK.txt",
    "Producer_WHITE.txt"
};

int
main(int argc, char **argv) {
    int id; // shared memory id
    shmem_obj_t *shmptr; // shared memory ptr

    int i, err;
    FILE *fp;
    char sbuf[SBUF_MAX] = {0};
    struct timeval tv = {0};

    Color_t mycolor = atoi(argv[1]);

    // get shared memory id using the keystr
    id = shmget (atoi(argv[2]), 0, 0);
    if(id==-1) {
        fprintf(stderr, "Producer %s shmget failed", colors[mycolor]);
        exit(1);
    }

    //attach this to our address space
    shmptr = (shmem_obj_t *) shmat(id, NULL, 0);
    if(shmptr == (void *)-1) {
        fprintf(stderr, "Producer %s shmat failed", colors[mycolor]);
        exit(2);
    }


    for (i=0; i<NUM_ITEMS; ++i) {
        // err= put_item(sbuf);
        if ( err=pthread_mutex_lock(&(shmptr->lock)) ) { // try to acquire lock
            fprintf(stderr,"%s\n", strerror(err));
        }

        // open the output file
        fp = fopen(filename[mycolor],"a");

        while ((shmptr->available)>=BUF_SIZE) {
            while ( pthread_cond_wait( &(shmptr->space_available), &(shmptr->lock) )!=0 ) {
                /* pthread_mutex_unlock(&(shmptr->lock));
                fprintf(stderr, "%s\n",strerror(err)); */
            }
        }

        // printf("Producer %s got lock ; ", colors[buffer[pfill].color]);
        
        gettimeofday(&tv, NULL);
        snprintf(sbuf, SBUF_MAX, "%s %lu\n", colors[mycolor], (1000000*tv.tv_sec + tv.tv_usec));

        strcpy(shmptr->buffer[shmptr->pfill], sbuf);
        shmptr->pfill = (shmptr->pfill+1)%BUF_SIZE;
        (shmptr->available)++;

        // printf("AJ_DEBUG Producer %s writing this %s to file\n",colors[color], sbuf);
        fprintf(fp, "%s", sbuf);

        memset(&tv, 0, sizeof(struct timeval));
        memset(sbuf, 0, SBUF_MAX);
        fclose(fp);

        if( err=pthread_cond_signal(&(shmptr->item_available)) ) {
            /* pthread_mutex_unlock( &(shmptr->lock) );
            fprintf(stderr, "%s\n",strerror(err));*/
        }

        // printf("AJ_DEBUG Producer released lock\n");
        pthread_mutex_unlock( &(shmptr->lock) ); // release lock
    }

    // rest - shm, file closing and signal to main
    shmdt((void *)shmptr);
    printf("Producer done\n");
    return 0;
}
