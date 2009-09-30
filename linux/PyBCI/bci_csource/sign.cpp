

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
#define FONT 3
#define BMP 4
#define NOSHAPE 0 

int sign_window;
double sign_colors[3][3];
unsigned int color_bg; /* background color */
unsigned int color; /* color of the sign */
unsigned int shape; /* shape of the sign */
double trigger_size = 0.5; /* size of the sign */
unsigned long trigger_time = 0; /* showing time of the sign */

unsigned int numof_texture = 0; /* number of the texture to be shown */
GLuint textures[5];			/* storage for the textures */

bool timer = false; /* true if timer is set */
bool glut_initialized = false; /* to avoid initializing a second time */

/* size transformation variables to keep the trigger size constant relative to the window size */
float fx, fy; 

void OnResizeWindow (int sx, int sy)
{
	if (sy == 0)
	{
		sy = 1;
	}
	if (sx == 0)
	{
		sx = 1;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    float fmin = min ( sx, sy );
    float fmax = max ( sx, sy );

    float fR  = fmin / fmax;
    float fR2 = fmax / fmin;

    if ( sx > sy )
    {
       gluOrtho2D   ( -fR2, fR2, -fR, fR );
       fx = 1.0f;
       fy = fR;
    }
    else
    {
        gluOrtho2D   ( -fR, fR, -fR2, fR2 );
        fx = fR;
        fy = 1.0f;

    }
    glViewport( 0, 0, sx, sy );

	glMatrixMode(GL_MODELVIEW);
}


void init()
{
	sign_colors[0][0] = 0; /* black rgb */
	sign_colors[0][1] = 0;
	sign_colors[0][2] = 0;
	sign_colors[1][0] = 1; /* white rgb */
	sign_colors[1][1] = 1;
	sign_colors[1][2] = 1;
	sign_colors[2][0] = 0.6; /* grey rgb */
	sign_colors[2][1] = 0.6;
	sign_colors[2][2] = 0.6;

	shape = NOSHAPE;

	glClearColor(sign_colors[color_bg][0], sign_colors[color_bg][1], sign_colors[color_bg][2], 1);

	glClearDepth(1.0f); 

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL); 

	glShadeModel(GL_SMOOTH); 
	glEnable (GL_POINT_SMOOTH);	/* antialiasing */
	glEnable (GL_LINE_SMOOTH);
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 
 
	string f;
	//f = GetPath(); /* get the path of the bitmap files */
	f = "data/images/";
	int l = f.length();
	
	/* load the implemented textures - there might be a better way? */
	f.replace(l, 100, "/R.bmp");
	LoadGLTextures((char*)f.c_str(), 0);
	f.replace(l, 100, "/L.bmp");
	LoadGLTextures((char*)f.c_str(), 1);
	f.replace(l, 100, "/sr.bmp");
	LoadGLTextures((char*)f.c_str(), 2);
	f.replace(l, 100, "/sl.bmp");
	LoadGLTextures((char*)f.c_str(), 3);
	f.replace(l, 100, "/grey.bmp");
	LoadGLTextures((char*)f.c_str(), 4); 
}



unsigned int sign(void* param)
{
	THREAD_PARAM* pParam = reinterpret_cast <THREAD_PARAM*> (param); 

	if (glut_initialized == false)
	{
		glutInit(&pParam->argc, pParam->argv); /* initialize glut bibliography */

		/* double buffer (creating the graphic 'off-scence' and showing by command), rgba-colors, depth buffer */
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
		
		glutInitWindowPosition(pParam->window_position_x, pParam->window_position_y);
		glutInitWindowSize(pParam->window_size_x, pParam->window_size_y);
	}

	sign_window = glutCreateWindow("Sign Window");

	glutReshapeFunc(OnResizeWindow);
	glutDisplayFunc(display);

	if (glut_initialized == false)
	{
		init(); /* initialize Open GL */
		glut_initialized = true;
	}
	else 
	{
		glClearColor(sign_colors[color_bg][0], sign_colors[color_bg][1], sign_colors[color_bg][2], 1); 

		glClearDepth(1.0f); 
		
		glEnable(GL_TEXTURE_2D);

		string f;
		//f = GetPath(); /* get the path of the bitmap files */
		f = "data/images/";
		int l = f.length();
	
		/* load the implemented textures - there might be a better way? */
		f.replace(l, 100, "/R.bmp");
		LoadGLTextures((char*)f.c_str(), 0);
		f.replace(l, 100, "/L.bmp");
		LoadGLTextures((char*)f.c_str(), 1);
		f.replace(l, 100, "/sr.bmp");
		LoadGLTextures((char*)f.c_str(), 2);
		f.replace(l, 100, "/sl.bmp");
		LoadGLTextures((char*)f.c_str(), 3);
		f.replace(l, 100, "/grey.bmp");
		LoadGLTextures((char*)f.c_str(), 4);
	}

	glutMainLoop(); /* calls display function if necessary */
	
	return 1;
}


void display()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); /* clear buffer */
	glLoadIdentity(); /* reset drawer */	

	if (shape == TRIANGLES)
	{
		glColor3f((GLfloat)sign_colors[color][0], (GLfloat)sign_colors[color][1], (GLfloat)sign_colors[color][2]);
		glBindTexture( GL_TEXTURE_2D, NULL); 
		glBegin(GL_TRIANGLES);
		glVertex2f( 0.0f, trigger_size*fy); /* up */  
		glVertex2f(-trigger_size*fx,-trigger_size*fy); /* bottom left */  
		glVertex2f( trigger_size*fx,-trigger_size*fy); /* bottom right */  
		glEnd();
	}

	else if (shape == QUADS)
	{
		glColor3f((GLfloat)sign_colors[color][0], (GLfloat)sign_colors[color][1], (GLfloat)sign_colors[color][2]);
		glBindTexture( GL_TEXTURE_2D, NULL);
		glBegin(GL_QUADS);
		glVertex2f( trigger_size*fx, trigger_size*fy); /* up right */
		glVertex2f(-trigger_size*fx, trigger_size*fy); /* up left */
		glVertex2f(-trigger_size*fx, -trigger_size*fy); /* bottom left */  
		glVertex2f( trigger_size*fx, -trigger_size*fy); /* bottom right */ 
		glEnd();
	}

	else if (shape == BMP)
	{
		glColor3f(1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, textures[numof_texture]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(-trigger_size*fx, -trigger_size*fy);
		glTexCoord2f(1.0f, 0.0f); glVertex2f( trigger_size*fx, -trigger_size*fy);
		glTexCoord2f(1.0f, 1.0f); glVertex2f( trigger_size*fx,  trigger_size*fy);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(-trigger_size*fx,  trigger_size*fy);
		glEnd();
		glBindTexture( GL_TEXTURE_2D, NULL); 
	}

	else if (shape == FONT)
	{
		glRasterPos3f(0.0f, 0.0f, 1.0f);
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, 'R');
	}

	else if (shape != NOSHAPE)
	{
		printf("Error: Sign mode is unavailable.\n");
	}
	
	send_trigger(5); /* send trigger when sign is shown */
	sleep(10);
	send_trigger(0); /* delete port (0) - still necessary under linux? */
	
	glutSwapBuffers(); /* show graphic (double buffer) */
}


void give_sign(int form, int col, unsigned long time, double size, unsigned int texture)
{
	color = col; 
	shape = form;
        trigger_size = size;
	trigger_time = time;
	numof_texture = texture;

	glutSetWindow(sign_window);
	glutPostRedisplay();
	if (timer == false) /* do not set the timer if already done */
	{
		timer = true;
	    glutTimerFunc(trigger_time, timer_func, 0);
	}
}


void timer_func(int value)
{
	shape = NOSHAPE;
	glutPostRedisplay();
	timer = false;
}


int LoadGLTextures(char *filename, unsigned int numof_texture)									
{
	int Status = false;						

	BMPImg Image;		

	/* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
	if (Image.Load(filename))
	{
		Status = true;							

		glGenTextures(1, &textures[numof_texture]);	/* Create The Texture */

		/* Typical Texture Generation Using Data From The Bitmap */
		glBindTexture(GL_TEXTURE_2D, textures[numof_texture]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, Image.GetWidth(), Image.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, Image.GetImg());
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	}

	return Status;
} 

void set_background_color(int color)
{
	color_bg = color;
}

void send_trigger(int out)
{
	if (ioperm(TRIGGER_PORT, 1, 1)) /* ask for permission */
	{
		outb(out, TRIGGER_PORT);
	}
	else
	{
		printf("Warning: Parallel Port Trigger has not been sent.\n");
	}
}
