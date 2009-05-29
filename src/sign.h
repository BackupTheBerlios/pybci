


/*Copyright (c) <2009> <Benedikt Zoefel>

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.*/



#ifndef SIGN_H
#define SIGN_H


#include <windows.h>
#include <stdio.h>
#include "glut.h"
#include "outport.h"

/* variables for the sign thread - passed by bci.cpp - */
typedef struct
{
	int argc;
	char** argv;
	int window_position_x; /* position of sign window */
	int window_position_y;
	int window_size_x; /* size of the sign window */
	int window_size_y;
}THREAD_PARAM; 


void init (void);
void display (void);

/* triggers a sign on a white background for the time <time> (milliseconds) in the shape <form> (1 or TRIANGLES for triangles, 2 or QUADS for quads). When the sign is shown a trigger ('5') is sended via the parallel port (using <outport>). */
extern "C" __declspec(dllexport) void give_sign(int form, unsigned long time);

/* if trigger mode is SIGNS_AVAILABLE this function is called by bci_source.cpp to create a parallel thread to give signs as blinking shapes (triangles or quads) for a specified time by <give_sign> */
DWORD WINAPI sign(LPVOID param);

#endif
