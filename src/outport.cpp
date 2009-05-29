
#include "outport.h"

inpfuncPtr inp32fp;
oupfuncPtr oup32fp;


int connect_outport()
{
     HINSTANCE hLib;

     hLib = LoadLibrary("inpout32.dll"); 

     if (hLib == NULL) 
	 {
          fprintf(stderr,"LoadLibrary Failed.\n");
		  Sleep(1000);
          return -1;
     }

	 /* assigning the pointers to the port adresses */
     inp32fp = (inpfuncPtr)GetProcAddress(hLib, "Inp32");

     if (inp32fp == NULL) 
	 {
          fprintf(stderr,"GetProcAddress for Inp32 Failed.\n");
		  Sleep(1000);
          return -1;
     }

     oup32fp = (oupfuncPtr) GetProcAddress(hLib, "Out32");

     if (oup32fp == NULL)
	 {
          fprintf(stderr,"GetProcAddress for Oup32 Failed.\n");
		  Sleep(1000);
          return -1;
     }
	 return 1;
}


short Inp32 (short portaddr)
{
	return (inp32fp)(portaddr);
}

void  Out32 (short portaddr, short trigger)
{
	(oup32fp)(portaddr, trigger);
} 
