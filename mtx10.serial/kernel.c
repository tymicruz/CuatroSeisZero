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
PROC *kfork();

int body()
{
   char c, CR, buf[64];
   while(1){
      printf("=======================================\n");
      //printQueue(readyQueue);      

      printf("proc %d %s in Kmode\n", running->pid, running->name);
      printf("input a command (s|f|u|q|i|o) : ");
      c=getc(); putc(c); CR=getc(); putc(CR);
      switch(c){
          case 's' : tswitch(); break;
          case 'u' : printf("\nTask %d ready to go U mode\n", running->pid);
                     goUmode(); break;
          case 'f':  do_kfork(); break;
          case 'q' : kexit();   break;
          case 'i' : iline(); break;
          case 'o' : oline(); break;
      }
   }
}

/***************************************************************
  do_fork() creates a child task and returns the child pointer
  When scheduled to run, the child task resumes to body(pid) in 
  K mode. Its U mode environment is set to segment.
****************************************************************/
int do_kfork()
{  
   PROC *p; 
   int child;
   p = kfork("/bin/u1");
   if (p == 0){
       printf("kfork failed\n"); 
       return 0;
   } 
   printf("task %d return from do_kfork(): child=%d\n", running->pid,p->pid);
}

PROC *kfork(char *filename)
{
  PROC *p;
  int i,j,index, *ip;
  u16 es, segment, tsize, TOP;

  /*** get a proc for child task: ***/
  if ( (p = get_proc()) == NULL){
       printf("\nno more PROC  ");
       return 0;
  }
  /* initialize the new PROC and its stack */
  p->status = READY;
  p->ppid = running->pid;
  p->pri  = 1;
  p->inkmode = 1;

  /* clear "saved kmode registers" to 0 */ 
  for (i=1; i<=9; i++)
       p->kstack[SSIZE-i] = 0;

  /* fill in the needed entries */
  p->kstack[SSIZE-1] = (int)body;
  p->ksp = &(p->kstack[SSIZE - 9]); 

  /***** Fill in U mode information in proc *****/
  segment = (p->pid+1)*0x1000;
  printf("segment=%x\n", segment);
  load(filename, segment);

  TOP = 0;
  /**************************************************
    We know this task's segment. ustack is at the TOP 
    of this segment. We must make ustak contain:
  
         1   2   3  4  5  6  7  8  9  10 11 12
       flag uCS uPC ax bx cx dx bp si di es ds
     0x0200 seg  0  0  0  0  0  0  0  0 seg seg
  
    So, first a loop to set all to 0, then
    put_word(seg, segment, TOP-i*2); i=2,11,12;
   **************************************************/
 
   for (i=1; i<=12; i++){
       put_word(0, segment, TOP-2*i);
   }
  
   put_word(segment, segment, TOP-2*2);   /* uCS */
   put_word(segment, segment, TOP-2*11);  /* uES */
   put_word(segment, segment, TOP-2*12);  /* uDS */
   put_word(0x0200,  segment, TOP-2*1);   /* flag */

   /* initial USP relative to USS */
   p->usp = TOP - 2*12;      /* 32 K ustack */ 
   p->uss = segment;

   enqueue(&readyQueue, p);

   printf("Task %d forked a child %d at segment=%x\n",
           running->pid, p->pid, segment);
   nproc++;
   return p;
}

int copyImage(u16 s1,u16 s2, u16 size)
{
   int i; u16 w;
   for (i=0; i<size; i++){
       w = get_word(s1, 2*i);
       put_word(w, s2, 2*i);
   }
}

int fork()
{
  PROC *p;
  int i,index, *ip, *iq;
  u16 segment, size;

  /*** get a proc for child task: ***/
  if ( (p = get_proc()) == NULL){
       printf("\nno more PROC  ");
       return(-1);
  }
  /* initialize the new PROC and its kstack */
  p->status = READY;
  p->ppid = running->pid;
  p->pri  = 1;
  p->size = running->size;
  p->inkmode = 1;  
  /* clear "saved registers" to 0 */ 
  for (i=1; i<=9; i++)
       p->kstack[SSIZE-i] = 0;

  /* fill in the needed entries */
  p->kstack[SSIZE-1] = (int)goUmode;
  p->ksp = &p->kstack[SSIZE - 9]; 

  /***** Fill in U mode information in proc *****/
  segment = (p->pid+1)*0x1000;

  copyImage(running->uss, segment, 32*1024);

  /********************************************************
    copyImage copies parent's U image to child, including
    ustack contents.  However, the child's segment must be
    changed to its own, and its return pid in saved ax must
    be changed to 0
  *********************************************************/

   /*** first, child's uss and usp must be set: *****/
   p->uss = segment;
   p->usp = running->usp;   /* same as parent's usp */

   /*** next, change uDS, uES, uCS in child's ustack ****/
   put_word(segment, segment, p->usp);        /* uDS    */
   put_word(segment, segment, p->usp+2);      /* uES    */
   put_word(0, segment, p->usp+2*8);          /* ax = 0 */
   put_word(segment, segment, p->usp+2*10);   /* uCS    */
   
   enqueue(&readyQueue, p);

   printf("Task %d forked a child %d at segment=%x\n",
           running->pid, p->pid, segment);

   return p->pid;
}

int kmode()
{
  body();
}

char kbuf[1024];

int kexec(char *y)
{
   int i, *ip; 
   u16 segment, offset, tsize, TOP;
   char line[64], filename[32], f2[32], *cp, *cq, c;

   /* get parameter line from U space */
   cp = line; 
   while( (*cp=get_byte(running->uss, y)) != 0 ){
          y++; cp++;
   }

   // printf("kexec line=%s\n", line);

   /* extract filename to exec */
   cp = line;        cq = f2;
   while (*cp != ' ' && *cp != 0){
         *cq = *cp;
          cq++; cp++;
   }
   *cq = 0;
   strcpy(filename, f2);
 
   segment = running->uss;
 
   printf("Task %d exec to %s in segment=%x\n",running->pid,filename,segment); 
    
   load(filename, segment);       

   /* zero out U mode registers in ustack di to ax */
   for (i=1; i<10; i++){
       put_word(0, segment, -2*i);
   }
   /* re-initialize usp to new stack top */
   running->usp = -24;
   /*   1    2   3  4  5  6  7  8  9  10 11 12
   /*  flag uCS uPC ax bx cx dx bp si di es ds */

   put_word(segment, segment, -2*12);
   put_word(segment, segment, -2*11);
   put_word(0,       segment, -2*3);    /* uPC=0 */
   put_word(segment, segment, -2*2);    /* uCS   */
   put_word(0x0200,  segment, -2*1);    /* flag  */
}

char *hh[ ] = {"FREE   ", "READY  ", "SLEEP  ",  "ZOMBIE ", "BLOCK ",  0}; 
int kps()
{
   int i,j; 
   char *p, *q, buf[16];
   buf[15] = 0;

   printf("============================================\n");
   printf("  name         status      pid       ppid  \n");
   printf("--------------------------------------------\n");

   for (i=0; i<NPROC; i++){
       strcpy(buf,"               ");
       p = proc[i].name;
       j = 0;
       while (*p){
             buf[j] = *p; j++; p++;
       }      
       prints(buf);    prints(" ");
       
       if (proc[i].status != FREE){
           if (running==&proc[i])
              prints("running");
           else
              prints(hh[proc[i].status]);
           prints("     ");
           printd(proc[i].pid);  prints("        ");
           printd(proc[i].ppid);
       }
       else{
              prints("FREE");
       }
       printf("\n");
   }
   printf("---------------------------------------------\n");

   return(0);
}


#define NAMELEN 32
int chname(char * y)
{
  char buf[64];
  char *cp = buf;
  int count = 0; 

  while (count < NAMELEN){
     *cp = get_byte(running->uss, y);
     if (*cp == 0) break;
     cp++; y++; count++;
  }
  buf[31] = 0;

  printf("changing name of proc %d to %s\n", running->pid, buf);
  strcpy(running->name, buf); 
  printf("done\n");
}

