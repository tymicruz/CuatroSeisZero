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

#include "queue.c"
#include "wait.c"
#include "kernel.c"
#include "fe.c"
#include "int.c"
#include "vid.c"
#include "pv.c"
#include "kbd.c"

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
   
        p->next = &proc[i+1];
    }
    freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = 0;
    readyQueue = sleepList = 0;

    /**** create P0 as running ******/
    p = get_proc();
    p->status = RUNNING;
    p->ppid   = 0;
    p->parent = p;
    p->inkmode = 1;

    running = p;
    nproc = 1;
    printf("done\n");
} 

int scheduler()
{
  if (running->status == RUNNING){
      running->status = READY;
      enqueue(&readyQueue, running);
  }
  running = dequeue(&readyQueue);
  running->status = RUNNING;
}

int int80h(), tinth(), kbinth();
int set_vector(u16 vector, u16 addr)
{
    u16 location,cs;
    location = vector << 2;
    put_word(addr, 0, location);
    put_word(0x1000,0,location+2);
}
            
main()
{
    vid_init();
    printf("MTX starts in main()\n");
    init();      // initialize and create P0 as running

    set_vector(80,int80h);
    set_vector(9, kbinth);

    kbd_init();

    kfork("/bin/u1");     // P0 kfork() P1

    while(1){
      unlock();
      if (readyQueue)
          tswitch();   // P0 switch to run P1
      else
	halt();
    }
}
