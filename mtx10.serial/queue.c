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
int enqueue(PROC **queue, PROC *ptr)
{

   PROC *p = *queue;
   if (p == NULL || ptr->pri > p->pri){
         *queue = ptr;
          ptr->next = p;
   }
   else{
         while (p->next && ptr->pri <= p->next->pri)
                p = p->next;
         ptr->next = p->next;
         p->next = ptr;
   }

}

PROC *dequeue(PROC **queue)
{
  PROC *p = *queue;
  if (p)
     *queue = (*queue)->next;
  return p;
}

printQueue(PROC *queue)
{
   PROC *p = queue;
   printf("----------------------------\n");
   printf("readyQueue = ");
   while(p){
     printf("%d->", p->pid);
     p = p->next;
   }
   printf("\n----------------------------\n");
}

/******** getproc()/putproc() ***********/

PROC *get_proc()
{
   return(dequeue(&freeList));
}

put_proc(PROC *ptr)
{
   enqueue(&freeList, ptr);
}

