/********************************************************************
Copyright 2010-2015 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/
#include "ucode.c"

main()
{ 
  char name[64]; int cmd;
  int pid;

  while(1){
       pid = getpid();
       printf("==============================================\n");
       printf("I am task %d in U mode: segment=%x\n", pid, getcs());
       show_menu();
       printf("Command ? ");
       gets(name); 
       if (name[0]==0) 
           continue;

       cmd = find_cmd(name);
       switch(cmd){
           case 0 : getpid();  break;
           case 1 : ps();       break;
           case 2 : chname();   break;
           case 3 : kmode();    break;
           case 4 : kswitch();  break;
           case 5 : mywait();   break;
           case 6 : myexit();   break;
           case 7 : ufork();    break;
           case 8 : myexec("/bin/u2");  break;
           case 9 : sin();      break;
           case 10 : sout();     break;
           default: invalid(name); break;
       }
  }
}
