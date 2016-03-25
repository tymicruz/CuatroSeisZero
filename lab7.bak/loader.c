//#include <stdio.h>
//#include <string.h>
//#include "ext2.h"

//typedef unsigned char  u8;
//typedef unsigned short u16;
//typedef unsigned long  u32;

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

//struct dap dap, *dp;       // global dap struct
//u16 color = RED;           // initial color for putc()

SUPER *sp;
GD 	  *gp;
INODE *ip;
DIR   *dp;


#define  BOOTSEG 0x9000


//#include "bio.c" /d/<========= WRITE YOUR OWN io.c file !!!
// load a disk sector to (DS, addr), where addr is an offset in segment

int get_block(u16 blk, char *buf)
{
	u16 c = blk / 18;
	u16 h = ((2*blk)%36)/18;
	u16 s = ((2*blk)%36)%18;

  diskr(c, h, s, buf);    // call int13-43 in assembly
}

u16 nextDataBlock(INODE *ip, int blocknum){
	int i = 0, j = 0;
	u32 *intp1, *intp2;

	char level1[1024], level2[1024];

	int count = 0, blocks = 0;

	if(ip->i_size % 1024){
		blocks++;
	}

	//printf("first dblock: %d\n", ip->i_block[0]);
	//return (ip->i_block[0]);

	blocks += ip->i_size / 1024;

	for(i = 0; i < 12 && i < blocks; i++, count++){
		if(blocknum == count)
		{
      printf("%d\n", ip->i_block[i]);
			return ip->i_block[i];
		}
		if(ip->i_block[i] == 0)
		{
			return 0;
		}
	}

	if(i >= blocks){
		return 0;
	}

	blocks-=i;

	j = ip->i_block[12];

	get_block(j, level1);

	intp1 = level1;
	i = 0;
	while(intp1 < level1 + 1024 && intp1 != 0 && *intp1 != 0){
		if(blocknum == count)
		{
       printf("%d\n", *intp1);

			return (*intp1);
		}

		intp1++;
		i++;
		count++;
	}

	if(i >= blocks)
	{
		return 0;
	}

	blocks-=i;

	j = ip->i_block[13];

	get_block(j, level1);
	intp1 = level1;
	i = 0;

	while(intp1 < level1 + 1024 && intp1 !=0 && *intp1 != 0){
		get_block(*intp1, level2);
		intp2 = level2;

		while(intp2 < level2 + 1024 && intp2 != 0 && *intp2 != 0){
			if(blocknum == count)
			{
        printf("%d\n", *intp2);
				return (*intp2);
			}

			intp2++;
			i++;
			count++;
		}

		intp1++;
	}

	blocks-=i;

	if(blocks > 0) {return 0;} //technically we should check indirect blocks

	return 0;
}
int search(INODE *ip, char *name)
{
    char *cp, temp;
    u16 more, block_check = 0, i = 0;

    //printf("please print\n");
    //if passed a null pointer
    //if (parent == 0){printf("wtf null mide in fmyname()\n");return 0;}
    if(!ip) {
    	printf("wtf\n");
    	return 0;
    }
    //int block_num, offset;
    //idont need to get inode from disk because it is already in memory
    //getInodeBlockNumberAndOffset(parent->ino, &block_num, &offset);
   //printf("looking for %s:\n", name);
        //ip = (INODE *)inode_buf + 1;//get first inode which is root
    //load data block of root inode
    //printf("please print\n");
    get_block((u16)ip->i_block[block_check], dbuf);   
    //printf("please print\n");
   
    dp = dbuf;
    cp = dbuf;//same as cp = data_buf

    more = (u16)nextDataBlock(ip, block_check++);
    //printf("i_size %d\n", ip->i_size);
   
    //make sure we check all data blocks of dir, not just i_block[0]
    while(more)
    {
        more = (u16)nextDataBlock(ip, block_check++);
        //printf("1\n");

        while(cp < dbuf + 1024)
        {
        	//printf("2\n");
            temp = dp->name[dp->name_len];
            dp->name[dp->name_len] = 0;
            //printf("%s\n", dp->name);
            if(strcmp(dp->name, name) == 0)
            {
            	
                dp->name[dp->name_len] = temp;
                return dp->inode;
            }   

            dp->name[dp->name_len] = temp;
            //move cp and dp pointers
            cp+= dp->rec_len;
            dp = cp;
        }

        //if there's more get it, else we'll break on the next outer loop
        if(more)
        {
                get_block(more, dbuf);   
   
                dp = dbuf;
                cp = dbuf;//same as cp = data_buf
                //i could break here
        }
    }

    return 0;
}

int loadino(u16 perblock, u16 beginblock, u16 inostart, u16 ino, INODE* ip)
{
	u16 block;
	u16 offset;

  	block = (ino-1)/perblock + beginblock;
  	offset = (ino-1)%perblock;

  	get_block(block, ibuf);
  	
  	ip = ibuf;

  	ip+=offset;
}

char *names[] = {"bin", "u2"};
int myload(char *filename, u16 segment)
{
	//char buf[64];
	//char buf2[1024];
	u16 i, more, total_size, loaded_size;
	u16 ino = 2; // root inode
	u16 block, offset;
	u16 iperblock, ibeginblock;
	u16 *u16p;
	//data_block = 1;

	char *cp;

	get_block((u16)1, sbuf);

    sp = sbuf;

    //printf("magic number? %x\n", sp->s_magic);
    //printf("inode size: %d\n", sp->s_inode_size);
    //printf("inodes per group: %d\n", sp->s_inodes_per_group);
    iperblock = 1024 / sp->s_inode_size;

    get_block((u16)2, gbuf);

    gp = gbuf;

    //printf("data block bitmap : %d\n", gp->bg_block_bitmap);
    //printf("inode block bitmap: %d\n", gp->bg_inode_bitmap);
  	//printf("first inode block: %d\n", gp->bg_inode_table);
  	ibeginblock = gp->bg_inode_table;

  	block = (ino-1)/iperblock + ibeginblock;
  	offset = (ino-1)%iperblock;

  	get_block(block, ibuf);

  	ip = ibuf;

  	ip+=offset; //root inode

  		strcpy(sbuf, filename);

  		cp = strtok (sbuf,"/");
  		while (cp != 0)
  		{
  			//printf("myload2\n");
  			ino = (u16)(search(ip, cp));
  			//printf ("%s\n",cp);
  			//printf("myload3\n");
  			if(!ino){
  				printf("fail\n");
  				return 0;
  			}else{

  				cp = strtok (0, "/");

  				if(!cp) {
  					//printf("found the fast piece\n");
  					break;
  				}

  				block = (ino-1)/iperblock + ibeginblock;
  				offset = (ino-1)%iperblock;

  				//printf("good ino: %d, block: %d, offset: %d\n", ino, block, offset);

  				get_block(block, ibuf);

  				ip = ibuf;

  				ip+=offset;

  				if((ip->i_mode & 0xF000) == 0x4000){
  					//printf("dir good.\n");

  				}else{
  					//printf("can't go any farther.\n");
  				}
  			}


  		}


  		printf("load ino %d data blocks to %x\n", ino, segment);
  		printf("success!\n");

  		//load data blocks of ino we just found

  		//first load ino and point ip to it
  	block = (ino-1)/iperblock + ibeginblock;
  	offset = (ino-1)%iperblock;

  	get_block(block, ibuf);

  	ip = ibuf;

  	ip+=offset;
  		//here ino is here let's get data blocks
  	block = 0;
  	offset = 0;
  	total_size = ip->i_size;
  	loaded_size = 0;

  		while(more = nextDataBlock(ip, block++)){
  			bzero(dbuf, 1024);
  			get_block(more, dbuf);

  			u16p = dbuf;

        if(!loaded_size){
          printf("skip header\n");
          u16p+= 16;
        }

  			while(u16p < dbuf + 1024 && loaded_size < total_size){

  				put_word((u16)(*u16p), segment, (offset)*2); //load 2 bytes at a time
  				u16p++; //point to next 2 byte word in the buf
  				offset++;
  				loaded_size += 2;
  			}

  		}

      printf("load size %d vs. total size %d\n", loaded_size, total_size);

  	// for (i=1; i<=12; i++){
   //     put_word(0, segment, -2*i);
   // }

   // put_word(0x0200,  segment, -2*1);   /* flag */  
   // put_word(segment, segment, -2*2);   /* uCS */
   // put_word(segment, segment, -2*11);  /* uES */
   // put_word(segment, segment, -2*12);  /* uDS */
    printf("tyler's load %s to %x complete.\n", filename, segment);
   // /* initial USP relative to USS */
   // p->usp = -2*12; 
   // p->uss = segment;

  		return 1;



}















































// get_block((u16)ip->i_block[0], dbuf);

//   	dp = dbuf;
//   	cp = dp;

//   	more = nextDataBlock(ip, datablock++);

//   	while(more)
//   	{

//   		more = nextDataBlock(ip, datablock++)
//   		ino = 0;

//   		while(dp < dbuf + 1024){
//   			temp = dp->name[dp->name_len];
//   			dp->name[dp->name_len] = 0;
//   			printf("%s\n", dp->name);

//   			if(strcmp(filename, dp->name) == 0){

//   				dp->name[dp->name_len] = temp;
//   				ino = dp->inode;



//   				break;

//   			}

//   			dp->name[dp->name_len] = temp;
//   			cp+=dp->rec_len;
//   			dp = cp;
//   		}

//   		if(ino){ //found go to next inode

// 		  	block = (ino-1)/iperblock + ibeginblock;
// 		  	offset = (ino-1)%iperblock;

// 		  	get_block(block, ibuf);

// 		  	ip = ibuf;

// 		  	ip+=offset;
//   		}
//   		else if(more) //didn't find look for more data blocks
//         {
//                 get_block(more, dbuf);   
   
//                 dp = dbuf;
//                 cp = dbuf;//same as cp = data_buf
//                 //i could break here
//         }
//         else{
//         	return -1;
//         }

//   	}