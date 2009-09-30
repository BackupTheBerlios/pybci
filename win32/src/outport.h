

#include <stdio.h>
#include <conio.h>
#include <windows.h>

#ifndef OUTPORT_H
#define OUTPORT_H

#define TRIGGER_PORT ((short) 0x10A0) /* adress of the parallel port */

/* pointer to short _stdcall Inp32(short PortAddress) in inpout32.dll */
typedef short (_stdcall *inpfuncPtr)(short portaddr);
/* pointer to void _stdcall Out32(short PortAddress, short data) in inpout32.dll */
typedef void (_stdcall *oupfuncPtr)(short portaddr, short trigger);

/* Read parallel port */
short  Inp32 (short portaddr);
/* Send signal via parallel port. Arguments are port adress and signal. */
void  Out32 (short portaddr, short trigger);

/* Loads inpout32.dll und gets port adresses */
int connect_outport(void);

#endif
