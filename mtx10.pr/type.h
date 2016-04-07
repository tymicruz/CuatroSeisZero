typedef unsigned char   u8;
typedef unsigned short u16;
typedef unsigned long  u32;
typedef unsigned short ushort;
typedef unsigned long  ulong;

#define NULL     0
#define NPROC    9
#define SSIZE 1024

/******* PROC status ********/
#define FREE     0
#define READY    1
#define RUNNING  2
#define SLEEP    3
#define BLOCK    4
#define ZOMBIE   5

typedef struct proc{
    struct proc *next;
    int    ksp;
    int    uss, usp;
    int    inkmode;

    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int    ppid;               // parent pid
    struct proc *parent;
    int    priority;
    int    event;
    int    exitCode;
    char   name[32];
    int    kstack[SSIZE];      // per proc stack area
}PROC;