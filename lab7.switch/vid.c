
/************* VIDEO DRIVER vid.c file of MTX kernel *********************/
#define VDC_INDEX      0x3D4
#define VDC_DATA       0x3D5
#define CUR_SIZE          10  /* cursor size register */
#define VID_ORG           12  /* start address register */
#define CURSOR            14  /* cursor position register */

#define LINE_WIDTH        80  /* # characters on a line */
#define SCR_LINES         25  /* # lines on the screen */
#define SCR_BYTES 4000  /* bytes of ONE screen=25*80 */

#define CURSOR_SHAPE      15    /* block cursor for MDA/HGC/CGA/EGA/VGA... */

// attribute byte: 0x0HRGB, H=highLight; RGB determine color
#define HGREEN          0x0A
#define HCYAN           0x0B
#define HRED            0x0C
#define HPURPLE         0x0D
#define HYELLOW         0x0E

u16 base     = 0xB800;    // VRAM base address
u16 vid_mask = 0x3FFF;    // mask=Video RAM size - 1

u16 offset;               // offset from VRAM base
int color;                // atttirbute byte
int org;                  // current display origin r.e.VRAM base
int row, column;          // logical row, col position

// vid_init() initializes the display org=0 (row,col)=(0,0)
int vid_init()
{ 
  int i, w;
  org = row = column = 0;
  color = HYELLOW;

  set_VDC(CUR_SIZE, CURSOR_SHAPE);    // set cursor size  
  set_VDC(VID_ORG, 0);                // display origin to 0
  set_VDC(CURSOR, 0);               // set cursor position to 0

  // clear screen
  w = 0x0700;    // White, blank char // attribute byte=0000 0111=0000 0RGB
  for (i=0; i<25*80; i++)
      put_word(w, base, 0+2*i);       // write 24*80 blanks to vRAM
}

/*************************************************************************
 scroll(): scroll UP one line
**************************************************************************/
int scroll()
{
  int i;
  u16 w, bytes;  

  // test offset = org + ONE screen + ONE more line
  offset = org + SCR_BYTES + 2*LINE_WIDTH;
  if (offset <= vid_mask){   // offset still within vRAM area
    org += 2*LINE_WIDTH;     // just advance org by ONE line
  }
  else{  // offset exceeds vRAM area ==> reset to vRAM beginning by    
         // copy current rows 1-24 to BASE, then reset org to 0
    for (i=0; i<24*80; i++){
      w = get_word(base, org+160+2*i);
      put_word(w, base, 0+2*i);
    }  
    org = 0;
  }
  // org has been set up correctly
  offset = org + 2*24*80;   // offset = beginning of row 24

  // copy a line of BLANKs to row 24
  w = 0x0C00;  // HRGB=1100 ==> HighLight RED, Null char
  for (i=0; i<80; i++)                  
    put_word(w, base, offset + 2*i);
  set_VDC(VID_ORG, org >> 1);   // set VID_ORG to org     
}


/***************************************************************
    With the video driver, this is the only putc() in MTX
***************************************************************/
// display a char, handle special chars '\n','\r','\b'
int putc(char c)  
{
  int pos, w, offset;

  if (c=='\n'){
    row++;
    if (row>=25){
      row = 24;
      scroll();
    }
    pos = 2*(row*80 + column);
    offset = (org + pos) & vid_mask;
    set_VDC(CURSOR, offset >> 1);
    return; 
  }
  if (c=='\r'){
     column=0;
     pos = 2*(row*80 + column);
     offset = (org + pos) & vid_mask;
     set_VDC(CURSOR, offset >> 1);
     return;
  }
  if (c=='\b'){
    if (column > 0){
      column--;
      pos = 2*(row*80 + column);
      offset = (org + pos) & vid_mask;
      put_word(0x0700, base, offset);
      set_VDC(CURSOR, offset >> 1); 
    }
    return;
  }
  // c is an ordinary char
  pos = 2*(row*80 + column);  
  offset = (org + pos) & vid_mask;
  w = color;
  w = (w << 8) + c;
  put_word(w, base, offset);
  column++;
  if (column >= 80){
    column = 0;
    row++;
    if (row>=25){
      row = 24;
      scroll();
    }
  }
  // calculate new offset
  pos = 2*(row*80 + column);
  offset = (org + pos) & vid_mask;
  set_VDC(CURSOR, offset >> 1);
}     

/*===========================================================================*
 *        set_VDC                      *
 *===========================================================================*/
int set_VDC(u16 reg, u16 val) // set register reg to val
{
  int SR;
  lock();     /* try to stop h/w loading in-between value */
  out_byte(VDC_INDEX, reg); /* set the index register */
  out_byte(VDC_DATA,  (val>>8) & 0xFF); /* output high byte */
  out_byte(VDC_INDEX, reg + 1); /* again */
  out_byte(VDC_DATA,  val&0xFF);  /* output low byte */
  unlock();
}
/********************* end of vid.c file ***********************************/