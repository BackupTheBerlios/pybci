


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



#define BINOUT "data.dat"


#include "bci_source.h"

ULONG nChannels; /* number of available channels (detected online) */

ULONG points; /* number of samples in one block sent via TCP/IP-Port (detected online) */

short *array_one; /* data arrays */
short *array_two;

char locked_array; /* indicates which array is busy returning data */

/* number of current data block for data array */
unsigned long blocknum_one;
/* number of current data block for data array */
unsigned long blocknum_two; 
/* number of the last data block (used to detect overflow) */
ULONG nLastBlock; 

/* number of samples in one block (detected online) * number of channels */
unsigned int blocksize; 
/* number of blocks needed for one data array (detected online) */
unsigned long numof_blocks;
/* <numof_blocks> * <blocksize> = number of samples in one data array (detected online) */
unsigned long arraysize;

unsigned long counter_one; /* counter of 'full' data arrays */
unsigned long counter_two; 

unsigned long returned_one;  /* counter of returned data arrays */
unsigned long returned_two;

unsigned int sample_rate;

/* number of channels to read. In the data arrays the data of each available channel (<nChannels>) is stored,
the labels are specified (for sample returning calling <get_sample>) just for the <numof_channels> channels. */
unsigned int numof_channels; 

unsigned long numof_samples; /* number of samples stored in the data array before returning the data. If this number is very low, there could be speed problems (overflow, overstrained CPU...) */

bool reading = false; /* True if Brain Recorder is running. */
bool recorder_stopped = false; /* True if Brain Recorder has been stopped, but the reading process is still not finished (by <end_bci>). */

bool security_mode = false; /* If true, a warning is raised if the number of returned blocks is not equal to the read ones. That may be useful if you want to be sure not to miss samples/data blocks or to avoid reading blocks twice. */

bool reset_counter = false; /* If true, the array counter (important in <security_mode>) are reseted the next time it is possible */


pthread_t read_thr, sign_thr; /* thread handle for the reading and signing threads */
pthread_mutex_t mutex_one = PTHREAD_MUTEX_INITIALIZER; /* mutex for locking data arrays */
pthread_mutex_t mutex_two = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t waiting_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t free_array = PTHREAD_COND_INITIALIZER; /* signal for a data array that is unlocked for returning */
pthread_cond_t stopped = PTHREAD_COND_INITIALIZER; /* signal for stopped recorder */
pthread_cond_t disconnecting = PTHREAD_COND_INITIALIZER; /* signal: bci disconnecting requested, data block returned successfully */

void* start_reading(void* ptr)
{
	get_data();
}

void* start_signing(void* ptr)
{
	sign(ptr);
}

RDA_MessageHeader* pHeader; /* message from TCP/IP-Port */
GUID GUID_RDAHeader  = { 0x4358458e, 0xc996, 0x4c86, { 0xaf, 0x4a, 0x98, 0xbb, 0xf6, 0xc9, 0x14, 0x50 } };
int nResult; /* type of incoming message */

int hSocket;

FILE *stop_remarks; /* in this file the number of returned arrays is written if the user has stopped the Recorder without calling <bci_end> (assuming something occurred unvoluntarily) */

/* These three lines are only compiled if BINOUT is defined */
#ifdef BINOUT
	FILE *binoutfile;
#endif

int d2i(double d) /* just a little rounding function */
{
  return d<0?d-.5:d+.5;
}


tcp::tcp(int argc, char** argv, unsigned int mode, unsigned int numchan, signed int level)
{
	#ifdef BINOUT
		binoutfile = fopen(BINOUT,"wb");
	#endif

	stop_remarks = fopen("stop_remarks.txt", "w");
	
	numof_channels = numchan;

	init_returning_speed(level);

	if (mode == SIGNS_AVAILABLE) 
	{
		THREAD_PARAM thread_param; /* parameters of the sign thread */
		int window_position_x = 100; /* position of sign window, X-value */
		int window_position_y = 100; /* Y-value */
		int window_size_x = 1000; /* size of the sign window33, X-Value */
		int window_size_y = 800; /* Y-value */

		/* struct for sign thread */
		thread_param.window_position_x = window_position_x;
		thread_param.window_position_y = window_position_y;
		thread_param.window_size_x = window_size_x;
		thread_param.window_size_y = window_size_y;
		thread_param.argc = argc;
		thread_param.argv = argv;

		//sign_h = CreateThread(NULL, 0, sign, &thread_param, 0, NULL);
		pthread_create(&sign_thr, NULL, start_signing, &thread_param);
	}

	connect_tcp(argc, argv);

	printf("Please start Brain Recorder.\n");
	prepare();

	pthread_create(&read_thr, NULL, start_reading, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


void prepare()
{
	while(1) 
	{
		pHeader = NULL;
		nResult = GetServerMessage(hSocket, &pHeader);

		if (nResult > 0)	
		{
			RDA_MessageStart* pMsgStart = NULL;
			RDA_MessageData* pMsgData = NULL;
			switch(pHeader->nType)
			{
				case 1:			
					printf("\nPreparing...\n\n");
					pMsgStart = (RDA_MessageStart*)pHeader;
					nChannels = pMsgStart->nChannels; /* detecting number of available channels */
					printf("%i channels found.\n\n", nChannels);

				break;

				case 2:			
					pMsgData = (RDA_MessageData*)pHeader;
					points = pMsgData->nPoints; /* detect number of samples in one data block */ 
					
					if (numof_samples < points)
					{
						printf("Warning: Number of samples in one data array has been set to %i.\n", points);
						numof_samples = points; /* store at least one block */
					}
					
					blocksize = nChannels*points; /* blocksize: one block for each channel */

					numof_blocks = numof_samples/points; /* number of blocks needed for one data array */

					if ((double)numof_blocks != ceil((double)numof_samples/(double)points)) 
					{ /* number of samples in one data array is not a multiple of the number in one data block? take the next possible lower case. */
						numof_samples = numof_samples-numof_samples%points;
						printf("Warning:  Due to Hardware reasons number of samples has been set to %li.\n", numof_samples);
						numof_blocks = numof_samples/points;
					}

					arraysize = numof_blocks*blocksize;
					
					array_one = new short [arraysize];
					array_two = new short [arraysize];

					if (nLastBlock != -1 && pMsgData->nBlock > nLastBlock + 1)
					{
						printf("******* Overflow ******\n Consider reducing returning speed.\n");
					}
					nLastBlock = pMsgData->nBlock;

					printf("Preparation successfully completed.\n\n");					
					return;

				default:
				break;

				if (pHeader) free(pHeader);
			}
		}
		else if (nResult == -2)
		{
			close(hSocket);
			printf("Server has closed the connection.\n");
			while(1);
		}
		else if (nResult < 0)
		{
			close(hSocket);
			printf("An error occured during message receiving.\n");
			while(1);
		}
	}
}



void tcp::connect_tcp(int argc, char** argv)
{															
	sockaddr_in addr;
       
	hSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (hSocket < 0)
	{
		printf("Error: can't create socket!\n");
		while(1);
	}

	bool bTCP_NODELAY = true;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT_NUMBER);
	
	char* host;
        
	if (argc < 2)
	{
	/* Assume localhost if no argument
	   This is not tested yet on a unix system,
           because Brain Vision Recorder is not unix-compatible. */
		host = "127.0.0.1"; // alternative: INADDR_ANY?
	}
	else
	{
		host = argv[1];
	}
	
	hostent* pHost = gethostbyname(host);

	if (pHost == NULL)
	{
		close(hSocket);
		printf("Can't find host '%s'.\n", argv[1]);
		while(1);
	}
	if (pHost->h_addr_list == 0)
	{
		close(hSocket);
		printf("Can't initialize Remote Data Access.\n");
		while(1);
	}

	addr.sin_addr = **((in_addr**)(pHost->h_addr_list));
	if (connect(hSocket, (sockaddr *)&addr, sizeof(addr)) < 0) 
	{
		close(hSocket);
		printf("Can't connect to server.\n");
		while(1);
	}
	else
		printf("\nConnected to server '%s'.\n\n", host);
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


void get_data()
{	
	blocknum_one = 1; /* number of current data block for data array */
	blocknum_two = 1;
	counter_one = 0; /* counter of the full arrays */
	counter_two = 0;
	returned_one = 0; /* counter of the returned arrays */
	returned_two = 0;
	locked_array = 2; /* number of the array that is locked at the moment for returning its data */

	reading = true;

	while (reading == true) /* infinite loop until reading is set false */
	{
		pHeader = NULL;
		nResult = GetServerMessage(hSocket, &pHeader); 

		/* message number indicates start and stop of the recorder or incoming data */

		if (nResult > 0)	/* message is present */
		{
			RDA_MessageStart* pMsgStart = NULL;
			RDA_MessageData* pMsgData = NULL;
			switch(pHeader->nType)
			{
				case 1:			/* start of the recorder */
					printf("Recorder started.\n");
					nLastBlock = -1;
				break;

				case 2:			/* data available */
					pMsgData = (RDA_MessageData*)pHeader;

				/* Simple overflow test: Has the data been returned in time? */
					if (nLastBlock != -1 && pMsgData->nBlock > nLastBlock + 1)
					{
						printf("******* Overflow ******\n Consider reducing returning speed.\n");
					}

					nLastBlock = pMsgData->nBlock;

					read_data(pMsgData); /* read the data of the message */
				
				break;

				case 3:			/* recorder has been stopped */
					printf("Recorder stopped.\n");
					
					recorder_stopped = true;

					if (array_one != NULL)
					{
						delete[] array_one;
						array_one = NULL;
					}
					if (array_two != NULL)
					{
						delete[] array_two;
						array_two = NULL;
					}
					
					if (reading == true) /* stop has not been caused by <end_bci> */
					{
						timeval stop_time;
						gettimeofday(&stop_time, 0);

						array_one = new short [arraysize];
						array_two = new short [arraysize];

						fprintf(stop_remarks, "Recorder stopped at system time %s, %li data arrays returned.\n", stop_time, returned_one+returned_two);
					}

					blocknum_one = 1; /* reset variables */
					blocknum_two = 1;
					counter_one = 0; 
					counter_two = 0;
					locked_array = 2;
					pthread_cond_signal(&stopped);

				break;

				default:
				break;
			
			}
		}
		else if (nResult == -2)
		{
			close(hSocket);
			printf("Server has closed the connection.\n");
			while(1); /* this is to improve */ 
		}

		else if (nResult < 0)
		{
			close(hSocket);
			printf("An error occured during message receiving.\n");
			while(1);
		}
			
	}
	pthread_cond_signal(&disconnecting); /* bci disconnecting requested, data block returned successfully */ 
}


///////////////////////////////////////////////
///////////////////////////////////////////////


void read_data(RDA_MessageData* pMsg)
{	
	unsigned int numof_sample = 0; /* number of sample in current data block */
	switch (locked_array) /* locked for returning data   */
	{
		case 2: /* numof_sample_one and two is the number of sample in the array that is currently used for storing the data  
			 if the more than one data block is needed to fill the array the blocks are stored one after another until the array is 'full'. */
			
			for (unsigned int numof_sample_one = blocksize*(blocknum_one-1); numof_sample_one < blocksize*(blocknum_one-1)+blocksize; numof_sample_one++)
			{
				array_one[numof_sample_one] = pMsg->nData[numof_sample];
				numof_sample++;
							
				if (numof_sample_one == arraysize-1) // if the array is full...
				{
					locked_array = 1; /* ...the other one is locked... */
					blocknum_two = 1; /*...counter of the other array is set to 1 */
					counter_one++; /*... counter of full arrays is updated... */
					pthread_cond_signal(&free_array); /* ...now the data is ready to be returned. */
					
					#ifdef BINOUT
					/* Write binary output to binoutfile */
						fwrite(array_one,sizeof(short),arraysize,binoutfile);
					#endif

					break; /* cancel switch-loop */
				}
			}
			blocknum_one++; /* number of blocks is updated, unless it is full anyway. */
			break;
	
		case 1: /* same procedure if first array is locked. */

			for (unsigned int numof_sample_two = blocksize*(blocknum_two-1); numof_sample_two < blocksize*(blocknum_two-1)+blocksize; numof_sample_two++)
			{
				array_two[numof_sample_two] = pMsg->nData[numof_sample];
				numof_sample++;

				if (numof_sample_two==arraysize-1) 
				{
					locked_array = 2;
					blocknum_one = 1;
					counter_two++;
					pthread_cond_signal(&free_array);

					#ifdef BINOUT
					fwrite(array_two,sizeof(short),arraysize,binoutfile);
					#endif

					break;
				}
			}
			blocknum_two++;
			break;

								
		default:
		break;
	}
}

int demanding_access()
{
	pthread_cond_wait(&free_array, &waiting_mutex);	
	
	if (reset_counter == true) /* counter reset has been requested */ 
	{
		if (locked_array == 2)
		{
			counter_two = 1;
			returned_two = 0;
			counter_one = 0;
			returned_one = 0;
			reset_counter = false;
		}
		else if (locked_array == 1)
		{
			counter_two = 0;
			returned_two = 0;
			counter_one = 1;
			returned_one = 0;
			reset_counter = false;
		}
	}

	if (recorder_stopped == true && reading == true) /* the Recorder has been stopped while demanding data */
	{
		return 2;
	}
	
	else
	{
		return 1;
	}
}


short return_samples(unsigned long n, unsigned int channel, bool lastone) 
{
	if (channel > nChannels)
	{
		printf("Error: Number of channel to evaluate exceeds the number of available channels.\n");
		return 0;
	}

	if (n > numof_samples)
	{
		printf("Error: Number of sample to be returned exceeds the number of samples stored in the data array.\n");
		return 0;
	}

	if (locked_array == 2  && reading == true)
	{
		if (counter_two-1 > returned_two && security_mode == true)
		{
			printf("Warning: Number of read arrays exceeds number of returned ones.\n Consider reducing returning speed.\t%i\t%i\n", counter_two-1, returned_two);
		    returned_two = counter_two-1;
		}

		if (counter_two-1 < returned_two && security_mode == true)
		{
			printf("Warning: Number of read arrays does not match the number of returned ones.\t%i\t%i\n", counter_two-1, returned_two);
			returned_two = counter_two-1;		
		}
		
		if (n == numof_samples && lastone == true)
		{
			returned_two = returned_two+1; /* whole array has been returned */ 
		}
		
		return array_two[((channel-1)+(n-1)*nChannels)];  /* in the data arrays the data of each of the <nChannels> is stored */
	}
 
	else if (locked_array == 1 && reading == true)
	{
		if (counter_one-1 > returned_one && security_mode == true)
		{
			printf("Warning: Number of read arrays exceeds number of returned ones.\n Consider reducing returning speed.\t%i\t%i\n", counter_one-1, returned_one);
			returned_one = counter_one-1;
		}

		if (counter_one-1 < returned_one && security_mode == true)
		{
			printf("Warning: Number of read arrays does not match the number of returned ones.\t%i\t%i\n", counter_one-1, returned_one);
			returned_one = counter_one-1;
		}

		if (n == numof_samples && lastone == true)
		{
			returned_one = returned_one+1; 
		}

		return array_one[((channel-1)+(n-1)*nChannels)];
	} 

	else
	{
		printf("Undefined error while returning data.\n");
		return 0;
	}
}


int GetServerMessage(int socket, RDA_MessageHeader** ppHeader)
/* get message from TCP/IP-Port */
{
	timeval tv; tv.tv_sec = 0; tv.tv_usec = 5000;	// 50 ms.
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(socket, &readfds);
	int nResult = select(socket+1, &readfds, NULL, NULL, &tv);
	if (nResult != 1) return nResult;

	RDA_MessageHeader header;
	char* pData = (char*)&header;
	int nLength = 0;
	bool bFirstRecv = true;
	/* Retrieve header. */
	while(nLength < sizeof(header))
	{
		int nReqLength = sizeof(header) - nLength;
		nResult = recv(socket, (char*)pData, nReqLength, 0);

		/* When select() succeeds and recv() returns 0 the server has closed the connection. */
		if (nResult == 0 && bFirstRecv)	return -2;
		bFirstRecv = false;
		if (nResult < 0) return nResult;
		nLength += nResult;
		pData += nResult;
	}

	/* Check for correct header GUID - this is to improve. */
	if (header.guid.Data1 != GUID_RDAHeader.Data1 || header.guid.Data2 != GUID_RDAHeader.Data2 || header.guid.Data3 != GUID_RDAHeader.Data3 || header.guid.Data4[0] != GUID_RDAHeader.Data4[0] || header.guid.Data4[1] != GUID_RDAHeader.Data4[1] || header.guid.Data4[2] != GUID_RDAHeader.Data4[2] || header.guid.Data4[3] != GUID_RDAHeader.Data4[3] || header.guid.Data4[4] != GUID_RDAHeader.Data4[4])  
	{
		printf("Wrong header GUID");
		return -1;
	}  

	*ppHeader = (RDA_MessageHeader*)malloc(header.nSize);
	if (!*ppHeader)
	{
		printf("Wrong header GUID");
		return -1;
	}

	memcpy(*ppHeader, &header, sizeof(header));
	pData = (char*)*ppHeader + sizeof(header);

	nLength = 0;
	int nDatasize = header.nSize - sizeof(header);

	/* Retrieve rest of block. */
	while(nLength < nDatasize)
	{
		int nReqLength = nDatasize - nLength;
		nResult = recv(socket, (char*)pData, nReqLength, 0);
		if (nResult < 0) return nResult;
		nLength += nResult;
		pData += nResult;
	}
	return 1;
}



////////////////////////////////////////////
////////////////////////////////////////////


int main(int argc, char** argv)
{
	start_bci(argc, argv, 1, 1, 1);
}

void start_bci(int argc, char** argv, unsigned int mode, unsigned int numchan, signed int level)
{
	tcp data(argc, argv, mode, numchan, level); 
}


char check_readingstate()
{
	return recorder_stopped;
}


unsigned int get_blocksize()
{
	return points;
}

unsigned long get_numof_samples()
{
	return numof_samples;
}


void set_security_mode (bool mode)
{
	security_mode = mode;
}


void reset_security_mode()
{
	reset_counter = true;
}

void tcp::init_returning_speed(signed int level)
{
	/* setting speed... */
	if (level == -10)  /* very slow level */
	{
		numof_samples = 300;
	}

	else if (level == 10) /* as fast as its possible */
	{
		numof_samples = points;
	}

	else if (-9 <= level && level <= 9)
	{	
	level = 10*level;   /* now values from -90 to 90 possible */
	numof_samples = 100-level; 
	}

	else
	{
		printf("Error: This speed level is not available. Current number of samples in one data array - %i - is retained.\n", numof_samples);
	}
}


					

void set_returning_speed(signed int level)
{
	printf("\nReset of returning speed requested.\n");

	if (reading == true)
	{
		reading = false;

		pthread_cond_wait(&disconnecting, &waiting_mutex); /* finish reading the current data block */

		if (recorder_stopped != true)  /* not necessary if Recorder already stopped */
		{
			printf("\nPlease stop Brain Recorder.\n");

			pHeader = NULL;
			nResult = GetServerMessage(hSocket, &pHeader); 

			while (pHeader->nType != 3) /* check if Recorder has been stopped */
			{
				GetServerMessage(hSocket, &pHeader);
			}
		}	
	}

	printf("\nResetting...\n");

	/* clearing up... */
	if (array_one != NULL)
	{
		delete[] array_one;
		array_one = NULL;
	}
	if (array_two != NULL)
	{
		delete[] array_two;
		array_two = NULL;
	}

	if (read_thr != NULL)
	{
		pthread_exit(&read_thr);
	}

	if (pHeader) free(pHeader);
		
	/* setting speed... */
	if (level == -10)  /* very slow level */
	{
		numof_samples = 300;
	}

	else if (level == 10) /* as fast as its possible */
	{
		numof_samples = points;
	}

	else if (-9 <= level && level <= 9)
	{	
	level = 10*level;   /* now values from -90 to 90 possible */
	numof_samples = 100-level; 
	}

	else
	{
		printf("Error: This speed level is not available. Current number of samples in one data array - %i - is retained.\n", numof_samples);
	}

	/* ...and restarting. */

	printf("Please restart Brain Recorder.\n");
	prepare();

	pthread_create(&read_thr, NULL, start_reading, NULL);
}



void end_bci()
{
	if (reading == true) /* avoid ending if Recorder is not running yet */
	{
		reading = false;

		pthread_cond_wait(&disconnecting, &waiting_mutex); /* finish reading the current data block */

		if (recorder_stopped != true)  /* not necessary if Recorder already stopped */
		{
			printf("\nPlease stop Brain Recorder.\n\n");

			pHeader = NULL;
			nResult = GetServerMessage(hSocket, &pHeader); 

			while (pHeader->nType != 3) /* check if Recorder has been stopped */
			{
				GetServerMessage(hSocket, &pHeader);
			}
		}

		if (array_one != NULL)
		{
			delete[] array_one;
			array_one = NULL;
		}
		if (array_two != NULL)
		{
			delete[] array_two;
			array_two = NULL;
		}

		if (read_thr != NULL)
		{
			pthread_exit(&read_thr);
		}

		if (sign_thr != NULL)
		{
			pthread_exit(&sign_thr);
		}

		if (pHeader) free(pHeader);

		close(hSocket);
		
		#ifdef BINOUT
			fclose(binoutfile);
		#endif

		/* Clear keyboard queue */
		//if (_kbhit()) while(!getch());	
	}
}


