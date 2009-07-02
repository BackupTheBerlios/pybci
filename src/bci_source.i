

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



%define BCI_SOURCE_DOC
"This module allows online access to eeg data"
%enddef

%module(docstring = BCI_SOURCE_DOC) bci_source    // module is named

%{
#define SWIG_FILE_WITH_INIT                 // declarations
#include "bci_source.h"                          // and headers
%}


%typemap(in) char ** {
  /* Check if is a list */
  if (PyList_Check($input)) {
    int size = PyList_Size($input);
    int i = 0;
    $1 = (char **) malloc((size+1)*sizeof(char *));
    for (i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyString_Check(o))
	$1[i] = PyString_AsString(PyList_GetItem($input,i));
      else {
	PyErr_SetString(PyExc_TypeError,"list must contain strings");
	free($1);
	return NULL;
      }
    }
    $1[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError,"not a list");
    return NULL;
  }
}

// This cleans up the char ** array we malloc'd before the function call
%typemap(freearg) char ** {
  free((char *) $1);
}


%typemap(in) unsigned int * 
{
  /* Check if is a list */
  if (PyList_Check($input)) 
  {
    int size = PyList_Size($input);
    for (int i = 0; i < size; i++) 
    {
      PyObject *o = PyList_GetItem($input,i);
    }
  } 
	else 
	{
    PyErr_SetString(PyExc_TypeError,"not a list");
    return NULL;
  }
}



%feature("autodoc","start_bci (int argc, char** argv, unsigned int mode, unsigned int channum, signed int level)

Function to start bci. Has to be called in an seperate thread. Parameters are the name of the server
(if the software is not running on the same computer that is receiving the data; otherwise skip.),
sign mode (0 or SIGNS_UNAVAILABLE if no signs are to be shown and 1 or SIGNS_AVAILABLE if you want to trigger signs
(see sign documentation), number of channels to evaluate (channel labels are by default from 1 to <numof_channels>-1,
 eog channel is <numof_channels>; use <change_channellabels> to change that) and speed of returning arrays
(from -9 (very slow) to 9 (very fast) , with possible exceptions of level
-10 (slowest level that is possible) and 10 (as fast as possible)).  

") start_bci (int argc, char** argv, unsigned int mode, unsigned int channum, signed int level);
void start_bci (int argc, char** argv, unsigned int mode, unsigned int channum, signed int level);


%feature("autodoc","end_bci(void);

Function to be called to end bci, close the server connection and delete allocated memory. 
") end_bci(void);
void end_bci(void);


%feature("autodoc","get_blocksize(void);

May be used to check the number of samples in one data block, that is sent (to the server).
") get_blocksize(void);
unsigned int get_blocksize(void);


%feature("autodoc","get_numof_samples(void);

May be used to check the number of samples in one data array, storing the data.
") get_numof_samples(void);
unsigned long get_numof_samples(void);


%feature("autodoc","check_readingstate(void);

May be used to check if the Recorder is running (just in that case data may be available to be returned) externally.
 Returns 1 or True if it is running, otherwise 0 or False.
 
") check_readingstate(void);
char check_readingstate(void);



%feature("autodoc","demanding_access(void);

Returns if data is available for returning, triggered by <read_data>, or
	   if the Recorder has been stopped while asking for data to give some kind of 'sign'.
	   
") demanding_access(void);
int demanding_access(void);



%feature("autodoc","set_security_mode(bool mode);

Sets the security mode. If true (false is default), a warning is raised if the number of returned blocks is not equal
 to the read (that is, 'incoming') ones. That may be useful if you want to be sure not to miss samples/data blocks
  or to avoid reading blocks twice. 
  
")set_security_mode (bool mode);
void set_security_mode (bool mode);


%feature("autodoc","reset_security_mode(void);

Resets the counters for read and returned data arrays. This may be useful if you do not want to have all the data be returned since
 the Recorder is running and still be sure not to miss data while requesting it. 
 
")reset_security_mode(void);
void reset_security_mode(void);



%feature("autodoc","set_returning_speed(signed int level);

Resets the returning speed of data arrays. Speed levels from -9 (very slow) to 9 (very fast) are possible, with possible
 exceptions of level -10 (slowest level that is possible) and 10 (as fast as possible).
If you are dependent on receiving the data as fast as possible, you should choose a high level.
Thanks to the improved CPU architecture the speed is set on a pretty fast value by default anyway,
 so increasing the speed is usually not necessary.
 
")set_returning_speed(signed int level);
void set_returning_speed(signed int level);


%feature("autodoc","return_samples(unsigned long n, unsigned int channel);

Function to be called to get available data. Returns sample <n> from channel number (from 1 to <numof_channels> is possible) <channel>.
 Usually <demanding_access> should be calles first to make sure that there is data available for returning, otherwise it is
  possible that you will get the same samples twice. The maximum value of <n> is <numof_samples> 
  consider calling <eval_numof_samples> if unsure...). 

") return_samples(unsigned long n, unsigned int channel);
short return_samples(unsigned long n, unsigned int channel);



%feature("autodoc", "give_sign(int form, unsigned long time);

Triggers a sign on a white background for the time <time> (milliseconds) in the shape <form> (1 or TRIANGLES for triangles,
 2 or QUADS for quads). When the sign is shown a trigger ('5') is sended via the parallel port (using <outport>).

") give_sign(int form, unsigned long time);
void give_sign(int form, unsigned long time);


%feature("autodoc", "change_channellabels(unsigned int channel, unsigned int label, bool restart);

May be used to specify the channels you want to get data from. The channel labels you specify here have to match with the
 !number #! (not the label or the physical channel number) that is declared in the Brain Recorder Software.
  Arguments are the <number> of the channel (in your 'local channelarray', that is, the number you have to declare in <get_sample> later on),
   the matching <label> and <restart> to declare if you want to restart (not necessary if you want to label more than one channel).
   
") change_channellabels(unsigned int channel, unsigned int label, bool restart);
void change_channellabels(unsigned int channel, unsigned int label, bool restart);


%feature("autodoc", "set_trigger_size(double size);

Sets the size of the trigger that is shown by :cfunc:`give_sign`. The range from 1 to 10 is possible.
") set_trigger_size(double size);
void set_trigger_size(double size);
