/* CSci5103 Fall 2016
* Assignment# 1
* name: Ajay Sundar Karuppasamy
* student id: 5298653
* x500 id: karup002
* CSELABS machine: kh4240-01.cselabs.umn.edu
*/

#ifndef PRODCON_H_INCLUDED
#define PRODCON_H_INCLUDED

/* Headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>

/* named constants */
#define BUFSIZE 1024
#define FILEPERM 0740
#define SHM_PPERM 0666
#define SHM_CPERM SHM_PPERM

#define PRODCON_DEBUG 0
#define debug_printf(fmt, ...) \
	do	 { \
	 	if (PRODCON_DEBUG) fprintf(stderr, fmt, __VA_ARGS__); \
	 } while (0) 

/* data structures */
struct Data
{
  char buffer[BUFSIZE];
  int count;
};

void producer (int fdin, int ch_pid);
void consumer (void);
void on_sigusr1(int signum);
void on_sigusr2(int signum);

#endif // PRODCON_H_INCLUDED
