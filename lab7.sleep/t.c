#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
OFT oft[NOFT];
PIPE pipe[NPIPE];

int procSize = sizeof(PROC);
int nproc = 0;
int goUmode();

int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
               "Saturn", "Uranus", "Neptune" };

char sbuf[1024], gbuf[1024], ibuf[1024], dbuf[1024];

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/
//#include "vid.c"

#include "bio.c"
#include "queue.c"
#include "loader.c"

#include "wait.c"             // YOUR wait.c   file
#include "kernel.c"           // YOUR kernel.c file
#include "int.c"              // YOUR int.c    file
#include "pipe.c"
#include "timer.c"

int color;
#define BASECOLOR 0x0D

int init()
{
    PROC *p; int i, j;
   color = BASECOLOR;
    printf("init ....");
    for (i=0; i<NPROC; i++){   // initialize all procs
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0; 
        p->inkmode = 1; 
        strcpy(proc[i].name, pname[i]);
        p->next = &proc[i+1];

        for(j=0; j < NFD; j++){
          p->fd[j] = 0;
        }
    }

    freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = 0;
    readyQueue = sleepList = 0;


    for(i=0; i < NOFT; i++){
      oft[i].refCount = 0;
    }

    for(i=0; i < NPIPE; i++){
      pipe[i].busy = 0;
    }


    /**** create P0 as running ******/
    p = get_proc(&freeList);
    p->status = RUNNING;
    p->ppid   = 0;
    p->parent = p;
    running = p;
    nproc = 1;
    //printf("p0 pid: %d\n", p->pid);
    printf("done\n");
} 

int scheduler()
{
    // if (running->status == READY)
    //     enqueue(&readyQueue, running);
    //  running = dequeue(&readyQueue);
    if (running->status == RUNNING){
        running->status = READY;
        enqueue(&readyQueue, running);
    }
    running = dequeue(&readyQueue);
    running->status = RUNNING;
   // running->time = 5;
    color = BASECOLOR - running->pid; 
}

int int80h(), tinth();

int set_vector(u16 vector, u16 handler)
{
     // put_word(word, segment, offset)
     put_word(handler, 0, vector<<2);
     put_word(0x1000,  0,(vector<<2) + 2);
}
        


main()
{
	//int i = 0; //char buf[1];
// vid_init();
 //ktest();
	color = BASECOLOR;
    printf("\nMTX starts in main()!\n");
    //printf("sizeof: %d\n", procSize);
    init();      // initialize and create P0 as running
    set_vector(80, int80h);

    kfork("/bin/u1");     // P0 kfork() P1
    kfork("/bin/u1");     // P0 kfork() P2
    kfork("/bin/u1");     // P0 kfork() P3
    kfork("/bin/u1");     // P0 kfork() P3


    lock();

    set_vector(8, tinth);
    timer_init();


    while(1){
      printf("P0 running\n");
      if (nproc==2 && proc[1].status != READY)
	  printf("no runable process, system halts\n");
      while(!readyQueue);
      printf("P0 switch process\n");
      //getc();
      tswitch();   // P0 switch to run P1
   }
}
