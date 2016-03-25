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
/******************** timer.c file *************************************/
#define LATCH_COUNT     0x00     /* cc00xxxx, c = channel, x = any */
#define SQUARE_WAVE     0x36     /* ccaammmb, a = access, m = mode, b = BCD */

#define TIMER_FREQ   1193182L    /* clock frequency for timer in PC and AT */
#define TIMER_COUNT  TIMER_FREQ/60 /* initial value for counter*/

#define TIMER0       0x40
#define TIMER_MODE   0x43
#define TIMER_IRQ       0

u16 tick;

typedef struct tn{
       struct tn *next;
       int    time;
       PROC   *who;
} TNODE;

TNODE tnode[NPROC], *tq, *ft;
u16 hours, minutes, seconds;

int enable_irq(u16 irq_nr)
{
  lock();
    out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));

}

int timer_init()
{
  /* Initialize channel 0 of the 8253A timer to e.g. 60 Hz. */
  int i;
  printf("timer init\n");
  tick = 0; 
  hours = minutes = seconds = 0;
  out_byte(TIMER_MODE, SQUARE_WAVE);  // set timer to run continuously
  out_byte(TIMER0, TIMER_COUNT);  // timer count low byte
  out_byte(TIMER0, TIMER_COUNT >> 8); // timer count high byte 
  enable_irq(TIMER_IRQ); 

  ft = &tnode[0]; 
  for (i=0; i<NPROC; i++)
    tnode[i].next = &tnode[i+1];
  tnode[NPROC-1].next=0;
  tq = 0;
}

/*===================================================================*
 *        timer interrupt handler                *
 *===================================================================*/
 int thandler()
 {
  int i; PROC *p; TNODE *tp;
  u16 s1, s2, m1,m2,h1, h2;

   s1 = seconds / 10;
  s2 = seconds % 10;
  m1 = minutes / 10;
  m2 = minutes % 10;
  h1 = hours / 10;
  h2 = hours % 10;

  //convert to ascii value
  s1 += '0';
  s2 += '0';
  m1 += '0';
  m2 += '0';
  h1 += '0';
  h2 += '0';

  h1 = (color << 8) + h1;
  h2 = (color << 8) + h2;
  m1 = (color << 8) + m1;
  m2 = (color << 8) + m2;
  s1 = (color << 8) + s1;
  s2 = (color << 8) + s2;

  put_word(h1, base, offset+(72*2));
  put_word(h2, base, offset+(73*2));
  put_word(((u16)':')+ (color << 8), base,offset+(74*2));
  put_word(m1, base, offset+(75*2));
  put_word(m2, base, offset+(76*2));
  put_word(((u16)'.')+ (color << 8), base, offset+(77*2));
  put_word(s1, base, offset+(78*2));
  put_word(s2, base, offset+(79*2));
  
  tick++; 

  tick %= 60;

  if (tick == 0){                      // at each second
    seconds++;
    if(seconds == 60)
     minutes++;
   if(minutes == 60)
    hours++;
  seconds %= 60;
  minutes %=60;
  hours %= 24;
  
 

  //printf("base: %x\n", base);
  //printf("offset: %x\n", offset);
    //printf("%d::%d::%d\n", hours,minutes,seconds);
     /* processing timer queue elements */
     if (tq){ // do these only if tq not empty
       tp = tq;
       while (tp){
         tp->time--;
             printTQ();   
             if (tp->time <= 0){ // wakeup sleepers
               printf("\ntimer interrupt handler : wakeup %d\n", tp->who->pid);
               kwakeup(&tp->who->time);
               tq = tp->next;
               put_tnode(tp);
               tp = tq;
             }
             else{
               break;
             }
           }
         }

        // if (running->inkmode == 1){
       // printf("proc%d time remain = %d\n", running->pid, running->time);
       //   running->time--;
        //}
      }
  out_byte(0x20, 0x20);                // tell 8259 PIC EOI

 //  if (running->time <= 0){
 //    printf("proc%d time up, switch process\n", running->pid);
 //   tswitch();
 // }
}

TNODE *get_tnode()
{
    TNODE *tp;
    tp = ft;
    ft = ft->next;
    return tp;
}

int put_tnode(TNODE *tp)
{
    tp->next = ft;
    ft = tp;
}

int printTQ()
{
   TNODE *tp;
   tp = tq;
   printf("timerQueue = ");
   while(tp){
      printf(" [P%d: %d] ==> ", tp->who->pid, tp->time);
      tp = tp->next;
   }
   printf("\n");
}

itimer(int time)
{
    TNODE *t, *p, *q;
    int ps;

    // CR between clock and this process
    ps = int_off();
    t = get_tnode();
    t->time = time;
    t->who = running;
    /******** enter into tq ***********/
    if (tq==0){
        tq = t;
        t->next = 0;
    }
    else{
          q = p = tq;
          while (p){ 
              if (time - p->time < 0) 
                  break;  
              time -= p->time;
              q = p;
              p = p->next;
          }
          if (p){ 
              p->time -= time;
          }
          t->time = time;
          if (p==tq){
              t->next = tq;
              tq = t;
          }
          else{
                t->next = p;
                q->next = t;
          }
    }
    int_on(ps);
    //printTQ();
    printf("PROC%d going to sleep\n", running->pid);
    ksleep(&running->time);  // sleep on its own time address
}
