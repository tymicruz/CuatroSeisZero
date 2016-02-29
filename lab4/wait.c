  //            wait.c file:

    //   YOUR wait.c file as in LAB3
// int ready(PROC *p)
// {
//   p->event = 0;
//   p->status = READY;
//   enqueue(&readyQueue, p);
//   printf("wakeup proc %d\n", p->pid);
// }

int ksleep(int event){

	running->event = event;
	running->status = SLEEP;

	enqueue(&sleepList, running);

	tswitch();
}

int kwakeup(int event){

	PROC *p, *tempSleep = 0;

	while(p = dequeue(&sleepList)){
		if (p->event == event){
			p->status = READY;
			enqueue(&readyQueue, p);
			printf("kwakeup: wakeup %d\n", p->pid);
			continue;
		}
		enqueue(&tempSleep, p);
	}

	sleepList = tempSleep;
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

