#define NPROC 9                // number of PROCs
#define SSIZE 1024             // per proc stack area 
#define RED 

#define FREE	0
#define READY 	1
#define RUNNING 2
#define STOPPED 3
#define SLEEP 	4
//#define BLOCK	3
#define ZOMBIE	5

//#include "bio.c"

typedef struct proc{
    struct proc *next;
    int    *ksp;               // saved ksp when not running
    						   //	0    1     2     3      4
    int    status;			   // FREE|READY|SLEEP|BLOCK|ZOMBIE
    int    priority;		   // priority
    int    pid;                // add pid for identify the proc
    int    ppid;			   // parent pid
    int    event;
    int    exitCode;
    struct proc *parent; 	   // pointer to parent PROC

    int    kstack[SSIZE];      // proc stack area
}PROC;

int  procSize = sizeof(PROC);
#define BUF_SIZE 256
char buf[BUF_SIZE];
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;    // define NPROC procs
int nproc = 0, rflag = 0;
extern int color;

int body();

#include "queue.c"
#include "kernel.c"


do_tswitch(){
	tswitch();
}

do_kfork(){
	kfork();
}

do_exit(){

	int val = 0;
	bzero(buf, BUF_SIZE);
	printf("enter exitValue: ");
	gets(buf);

	val = atoi(buf);
	//printf("exitValue: %d\n", val);

	kexit(val);
}

do_wakeup(){
	int val = 0;
	bzero(buf, BUF_SIZE);
	printf("enter wakeup event: ");
	gets(buf);

	val = atoi(buf);

	kwakeup(val);
}

do_sleep(){
	int val = 0;
	bzero(buf, BUF_SIZE);
	printf("enter sleep event: ");
	gets(buf);

	val = atoi(buf);
	//printf("exitValue: %d\n", val);

	ksleep(val);
}

do_wait(){
	int pid, status;

	pid = kwait(&status);

	if(pid < 0){
		printf("running %d has no children to wait for\n", running->pid);
	}


	printf("dead child pid: %d | exitStatus: %d", pid, status);
	
}


int body()
{ 
   char c;
        color = running->pid + 7;

   printf("\nproc %d resumes to body()\n", running->pid);
   while(1){
     color = running->pid + 7;
     printQueue(freeList, "freeList");
     printQueue(readyQueue, "readyQueue");

     printf("proc %d running : \nenter a key (s|q|f | z|a|w): ", running->pid);
     c = getc();
	printf("%c\n", c);
     switch(c)
     {
     	case 's':
     		tswitch();
     	break;

     	case 'q':
     		do_exit();
     	break;

     	case 'f':
     		do_kfork();
     	break;
     	case 'z':
     		//printf("sleep\n");
     		do_sleep();
     	//go to sleep on an even
     	break;
     	case 'a':
     		//printf("wake up\n");
     		do_wakeup();
     	//wakeup all procs sleeping on event
     	break;
     	case 'w':
     		//printf("wait\n");
     		do_wait();
     	//wait for a ZOMBIE child
     	break;
     }

     
     //tswitch();
   }
}


int init()
{
   PROC *p;
   int i, j;

   /* initialize all proc's */
   for (i=0; i<NPROC; i++){
       p = &proc[i];
       p->pid = i;    
       p->ppid = 0;	//default parent pid = 0
       p->parent = 0; //null to parent parent
       p->next = 0; //null pointer to next
       p->status = FREE; //status = free
       p->priority = 0;
       p->event = 0;
       p->exitCode = 0;
   }     

   //freeList = %proc[0]

   running = &proc[0];                    // P0 is running 
   running->status = READY; //set running proc (p0) status to READY (2)

   //freeList init
   for(i = 1; i < NPROC - 1; i++)
   {
   		p = &proc[i];
   		p->next = &proc[i+1];
   }

   readyQueue = sleepList = 0;

   if(NPROC > 1)
   	freeList = &proc[1];
   else
   	freeList = 0;

   nproc = 1;


   printf("init complete, mane\n");

 }
   
int scheduler()
{
   if(running->status == READY)
   {
   	enqueue(&readyQueue, running);
   }

   running = dequeue(&readyQueue);
   rflag = 0;
}

main()
{
	printf("\nMTX starts in main() bro\n");
	init();

	kfork();

	while(1){
		printf("proc 0  running : enter a key : \n");
		getc();
		if(readyQueue)
		{	
			tswitch();
		}
		else
		{
			printf("proc 0 is running, stuck,\n");
			break;
		}

		//w
	}
}


