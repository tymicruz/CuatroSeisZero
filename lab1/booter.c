//#include <stdio.h>
//#include <string.h>
//#include "ext2.h"

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#include "ext2.h"

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc GD;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR;

#define  GREEN  10         // color byte for putc()
#define  CYAN   11
#define  RED    12
#define  MAG    13
#define  YELLOW 14

struct partition {         // Partition table entry in MBR
       u8  drive;          // 0x80 - active 
       u8  head;	   // starting head 
       u8  sector;	   // starting sector 
       u8  cylinder;       // starting cylinder 
       u8  type;	   // partition type 
       u8  end_head;       // end head 
       u8  end_sector;	   // end sector 
       u8  end_cylinder;   // end cylinder 
       u32 start_sector;   // starting sector counting from 0 
       u32 nr_sectors;     // nr of sectors in partition 
};

struct dap{                // DAP for extended INT 13-42  
       u8   len;           // dap length=0x10 (16 bytes)
       u8   zero;          // must be 0  
       u16  nsector;       // number of sectors to read: 1 to 127
       u16  addr;          // memory address = (segment:addr)
       u16  segment;    
       u32  s1;            // low  4 bytes of sector#
       u32  s2;            // high 4 bytes of sector#
};

struct dap dap, *dp;       // global dap struct
u16 color = RED;           // initial color for putc()

#define  BOOTSEG 0x9000


//#include "bio.c" /d/<========= WRITE YOUR OWN io.c file !!!!

char mbr[512];
char ans[64];

// load a disk sector to (DS, addr), where addr is an offset in segment

int getSector(u32 sector, char *addr)
{
  dp->addr    = addr;
  dp->s1      = (u32)sector;
  diskr();    // call int13-43 in assembly
}

int main()
{
  int i = 0;
  struct partition *p;
  prints("booter start in main()\n");

  // initialize the dap struct
  dp = &dap;
  dp->len  = 0x10;        // must be 16
  dp->zero = 0;           // must be 0
  dp->nsector = 1;        // load one sector
  dp->addr = 0;           // will set to addr              
  dp->segment = BOOTSEG;  // load to which segment
  dp->s1 = 0;             // will set to LBA sector#
  dp->s2 = 0;             // high 4-byte address s2=0

  getSector((u32)0, (u16)mbr); // get MBR
//  ==========================================================
//  In the MBR, partition table begins at byte offset 0x1BE
  // DO #1:  WRITE C CODE TO PRINT THE PARTITIONS
//  ==========================================================
  color = CYAN;
//  while(1){
    prints("what's your name? \n");
    gets(ans);

//	prints(ans);
    if (strcmp(ans, "quit")==0){
      prints("\nexit main()\n");
    }
   printf("\nWelcome %s! %d\n", ans, ++i);
    //break;
   printf("let's start booter\n");
 // }
}
