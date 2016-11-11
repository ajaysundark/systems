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

    static int shmem_id; // shared memory identifier
    static shmem_obj_t *shmem_ptr; // pointer to shared memory
    static key_t key = 4455;
    static int flag = 1023;

    int i, error, size;
    pid_t pid;
    char keystr[10] = {0};

    // create locks and conditions and mark as shared type
    pthread_mutexattr_t attr_sh_lock;
    pthread_condattr_t attr_sh_space_available;
    pthread_condattr_t attr_sh_item_available;

    // create a shared memory segment
    shmem_id = shmget(key, sizeof(shmem_obj_t), flag);
    if(shmem_id == -1) {
        perror("Failed to created a shared memory segment\n");
        exit(1);
    }
    sprintf(keystr, "%d", key);

    // attach the memory to our address space
    shmem_ptr = (shmem_obj_t *) shmat (shmem_id, NULL, flag);
    if (shmem_ptr == (void *)-1) {
        perror("Error attaching shared memory space to main\n");
        exit(2);
    }

    // initialize shared memory variables, locks etc.
    memset(shmem_ptr, 0, sizeof (shmem_obj_t));

    // mark the mutexes and conditions as shared
    pthread_mutexattr_init( &attr_sh_lock );
    pthread_condattr_init ( &attr_sh_space_available);
    pthread_condattr_init ( &attr_sh_item_available);
    
    pthread_mutexattr_setpshared( &attr_sh_lock, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared( &attr_sh_space_available, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared( &attr_sh_item_available, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init( &(shmem_ptr->lock), &attr_sh_lock );
    pthread_cond_init( &(shmem_ptr->space_available), &attr_sh_space_available);
    pthread_cond_init( &(shmem_ptr->item_available), &attr_sh_item_available);

    // all set; fork the processes to perform
    if( (pid=fork()) == -1 ) {
        perror("Fork failed\n");
        exit(3);
    }
    else if(pid==0) {
        // first child - RED process
        Color_t rcolor = red;
        char colorstr[10];
        sprintf(colorstr, "%d", rcolor);
        execl ("./producer", "producer", colorstr, keystr, NULL);
    }
    else {
        // parent == main process
        pid_t pid2;
        if ( (pid2=fork()) == -1) {
            perror ("Fork failed\n");
            exit(3);
        }
        else if (pid2==0) {
        // next process - BLACK
            Color_t bcolor = black;
            char colorstr[10];
            sprintf(colorstr, "%d", bcolor);
            execl ("./producer", "producer", colorstr, keystr, NULL);
        }
        else {
            // parent == main
            pid_t pid3;
            if ( (pid3=fork()) == -1) {
                perror ("Fork failed\n");
                exit(3);
            }
            else if (pid3==0) {
            // next process - WHITE
                Color_t wcolor = white;
                char colorstr[10];
                sprintf(colorstr, "%d", wcolor);
                execl("./producer", "producer", colorstr, keystr, NULL);
            }
            else {
                // parent == main
                pid_t pid4;
                if ( (pid4=fork()) == -1) {
                    perror ("Fork failed\n");
                    exit(3);
                }
                else if (pid4==0) {
                    // next process = consumer
                    execl("./consumer", "consumer", keystr, NULL);
                }
                else {
                    // parent = main thread
                    // wait for all threads to complete
                    wait( NULL );
                    printf("Done");

                    // detach the shared memory segment
                    shmdt( (void *)shmem_ptr);
                }
            }
        }
    }

    shmctl(shmem_id ,IPC_RMID, NULL); //destroy the shared memory segment
    return 0;
}

