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
struct semaphore{
  int value;
  PROC *queue;
};

int P(struct semaphore *s)
{
  int sr = int_off();

   s->value--;
   if (s->value < 0){
      running->status=BLOCK;
      enqueue(&s->queue, running);
      tswitch();
   }

   int_on(sr);

}

int V(struct semaphore *s)
{
    PROC *p;
    int sr = int_off();

    s->value++;
    if (s->value <= 0){
        p = dequeue(&s->queue);
        p->status = READY;
        enqueue(&readyQueue, p);
    }

    int_on(sr);
}
