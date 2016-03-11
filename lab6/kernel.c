// kernel.c file:

// YOUR kernel.c file as in LAB3

// MODIFY kfork() to

int makeUimage(char *filename, PROC *p)
{
  u16 i, segment;

  // make Umode image by loading /u1 into segment
  printf("makeUimage for pid: %d\n", p->pid);
  segment = ((p->pid) + 1)*0x1000;
  

  load(filename, segment); // check this to see if successful

  //printf("makeUimage load return value: %d\n", i);

  /***** Fill in U mode information in proc *****/
  /**************************************************
    We know segment=0x2000 + index*0x1000 ====>
    ustack is at the high end of this segment, say TOP.
    We must make ustak contain:
          1   2   3  4  5  6  7  8  9 10 11 12
       flag uCS uPC ax bx cx dx bp si di es ds
     0x0200 seg  0  0  0  0  0  0  0  0 seg seg
  
    So, first a loop to set all to 0, then
    put_word(seg, segment, -2*i); i=2,11,12;
   **************************************************/
 
   for (i=1; i<=12; i++){
       put_word(0, segment, -2*i);
   }
   put_word(0x0200,  segment, -2*1);   /* flag */  
   put_word(segment, segment, -2*2);   /* uCS */
   put_word(segment, segment, -2*11);  /* uES */
   put_word(segment, segment, -2*12);  /* uDS */
   printf("made at seg: %x\n", segment);
   /* initial USP relative to USS */
   p->usp = -2*12; 
   p->uss = segment;
   return 0;
}

int copyImage(u16 parentSegment, u16 childSegment, u16 numWords){
	u16 i;
	printf("copy from %x to %x\n", parentSegment, childSegment);

	for(i =0; i < numWords; i++){
		put_word(get_word(parentSegment, i*2), childSegment, i*2); //2 should be changed to word size in order to be more robust
	}
}

PROC *kfork(char *filename){

	int i = 0;
	int child_segment, parent_segment, word;

	PROC *p;
	p = get_proc(&freeList);
	//printf("1p1 pid: %d\n", p->pid);

	//empty free list
	if(p == 0)
	{
		printf("freeList is empty (kfork fail)\n");
		return p;
	}
	//printf("2p1 pid: %d\n", p->pid);
	child_segment = ((p->pid)+1)*0x1000;
	parent_segment = ((running->pid)+1)*0x1000; //or running->uss
	//printf("3p1 pid: %d\n", p->pid);

	nproc++;
	//printf("4p1 pid: %d\n", p->pid);//4
	p->status = READY;
	//printf("5p1 pid: %d\n", p->pid);//5
	p->priority = 1;
	// printf("6p1 pid: %d\n", p->pid);//6
	enqueue(&readyQueue, p);
	//printf("6p1 pid: %d\n", running->pid);
	p->ppid = running->pid;
	//printf("7p1 pid: %d\n", p->pid);//7
	p->parent = running;
	//printf("forked pid: %d\n", p->pid);//8
	//init p's kstack

	for(i = 1; i <= 9; i++)
	{
		p->kstack[SSIZE - i] = 0;
	}

	p->kstack[SSIZE - 1] = (int)goUmode;
	//printf("p1 pid: %d\n", p->pid);
	//point stack pointer to top of stack (lower addr)
	p->ksp = &(p->kstack[SSIZE - 9]);
	//printf("p1 pid: %d\n", p->pid);
	

	if(filename){
		//printf("p1 pid: %d\n", p->pid);
		makeUimage(filename, p);
	}else{
		copyImage((u16)running->uss, (u16)child_segment, (u16)32 * 1024);
		p->usp = running->usp;
		p->uss = child_segment;
/*************************************************************************
  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/

//keep new process in its own bed
		put_word(0, child_segment, (p->usp)+(2*8)); //uax reg
		put_word(child_segment, child_segment, (p->usp)+(2*0)); //ds
		put_word(child_segment, child_segment, (p->usp)+(2*1)); //es
		put_word(child_segment, child_segment, (p->usp)+(2*10)); //cs
	}

	  printf("Proc %d kforked a child %d at segment=%x\n",
          running->pid, p->pid, child_segment);

    //copy parent's fds to child
    for(i = 0; i < NFD; i++){

      p->fd[i] = running->fd[i];

      if(running->fd[i]){
        running->fd[i]->refCount++;

        if(running->fd[i]->mode == READ_PIPE){
          running->fd[i]->pipe_ptr->nreader++;
        }

        if(running->fd[i]->mode == WRITE_PIPE){
          running->fd[i]->pipe_ptr->nwriter++;
        }

      }

    }

	return p;
}

int do_tswitch()
{
  printf("proc %d tswitch()\n", running->pid);
  tswitch();
  printf("proc %d resumes\n", running->pid);
}

int do_kfork()
{
  PROC *p;
  printf("proc %d kfork a child ", running->pid);
  p = kfork("/bin/u1");  // every proc has /bin/u1 as Umode image
  if (p == 0)
     printf("kfork failed\n");
  else
    printf("child pid = %d\n", p->pid);
}

int do_exit(int exitValue)
{
  int exitValue;
  if (running->pid == 1 && nproc > 2){
      printf("other procs still exist, P1 can't die yet !%c\n",007);
      return -1;
  }
  //**************************************
  printf("enter an exitValue (0-9) : ");
  exitValue = (getc()&0x7F) - '0'; 
  printf("%d\n", exitValue);
  //*************************************
  kexit(exitValue);
}

int do_wait(int *ustatus)
{
  int child, status;
  child = kwait(&status);
  if (child<0){
    printf("proc %d wait error : no child\n", running->pid);
    return -1;
  }
  printf("proc %d found a ZOMBIE child %d exitValue=%d\n", 
	   running->pid, child, status);
  // write status to Umode *ustatus
  put_word(status, running->uss, ustatus);
  return child;
}


int body()
{
  char c;
  printf("proc %d starts new body()\n", running->pid);

  while(1){
  	printf("proc %d resumes to body()\n", running->pid);//added
  //	goUmode();//added  to go directed
  	//continue;

    printf("-----------------------------------------\n");
    printQueue(freeList, "freelist:");
    printQueue(readyQueue, "readyQueue:");
    printQueue(sleepList, "sleepList:");
    printf("-----------------------------------------\n");

    printf("proc %d running: parent = %d  enter a char [s|f|w|q|u] : ", 
	   running->pid, running->parent->pid);
    c = getc(); printf("%c\n", c);
    switch(c){
       case 's' : do_tswitch();   break;
       case 'f' : do_kfork();     break;
       case 'w' : do_wait();      break;
       case 'q' : do_exit();      break;
       case 'u' : goUmode();      break;
    }
  }
}

kmode(){
  body();
}





