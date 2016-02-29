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
//#include "queue.c"

typedef struct proc{
    struct proc *next;
    int    *ksp;               // saved ksp when not running
    						   //	0    1     2     3      4
    int    status;			   // FREE|READY|SLEEP|BLOCK|ZOMBIE
    int    priority;		   // priority
    int    pid;                // add pid for identify the proc
    int    ppid;			   // parent pid
    struct proc *parent; 	   // pointer to parent PROC

    int    kstack[SSIZE];      // proc stack area
}PROC;

int  procSize = sizeof(PROC);

PROC proc[NPROC], *running, *freeList, *readyQueue;    // define NPROC procs
extern int color;

exit();
int body();

//

PROC * get_proc(PROC **list)
{
	PROC *p = *list;

	if(*list == 0)
		return 0;

	*list = p->next;

	p->next = 0;

	return p;
}

//put proc in beginning of list
put_proc(PROC **list, PROC *p)
{
	PROC *curr = *list;

	if(p)
	{
		// p->status = 0;
		// p->ppid = 0;	//default parent pid = 0
  //      	p->parent = 0; //null to parent parent
		p->next = *list;
		(*list)->next = p;
	}

}

enqueue(PROC **queue, PROC *p)
{
	PROC *curr = *queue;

	//if null proc return
	if(p == 0)
	{
		return;
	}

	//if queue is empty point queue to p
	//or
	//if p has highest prior, put in front
	if(curr == 0 || p->priority > curr->priority)
	{
		p->next = *queue;
		*queue = p;
		return;
	}

	while(curr->next && p->priority <= curr->next->priority)
	{
		curr = curr->next;
	}

	p->next = curr->next;
	curr->next = p;

}

PROC * dequeue(PROC **queue)
{
	PROC *p = *queue;

	if(p)
	{
		*queue = p->next;
		p->next = 0;
	}

	return p;
}

printQueue(PROC *queue, char *title)
{
	printf("%s ", title);

	while(queue)
	{


		printf("[%d, %d]->", queue->pid, queue->priority);
		queue = queue->next;
	}

	printf("NULL\n");
}

//

exit()
{
	running->status = ZOMBIE;
	tswitch();
}

PROC *kfork()
{
	int i = 0;
	PROC *p = get_proc(&freeList);

	//empty free list
	if(p == 0)
	{
		printf("freeList is empty (kfork fail)\n");
		return p;
	}

	p->status = READY;
	p->priority = 1;
	p->ppid = running->pid;
	p->parent = running;

	//init p's kstack

	for(i = 1; i <= 9; i++)
	{
		p->kstack[SSIZE - i] = 0;
	}

	p->kstack[SSIZE - 1] = (int)body;

	//point stack pointer to top of stack (lower addr)
	p->ksp = &(p->kstack[SSIZE - 9]);

	enqueue(&readyQueue, p);

	return p;
}


int body()
{ 
   char c;
   printf("proc %d resumes to body()\n", running->pid);
   while(1){
     color = running->pid + 7;
     printQueue(freeList, "freeList");
     printQueue(readyQueue, "readyQueue");

     printf("proc %d running : \nenter a key (s|q|f): ", running->pid);
     c = getc();
	printf("%c\n", c);
     switch(c)
     {
     	case 's':
     		tswitch();
     	break;

     	case 'q':
     		exit();
     	break;

     	case 'f':
     		kfork();
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
   }     

   //freeList = %proc[0]

   running = &proc[0];                    // P0 is running 
   running->status = 1; //set running proc (p0) status to READY (2)

   //freeList init
   for(i = 1; i < NPROC - 1; i++)
   {
   		p = &proc[i];
   		p->next = &proc[i+1];
   }

   if(NPROC > 1)
   	freeList = &proc[1];
   else
   	freeList = 0;


   printf("init complete, mane\n");

 }
   
int scheduler()
{
   if(running->status == READY)
   {
   	enqueue(&readyQueue, running);
   }

   running = dequeue(&readyQueue);
}

main()
{
	printf("\nMTX starts in main()\n");
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


