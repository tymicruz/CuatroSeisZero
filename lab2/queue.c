
// PROC * get_proc(PROC **list)
// {
// 	PROC *p = *list;

// 	if(*list == 0)
// 		return 0;

// 	*list = p->next;

// 	p->next = 0;

// 	return p;
// }

// //put proc in beginning of list
// put_proc(PROC **list, PROC *p)
// {
// 	PROC *curr = *list;

// 	if(p)
// 	{
// 		// p->status = 0;
// 		// p->ppid = 0;	//default parent pid = 0
//   //      	p->parent = 0; //null to parent parent
// 		p->next = *list;
// 		*list->next = p;
// 	}

// }

// enqueue(PROC **queue, PROC *p)
// {
// 	PROC *curr = *queue;

// 	//if null proc return
// 	if(p == 0)
// 	{
// 		return;
// 	}

// 	//if queue is empty point queue to p
// 	//or
// 	//if p has highest prior, put in front
// 	if(curr == 0 || p->priority > curr->priority)
// 	{
// 		p->next = *queue;
// 		*queue = p;
// 		return;
// 	}

// 	while(curr->next && p->priority <= curr->next->priority)
// 	{
// 		curr = curr->next;
// 	}

// 	p->next = curr->next;
// 	curr->next = p;

// }

// PROC * dequeue(PROC **queue)
// {
// 	PROC *p = *queue;

// 	if(p)
// 	{
// 		*queue = p->next;
// 		p->next = 0;
// 	}

// 	return p;
// }

// printQueue(PROC *queue)
// {

// 	while(queue)
// 	{
// 		printf("[%d, %d]->", queue->pid, queue->priority);
// 		queue = queue->next;
// 	}

// 	printf("NULL");
// }