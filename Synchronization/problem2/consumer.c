/* CSci5103 Fall 2016
 * Assignment# 4
 * name: Ajay Sundar Karuppasamy
 * student id: 5298653
 * x500 id: karup002
 * CSELABS machine: csel-kh4240-01.cselabs.umn.edu
 *
 * */


#include "prod_cons.h"

int
main(int argc, char **argv) {
    int id; // shared memory id
    shmem_obj_t *shmptr; // shared memory ptr

    int i, err;
    FILE *fp;
    char sbuf[SBUF_MAX] = {0};

    // get shared memory id using the keystr
    id = shmget (atoi(argv[1]), 0, 0);
    if(id==-1) {
        perror("Consumer shmget failed");
        exit(1);
    }

    //attach this to our address space
    shmptr = (shmem_obj_t *) shmat(id, NULL, 0);
    if(shmptr == (void *)-1) {
        perror("Consumer shmat failed");
        exit(2);
    }

    for(i=0; i<3*NUM_ITEMS; ++i) {
        if ( err=pthread_mutex_lock(&(shmptr->lock)) ) { // try to acquire lock
            fprintf(stderr,"%s\n", strerror(err));
        }

        // printf("AJ_DEBUG Consumer got lock ; ");

        while(! (shmptr->available) ) { // check if resource available 
            while ( pthread_cond_wait( &(shmptr->item_available), &(shmptr->lock) ) != 0) {
                /* pthread_mutex_unlock ( &(shmptr->lock) ); // Don't leave the resource locked!
                fprintf(stderr, "%s\n",strerror(err));*/
            }
        }

        // open the output file
        fp = fopen("Consumer.txt","a");

        strcpy(sbuf, shmptr->buffer[shmptr->pget]);
        shmptr->pget = (shmptr->pget+1)%BUF_SIZE; // increment buffer pointer to get next
        (shmptr->available)--;

        // printf("AJ_DEBUG Consumer writing this %s to file\n", sbuf);
        fprintf(fp, "%s", sbuf);
        memset(sbuf, 0, SBUF_MAX);

        fclose(fp);


        if ( err = pthread_cond_signal( &(shmptr->space_available) ) ) {
            /* pthread_mutex_unlock( &(shmptr->lock) );
            fprintf(stderr, "%s\n",strerror(err)); */
        }
    
        // printf("AJ_DEBUG Consumer released lock\n");

        err=pthread_mutex_unlock( &(shmptr->lock) );  // release lock
      // printf("AJ_DEBUG Done executing Consumer thread\n");
    }

    // rest - shm, file closing and signal to main
    printf("Consumer done\n");
    shmdt((void *)shmptr);
    return 0;
}
