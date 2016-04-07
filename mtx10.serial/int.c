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
/************** syscall routing table ***********/
int kcinth() 
{
  u16 x, y, z, w, r; 
  u16 seg, off;

  seg = running->uss; off = running->usp;

  x = get_word(seg, off+13*2);
  y = get_word(seg, off+14*2);
  z = get_word(seg, off+15*2);
  w = get_word(seg, off+16*2);
  
   switch(x){
       case 0 : r = running->pid;    break;
       case 1 : r = kps();           break;
       case 2 : r = chname(y);       break;
       case 3 : r = kmode();         break;
       case 4 : r = tswitch();       break;
       case 5 : r = kwait();         break;
       case 6 : r = kexit();         break;
       case 7 : r = fork();          break;
       case 8 : r = kexec(y);        break;

       case 9 : r = sout(y); break;
       case 10: r = sin(y); break;

       case 99: r = kexit();                break;

       default: printf("invalid syscall # : %d\n", x);

   }
   put_word(r, seg, off+2*8);
}


int sin(char *y)
{
  char inline[128], *p, c;
   int n;

   sgetline(inline);
   n = strlen(inline);

   p = inline;
   
   while (*p != 0){
     put_byte(*p, running->uss, y);
       p++; y++;
   }
   put_byte(0, running->uss, y);
   return n;
}

int sout(char *y)
{
  char outline[128], *p, c;
   p = outline;
   c = get_byte(running->uss, y);
   while (c != 0){
       *p = c;
       p++; y++;
       c = get_byte(running->uss, y);
   }
   *p = 0;

   printf("outline = %s",outline);

   sputline(outline);
   return 0;
}


