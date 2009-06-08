# This file was automatically generated by SWIG (http://www.swig.org).
# Version 1.3.39
#
# Do not make changes to this file unless you know what you are doing--modify
# the SWIG interface file instead.
# This file is compatible with both classic and new-style classes.

"""
This module allows online access to eeg data
"""

from sys import version_info
if version_info >= (2,6,0):
    def swig_import_helper():
        from os.path import dirname
        import imp
        fp = None
        try:
            fp, pathname, description = imp.find_module('_bci_source', [dirname(__file__)])
        except ImportError:
            import _bci_source
            return _bci_source
        if fp is not None:
            try:
                _mod = imp.load_module('_bci_source', fp, pathname, description)
            finally:
                fp.close()
                return _mod
    _bci_source = swig_import_helper()
    del swig_import_helper
else:
    import _bci_source
del version_info
try:
    _swig_property = property
except NameError:
    pass # Python < 2.2 doesn't have 'property'.
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'SwigPyObject':
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
    raise AttributeError(name)

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

try:
    _object = object
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0



def start_bci(*args):
  """
    start_bci (int argc, char** argv, unsigned int mode, unsigned int channum)

    Function to start bci. Has to be called in an seperate thread. Parameters are the name of the server
    (if the software is not running on the same computer that is receiving the data; otherwise skip.),
    sign mode (0 or SIGNS_UNAVAILABLE if no signs are to be shown and 1 or SIGNS_AVAILABLE if you want to trigger signs
    (see sign documentation) and number of channels to evaluate (channel labels are by default from 1 to <numof_channels>-1,
     eog channel is <numof_channels>; use <change_channellabels> to change that). 


    """
  return _bci_source.start_bci(*args)

def end_bci():
  """
    end_bci(void);

    Function to be called to end bci, close the server connection and delete allocated memory. 

    """
  return _bci_source.end_bci()

def get_blocksize():
  """
    get_blocksize(void);

    May be used to check the number of samples in one data block, that is sent (to the server).

    """
  return _bci_source.get_blocksize()

def get_numof_samples():
  """
    get_numof_samples(void);

    May be used to check the number of samples in one data array, storing the data.

    """
  return _bci_source.get_numof_samples()

def check_readingstate():
  """
    check_readingstate(void);

    May be used to check if the Recorder is running (just in that case data may be available to be returned) externally.
     Returns 1 or True if it is running, otherwise 0 or False.
     

    """
  return _bci_source.check_readingstate()

def demanding_access():
  """
    demanding_access(void);

    Returns if data is available for returning, triggered by <read_data>, or
    	   if the Recorder has been stopped while asking for data to give some kind of 'sign'.
    	   

    """
  return _bci_source.demanding_access()

def set_security_mode(*args):
  """
    set_security_mode(bool mode);

    Sets the security mode. If true (false is default), a warning is raised if the number of returned blocks is not equal
     to the read (that is, 'incoming') ones. That may be useful if you want to be sure not to miss samples/data blocks
      or to avoid reading blocks twice. 
      

    """
  return _bci_source.set_security_mode(*args)

def reset_security_mode():
  """
    reset_security_mode(void);

    Resets the counters for read and returned data arrays. This may be useful if you do not want to have all the data be returned since
     the Recorder is running and still be sure not to miss data while requesting it. 
     

    """
  return _bci_source.reset_security_mode()

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
    return_samples(unsigned long n, unsigned int channel);

    Function to be called to get available data. Returns sample <n> from channel number (from 1 to <numof_channels> is possible) <channel>.
     Usually <demanding_access> should be calles first to make sure that there is data available for returning, otherwise it is
      possible that you will get the same samples twice. The maximum value of <n> is <numof_samples> 
      consider calling <eval_numof_samples> if unsure...). 


    """
  return _bci_source.return_samples(*args)

def give_sign(*args):
  """
    give_sign(int form, unsigned long time);

    Triggers a sign on a white background for the time <time> (milliseconds) in the shape <form> (1 or TRIANGLES for triangles,
     2 or QUADS for quads). When the sign is shown a trigger ('5') is sended via the parallel port (using <outport>).


    """
  return _bci_source.give_sign(*args)

def change_channellabels(*args):
  """
    change_channellabels(unsigned int channel, unsigned int label, bool restart);

    May be used to specify the channels you want to get data from. The channel labels you specify here have to match with the
     !number #! (not the label or the physical channel number) that is declared in the Brain Recorder Software.
      Arguments are the <number> of the channel (in your 'local channelarray', that is, the number you have to declare in <get_sample> later on),
       the matching <label> and <restart> to declare if you want to restart (not necessary if you want to label more than one channel).
       

    """
  return _bci_source.change_channellabels(*args)


