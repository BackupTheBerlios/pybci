# This file was automatically generated by SWIG (http://www.swig.org).
# Version 1.3.36
#
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

"""
This module allows online access to eeg data
"""

import _bci_source
import new
new_instancemethod = new.instancemethod
try:
    _swig_property = property
except NameError:
    pass # Python < 2.2 doesn't have 'property'.
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'PySwigObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types



def start_bci(*args):
  """
    start_bci (int argc, char** argv, unsigned int mode, unsigned int channum, signed int level)

    Function to start bci. Has to be called in an seperate thread. Parameters are the name of the server
    (if the software is not running on the same computer that is receiving the data; otherwise skip.),
    sign mode (0 or SIGNS_UNAVAILABLE if no signs are to be shown and 1 or SIGNS_AVAILABLE if you want to trigger signs
    (see sign documentation), number of channels to evaluate (channel labels are by default from 1 to <numof_channels>-1,
     eog channel is <numof_channels>; use <change_channellabels> to change that) and speed of returning arrays
    (from -9 (very slow) to 9 (very fast) , with possible exceptions of level
    -10 (slowest level that is possible) and 10 (as fast as possible)).  


    """
  return _bci_source.start_bci(*args)

def end_bci(*args):
  """
    end_bci(void);

    Function to be called to end bci, close the server connection and delete allocated memory. 

    """
  return _bci_source.end_bci(*args)

def get_blocksize(*args):
  """
    get_blocksize(void);

    May be used to check the number of samples in one data block, that is sent (to the server).

    """
  return _bci_source.get_blocksize(*args)

def get_numof_samples(*args):
  """
    get_numof_samples(void);

    May be used to check the number of samples in one data array, storing the data.

    """
  return _bci_source.get_numof_samples(*args)

def check_readingstate(*args):
  """
    check_readingstate(void);

    May be used to check if the Recorder is running (just in that case data may be available to be returned) externally.
     Returns 1 or True if it is running, otherwise 0 or False.
     

    """
  return _bci_source.check_readingstate(*args)

def demanding_access(*args):
  """
    demanding_access(void);

    Returns if data is available for returning, triggered by <read_data>, or
    	   if the Recorder has been stopped while asking for data to give some kind of 'sign'.
    	   

    """
  return _bci_source.demanding_access(*args)

def set_security_mode(*args):
  """
    set_security_mode(bool mode);

    Sets the security mode. If true (false is default), a warning is raised if the number of returned blocks is not equal
     to the read (that is, 'incoming') ones. That may be useful if you want to be sure not to miss samples/data blocks
      or to avoid reading blocks twice. 
      

    """
  return _bci_source.set_security_mode(*args)

def reset_security_mode(*args):
  """
    reset_security_mode(void);

    Resets the counters for read and returned data arrays. This may be useful if you do not want to have all the data be returned since
     the Recorder is running and still be sure not to miss data while requesting it. 
     

    """
  return _bci_source.reset_security_mode(*args)

def set_returning_speed(*args):
  """
    set_returning_speed(signed int level);

    Resets the returning speed of data arrays. Speed levels from -9 (very slow) to 9 (very fast) are possible, with possible
     exceptions of level -10 (slowest level that is possible) and 10 (as fast as possible).
    If you are dependent on receiving the data as fast as possible, you should choose a high level.
    Thanks to the improved CPU architecture the speed is set on a pretty fast value by default anyway,
     so increasing the speed is usually not necessary.
     

    """
  return _bci_source.set_returning_speed(*args)

def return_samples(*args):
  """
    return_samples(unsigned long n, unsigned int channel, bool lastone);

    Function to be called to get available data. Returns sample <n> from channel number (from 1 to <numof_channels> is possible) <channel>.
     Usually <demanding_access> should be called first to make sure that there is data available for returning, otherwise it is
      possible that you will get the same samples twice. The maximum value of <n> is <numof_samples> 
      consider calling <eval_numof_samples> if unsure...).
      <lastone> has to be set to True if this is the last channel that is accessed in the respective data array. 


    """
  return _bci_source.return_samples(*args)

def give_sign(*args):
  """
    give_sign(int form, int col, unsigned long time, double size, unsigned int texture);

    Triggers a sign on a white background for the time <time> (milliseconds) in the shape <form> 
    (1 or TRIANGLES for triangles, 2 or QUADS for quads, 3 or FONT for text, 4 or BMP for bitmaps).
     When the sign is shown a trigger ('5') is sended via the parallel port (using <outport>).


    """
  return _bci_source.give_sign(*args)

def set_background_color(*args):
  """
    set_background_color(int color);

    Sets the background color.


    """
  return _bci_source.set_background_color(*args)


