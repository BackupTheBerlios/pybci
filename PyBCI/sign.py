


"""Copyright (c) <2009> <Benedikt Zoefel>

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
OTHER DEALINGS IN THE SOFTWARE."""


import time
from Tkinter import *
from threading import *
from bci_source import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL import *


class Sign_tk(Thread):
    """This class is used just internally to start a new thread for giving signs Tkinter based."""
    def __init__(self, width_win, height_win, color_bg, color_trigger):
        Thread.__init__(self)
        self.height_win = height_win
        self.width_win = width_win
        self.size = 0
        self.time = 0
        self.shape = 1
        self.text = 'NoText'
        self.color_bg = color_bg
        self.color_trigger = color_trigger
         
        self.win = Tk()

        self.canvas = Canvas(self.win, height=self.height_win, width=self.width_win, bg=self.color_bg)

        self.canvas.pack()

        self.update = Event()

    def run(self):
        while(True):
          self.update.wait()
          if self.shape == 1:
              stimulus = self.canvas.create_polygon(self.width_win/2-self.size/2,
                        self.height_win/2+self.size/2,
                        self.width_win/2+self.size/2,
                        self.height_win/2+self.size/2,
                        self.width_win/2,
                        self.height_win/2-self.size/2, fill = self.color_trigger)

          elif self.shape == 2:
              stimulus = self.canvas.create_rectangle(self.width_win/2-self.size/2,
                        self.height_win/2-self.size/2,
                        self.width_win/2+self.size/2,
                        self.height_win/2+self.size/2, fill = self.color_trigger)
              
          elif self.shape == 3:
              stimulus = self.canvas.create_text(self.width_win/2,
                        self.height_win/2, justify = CENTER, fill = self.color_trigger,
                        font = ('TimesNewRoman', self.size), text=self.text)

          self.canvas.update()
        
          self.canvas.after(int(self.time), self.canvas.delete(stimulus))

          self.update.clear()

    def _give_sign(self, shape, trigger_size, time, texture, text):
        """
        This function is used just internally to set a trigger event.
        """
        self.shape = shape
        self.size = int(trigger_size*1000)
        self.time = time
        self.text = text
        self.update.set()


class Sign(Thread):
    """This class is used just internally to start a new thread for giving signs c++ based."""
    def __init__(self, shape, color_bg, color_trigger):
        Thread.__init__(self)
        self.shape = shape
        self.shape_toshow = 1
        self.time = 1
        self.size = 1
        self.colors = {'black':0, 'white':1, 'grey':2}
        self.color_bg = self.colors[color_bg]
        self.color_trigger = self.colors[color_trigger]
        self.sign = Event()
        set_background_color(self.color_bg)
        

    def run(self):
        while(True):
            self.sign.wait()
            give_sign(self.shape.get(self.shape_toshow, 1), self.color_trigger, self.time, self.size, self.texture)
            self.sign.clear()

    def _give_sign(self, shape, trigger_size, time, texture, text):
        """
        This function is used just internally to set a trigger event.
        """
        self.shape_toshow = shape
        self.size = trigger_size
        self.time = time
        self.text = text
        self.texture = texture-1
        self.sign.set()


class Sign_py(Thread):
    """This class is used just internally to start a new thread for giving signs OpenGL based."""
    def __init__(self, width_win, height_win, color_bg, color_trigger):
        Thread.__init__(self)
        self.shape_toshow = 0
        self.time = 1
        self.size = 1
        self.height_win = height_win
        self.width_win = width_win
        self.colors = {'black':0, 'white':1, 'grey':0.6}
        self.color_bg = color_bg
        self.color_trigger = color_trigger
        self.color_toshow = 'grey'
        self.glut_initialized = False
        self.fx = 0.1
        self.fy = 0.1

    def run(self):
        if self.glut_initialized == False:
            
            glutInit(0) # initialize glut bibliography 

	    # double buffer (creating the graphic 'off-scence' and showing by command), rgba-colors, depth buffer 
	    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
	
	    glutInitWindowPosition(100, 100)
	    glutInitWindowSize(self.width_win, self.height_win)

	    self.sign_window = glutCreateWindow("Sign Window")

	glutDisplayFunc(self.display)
	glutReshapeFunc(self._OnResizeWindow)

        self.init() # initialize Open GL

	glutMainLoop() # calls display function if necessary 
        
    def init(self):

	glClearColor(self.colors[self.color_bg], self.colors[self.color_bg], self.colors[self.color_bg], 1)

	glClearDepth(1.0)

        if self.glut_initialized == False:
	    glEnable(GL_DEPTH_TEST)
	    glDepthFunc(GL_LEQUAL) 

	    glShadeModel(GL_SMOOTH)
	    glEnable (GL_POINT_SMOOTH)	# antialiasing 
	    glEnable (GL_LINE_SMOOTH)
	
	    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)

	glMatrixMode(GL_PROJECTION) 
	glLoadIdentity()
	gluPerspective(45.0, 1.0, 1.0, 100.0)

	glMatrixMode(GL_MODELVIEW) 
	glLoadIdentity()   #  'reset'

	self.glut_initialized = True


    def display(self):
        """Function that is redisplayed when giving signs."""
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) # clear buffer 
	glLoadIdentity()   # reset drawer 	

	glColor3f(self.colors[self.color_toshow], self.colors[self.color_toshow], self.colors[self.color_toshow])

	if self.shape_toshow == 1:
	    glBegin(GL_TRIANGLES)
	    glVertex2f( 0.0, self.size*self.fy) # up   
	    glVertex2f(-self.size*self.fx, -self.size*self.fy) # bottom left   
	    glVertex2f(self.size*self.fx, -self.size*self.fy) # bottom right   
	    glEnd()

	elif self.shape_toshow == 2:
	    glBegin(GL_QUADS);
	    glVertex2f( self.size*self.fx, self.size*self.fy)   # up right 
	    glVertex2f(-self.size*self.fx, self.size*self.fy)   # up left 
	    glVertex2f(-self.size*self.fx, -self.size*self.fy)  # bottom left   
	    glVertex2f( self.size*self.fx, -self.size*self.fy)  # bottom right  
	    glEnd()

	#elif self.shape_toshow == 3:
        #    glRasterPos3f(0, 0, 0)
        #    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, self.text)

        else: pass

	glutSwapBuffers() # show graphic (double buffer)

	time.sleep(self.time)
	
	self.shape_toshow = 0
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) # back to grey

	glutSwapBuffers()


    def _give_sign(self, shape, trigger_size, duration, texture, text):
        """
        This function is used just internally to set a trigger event.
        """
        self.shape_toshow = shape
        self.size = trigger_size
        self.time = float(duration/1000)
        self.text = text

        self.color_toshow = self.color_trigger

        glutSetWindow(self.sign_window)
        glutPostRedisplay()


    def _OnResizeWindow (self, sx, sy):
        if sy == 0:
	    sy = 1
	if sx == 0:
	    sx = 1

	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()

        fmin = min ( sx, sy )
        fmax = max ( sx, sy )

        fR  = float(fmin)/fmax
        fR2 = float(fmax)/fmin

        if  sx > sy:
            gluOrtho2D(-fR2, fR2, -fR, fR)
            self.fx = 1.0
            self.fy = fR
        else:
            gluOrtho2D(-fR, fR, -fR2, fR2)
            self.fx = fR
            self.fy = 1.0

        glViewport(0, 0, sx, sy)

	glMatrixMode(GL_MODELVIEW)




        
