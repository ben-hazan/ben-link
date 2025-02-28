// this file's bibliography is the i-net.
/*------- LPT.H ------*/
#ifndef __LPT_CPP
#define __LPT_CPP

#ifndef __DOS_H
#include <dos.h>
#endif

/*---------- Return codes ----------*/
#define OK 0
#define NO_PORT 1
#define START_TIMEOUT 2
#define SYNC_TIMEOUT 3
#define CHAR1_TIMEOUT 1
#define GEN_TIMEOUT 2
#define COUNT_FAIL 3
#define CSUM_FAIL 4
/*---------- Bits check ------------*/
#define INPUT_SYNC_BIT  0x80
#define OUTPUT_SYNC_BIT 0x10

/*---------- Global Vars -----------*/

unsigned port;                // lpt's address.

/*---------- Interface -------------*/

int pp_init(int prt);		// initializes port for 2 direction transfer.
int pp_send_n_bytes(int count, unsigned char *input); // sends n bytes to port.
int pp_read_n_bytes(int count, unsigned char *output); // reads n bytes from port.
int pp_read_block(int *count, unsigned char *output); // send a block (using error detection).
int pp_send_block(int count, unsigned char *input); // read a block (using error detection).
void pp_delay();

/*---------- Implementation --------*/

int pp_init(int prt)
{
	unsigned p;
	static unsigned int far *bios_ptr = (unsigned int far*)MK_FP(0x40, 8);
		// to read lpt's address from bios.

	if (prt > 2 || prt < 0) return(NO_PORT);
		// limit lpt.
	p = *(prt + bios_ptr);
		// read lpt's port.
	if (p == 0) return(NO_PORT);
	else {
		outportb(p, 0);
			// initialize to zero output.
		port = p;
			// save lpt's address.
		return(OK);
	}
}


#define NDEL   100
// a simple delay (not very long one too).
void pp_delay(void)
{
   int i;
   
   i = 0;
   while(i < NDEL) i++;
}


int pp_send_n_bytes(int count, unsigned char *input)
{
   asm push si
   asm push di

   asm mov  si, input


   asm mov  di, count
   asm mov  bh, INPUT_SYNC_BIT
   asm mov  dx, port

MAIN_LOOP:
   // on how to send a byte look for the algorithem in the manual.
   asm mov  bl, [si]


   asm inc  dx
   asm xor  cx, cx
L0:
   asm in   al, dx
   asm test al, bh
   asm jnz  L1
   asm loop L0
   
   asm jmp  BREAK_LOOP
   
L1:
   asm dec  dx
   asm mov  al, bl
   asm and  al, 0xf
   asm or   al, OUTPUT_SYNC_BIT
   asm out  dx, al

   asm inc  dx
   asm xor  cx, cx
L2:
   asm in   al, dx
   asm test al, bh
   asm jz   L3
   asm loop L2
   
   asm jmp  BREAK_LOOP

L3:
   asm dec  dx

#ifdef XT_CODE
   asm mov  cl, 4
   asm shr  bl, cl
#else
   asm shr  bl, 4
#endif   

   asm mov  al, bl
   asm out  dx, al 

   asm inc  si
   asm dec  di
   asm jnz  MAIN_LOOP

   asm inc  dx
L6:

	asm in   al, dx
	asm test al, bh
	asm jz  L6


BREAK_LOOP:
   asm mov  ax, count
   asm sub  ax, di
   asm pop  di
   asm pop  si
   return(_AX);

}


int pp_read_n_bytes(int count, unsigned char *output)
{
   asm push si
   asm push di

   asm mov  di, output

   asm mov  si, count
   asm mov  bh, INPUT_SYNC_BIT
   asm mov  dx, port

MAIN_LOOP:
   asm xor  cx, cx
   asm inc  dx

L0:
   asm in   al, dx
   asm test al, bh
   asm jz   L1
   asm loop L0

   asm jmp  BREAK_LOOP

L1:asm in   al, dx
   asm mov  ah, al
   asm dec  dx
   asm mov  al, OUTPUT_SYNC_BIT
   asm out  dx, al

   asm shr  ah, 3

   asm xor  cx, cx
   asm inc  dx
L2:
   asm in   al, dx
   asm test al, bh
   asm jnz  L3
   asm loop L2

   asm jmp  BREAK_LOOP

L3:asm in   al, dx
   asm shl  al, 1
   asm and  al, 0xf0
   asm or   ah, al

   asm dec  dx
   asm xor  al, al
   asm out  dx, al

   asm mov  [di], ah


   asm inc  di
   asm dec  si
   asm jnz  MAIN_LOOP

BREAK_LOOP:
   pp_delay();

   asm mov  ax, count
   asm sub  ax, si

   asm pop  di
   asm pop  si
   return(_AX);
}



int pp_send_block(int count, unsigned char *input)
{
   unsigned csum;

   int c2[2];
   int j;

   asm mov  dx, 0
   asm mov  cx, count
   asm shr  cx, 1
/*   asm mov  ah, 0 */
   asm push ds

   asm mov  si, input

   asm cld
L1:
   asm lodsw
   asm add  dx, ax
   asm loop L1
   asm pop  ds

   asm mov  csum, dx

   c2[0] = c2[1] = count;
   j = pp_send_n_bytes(4, (char *)c2); // sends the number of bytes.
   if (j == 0) return(CHAR1_TIMEOUT);
   if (j != 4) return(GEN_TIMEOUT);

   j = pp_send_n_bytes(count, input);
   if (j != count) return(GEN_TIMEOUT); // sends the bytes.

   j = pp_send_n_bytes(2, (char *)&csum);
   if (j != 2) return(GEN_TIMEOUT);       // sends the sum of the bytes.
					  // detects errors.
   return(OK);
}



int pp_read_block(int *count, unsigned char *output)
{
   int  k;
   unsigned csum_in, csum_cal;
   int c[2];

   k = pp_read_n_bytes(4, (unsigned char *)c);
   if (k == 0) return(CHAR1_TIMEOUT);
   if (k != 4) return(GEN_TIMEOUT);

   if (c[1] != c[0]) return(COUNT_FAIL);

   if (c[0]>(*count)) return COUNT_FAIL;   // Checking size not to big
					   // So we won't step on already
					   // alocated memory.
					   // (im really proud of myself detecting this bug)
					   // (on the internet it wasn't mentioned).
   k = pp_read_n_bytes(c[1], output);
   if (k != c[0]) return(GEN_TIMEOUT);

   k = pp_read_n_bytes(2, (unsigned char *)&csum_in);
   if (k != 2) return(GEN_TIMEOUT);

   k=c[0];

   asm mov  dx, 0
   asm mov  cx, k
   asm shr  cx, 1
/*   asm mov  ah, 0 */
   asm push ds
   asm mov  si, output

   asm cld
L1:
   asm lodsw
   asm add  dx, ax
   asm loop L1
   asm pop  ds

   asm mov  csum_cal, dx

   if (csum_in != csum_cal) return(CSUM_FAIL);

   *count = c[0];
   return(OK);
}
#endif