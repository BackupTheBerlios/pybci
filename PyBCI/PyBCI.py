

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





import numpy as N
from PyBCI.bci_source import *
from threading import *
import pickle
from scipy.io import savemat
import ConfigParser
import sys
import time
from string import atoi

data_restart = False

class Connect(Thread):
    """This class is used just internally to start the BCI in a seperate thread."""
    def __init__(self, numof_channels, mode, server):
        Thread.__init__(self)

        if server == 'localhost':
            start_bci(1, ['localhost'], mode, numof_channels)
        else:
            start_bci(2, ['newserver', server], mode, numof_channels)
                
        
class Sign(Thread):
    """This class is used just internally to start a new thread for giving signs."""
    def __init__(self, shape):
        Thread.__init__(self)
        self.shape = shape

    def trigger_sign(self, time):
        give_sign(self.shape, time)

        

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
       
    """
    def __init__(self, config_file):
        
        # Create configuration parser with defaults
        config = ConfigParser.RawConfigParser({'server': 'localhost', 'shape': 'None', 'security_mode': False, 'saving_mode': False, 'file': 'Nofile', 'format': 'binary', 'resolution': '0.1'})
        if config.read(config_file):

            self.numof_channels = atoi(config.get('technics', 'numof_channels'))
            self.sample_rate = atoi(config.get('technics', 'sample_rate'))
            self.server = config.get('technics', 'server')
            self.resolution = float(config.get('technics', 'resolution'))
            self.mode = config.get('visualization', 'mode')
            self.shape = config.get('visualization', 'shape')
            self.saving_mode = config.getboolean('data', 'saving_mode')
            self.data_file = config.get('data', 'file')
            self.format = config.get('data', 'format')
            self.security_mode = config.getboolean('security', 'security_mode')
            
            self.data_restart = False
            
        else:
            print 'Error: Configuration file cannot be found.'
            sys.exit(-1)

        if self.security_mode == True:
            set_security_mode(True)
        elif self.security_mode != False:
            print 'Warning: This security mode is not available. Set to "False" by default.'
            set_security_mode(False)

        if self.saving_mode == True:
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
            self.mode = 1
            if self.shape == 'None':
                print 'Warning: Mode is set to "signs_enabled" but no sign shape has been specified. Set to "triangles" by default.' 
                self.shape = 1
            elif self.shape == 'triangles':
                self.shape = 1
            elif self.shape == 'quads':
                self.shape = 2
            else:
                print 'Warning: This shape is not available. Set to "triangles" by default.'
                self.shape = 1
        elif self.mode == 'signs_disabled':
            self.mode = 2
        else:
            print 'Warning: This mode is not available. Switched to "signs_disabled".'
            self.mode = 2

        connect = Connect(self.numof_channels, self.mode, self.server)   # start BCI in a seperate thread
        self.sign = Sign(self.shape)   # start signing mode in a seperate thread
        self.blocksize = get_blocksize()    # number of samples in one data block sent by Brain Recorder
        self.numof_samples = get_numof_samples()    # number of samples in one data storing array

    def trigger_sign(self, time):
        """If <mode> is 'signs_enabled' you may use this function to give a sign in a seperate
        window with the shape <shape>. It is shown for <time> milliseconds."""
        self.sign.trigger_sign(time)

    def reset_security_mode(self):
        """Resets the counters for read and returned data arrays. This may be useful if you do not want to have all the data be returned since the Recorder is running and still be sure
            not to miss data while requesting it. Mainly used internally."""
        reset_security_mode()

    def set_security_mode(self, mode):
        """Sets the security mode. If true (false is default), a warning is raised if the number
           of returned blocks is not equal to the read ones.
           That may be useful if you want to be sure not to miss samples/data blocks or to avoid
           reading blocks twice. See also class documentation and example.py"""
        set_security_mode(mode)

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

    def change_channellabels(self, channel, label, restart):
        """Herewith you are able to relable the channels you want to get data from.
           <channel> is the number of the <numof_channel> channels you have declared creating
            the BCI class, <label> is the matching label for this <channel>.
            
            Be careful: The channel labels you specify here have to match with
            the !number #! (not the label or the physical channel number)
            that is declared in the Brain Recorder Software.
            
            Because of this, an alternative for relabeling channels is just to put
            the channels you want to 'read' at the beginning of the Recorder Software
            one below the other. By default the labels are [1, 2, 3 ... <numof_channels]
            (Note: 'eog' is remarked when relabeling just for future eog evaluations...).
            
            You may set <restart> to True if you want to restart.
            This is not necessary if you want to label more than one channel. """
        change_channellabels(channel, label, restart)

    def __get_sample(self, n, channel):
        """This function is only used internally for security reasons, so the minumum number
           of samples to read at once is one block, using <get_datablock>.
           
           This function returns just the sample <n> for channel <channel> in the
           current data block."""
        sample = return_samples(n, channel)
        return sample
    
    def get_datablock(self):
        """
            If you want to get just the current datablock (that is, one array the data is
            stored in, usually just a few samples) you may use this function.
            
            The data is returned in a numpy array [channels][samples].
        """
        data = N.zeros((self.numof_channels, self.numof_samples))
        # returns with 1 if new data available for returning and with 2 if the Recorder has been stopped
        state = demanding_access() 

        if state == 1:
            for channel in range(self.numof_channels):    # for each channel...
                for sample in range(self.numof_samples):   # ...get each sample in the current data array...
                    data[channel][sample] = self.__get_sample(sample+1, channel+1)
            return data*self.resolution   # ...and keep in mind the data resolution...
        
        elif self.supervision_mode:
            self.data_restart = True
            if self.saving_mode == True:
                # print five zeros to sign stopped Recorder in data file
                    self.save_data(self.data_file, self.format, N.zeros(5)) 
            return
        
        elif self.supervision_mode == False:
            self.get_datablock()  # just restart waiting for data if restart is not necessary


    def get_data(self, time, security_mode = True, supervision_mode = True):
        """
            'Main' function to get data. For the specified <time> (in seconds) data
            is stored and then returned in a numpy array [channels][samples].
            
            If <security_mode> is set to true (this is default), a warning is raised if
            the number of returned blocks is not equal to the read ones.
            That may be useful if you want to be sure not to miss samples/data blocks
            or to avoid reading blocks twice (see also function documentation).
            
            If <supervision_mode> is true (this is default), the Python data array is deleted
            and the data reading process is restarted if the Brain Recorder has been stopped
            to avoid 'old' data in the requested data array. Five zeros are written in the data file
            (if saving_mode is switched on) to 'sign' this stopping.
            
        """
        self.time = time
        self.supervision_mode = supervision_mode
        samples_per_second = self.sample_rate   
        samples_requested = samples_per_second*time
        blocks_requested = samples_requested/self.numof_samples    # number of blocks (completely 'filled' data arrays) available in the specified time

        data = N.zeros((self.numof_channels, self.numof_samples*blocks_requested))

        if self.security_mode == True:
            set_security_mode(True)
            reset_security_mode()    # otherwise you'll probably get a warning message, because Brain Recorder has been started before requesting data
        elif self.security_mode == False:
            set_security_mode(False)
        else:
            print 'Warning: This security mode is not available. Set to "True" by default.'
            set_security_mode(True)
            reset_security_mode()

        # maybe the following loops are worth improving?

        if supervision_mode:    # two cases to avoid even more loops...
            while self.data_restart == False:
                print 'Getting data for', time, 'seconds...' 
                for block in range(blocks_requested):
                    if self.data_restart == False:
                        # get the current datablock for <blocks_requested> times and store it in a new numpy array    
                        data[:,block*self.numof_samples:(block+1)*self.numof_samples] = self.get_datablock()

                if self.data_restart == False:   # just if everything was all right :)
                    print 'Data collected.'

                    if self.saving_mode == True:
                        self.save_data(self.data_file, self.format, data)
                    return data

            # this is executed just if data_restart has been set to true by the supervising thread
            print 'Recorder stopped - Data collection is reseted. Please restart.'
            self.data_restart = False
            self.get_data(self.time, security_mode) # restart the current function

        elif supervision_mode == False:
            print 'Getting data for', time, 'seconds...' 
            for block in range(blocks_requested):   
                data[:,block*self.numof_samples:(block+1)*self.numof_samples] = self.get_datablock()

            print 'Data collected.'

            if self.saving_mode == True:
                self.save_data(self.data_file, self.format, data)

            return data


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
            N.save(data_file, data.T)
        elif format == 'pickle':
            pickle.dump(data.T, data_file)
        elif format == 'plain':
            N.savetxt(data_file, data.T, fmt='%.3f', delimiter='    ')
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
        
        
        

    
        
  

