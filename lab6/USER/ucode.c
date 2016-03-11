// ucode.c file

char *cmd[]={"getpid", "ps", "chname", "kmode", "switch", "wait", "exit", "fork", "exec", "pipe", "pfd", "rpipe", "wpipe", "cpipe",0};

#define LEN 64

int show_menu()
{
 printf("*********************************  Menu  *********************************\n");
 printf("*  ps chname kmode switch wait exit fork exec pipe pfd rpipe wpipe cpipe *\n");
 printf("**************************************************************************\n");
 printf("\n");
}

int find_cmd(char *name)
{
  //loop through and return index + 1
  int i = 0;


  while(cmd[i]){

    if(strcmp(name, cmd[i]) == 0 ){
      printf("found_cmd: %s\n", cmd[i]);
      return i;
    } 

    i++;
  }

  //return 1;

  return -1;
  // return command index
  // if(strcmp("ps", name) == 0) return 1;

  // if(strcmp("chname", name) == 0) return 2;

  // if(strcmp("kfork", name) == 0) return 3;

  // if(strcmp("switch", name) == 0) return 4;

  // if(strcmp("wait", name) == 0) return 5;

  // if(strcmp("exit", name) == 0) return 6;

  // if(strcmp("exec", name) == 0) return 7;

  return -1;

}

int getpid()
{
 return syscall(0,0,0);
}

int ps()
{
 return syscall(1, 0, 0);
}

int chname()
{
  char s[32];
  printf("input new name : ");
  gets(s);
  return syscall(2, s, 0);
}

int exec()
{
  char s[32];
  int success;
  printf("input filename : ");
  gets(s);
  success = syscall(8, s, 0);

  if(success < 0){
    printf("your exec request failed.\n");
  }
  else{
    printf("we will never come here.\n");
  }

  return success;
}

int kmode()
{
  printf("kmode not yet implemented\n");
  syscall(3, 0, 0);
}

int kfork()
{   
  int child, pid;
  pid = getpid();
  printf("proc %d enter kernel to kfork a child\n", pid); 
  child = syscall(7, 0, 0);
  if(child)
    printf("proc %d kforked a child %d\n", pid, child);

  return child;
}    

int kswitch()
{
  return syscall(4,0,0);
}

int wait()
{
  int child, exitValue;
  printf("proc %d enter Kernel to wait for a child to die\n", getpid());
  child = syscall(5, &exitValue, 0);
  printf("proc %d back from wait, dead child=%d", getpid(), child);
  if (child>=0)
    printf("exitValue=%d", exitValue);
  else{
    printf("no children to wait for");
  }
  printf("\n"); 
} 

int geti()
{
  // return an input integer
  char buffer[32];
  int i;

  bzero(buffer, 32);

  gets(buffer);

  i = atoi(buffer);

  return i;
}

int exit()
{
 int exitValue;
 printf("enter an exitValue: ");
 exitValue = geti();
 printf("exitvalue=%d\n", exitValue);
 printf("enter kernel to die with exitValue=%d\n", exitValue);
 _exit(exitValue);
}

int _exit(int exitValue)
{
  return syscall(6,exitValue,0);
}

int pipe(){
  int pd[2];

  //printf("pipe not yet implemented\n");
  syscall(9,pd,0);

  printf("pd from kernel: pd[0]=%d, pd[1]=%d\n", pd[0], pd[1]);
}

int pfd(){
  //printf("pfd not yet implemented\n");
  syscall(10,0,0);
}
int read_pipe(){
  //printf("read_pipe not yet implemented\n");
  char buf[1024];
  int fd, nbytes;
  int n = 0;

  printf("enter fd to read: ");
  fd = geti();

  printf("enter nbytes to read: ");
  nbytes = geti();
  bzero(buf, 1024);
  n = syscall(11, fd, buf, nbytes);

  if(n >=0){
    printf("%d bytes read from.\n", n);
    printf("content read: %s\n", buf);
  }else{
    printf("read pipe failed in kernel\n");
  }

}
int write_pipe(){
  //printf("write_pipe not yet implemented\n");
  char buf[1024];
  int fd, nbytes;
  int n = 0;

  printf("enter fd to write to: ");
  fd = geti();

  printf("enter text to write: ");
  bzero(buf, 1024);
  gets(buf);

  nbytes = strlen(buf);
  
  printf("fd=%d nbytes=%d : %s\n", fd, nbytes, buf);
  n = syscall(12,fd, buf, nbytes);

  if(n>=0){
    printf("\nback to Umode, wrote %d bytes to pipe\n", n);

  }else{
    printf("write pipe failed\n");
  }


}
int close_pipe(){
  //printf("close_pipe not yet implemented\n");
  int fd;
  printf("fd to close: ");
  fd = geti();

  syscall(13,fd,0);
}


int invalid(char *name)
{
  printf("Invalid command : %s\n", name);
}

