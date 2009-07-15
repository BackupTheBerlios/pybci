


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

class Sign(Thread):
    """This class is used just internally to start a new thread for giving signs."""
    def __init__(self, width_win, height_win, color_bg, color_trigger):
        Thread.__init__(self)
        self.height_win = height_win
        self.width_win = width_win
        self.size = 0
        self.time = 0
        self.shape = 1
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
                        self.height_win/2-self.size/2, fill=self.color_trigger)

          elif self.shape == 2:
              stimulus = self.canvas.create_rectangle(self.width_win/2-self.size/2,
                        self.height_win/2-self.size/2,
                        self.width_win/2+self.size/2,
                        self.height_win/2+self.size/2, fill=self.color_trigger)

          self.canvas.update()
        
          self.canvas.after(self.time, self.canvas.delete(stimulus))

          self.update.clear()

    def _give_sign(self, shape, trigger_size, time):
        """
        This function is used just internally to set a trigger event.
        """
        self.shape = shape
        self.size = trigger_size
        self.time = time
        self.update.set()


class Sign_c(Thread):
    """This class is used just internally to start a new thread for giving signs c++ based."""
    def __init__(self, shape):
        Thread.__init__(self)
        self.shape = shape
        self.shape_toshow = 1
        self.time = 1
        self.sign = Event()

    def run(self):
        while(True):
            self.sign.wait()
            give_sign(self.shape.get(self.shape_toshow, 1), self.time)
            self.sign.clear()



