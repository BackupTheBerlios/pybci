

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



#include "sign.h"

#define TRIANGLES 1
#define QUADS 2
#define NOSHAPE 0 

BOOL bShowMessageBox;
HWND hParentWnd;

int sign_window;
int sign_colors[3][3];
unsigned int color; /* color of the sign */
unsigned int shape; /* shape of the sign */
double trigger_size = 0.5; /* size of the sign */

bool glut_initialized = false; /* to avoid initializing a second time */

void init()
{
	sign_colors[0][0] = 0; /* black rgb */
	sign_colors[0][1] = 0;
	sign_colors[0][2] = 0;
	sign_colors[1][0] = 255; /* white rgb */
	sign_colors[1][1] = 255;
	sign_colors[1][2] = 255;
	sign_colors[2][0] = 153; /* grey rgb */
	sign_colors[2][1] = 153;
	sign_colors[2][2] = 153;

	shape = NOSHAPE;

	glClearColor(0.6f, 0.6f, 0.6f, 0.6f); /* grey background */

	glClearDepth(1.0f); 

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL); 

	glShadeModel(GL_SMOOTH); 
	glEnable (GL_POINT_SMOOTH);	/* antialiasing */
	glEnable (GL_LINE_SMOOTH);
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 

	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();
	gluPerspective(45.0, 1.0, 1.0, 100.0);

	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity(); /* 'reset' */
}



DWORD WINAPI sign(LPVOID param)
{
	THREAD_PARAM* pParam = reinterpret_cast <THREAD_PARAM*> (param); 

	if (glut_initialized == false)
	{
		connect_outport();  /* load outport dll */ 

		glutInit(&pParam->argc, pParam->argv); /* initialize glut bibliography */

		/* double buffer (creating the graphic 'off-scence' and showing by command), rgba-colors, depth buffer */
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
	
		glutInitWindowPosition(pParam->window_position_x, pParam->window_position_y);
		glutInitWindowSize(pParam->window_size_x, pParam->window_size_y);

	}

	sign_window = glutCreateWindow("Sign Window");

	glutDisplayFunc(display);

	if (glut_initialized == false)
	{
		init(); /* initialize Open GL */
		glut_initialized = true;
	}
	else 
	{
		glClearColor(0.6f, 0.6f, 0.6f, 0.6f); /* white background */

		glClearDepth(1.0f); 

		glMatrixMode(GL_PROJECTION); 
		glLoadIdentity();
		gluPerspective(45.0, 1.0, 1.0, 100.0);

		glMatrixMode(GL_MODELVIEW); 
		glLoadIdentity(); /* 'reset' */
	}

	glutMainLoop(); /* calls display function if necessary */
	
	return 1;
}


void display()
{	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); /* clear buffer */
	glLoadIdentity(); /* reset drawer */	

	glColor3f((GLfloat)sign_colors[color][0]/255, (GLfloat)sign_colors[color][1]/255, (GLfloat)sign_colors[color][2]/255);

	glTranslatef(0.0f, 0.0f, -15.0f); 
	
	if (shape == TRIANGLES)
	{
		glBegin(GL_TRIANGLES);
		glVertex3f( 0.0f, trigger_size, 0.0f); /* up */  
		glVertex3f(-trigger_size,-trigger_size, 0.0f); /* bottom left */  
		glVertex3f( trigger_size,-trigger_size, 0.0f); /* bottom right */  
		glEnd();
	}

	else if (shape == QUADS)
	{
		glBegin(GL_QUADS);
		glVertex3f( trigger_size, trigger_size, 0.0f); /* up right */
		glVertex3f(-trigger_size, trigger_size, 0.0f); /* up left */
		glVertex3f(-trigger_size, -trigger_size, 0.0f); /* bottom left */  
		glVertex3f( trigger_size, -trigger_size, 0.0f); /* bottom right */ 
		glEnd();
	}

	else if (shape != NOSHAPE)
	{
		printf("Error: Sign mode is unavailable.\n");
	}
	
	Out32(TRIGGER_PORT, 5); /* send trigger if sign has been shown */

	glutSwapBuffers(); /* show graphic (double buffer) */
}


void give_sign(int form, unsigned long time)
{
	color = 0; /* black */
	shape = form;
	glutSetWindow(sign_window); 
	glutPostRedisplay();

	Sleep(time);

	color = 2; /* grey */
	glutSetWindow(sign_window); 
	glutPostRedisplay();
}

void set_trigger_size(double size)
{
	trigger_size = size/10; /* change the value from 1...10 to 0.1...1 */
}