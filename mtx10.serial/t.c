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

PROC proc[NPROC], *freeList, *running, *readyQueue, *sleepList;
int procSize = sizeof(PROC);

char *pname[]={"Sun","Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
               "Saturn",  "Uranus","Neptune"};
int nproc;

#include "queue.c"
#include "wait.c"
#include "kernel.c"
#include "int.c"
#include "pv.c"
#include "serial.c"

int body();

int init()
{
  int i,j,fd; PROC *p;
   for (i=0; i<NPROC; i++){
     p = &proc[i];
       p->pid = i;
       p->status = FREE;
       p->next = &proc[i+1];
       strcpy(p->name, pname[i]);
       printf("%d %s ", p->pid, proc[i % 4].name);
   }
   printf("\n");

   proc[NPROC-1].next = 0;
   freeList = &proc[0];   
   readyQueue = 0;
   sleepList = 0;

   p = get_proc();
   p->pri = 0;
   p->status = READY;
   p->parent = p;
   p->inkmode = 1;
   running = p;
   nproc = 1;
}

int int80h(), s0inth(), s1inth();

int set_vec(u16 vector, u16 addr)
{
    u16 location,cs;
    location = vector << 2;
    cs = getcs();
    put_word(addr, 0, location);
    put_word(cs,0,location+2);
}

main()
{
      int i, pid, status;  char c, d;
      printf("Welcome to the 460 Multitasking System\n");

      printf("initializing ......\n");
        init();
      printf("initialization complete\n");

      set_vec(80, int80h);

      kfork("/bin/u1"); 

      //set_vec(11, s1inth);
      set_vec(12, s0inth);
      sinit();

      while(1){
        if (readyQueue)
           tswitch();
      }
      printf("all dead, happy ending\n");
}

int scheduler()
{ 
    //printf("task %d in scheduler\n", running->pid);
    if (running->status == READY)
       enqueue(&readyQueue,running);
    running = dequeue(&readyQueue);
}
