#include "ucode.c"
int color;
main()
{ 
  char name[64]; int pid, cmd;
color = 0x0E;
  while(1){
    printf("----------------------------------------------\n");
    pid = getpid();
    
       
    
    printf("Estoy proc %d en el modo usuario: segmento corriendo =%x\n", pid, getcs());
    show_menu();
    printf("La orden ? ");
    gets(name); 
    if (name[0]==0) 
        continue;

    cmd = find_cmd(name);
    switch(cmd){
           case 0 : getpid();   break;
           case 1 : ps();       break;
           case 2 : chname();   break;
           case 3 : pid = kfork();   
           printf("fork returned: %d\n", pid);
            break;
           case 4 : kswitch();  break;
           case 5 : wait();     break;
           case 6 : exit();     break;
           case 7 : pid = exec();   
           printf("exec returned: %d\n", pid);  break;

           default: invalid(name); break;
    }
  }
}



