

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
import numpy as N
import matplotlib
import ConfigParser
from optparse import OptionParser
from numpy.fft import fft
from string import atoi
from PyBCI import *
matplotlib.use('WXAgg') # do this before importing pylab

import matplotlib.pyplot as P

"""
This is a tool for evaluating the current levels of certain
EEG frequency ranges.
For usage information, type 'freqticker.py -h'.
"""

parser = OptionParser(usage= ' freqticker [options] ')

parser.add_option('--dt', '--updating_time',
                  default = 0.1, type='float',
                  help = 'Period of time (in seconds) after that the frequency\
                          ticker is updated. Default is 100ms.')
                          
opts, args = parser.parse_args()


config = ConfigParser.RawConfigParser()
config.read('BCI.cfg')
sample_rate = atoi(config.get('technics', 'sample_rate'))
numof_channels = atoi(config.get('technics', 'numof_channels'))
# take next power of two of the sample rate as fft length:
# more than a freq resolution of 1 Hz should not be necessary,
# assuming sample rate 500 Hz
length_fft = pow(2,N.ceil(N.log2(sample_rate)))

ticktime = opts.dt  # default is 0.1
numof_samples = int(round(float(sample_rate*ticktime)))

fig = P.figure()

# most of the following could be done much nicer in a loop, I know... :)
axtheta = fig.add_subplot(111)
axalpha = fig.add_subplot(111)
axbeta = fig.add_subplot(111)
axgamma = fig.add_subplot(111)

alpha = [0]*50 # plot the 50 last values
beta = [0]*50
theta = [0]*50
gamma = [0]*50
data = N.zeros((numof_channels, length_fft))

linealpha, = axalpha.plot(alpha)
linebeta, = axbeta.plot(beta)
linetheta, = axtheta.plot(theta)
linegamma, = axgamma.plot(gamma)

P.figlegend((linealpha,linebeta,linetheta,linegamma), ('Alpha', 'Beta', 'Theta', 'Gamma'), 'upper right')
P.title('Frequency ticker')

P.ylim(0,10)
P.ylabel('uV')
P.xlabel('tick unit')

bci = BCI('BCI.cfg')
time.sleep(3)

def update_timer(event):
    new_data = bci.get_data(ticktime)
    # this is a moving fft, each time with 'numof_samples' new data samples
    data[:,0:length_fft-numof_samples] = data[:,numof_samples:length_fft]
    data[:,length_fft-numof_samples:length_fft] = new_data
    spec = N.abs(fft(data, n=int(length_fft)))
    del alpha[0]
    del beta[0]
    del theta[0]
    del gamma[0]
    # the rounding stuff between [] is to calculate the position of a respective frequency band within the fft array
    alpha.append(N.mean(N.mean(spec,axis=0)[int(round(8*(length_fft/2-1)/float(sample_rate/2))):int(round(13*(length_fft/2-1)/float(sample_rate/2)))])/float(length_fft))
    beta.append(N.mean(N.mean(spec,axis=0)[int(round(13*(length_fft/2-1)/float(sample_rate/2))):int(round(31*(length_fft/2-1)/float(sample_rate/2)))])/float(length_fft))
    theta.append(N.mean(N.mean(spec,axis=0)[int(round(3*(length_fft/2-1)/float(sample_rate/2))):int(round(9*(length_fft/2-1)/float(sample_rate/2)))])/float(length_fft))
    gamma.append(N.mean(N.mean(spec,axis=0)[int(round(30*(length_fft/2-1)/float(sample_rate/2))):int(round(71*(length_fft/2-1)/float(sample_rate/2)))])/float(length_fft))

    linealpha.set_ydata(alpha)
    linebeta.set_ydata(beta)
    linetheta.set_ydata(theta)
    linegamma.set_ydata(gamma)
    fig.canvas.draw()                 # redraw the canvas


import wx
id = wx.NewId()
actor = fig.canvas.manager.frame
timer = wx.Timer(actor, id=id)
timer.Start(100) # update every 100ms
wx.EVT_TIMER(actor, id, update_timer)

P.show()
