/* CSci5103 Fall 2016
* Assignment# 1
* name: Ajay Sundar Karuppasamy
* student id: 5298653
* x500 id: karup002
* CSELABS machine: kh4240-01.cselabs.umn.edu
*/

#include "prodcon.h"

static int shmem_id; // uniq id with which the shared memory segment would be identified
static struct Data *dataptr; // pointer to shared memory segment
static const char fileout[] = "output"; // output file consumer should write to 
static mode_t perms; // file permission for output file

int
main (int argc, char *argv[])
{
  int fdin; // file-descriptor for the input
  int pid = -1;

  if (argc != 2) {
    fprintf(stderr, "Incorrect usage. Usage : %s input-file.txt", argv[0]);
    perror("");
    exit(1);
  }

  if ((fdin=open(argv[1], O_RDONLY)) == -1) {
    fprintf(stderr, "Error opening %s",argv[1]);
    perror("");
    exit(2);
  }

  /* create a shared memory segment */
  shmem_id = shmget(IPC_PRIVATE, sizeof(struct Data), IPC_CREAT|SHM_PPERM);
  if (shmem_id == -1) {
    perror("Failed to create shared memory segment");
    exit(3);
  }

  /* attach the shared memory to our address space */
  dataptr = (struct Data *) shmat(shmem_id, NULL, 0);
  if (dataptr == (void *)-1) {
    perror("Error attaching the sh.memory to parent addr. space");
    exit(3);
  }

  memset(dataptr, 0, sizeof(struct Data));

  // define the signal usage
  signal(SIGUSR1, on_sigusr1);
  signal(SIGUSR2, on_sigusr2);

  if ((pid=fork()) == -1) {
    perror("Fork failed");
    exit(4);
  }
  else if (pid) {
    /* parent aka producer */
    producer(fdin, pid);
  }
  else {
    /* child aka consumer */
    consumer();
  }
  
  close (fdin);

  waitpid(pid, NULL, 0); /* wait for the child process to terminate */
  shmdt((void *) dataptr); /* detach the segment from address space */
  shmctl(shmem_id, IPC_RMID, NULL); /* remove the shared memory segment */
  return 0;
}

void
producer (int fdin, int ch_pid)
{
  char lbuffer[BUFSIZE];
  int lcount = 0;

  sigset_t producer_set;
  sigemptyset(&producer_set); /* initialization of the structures; we don't */
                              /* need to block any other signals, so keeping empty mask */

  while (lcount=read(fdin, lbuffer, BUFSIZE))
  {      
    /* copy data from local buffer to shared buffer */
      dataptr->count = lcount;
      memcpy(dataptr->buffer, lbuffer, lcount);

    /* let the consumer know that the data is ready */
      kill(ch_pid, SIGUSR1);

    /* wait for SIGUSR2 from consumer */
    sigsuspend(&producer_set);
  }

  // report End-of-File to the consumer
  dataptr->count = -1;
  kill(ch_pid, SIGUSR1);
}

void
consumer (void)
{
  int fdout; // file-descriptor for the output
  struct Data *child_dataptr;
  int ppid = getppid();

  perms = FILEPERM;

 /* char *local_buffer, *head;
  local_buffer = (char *) malloc (20*BUFSIZE*sizeof(char));
  head = local_buffer;*/

  /* open output file */
  if ((fdout=open( fileout, (O_CREAT|O_WRONLY), perms )) == -1) {
    fprintf(stderr, "Error opening %s",fileout);
    perror("");
    exit(1);
  }
  
  /* attach the shared memory to our address space */
  child_dataptr = (struct Data *) shmat(shmem_id, NULL, 0);
  if (child_dataptr == (void *)-1) {
    perror("Error attaching the sh.memory to child addr. space");
    exit(2);
  }

  sigset_t consumer_set;
  sigemptyset(&consumer_set); /* initialization of the structures; we don't 
                              need to block any other signals, so keeping empty mask */

  while (child_dataptr->count != -1) 
  {
      while (child_dataptr->count==0) {
          /* wait till we get sigusr1 from producer */
          sigsuspend(&consumer_set);
      }

      if (child_dataptr->count != -1)
      {
          if(write(fdout, child_dataptr->buffer, child_dataptr->count)!=child_dataptr->count) {
             perror("Error writing to the output file");
             exit(3);
          }
          child_dataptr->count = 0;
          /* send sigusr2 to producer */
          kill(ppid, SIGUSR2);
      }
  }

  shmdt((void *) child_dataptr);
  close(fdout);

  //writebuftofile(local_buffer, fdout);

  /*free(local_buffer);*/

}

void on_sigusr1(int signum)
{
  debug_printf("Signal received : %s\n", "SIGUSR1");
}

void on_sigusr2(int signum)
{
  debug_printf("Signal received : %s\n", "SIGUSR2");
}
