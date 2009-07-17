

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



import numpy
from bci_source import *
from sign import *
from threading import *
import pickle
from scipy.io import savemat
import ConfigParser
import sys
import time
from string import atoi

data_restart = False

class Connect(Thread):
    """This class is used just internally to start the BCI in a separate thread."""
    def __init__(self, numof_channels, mode, server, level):
        Thread.__init__(self)

        if server == 'localhost':
            start_bci(1, ['localhost'], mode, numof_channels, level)
        else:
            start_bci(2, ['newserver', server], mode, numof_channels, level)
                

    
class BCI (object):
    """
       This is the main BCI class. The only thing you need to start the BCI, is a configuration file.

       In is recommended to use the function <make_config> from the module BCI.ConfigHelper to create this
       file automatically.
       See its docstring or the BCI documentation to get an idea how to use it exactly.
       If you keep the default configuration file structure, its possible to create it manually. See BCI documentation
       to see how to do it. There is also a BCI_config_templ.cfg as an example.
       
       To finally get the data you may use either <get_datablock> (to get just the current block)
       or <get_data> (see its documentation). In any case, the data is returned in a
       numpy array [channels][samples].
       The order of the channels is the same as you specify in the configuration file (1 to <numof_channels by
       default).
       
    """
    def __init__(self, config_file):
        
        # Create configuration parser with defaults
        config = ConfigParser.RawConfigParser({'server': 'localhost', 'security_mode': False, 'saving_mode': False,
                                               'file': 'Nofile', 'format': 'binary', 'resolution': '0.1', 'speed': '8',
                                               'color_bg': 'grey', 'color_trigger': 'black', 'size_window': '(1000,800)'})
        if config.read(config_file):

            self.numof_channels = atoi(config.get('technics', 'numof_channels'))
            self.sample_rate = atoi(config.get('technics', 'sample_rate'))
            self.server = config.get('technics', 'server')
            self.resolution = float(config.get('technics', 'resolution'))
            self.returning_speed = atoi(config.get('technics', 'speed'))
            self.mode = config.get('visualization', 'mode')
            self.color_bg = config.get('visualization', 'color_bg')
            self.color_trigger = config.get('visualization', 'color_trigger')
            self.size_window = eval(config.get('visualization', 'size_window'))

            if config.has_option('technics', 'channels'):
                self.channels = list(eval(config.get('technics', 'channels')))
                if len(self.channels) != self.numof_channels:
                    print 'Error: Some channels of the', self.numof_channels, 'channels are not labeled.'
                    sys.exit(-1)
            else:
                self.channels = []
                for channel in range(self.numof_channels):  # channel default
                    self.channels.append(channel+1)

            if config.has_section('data'):
                self.saving_mode = config.getboolean('data', 'saving_mode')
                self.data_file = config.get('data', 'file')
                self.format = config.get('data', 'format')
                
            else:
                self.saving_mode = config._defaults['saving_mode']
                self.data_file = config._defaults['file']
                self.format = config._defaults['format']

            if config.has_section('security'):
                self.security_mode = config.getboolean('security', 'security_mode')
            else:
                self.security_mode = config._defaults['security_mode']
                
            print 'Configuration file', config_file, 'read.'
            print 'Sample rate:', self.sample_rate, 'Hz.'
            print 'Data is read from', self.numof_channels, 'channels.'
            print 'Channel labels are:', self.channels
            print 'Returning speed is set on level', self.returning_speed, '.'
            print 'Brain Recorder resolution is', self.resolution, 'microvolt.'
            print
            
            self.data_restart = False
            
        else:
            print 'Error: Configuration file cannot be found.'
            sys.exit(-1)

        if self.security_mode == True:
            set_security_mode(True)
            print 'Security mode switched on.'
        elif self.security_mode != False:
            print 'Warning: This security mode is not available. Set to "False" by default.'
            set_security_mode(False)

        if self.saving_mode == True:
            print 'Saving mode switched on. Data is saved in file', self.data_file, '.'
            if self.data_file == 'Nofile':
                print 'Error: Saving mode is activated, but no data file is specified.'
            else:
                self.data_file = open(self.data_file, 'w')
                if self.format not in ['binary', 'pickle', 'plain', 'mat']:
                    'Error: This data format is not available. Changed to binary by default.'
                    self.format = 'binary'
                    
        elif self.saving_mode != False:
            print 'Warning: This saving mode is not available. Set to "False" by default.'
            self.saving_mode = False

        if self.mode == 'signs_enabled':
            print 'Sign mode enabled.'
            self.mode = 3
            self.shape = {1:1, 'triangle':1, 2:2, 'square':2, 3:3, 'text':3}

            self.sign = Sign(self.size_window[0], self.size_window[1], self.color_bg, self.color_trigger)
            self.sign.start()            
        
        elif self.mode == 'signs_enabled_c':
            print 'C++ sign mode enabled. Trigger size is', self.trigger_size,'.'
            self.mode = 1
            self.shape = {1:1, 'triangle':1, 2:2, 'square':2}
            set_trigger_size(5)

            self.sign = Sign_c(self.shape)   # start signing mode in a separate thread
            self.sign.start()
        
        elif self.mode == 'signs_disabled':
            print 'Sign mode disabled.'
            self.mode = 2
        else:
            print 'Warning: This mode is not available. Switched to "signs_disabled".'
            self.mode = 2

        connect = Connect(self.numof_channels, self.mode, self.server, self.returning_speed)   # start BCI in a separate thread

        self.blocksize = get_blocksize()    # number of samples in one data block sent by Brain Recorder
        self.numof_samples = get_numof_samples()    # number of samples in one data storing array

    def trigger_sign(self, shape, trigger_size, time, text = 'NoText'):
        """
        You may use this function to give a sign in a seperate
        window with the shape <shape>. It is shown for <time> milliseconds.

        Possible values for <shape> are
        1 or 'triangle' for a triangular shape   or
        2 or 'square' for a quadratic shape,     or
        3 or 'text' for text that you can specify as the argument <text>
        
        with 'triangle' as a default if the *shape* you specify is invalid.

        If <mode> is 'signs_enabled', the <trigger_size> is specified in
        pixels. If it is 'signs_enables_c', you just have to declare any
        arbitrary number - in this <mode> you have to change the trigger
        size by calling <set_trigger_size>.
        """
        if self.mode == 3:
            self.sign._give_sign(self.shape[shape], trigger_size, time, text)
            
        elif self.mode == 1:
            self.sign.shape_toshow = shape
            self.sign.time = time
            self.sign.sign.set()
                   

    def reset_security_mode(self):
        """Resets the counters for read and returned data arrays. This may be useful if you do
           not want to have all the data be returned
           since the Recorder is running and still be sure
           not to miss data while requesting it. Mainly used internally."""
        reset_security_mode()

    def set_security_mode(self, mode):
        """Sets the security mode. If true (false is default), a warning is raised if the number
           of returned blocks is not equal to the read ones.
           That may be useful if you want to be sure not to miss samples/data blocks or to avoid
           reading blocks twice. See also class documentation and usage.py"""
        set_security_mode(mode)

    def set_trigger_size(self, size):
        """Sets the size of a shown trigger - values in the range between 1 and 10 (decimal) are
            possible."""
        set_trigger_size(size)
       

    def set_returning_speed(self, level):
        """Resets the returning speed of data arrays. The most likely reason you want to use
           this function is if there are speed problems (overflow, overstrained CPU...),
            because data is available faster than you are able to get returned.
            
            Speed <level>s from -9 (very slow) to 9 (very fast) are possible, with possible
            exceptions of <level> -10 (slowest <level> that is possible)
            and 10 (as fast as possible).
            
            If you are dependent on receiving the data as fast as possible, you should choose
            a high level.
            Thanks to the improved CPU architecture the speed is set on a pretty fast value
            by default anyway, so increasing the speed is usually not necessary."""
        set_returning_speed(level)  
        self.numof_samples = get_numof_samples()

    def change_channellabels(self, channel, label):
        """Herewith you are able to relable the channels you want to get data from.
           <channel> is the number of the <numof_channel> channels you have declared creating
            the BCI class, <label> is the matching label for this <channel>.
            
            Be careful: The channel labels you specify here have to match with
            the !number #! (not the label or the physical channel number)
            that is declared in the Brain Recorder Software.
        """
        if channel <= self.numof_channels and label <= self.numof_channels:
            print 'Relabeling...'
            self.channels[channel-1] = label
            print 'Channel labels are now', self.channels
        else:
            print 'Warning: This is not possible. Either the selected channel or the label\
                  exceeds the number of available channels.'
    
    def __get_sample(self, n, channel, lastone):
        """This function is only used internally for security reasons, so the minumum number
           of samples to read at once is one block, using <get_datablock>.
           
           This function returns just the sample <n> for channel <channel> in the
           current data block.

           <lastone> has to be set to True if this is the last channel that is accessed
           in the respective data array."""
        sample = return_samples(n, channel, lastone)
        return sample
    
    def get_datablock(self):
        """
            If you want to get just the current datablock (that is, one array the data is
            stored in, usually just a few samples) you may use this function.
            
            The data is returned in a numpy array [channels][samples].
        """
        data = numpy.zeros((self.numof_channels, self.numof_samples))
        # returns with 1 if new data available for returning and with 2 if the Recorder has been stopped
        state = demanding_access()

        # this has to be set to True the last channel that is accessed in the respective data array
        lastone = False  
        if state == 1:
            for counter,channel in zip(range(self.numof_channels), self.channels):    # for each channel...
                for sample in range(self.numof_samples):   # ...get each sample in the current data array...
                    if counter == self.numof_channels-1 and sample == self.numof_samples-1:
                        lastone = True  # ...set the counter in the C++ function...
                    data[counter][sample] = self.__get_sample(sample+1, channel, lastone)
                    
            return data*self.resolution   # ...and keep in mind the data resolution...
        
        elif self.supervision_mode:
            self.data_restart = True
            if self.saving_mode == True:
                # print five zeros to sign stopped Recorder in data file
                    self.save_data(self.data_file, self.format, numpy.zeros(5))
        
        elif self.supervision_mode == False:
            self.get_datablock()  # just restart waiting for data if restart is not necessary


    def get_data(self, time, security_mode = True, supervision_mode = True):
        """
            'Main' function to get data. For the specified <time> (in seconds) data
            is stored and then returned in a numpy array [channels][samples].
            The order of the channels is the same as you specify in the configuration
            file (1 to <numof_channels> by default).
            
            If <security_mode> is set to true (this is default), a warning is raised if
            the number of returned blocks is not equal to the read ones.
            That may be useful if you want to be sure not to miss samples/data blocks
            or to avoid reading blocks twice (see also function documentation).
            
            If <supervision_mode> is true (this is default), the Python data array is deleted
            and the data reading process is restarted if the Brain Recorder has been stopped
            to avoid 'old' data in the requested data array. Five zeros are written in the data file
            (if saving_mode is switched on) to 'sign' this stopping.
            
        """
        self.time = float(time)
        self.supervision_mode = supervision_mode
        samples_per_second = self.sample_rate   
        samples_requested = samples_per_second*self.time
        blocks_requested = samples_requested/self.numof_samples    # number of blocks (completely 'filled' data arrays) available in the specified time

        if blocks_requested < 1:
            print 'Warning: There is no data available for returning in this short amount of time.'
            print 'Set to the least value (%1.4f) by default.' % (float(self.numof_samples)/self.sample_rate)       
            samples_requested = self.numof_samples
            blocks_requested = 1
            self.time = float(self.numof_samples)/self.sample_rate

        elif int(blocks_requested) != blocks_requested:
            print 'Warning: The specified amount of time does not fit into the structure of the returned data arrays.'
            blocks_requested = round(blocks_requested, 0)
            samples_requested = blocks_requested*self.numof_samples
            self.time = samples_requested/samples_per_second
            print 'Reading time is changed to %1.4f.' % (self.time) 
            
        blocks_requested = int(blocks_requested)
        
        self.data = numpy.zeros((self.numof_channels, self.numof_samples*blocks_requested))

        if self.security_mode != security_mode:
            if security_mode == True:
                set_security_mode(True)
                reset_security_mode()    # otherwise you'll probably get a warning message, because Brain Recorder has been started before requesting data
            elif security_mode == False:
                set_security_mode(False)
            else:
                print 'Warning: This security mode is not available. Set to "True" by default.'
                set_security_mode(True)
                reset_security_mode()

        # maybe the following loops are worth improving?

        if supervision_mode:    # two cases to avoid even more loops...
            while self.data_restart == False:
                print 'Getting data for', self.time, 'seconds...' 
                for block in range(blocks_requested):
                    if self.data_restart == False:
                        # get the current datablock for <blocks_requested> times and store it in a new numpy array    
                        self.data[:,block*self.numof_samples:(block+1)*self.numof_samples] = self.get_datablock()
                        
                    else: break

                if self.data_restart == False:   # just if everything was all right :)
                    print 'Data collected.'

                    if self.saving_mode == True:
                        self.save_data(self.data_file, self.format, self.data)

                    if self.security_mode != security_mode:
                        set_security_mode(security_mode == False) # reset the recurity mode to the 'origin'
                        if self.security_mode == True:
                            reset_security_mode()
         
                    return self.data

            # this is executed just if data_restart has been set to true
            print 'Recorder stopped - Data collection is reseted. Please restart.'
            self.data_restart = False
            # restart the current function - could be a recursive function, too.
            self.__restart_collection(self.time, security_mode) 

        elif supervision_mode == False:
            print 'Getting data for', self.time, 'seconds...' 
            for block in range(blocks_requested):   
                self.data[:,block*self.numof_samples:(block+1)*self.numof_samples] = self.get_datablock()

            print 'Data collected.'

            if self.saving_mode == True:
                self.save_data(self.data_file, self.format, self.data)

            if self.security_mode != security_mode:
                set_security_mode(security_mode == False) 
                if self.security_mode == True:
                    reset_security_mode()

            return self.data

    def __restart_collection(self, time, security_mode = True, supervision_mode = True):
        """This function is used just internally in case that the Recorder has been stopped.
            It is called by <get_data> to reset the data collection with the 'original'
            parameters."""
        self.get_data(time, security_mode, supervision_mode)

    def save_data(self, data_file, format, data):
        """
            This function saves the specified <data> to the <data_file> with format <format>.
            Mainly used class-internally when collecting data, but may be used externally
            (in that case you have to open the file first) also after the reading procedure
            to save valuable time.
            The <data> is saved transposed, so that each column is one channel.
            Possible <format>s are 'plain'(ascii txt), 'pickle', 'binary' and 'mat' (MATLAB-file).
        """
        if format == 'binary':
            numpy.save(data_file, data.T)
        elif format == 'pickle':
            pickle.dump(data.T, data_file)
        elif format == 'plain':
            numpy.savetxt(data_file, data.T, fmt='%.3f', delimiter='    ')
        elif format == 'mat':
            savemat(data_file, {'data':data.T})         
        else:
            print 'Error: This data format is not available.'

    def end_bci(self):
        """You should definitely call this function when choosing to end the BCI.
            The data files and the server connection is closed and allocated memory is deleted."""
        try:
            self.data_file.close()
        except:
            pass
        end_bci()
        
        
        

    
        
  

