int show_pipe(PIPE *p){
  int i,j;

  printf("------------ PIPE CONTENTS ------------\n");
  printf("nreader=%d  nwriter=%d  ", p->nreader, p->nwriter);
  printf("data=%d  room=%d\n", p->data, p->room);

  prints("contents=>");

  if(p->data){
    j = p->tail;

    for(i =0; i < p->data; i++){
      putc(p->buf[j++]);
      j = j % PSIZE;
    }
  }

  printf("<\n----------------------------------------\n");
}


char *MODE[ ] ={"READ      ", "WRITE     ", "RDWR      ", "APPEND    ", "READ_PIPE ", "WRITE_PIPE"};

int kpfd(){


	int i, count = 0;

	printf("Attempt print fds in kernel\n");

	for(i=0; i <NFD; i++){
		if(running->fd[i]) count++;
	}

	if(count == 0){
		printf("no opened file descriptors for this proc\n");
		return -1;
	}

	printf("=========== valid fd ===========\n");
	for(i=0;i < NFD; i++){
		if(running->fd[i]){
			printf("%d    %s refCount = %d\n",
				i, MODE[running->fd[i]->mode], running->fd[i]->refCount);
		}
	}

	printf("================================\n");
}

int kpipe(int *pd){


  //remove, just a test to put value in user space
  //put_word(4, running->uss, pd);
  //put_word(5, running->uss, pd + 1);

  PIPE *p; OFT *p0, *p1; int i;

  printf("attempt pipe in kernel\n");

  for(i=0; i <NPIPE; i++){
  	if(pipe[i].busy == 0){
  		break;
  	}
  }

  pipe[i].busy = 1;
  p = &pipe[i];
  p->head = p->tail = p->data = 0;
  p->room = PSIZE;

  for(i = 0; i < NOFT; i++){
  	if(oft[i].refCount == 0) break;
  }

  p0 = &oft[i]; p0->refCount = 1;

  for(i = 0; i < NOFT; i++){
  	if(oft[i].refCount == 0) break;
  }

  p1 = &oft[i]; p1->refCount = 1;

  //init fd of both ended of pipe
  p0->mode = READ_PIPE;
  p1->mode = WRITE_PIPE;

  p0->refCount = p1->refCount = 1;

  p0->pipe_ptr = p1->pipe_ptr = p;

  p->nreader = 1; p->nwriter = 1;

  //i feel like we can do this differently with 2 loops
  //the fds don't have to be back to back as far as I know
  for(i = 0; i < NFD; i+= 2){
  	if(running->fd[i] == 0 && running->fd[i+1] == 0){
  		running->fd[i] = p0;
  		running->fd[i+1] = p1;
  		break; 
  	}
  }

  put_word(i, running->uss, pd);
  put_word(i + 1, running->uss, pd + 1);

  printf("created pipe in kernel: file descriptors = [%d %d]\n", i, i + 1);

  return 0;//many places to return -1 if pipe creation attempt fails
}

int kclose_pipe(int fd){

  printf("Attempt to close (fd), (%d)\n", fd);

  if(fd >= NFD || fd < 0){
    printf("invalid file descriptor (out of bounds).\n");
    return -1;
  }

  if(!running->fd[fd]){
    printf("invalid file descriptor (not opened).\n");
    return -1;
  }

  running->fd[fd]->refCount--;

  if(running->fd[fd]->mode == WRITE_PIPE){
    running->fd[fd]->pipe_ptr->nwriter--;
    printf("Attempt writer close\n");

    if(running->fd[fd]->pipe_ptr->nwriter == 0){ //refCount of fd should = nwriter
      printf("last writer of pipe closing\n");
      if(running->fd[fd]->pipe_ptr->nreader <= 0){
        //free pipe
        running->fd[fd]->pipe_ptr->busy = 0;
        running->fd[fd]->refCount = 0;
        running->fd[fd] = 0;
        printf("pipe has been freed\n");
        printf("successful closing of fd: %d\n", fd);
        return 0;
      }

      running->fd[fd]->refCount = 0;

    }
    //wake up data
    kwakeup(&(running->fd[fd]->pipe_ptr->data));

  }
  else if(running->fd[fd]->mode == READ_PIPE){

    running->fd[fd]->pipe_ptr->nreader--;
    printf("Attempt reader close\n");

    if(running->fd[fd]->pipe_ptr->nreader == 0){
      printf("last reader of pipe closing\n");

      if(running->fd[fd]->pipe_ptr->nwriter == 0){

        running->fd[fd]->pipe_ptr->busy = 0;
        running->fd[fd]->refCount = 0;
        running->fd[fd] = 0;
        printf("pipe has been freed\n");
        printf("successful closing of fd: %d\n", fd);
        return 0;
      }
      
      running->fd[fd]->refCount = 0;
    }

    //wake up room
    kwakeup(&(running->fd[fd]->pipe_ptr->room));
    
  }

  running->fd[fd] = 0;

  printf("successful closing of fd: %d\n", fd);


  return 0;
}


int kread_pipe(int fd, char *buf, int n){

  int r = 0;
  OFT *op;
  PIPE *pp;
  char c;

  printf("Attempted to read %d bytes from fd (%d)\n", n, fd);

  if(n <= 0){
    return 0;
  }

  if(fd >= NFD || fd < 0){
    printf("invalid file descriptor (out of bounds).\n");
    return -1;
  }

  if(!running->fd[fd]){
    printf("invalid file descriptor (not opened).\n");
    return -1;
  }

  op = running->fd[fd];
  pp = op->pipe_ptr;

  if(op->mode != READ_PIPE){
    printf("fd (%d) is not for pipe read\n", fd);
    return -1;
  }

  printf("pipe before reading\n");
  show_pipe(pp);

  while(n){
    while(pp->data){
      c = pp->buf[(pp->tail)++];
      put_byte(c, running->uss, buf);
      pp->tail = pp->tail % PSIZE;
      pp->data--; pp->room++;
      n--; r++; buf++;

      if(n == 0) break;
    }

    if(n == 0 || r){
      kwakeup(&(pp->room));
      printf("pipe after reading\n");
      show_pipe(pp);
      return r;
    }

    //need some but didn't get none
    if(pp->nwriter){
      printf("pipe before reader goes to sleep\n");
      show_pipe(pp);
      kwakeup(&(pp->room));
      ksleep(&(pp->data));
      continue;
    }

    //pipe has no writer and no data
    printf("pipe had no writer and no data\n");
    return 0;

  }

  printf("how tf do we get here?\n");
  return 0;
}

int kwrite_pipe(int fd, char *buf, int n){

  char c; int r = 0;
  PIPE *pp; OFT* op;



  printf("Attempt pipe write of %d bytes to fd (%d)\n", n, fd);

  //  if(n <= 0){
  //   return 0;
  // }

  if(fd >= NFD || fd < 0){
    printf("invalid file descriptor (out of bounds).\n");
    return -1;
  }

  if(!running->fd[fd]){
    printf("invalid file descriptor (not opened).\n");
    return -1;
  }

  op = running->fd[fd];
  pp = op->pipe_ptr;

  if(op->mode != WRITE_PIPE){
    printf("fd (%d) is not for pipe read\n", fd);
    return -1;
  }

  printf("pipe before writing\n");
  show_pipe(pp);

  while(n){
    if(!pp->nreader){
      printf("proc %d : BROKEN_PIPE error\n", running->pid);
      kexit(0x0D);
    }

    while(pp->room && n){
      pp->buf[pp->head++] = get_byte(running->uss, buf);
      pp->head = pp->head % PSIZE;

      pp->data++; pp->room--;
      n--; r++; buf++;
    }

    kwakeup(&(pp->data));

    if(n==0){
      printf("pipe after writing\n");
      show_pipe(pp);

      return r;
    }

    printf("pipe before writer goes to sleep\n");
    show_pipe(pp);

    ksleep(&(pp->room));

  }
  
  return r;
}




