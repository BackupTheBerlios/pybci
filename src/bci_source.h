

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




////////////////////////////////////////////////////////////////////

/* Access to the TCP/IP-Port, reading data and parallel further evaluating and processing, using various threads. The data is stored in two arrays, one of them is used for storing the data, the other one for returning, alternately. */

////////////////////////////////////////////////////////////////////


#ifndef BCI_SOURCE_H
#define BCI_SOURCE_H

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <conio.h>
#include <io.h>
#include <winsock2.h>
#include <initguid.h>
#include <cmath>
#include <ctime>
#include <string>
#include <algorithm>
#include "RecorderRDA.h"
#include "sign.h"


using namespace std;

#define PORT_NUMBER	51234
#define SIGNS_AVAILABLE 1
#define SIGNS_UNAVAILABLE 0


/*  TCP/IP-Port is accessed to check incoming messages from Brain Vision Recorder software
(this part of the script has been inherited from Brain Vision). 
The number that is returned indicates either start or stop of the Recorder or incoming data. 
The detailed structure of the message is defined in RDARecorder.h. */ 
int GetServerMessage(SOCKET socket, RDA_MessageHeader** ppHeader);

/* block size and number of samples in one block is detected, data arrays with required size are allocated. returns after first data message. */
void prepare(void);

/* receives message types in an infinite loop until reading is set false (<end_bci>).
If a data message is available, <read_data> is called to read the data. */
void get_data(void);

/* Called by <get_data> if data is available. Data is stored in the data arrays until they are filled, then an event
(triggers the waiting function in <demanding_access> is raised to sign data available for returning. */
void read_data(RDA_MessageData* pMsg);

/* Thread for starting the BCI - starts receiving data via <get_data>.*/
DWORD WINAPI reading_thread(LPVOID param);

class tcp
{
	private:
		/* connection to TCP/IP-Port is established, socket is initialized. local host is assumed if no argument is specified (this part of the script
		has been inherited from Brain Vision). Called by class constructor.*/
		void connect_tcp(int argc, char** argv);

		/* Sets the initial speed level value. Called by class constructor. */
	    void init_returning_speed(signed int level);

	public:
		/* Constructor is started by <start_bci>. Starts the connection to the server, preparing, and the reading and signing (if specified) threads. */ 		
		tcp(int argc, char** argv, unsigned int mode, unsigned int channum, signed int level);

		~tcp(void)
		{
		}		
};


	/* May be used to check if the Recorder is running (just in that case data may be available to be returned) externally. 
	Returns 1 or True if it is running, otherwise 0 or False. */
	extern "C" __declspec(dllexport) char check_readingstate(void);

	/* Function to be called externally. Returns either if data is available for returning, triggered by <read_data>, or
	   if the Recorder has been stopped while asking for data to give some kind of 'sign'.*/  
	extern "C" __declspec(dllexport) int demanding_access(void);

	/* Function to be called externally to get available data. Returns sample <n> from channel number (from 1 to <numof_channels> is possible) <channel>.
	Usually <demanding_access> should be calles first to make sure that there is data available for returning, otherwise it is possible that you will
	get the same samples twice. The maximum value of <n> is <numof_samples> (consider calling <eval_numof_samples> if unsure...).
	<lastone> has to be set to True if this is the last channel that is accessed in the respective data array. */ 
	extern "C" __declspec(dllexport) short return_samples(unsigned long n, unsigned int channel, bool lastone);

	/* Function to start bci externally. Has to be called in an seperate thread. Parameters are the name of the server
	(if the software is not running on the same computer that is receiving the data; otherwise skip.), sign mode (0 or SIGNS_UNAVAILABLE
	if no signs are to be shown and 1 or SIGNS_AVAILABLE if you want to trigger signs (see sign documentation), number of channels to evaluate
	(channel labels are by default from 1 to <numof_channels>-1, eog channel is <numof_channels>; use <change_channellabels> to change that)
	and speed of returning arrays (from -9 (very slow) to 9 (very fast) , with possible exceptions of level
	-10 (slowest level that is possible) and 10 (as fast as possible)). */
	extern "C" __declspec(dllexport) void start_bci (int argc, char** argv, unsigned int mode, unsigned int channum, signed int level);

	/* Function to be calles externally to end bci, close the server connection and delete allocated memory. */
	extern "C" __declspec(dllexport) void end_bci(void);

	/* May be used to check the number of samples in one data block, that is sent (to the server). */		
	extern "C" __declspec(dllexport) unsigned int get_blocksize(void); 

	/* May be used to check the number of samples in one data array, storing the data. */		
	extern "C" __declspec(dllexport) unsigned long get_numof_samples(void); 

	/* Sets the security mode from externally. If true, a warning is raised if the number of returned blocks is not equal to the read ones. 
	That may be useful if you want to be sure not to miss samples/data blocks or to avoid reading blocks twice. */
	extern "C" __declspec(dllexport) void set_security_mode (bool mode);

	/* Resets the counters for read and returned data arrays. This may be useful if you do not want to have all the data be returned since the
	Recorder is running and still be sure not to miss data while requesting it. */
	extern "C" __declspec(dllexport) void reset_security_mode(void);

	/* Resets the returning speed of data arrays. Speed levels from -9 (very slow) to 9 (very fast) are possible, with possible exceptions of level
	-10 (slowest level that is possible) and 10 (as fast as possible). If you are dependent on receiving the data as fast as possible,
	you should choose a high level. Thanks to the improved CPU architecture the speed is set on a pretty fast value by default anyway,
	so increasing the speed is usually not necessary. */
	extern "C" __declspec(dllexport) void set_returning_speed(signed int level);


#endif


