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





"""This is an example to show a imaginable experimental usage of the PyBCI module.
   Just let it run an see what happens. :-)
   Note that it is possible to type in these commands bit by bit into a Python console
   (f.i., using IPython, http://ipython.scipy.org/).
"""


from PyBCI import BCI     # Everything we need is the BCI class as the 'main' class and...
from PyBCI.tools.ConfigHelper import make_config        # ...the helper to create a configuration file
import time

example_blockfile = open('example_blockfile.txt', 'w')

# Let's make a BCI with...
sample_rate = 500 #...Hz...
resolution = 0.1  # ...and a signal resolution of 0.1 microvolt in our Brain Recorder.


# We want to get data from...
numof_channels = 10 # ...channels...
# with the following positions (number #) within the Brain Recorder...
channels = [1,2,10,12,5,6,7,8,20,21]

# ...with a pretty high speed...
returning_speed = 8

# ...and to be able to show black signs in a separate window with the size 1000*800 pixels...
mode = 'signs_enabled'
color_trigger = 'black'
size_window = (1000,800)
color_bg = 'grey' # ...on a grey background.


# Additionally, we want to write the data into a binary file.

# Thus, we create a configuration file to save our parameters.
make_config('BCI.cfg', sample_rate = sample_rate, numof_channels = numof_channels,
            mode = mode, saving_mode = True,
            data_file = 'example_file.dat',
            format = 'binary', resolution = resolution,
            returning_speed = returning_speed, channels = channels
            color_bg = color_bg, color_trigger = color_trigger,
            size_window = size_window)



# To start the BCI, the only thing we need now is the configuration file, containing our specified parameters.
example_bci = BCI('BCI.cfg')


# We are not quite satisfied with the channel labels,
# because the channels we want to 'read' are on positions [1,2,3,12,5,10,7,8,20,30] in the Brain Recorder Software.
# So we have to relabel the channels on position 3, 6 and 10.

example_bci.change_channellabels(3, 3)
example_bci.change_channellabels(6, 10)
example_bci.change_channellabels(10, 30) 


# We want to let the EEG run a bit before we are interested in the data.
# This may be the case, for example, if you have to save your recordings first.
time.sleep(2)  



# We have noted that our computer is a bit slow and seems to be overstrained getting data so fast.
# Because of that, we are going to reduce the returning speed.
example_bci.set_returning_speed(0)   


# We trigger a sign with a size of 100 pixels for 300 milliseconds before getting the data by means of a square.
example_bci.trigger_sign('quads', 100, 300)   

# At the moment we are just interested in the first (current) data block (made up of <example_bci.numof_samples> 
# samples for each of the <numof_channels> channels).
# Unlike the <get_data>-function, <get_datablock> has no impemented security_mode, so if you want to use it (and it is not
# switched on in the config file) you have to do that manually. In that case, make sure to reset the counters, otherwise
# you'll probably get a warning.

example_bci.set_security_mode(True)
example_bci.reset_security_mode()
example_datablock = example_bci.get_datablock()
example_bci.set_security_mode(False)

# Just to show that this is possible (it may save time if you set <saving_mode> to False and write the data after
# collecting it) we call the saving function from 'externally'.
# In the <get_data>-function it is implemented if you set <saving_mode> to True.

example_bci.save_data(example_blockfile, 'plain', example_datablock)   
example_blockfile.close()     # In this case we have to close the file manually.


# Now let's get a bit more data. Therefore...
# ...we choose a larger trigger and show a sign for 500 ms, this time by a triangle...
example_bci.trigger_sign('triangle', 300, 500)   

# ...and request data for the next 10 seconds, with a <security_mode>, that gives us a warning if the number
# of returned blocks is not equal to the read (that is, 'incoming') ones.
# Also, we are not sure what may happen during these 10 seconds, so we'll switch on the <supervision_mode>
# (although both modes are switched on by default, anyway) to get the data collection restarted for 10 seconds
# if the Brain Recorder is stopped while collecting the data.
# Please note that the order of the channels in the data array is the one the channels are listed
# in <channels>.

example_data = example_bci.get_data(10, security_mode = True, supervision_mode = True)


# For now we are done. To finish and be sure that the allocated memory is cleared up savely,
# we call the ending function. 
example_bci.end_bci()   




