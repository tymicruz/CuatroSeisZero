PROC *kfork(){

	int i = 0;
	PROC *p = get_proc(&freeList);

	//empty free list
	if(p == 0)
	{
		printf("freeList is empty (kfork fail)\n");
		return p;
	}

	nproc++;
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

int kexit(int exitValue){
	int i, wakeupP1 = 0;
	PROC *p;

	if(running->pid==1 && nproc > 2){
		printf("other procs still exist, P1 can't die yet\n");
		return -1;
	}

	//give children to p1
	for(i = 1; i < NPROC; i++){
		p = &proc[i];
		if(p->status != FREE && p->ppid == running->pid){
			p->ppid = 1;
			p->parent = &proc[1];
			wakeupP1++;
		}
	}

	//record exitValue and become a Zome

	running->status = ZOMBIE;
	running->exitCode = exitValue;

	kwakeup(running->parent); //wake up parent if sleep to tell them you died //parrent sleep
	//wakeup parent

	if(wakeupP1){
		kwakeup(&proc[1]);
	}

	tswitch();
}

int ksleep(int event){

	running->event = event;
	running->status = SLEEP;

	tswitch();
}

int kwakeup(int event){

	int i; PROC *p;

	for (i = 1; i < NPROC; i++)
	{
		p = &proc[i];

		if(p->status == SLEEP && p->event == event){
			p->status = READY;
			p->event = 0;

			enqueue(&readyQueue, p);
		}
	}
}

int kwait(int *status){
	 PROC *p;int i, hasChild = 0;

	while(1){
		for(i = 1; i < NPROC; i++){
			p = &proc[i];

			if(p->status != FREE && p->ppid == running->pid){
				hasChild = 1;
				if(p->status == ZOMBIE){
					*status = p->exitCode;
					p->status = FREE;
					put_proc(&freeList, p);
					nproc--;
					return(p->pid);
				}
			}
		}

		if(!hasChild) return -1; //no children return ERROR
		
		ksleep(running); //stil had kids alive: sleep on PROC address
	}
}