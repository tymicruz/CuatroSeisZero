/********************************************************************
Copyright 2010-2015 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/
#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;

int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
               "Saturn", "Uranus", "Neptune" };

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/
#include "wait.c"
#include "kernel.c"
#include "int.c"
#include "fe.c"
#include "pv.c"
#include "pr.c"

int init()
{
    PROC *p;
    int i;
    printf("init ....");
    for (i=0; i<NPROC; i++){   // initialize all procs
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;  
        strcpy(proc[i].name, pname[i]);
        p->inkmode = 1;   
        p->next = &proc[i+1];
    }
    freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = 0;
    readyQueue = sleepList = 0;

    /**** create P0 as running ******/
    p = get_proc(&freeList);
    p->status = READY;
    p->ppid   = 0;
    p->parent = p;
    p->inkmode = 1;
    running = p;
    nproc = 1;

    // initialize HD and CD drivers
    pr_init();
} 

int scheduler()
{
    if (running->status == READY)
       enqueue(&readyQueue, running);
    running = dequeue(&readyQueue);
}

int int80h(), hdinth(), cdinth(), pinit();

int set_vec(u16 vector, u16 addr)
{
    u16 location,cs;
    location = vector << 2;
    put_word(addr, 0, location);
    put_word(0x1000,0,location+2);
}
            
main()
{
    printf("MTX starts in main()\n");
    init();      // initialize and create P0 as running
    set_vec(80,int80h);

   set_vec(15, pinit);
  

    kfork("/bin/u1");     // P0 kfork() P1
    while(1){
      if (readyQueue)
	tswitch();
      else
	halt();
    }
}
