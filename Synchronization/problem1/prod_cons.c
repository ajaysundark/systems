/* CSci5103 Fall 2016
 * Assignment# 4
 * name: Ajay Sundar Karuppasamy
 * student id: 5298653
 * x500 id: karup002
 * CSELABS machine: csel-kh4240-01.cselabs.umn.edu
 *
 * */


#include "prod_cons.h"

static char buffer[BUF_SIZE][SBUF_MAX];
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int pfill = 0;
static int pget = 0;
static int available = 0;

static FILE *fptrr, *fptrb, *fptrw, *fptrc;

static pthread_cond_t item_available = PTHREAD_COND_INITIALIZER;
static pthread_cond_t space_available = PTHREAD_COND_INITIALIZER;

static const char colors[3][6] = {
    "RED",
    "BLACK",
    "WHITE"
};

int
main(int argc, char **argv) {

    void *process_producers (void *arg);
    void *process_consumer (void *arg);

    /* parameters to be passed to the threads */
    int i, error, color;
    pthread_t *tid;

    /* open/create log file for each producer and consumer */
    fptrr = fopen ("Producer_RED.txt", "w+");
    fptrb = fopen ("Producer_BLACK.txt", "w+");
    fptrw = fopen ("Producer_WHITE.txt", "w+");
    fptrc = fopen ("Consumer.txt", "w+");

    /* manage thread creation and queuing */
    if( (tid = (pthread_t *) calloc(NUM_THREADS, sizeof(pthread_t))) == NULL) {
        fprintf(stderr, "Failed to allocate memory for thread IDs\n");
        return 1;
    }

    for (i=0; i<NUM_THREADS; ++i) {
        /* create threads for producers and a consumer */
        color = i; 

        if (i<3) { // init producers
            if (error = pthread_create(tid+i, NULL, process_producers, (void *)&i) )
                fprintf(stderr, "Failed to create producer thread : %u : %s\n", i, strerror(error));
        }
        else { // init consumer
            if (error = pthread_create(tid+i, NULL, process_consumer, (void *)&i) )
                fprintf(stderr, "Failed to create consumer thread : %s\n", strerror(error));
        }
        usleep(1000);
    }

    for (i=0; i<NUM_THREADS; ++i) {
        /* wait for all the threads to complete */
        if (error = pthread_join(tid[i], NULL) )
            fprintf(stderr, "Failed to join thread %u : %s\n", i, strerror(error));
    }

    /* all process should have been completed by now
     * safe to close fds and free up memory*/

    fclose(fptrr);
    fclose(fptrb);
    fclose(fptrw);
    fclose(fptrc);

    free(tid);
    return 0;
}

void *process_producers(void *param) {
    int color = *(int *)param;
    int i, err;
    FILE *fp;
    char sbuf[SBUF_MAX];
    struct timeval tv= {0};
    
    for (i=0; i<NUM_ITEMS; ++i) {
        // err= put_item(sbuf);
        if (err=pthread_mutex_lock(&lock)) { // try to acquire lock
            fprintf(stderr,"%s\n", strerror(err));
        }

        while (available>=BUF_SIZE) {
            if(err=pthread_cond_wait(&space_available, &lock)) {
                pthread_mutex_unlock(&lock);
                fprintf(stderr, "%s\n",strerror(err));
            }
        }

        // printf("Producer %s got lock ; ", colors[buffer[pfill].color]);
        
        switch(color) {
            case 0: fp = fptrr; break; 
            case 1: fp = fptrb; break;
            case 2: fp = fptrw; break;
        }

        gettimeofday(&tv, NULL);
        snprintf(sbuf, SBUF_MAX, "%s %lu\n", colors[color], (1000000*tv.tv_sec + tv.tv_usec));

        strcpy(buffer[pfill], sbuf);
        pfill = (pfill+1)%BUF_SIZE;
        available++;

        if(err=pthread_cond_signal(&item_available)) {
            pthread_mutex_unlock(&lock);
            fprintf(stderr, "%s\n",strerror(err));
        }

        // printf("AJ_DEBUG Producer released lock\n");

        // printf("AJ_DEBUG Producer %s writing this %s to file\n",colors[color], sbuf);
        fprintf(fp, "%s", sbuf);

        memset(&tv, 0, sizeof(struct timeval));
        memset(sbuf, 0, SBUF_MAX);
        pthread_mutex_unlock(&lock); // release lock

     // printf("AJ_DEBUG Done executing PRODUCER %s\n", colors[color]);
    }
    return NULL;
}

void *process_consumer(void *param) {
    int color = *(int *)param;
    int i, err;
    char sbuf[SBUF_MAX];

    for(i=0; i<3*NUM_ITEMS; ++i) {
        if (err=pthread_mutex_lock(&lock)) { // try to acquire lock
            fprintf(stderr,"%s\n", strerror(err));
        }

        // printf("AJ_DEBUG Consumer got lock ; ");

        while(!available) { // check if resource available 
            if (err = pthread_cond_wait(&item_available, &lock)) {
                pthread_mutex_unlock (&lock); // Don't leave the resource locked!
                fprintf(stderr, "%s\n",strerror(err));
            }
        }

        strcpy(sbuf, buffer[pget]);
        pget = (pget+1)%BUF_SIZE; // increment buffer pointer to get next
        available--;

        if (err = pthread_cond_signal(&space_available)) {
            pthread_mutex_unlock(&lock);
            fprintf(stderr, "%s\n",strerror(err));
        }

    
        // printf("AJ_DEBUG Consumer released lock\n");

        // printf("AJ_DEBUG Consumer writing this %s to file\n", sbuf);
        fprintf(fptrc, "%s", sbuf);

        memset(sbuf, 0, SBUF_MAX);
        err=pthread_mutex_unlock(&lock);  // release lock
     // printf("AJ_DEBUG Done executing Consumer thread\n");
    }
    return NULL;
}
