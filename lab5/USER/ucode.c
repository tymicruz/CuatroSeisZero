// ucode.c file

char *cmd[]={"getpid", "ps", "chname", "kfork", "switch", "wait", "exit", "exec", 0};

#define LEN 64

int show_menu()
{
   printf("***************** Menu *************************\n");
   printf("*  ps  chname  kfork  switch  wait  exit  exec *\n");
   printf("************************************************\n");
}

int find_cmd(char *name)
{
  //loop through and return index + 1


  // return command index
  if(strcmp("ps", name) == 0) return 1;

  if(strcmp("chname", name) == 0) return 2;

  if(strcmp("kfork", name) == 0) return 3;

  if(strcmp("switch", name) == 0) return 4;

  if(strcmp("wait", name) == 0) return 5;

  if(strcmp("exit", name) == 0) return 6;

  if(strcmp("exec", name) == 0) return 7;

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
    success = syscall(7, s, 0);

    if(success < 0){
      printf("your exec request failed.\n");
    }
    else{
      printf("we will never come here.\n");
    }

    return success;
}

int kfork()
{   
  int child, pid;
  pid = getpid();
  printf("proc %d enter kernel to kfork a child\n", pid); 
  child = syscall(3, 0, 0);
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

int invalid(char *name)
{
    printf("Invalid command : %s\n", name);
}
