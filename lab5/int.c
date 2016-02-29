
/*************************************************************************
  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/

/****************** syscall handler in C ***************************/

#define PA 13
#define PB 14
#define PC 15
#define PD 16
#define AX  8

char *cmds[] = {"getpid()", "ps()", "chname()", "kfork()", "tswitch()", "wait(&status)", "exit(value)", "exec(filename)"};

int kcinth()
{
   u16    segment, offset;
   int    a,b,c,d, r;
   segment = running->uss; 
   offset = running->usp;

//==> WRITE CODE TO GET get syscall parameters a,b,c,d from ustack 

   a = get_word(segment, offset + 2*PA);
   b = get_word(segment, offset + 2*PB);
   c = get_word(segment, offset + 2*PC);
   d = get_word(segment, offset + 2*PD);

   

   switch(a){
       case 0 : printf("hello in kernel a: %d cmd: %s\n", a, cmds[a]);r = kgetpid();        break;
       case 1 : printf("hello in kernel a: %d cmd: %s\n", a, cmds[a]);r = kps();            break;
       case 2 : printf("hello in kernel a: %d cmd: %s\n", a, cmds[a]);r = kchname(b);       break;
       case 3 : printf("hello in kernel a: %d cmd: %s\n", a, cmds[a]);r = kkfork();         break;
       case 4 : printf("hello in kernel a: %d cmd: %s\n", a, cmds[a]);r = ktswitch();       break;
       case 5 : printf("hello in kernel a: %d cmd: %s\n", a, cmds[a]);r = kkwait(b);        break;
       case 6 : printf("hello in kernel a: %d cmd: %s\n", a, cmds[a]);r = kkexit(b);        break;
       case 7 : printf("hello in kernel a: %d cmd: %s\n", a, cmds[a]);r = kexec(b);        break;

       case 99: kkexit(b);            break;
       default: printf("invalid syscall # : %d\n", a); 
   }

   put_word(r, segment, offset + 2*AX);
   return r;//this will be overwritten by the pop ax register when restoring back to uMode
}


//============= WRITE C CODE FOR syscall functions ======================

int kgetpid()
{
   // WRITE YOUR C code

	return running->pid;
}

char *hh[ ] = {"FREE   ", "READY  ", "RUNNING", "STOPPED", "SLEEP  ", 
               "ZOMBIE ",  0}; 

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
int kchname(char *name)
{
  char buf[NAMELEN];
  char *cp = buf;
  int count = 0; 

  while (count < NAMELEN){
     *cp = get_byte(running->uss, name);
     if (*cp == 0) break;
     cp++; name++; count++;
  }
  buf[NAMELEN - 1] = 0;

  printf("changing name of proc %d to %s\n", running->pid, buf);
  strcpy(running->name, buf); 
  printf("done\n");
}

#define FILENAMELEN 64
int kexec (char *ufilename){

	int success, i;
	char kfilename[FILENAMELEN];
  	char *cp = kfilename;
  	int count = 0; 
  	u16 segment = running->uss;

  while (count < FILENAMELEN){
     *cp = get_byte(running->uss, ufilename);
     if (*cp == 0) break;
     cp++; ufilename++; count++;
  }

  kfilename[FILENAMELEN - 1] = 0;

	//printf("proc %d exec(%s) attempt ", running->pid, kfilename);

	//success = kkexec(file);
	//printf("load: %d\n", load(kfilename, segment));

	//gets(kfilename);

 //   for (i=1; i<= 32 * 1024; i++){
   //    put_word(0, segment, 2*i);
   //}

	if(!myload(kfilename, segment)){
		printf("failed at %x\n", segment);
		return -1;
	}

	//load(kfilename, segment);

    /**************************************************
    We know segment=0x2000 + index*0x1000 ====>
    ustack is at the high end of this segment, say TOP.
    We must make ustak contain:
          1   2   3  4  5  6  7  8  9 10 11 12
       flag uCS uPC ax bx cx dx bp si di es ds
     0x0200 seg  0  0  0  0  0  0  0  0 seg seg
  
    So, first a loop to set all to 0, then
    put_word(seg, segment, -2*i); i=2,11,12;*/

	for (i=1; i<=12; i++){
       put_word(0, segment, -2*i);
   }


   put_word(0x0200,  segment, -2*1);   /* flag */  
   put_word(segment, segment, -2*2);   /* uCS */
   put_word(segment, segment, -2*11);  /* uES */
   put_word(segment, segment, -2*12);  /* uDS */
   printf("success at %x\n", segment);
   /* initial USP relative to USS */
   running->usp = -2*12; 
   //running->uss = segment;

   return running->pid;

}

int kkfork()
{
  //use you kfork() in kernel;
  //return child pid or -1 to Umode!!!

   PROC *p;
  printf("proc %d kfork a child ", running->pid);
  p = kfork(0);  // every proc has /bin/u1 as Umode image
  if (p == 0){
     printf("kfork failed\n");
     return -1;
  }
  else{
    printf("child pid = %d\n", p->pid);
    return p->pid;

  }

}

int ktswitch()
{
    tswitch();
}

int kkwait(int *ustatus)
{

  //use YOUR kwait() in LAB3;
  //return values to Umode!!!

  int child, status;
  child = kwait(&status);
  if (child<0){
    printf("proc %d wait error : no child\n", running->pid);
    return -1;
  }
  printf("proc %d found a ZOMBIE child %d with exitValue=%d\n", 
	   running->pid, child, status);
  // write status to Umode *ustatus
  put_word(status, running->uss, ustatus);
  return child;

}

int kkexit(int value)
{
    if(running->pid == 1 && nproc > 2){
    	printf("other procs still exist, P1 can't die yet !%c\n",007);
    	return -1;
    }

    kexit(value);
}
