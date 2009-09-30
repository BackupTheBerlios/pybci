


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

#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <sys/time.h>
#include <sys/io.h>
#include <GL/glut.h>
#include "BitmapLoader.h"

#define TRIGGER_PORT 0x10A0

using namespace std;

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
void timer_func (int value); /* function to clear the sign after a specified (<give_sign>) time */ 

/* Load BMP image files and convert to textures */
int LoadGLTextures(char *filename, unsigned int numof_texture);

/*Triggers a sign on a white background for the time <time> (milliseconds) in the shape <form> 
(1 or TRIANGLES for triangles, 2 or QUADS for quads, 3 or FONT for text, 4 or BMP for bitmaps).
 When the sign is shown a trigger ('5') is sended via the parallel port. */
void give_sign(int form, int col, unsigned long time, double size, unsigned int texture);

void set_background_color(int color);

/* if trigger mode is SIGNS_AVAILABLE this function is called by bci_source.cpp to create a parallel thread to give signs as blinking shapes (triangles or quads) for a specified time by <give_sign> */
unsigned int sign(void* param);

void send_trigger(int out); /* send trigger via parallel port */

#endif

