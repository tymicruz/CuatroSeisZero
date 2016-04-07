!********************************************************************
!Copyright 2010-2015 K.C. Wang, <kwang@eecs.wsu.edu>
!This program is free software: you can redistribute it and/or modify
!it under the terms of the GNU General Public License as published by
!the Free Software Foundation, either version 3 of the License, or
!(at your option) any later version.

!This program is distributed in the hope that it will be useful,
!but WITHOUT ANY WARRANTY; without even the implied warranty of
!MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!GNU General Public License for more details.

!You should have received a copy of the GNU General Public License
!along with this program.  If not, see <http://www.gnu.org/licenses/>.
!********************************************************************/
.globl _getc,_putc,_getcs, _getds
.globl _main,_prints, _syscall
.globl _diskio                 

        call _main

        mov   ax,#mes
        push  ax
        call  _prints
        pop   ax

dead:   jmp   dead

mes:    .asciz "BACK TO ASSEMBLY AGAIN\n\r"

_diskio:

_getcs:
        mov   ax, cs
        ret
_getds: mov   ax, ds
        ret
        !---------------------------------------------
        !  char getc[]   function: returns a char
        !---------------------------------------------
_getc:
        xorb   ah,ah           ! clear ah
        int    0x16            ! call BIOS to get a char in AX
        ret 

        !----------------------------------------------
        ! void putc[char c]  function: print a char
        !----------------------------------------------
_putc:           
        push   bp
        mov    bp,sp

        movb   al,4[bp]        ! get the char into aL
        movb   ah,#14          ! aH = 14
        movb   bl,#12
        int    0x10            ! call BIOS to display the char

        pop    bp
        ret

_syscall:
        int    80
        ret

