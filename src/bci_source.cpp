


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
unsigned int *channels; /* channel labels of the <numof_channels> channels. Has to be the number # in the channel list of the Brain Recorder Software */
unsigned int eog_channel; /* label eog channel */

unsigned long numof_samples; /* number of samples stored in the data array before returning the data. If this number is very low, there could be speed problems (overflow, overstrained CPU...) */

bool reading = false; /* True if Brain Recorder is running. */
bool recorder_stopped = false; /* True if Brain Recorder has been stopped, but the reading process is still not finished (by <end_bci>). */

bool security_mode = false; /* If true, a warning is raised if the number of returned blocks is not equal to the read ones. That may be useful if you want to be sure not to miss samples/data blocks or to avoid reading blocks twice. */

bool reset_counter = false; /* If true, the array counter (important in <security_mode>) are reseted the next time it is possible */

/* handle array for events to sign new data available for returning [0] and a stopped_recorder [1]
event one: new data available for returning
event two: Brain Recorder has been stopped, but the reading process is still not finished (by <end_bci>) */
HANDLE reading_state[2]; 
HANDLE reading_stopped; /* event: bci disconnecting requested, data block returned successfully */ 

HANDLE read_h; /* thread handle for the reading thread */
HANDLE sign_h; /* thread handle for the sign thread */

DWORD waiting; /* data requested - waiting for available data */
DWORD status;

RDA_MessageHeader* pHeader; /* message from TCP/IP-Port */
int nResult; /* type of incoming message */

SOCKET hSocket;

FILE *stop_remarks; /* in this file the number of returned arrays is written if the user has stopped the Recorder without calling <bci_end> (assuming something occurred unvoluntarily) */

/* These three lines are only compiled if BINOUT is defined */
#ifdef BINOUT
	FILE *binoutfile;
#endif

int d2i(double d) /* just a little rounding function */
{
  return d<0?d-.5:d+.5;
}


tcp::tcp(int argc, char** argv, unsigned int mode, unsigned int numchan)
{
	#ifdef BINOUT
		binoutfile = fopen(BINOUT,"wb");
	#endif

	stop_remarks = fopen("stop_remarks.txt", "w");
	
	numof_channels = numchan;

	channels = new unsigned int [numof_channels];

	for (unsigned int chan=0; chan <= numof_channels-1; chan++)
	{
		channels[chan] = chan+1; /* default: channel label is from 1 to <numof_channels>-1, eog channel is <numof_channels> */
	}

	eog_channel = numof_channels;

	numof_samples = 10; /* default */

	if (numof_channels >=2)
	{
		sort(channels, channels+numof_channels-1); /* sort channel labels */
	}
	channels[numof_channels-1] = eog_channel; /* eog channel as the last one in the array */

	for (int e=0; e<=1; e++)
	{
		reading_state[e] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	reading_stopped = CreateEvent(NULL, FALSE, FALSE, TEXT("BCI_stopped"));
	
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

		sign_h = CreateThread(NULL, 0, sign, &thread_param, 0, NULL);
	}

	connect_tcp(argc, argv);

	printf("Please start Brain Recorder.\n");
	prepare();

	read_h = CreateThread(NULL, 0, reading_thread, 0, 0, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


DWORD WINAPI reading_thread(LPVOID param)
{ 
	get_data();
	return 1;
}


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
					if (channels[numof_channels-1]>nChannels)
					{
						printf("Error: Just %i channels found.\n Please restart application.", nChannels);
						while(1);
					}

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
			closesocket(hSocket);
			printf("Server has closed the connection.\n");
			exit(1);
		}
		else if (nResult < 0)
		{
			closesocket(hSocket);
			printf("An error occured during message receiving.\n");
			exit(1);
		}
	}
}



void tcp::connect_tcp(int argc, char** argv)
{															
	/* Initialize sockets */
	WSADATA wsaData;
	WSAStartup(2, &wsaData);

	
	sockaddr_in addr;

	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hSocket == SOCKET_ERROR)
	{
		printf("Error: can't create socket!\n");
		while(1);
	}

	BOOL bTCP_NODELAY = TRUE;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT_NUMBER);
	

	/* No argument? -> assume "localhost". */
	char* pszHost = argc < 2 ? "localhost" : argv[1];

	/* A connection via "localhost" (127.0.0.1) uses internal buffers on WIN2000,
	 i.e. a sort of burst mode, so we use normal network access via hostname. */
	char buff[200];
	if (!stricmp(pszHost, "localhost"))
	{
		gethostname(buff, sizeof(buff));
		pszHost = buff;
	}
	
	hostent* pHost = gethostbyname(pszHost);
	if (pHost == NULL)
	{
		closesocket(hSocket);
		printf("Can't find host '%s'.\n", argv[1]);
		while(1);
	}
	if (pHost->h_addr_list == 0)
	{
		closesocket(hSocket);
		printf("Can't initialize Remote Data Access.\n");
		while(1);
	}

	addr.sin_addr = **((in_addr**)(pHost->h_addr_list));

	if (connect(hSocket, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) 
	{
		closesocket(hSocket);
		printf("Can't connect to server.\n");
		while(1);
	}
	else
		printf("\nConnected to server '%s'.\n\n", pszHost);
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
						char time [10];
						_strtime(time);

						array_one = new short [arraysize];
						array_two = new short [arraysize];

						fprintf(stop_remarks, "Recorder stopped at system time %s, %li data arrays returned.\n", time, returned_one+returned_two);
					}

					blocknum_one = 1; /* reset variables */
					blocknum_two = 1;
					counter_one = 0; 
					counter_two = 0;
					locked_array = 2;
					ResetEvent(reading_state[0]);  /* just in case the array was filled before and has not been returned */
					SetEvent(reading_state[1]); 

				break;

				default:
				break;
			
			}
		}
		else if (nResult == -2)
		{
			closesocket(hSocket);
			printf("Server has closed the connection.\n");
		}

		else if (nResult < 0)
		{
			closesocket(hSocket);
			printf("An error occured during message receiving.\n");
		}
			
	}
	SetEvent(reading_stopped); /* bci disconnecting requested, data block returned successfully */ 
}


///////////////////////////////////////////////
///////////////////////////////////////////////


void read_data(RDA_MessageData* pMsg)
{	
	unsigned int numof_sample = 0; /* number of sample in current data block */
	switch (locked_array) /* locked for returning data   */
	{
		case 2: /* numof_sample_one and two is the number of sample in the array currently used for storing the data  
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
					SetEvent(reading_state[0]); /* ...now the data is ready to be returned. */
					
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
					SetEvent(reading_state[0]);

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
	waiting = WaitForMultipleObjects(2, reading_state, FALSE, INFINITE);

	if (waiting == 0) /* data is available */
	{
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
		return 1;
	}
	else /* the Recorder has been stopped while demanding data */
	{
		return 2; /* just if Recorder has been stopped */
	}
}


short return_samples(unsigned long n, unsigned int channel) 
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
		
		}

		if (counter_two-1 < returned_two && security_mode == true)
		{
			printf("Warning: Number of read arrays does not match the number of returned ones.\t%i\t%i\n", counter_two-1, returned_two);
		
		}
		
		if (n == numof_samples && channel == numof_channels)
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
		}

		if (counter_one-1 < returned_one && security_mode == true)
		{
			printf("Warning: Number of read arrays does not match the number of returned ones.\t%i\t%i\n", counter_one-1, returned_one);
		
		}

		if (n == numof_samples && channel == numof_channels)
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


int GetServerMessage(SOCKET socket, RDA_MessageHeader** ppHeader)
/* get message from TCP/IP-Port */
{
	timeval tv; tv.tv_sec = 0; tv.tv_usec = 5000;	// 50 ms.
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(socket, &readfds);
	int nResult = select(1, &readfds, NULL, NULL, &tv);
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

	/* Check for correct header GUID. */
	if (header.guid != GUID_RDAHeader)
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



void start_bci(int argc, char** argv, unsigned int mode, unsigned int numchan)
{
	tcp data(argc, argv, mode, numchan); 
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


void change_channellabels(unsigned int channel, unsigned int label, bool restart)
{
	printf("\nRelabeling channel %i...\n", channel);

	if (reading == true)
	{
		reading = false;

		WaitForSingleObject(reading_stopped, INFINITE); /* finish reading the current data block */

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

	if (channel >= numof_channels+1)
	{
		printf("Error: This channel exceeds the number of channels you want to get data from.\n");
	}
	else
	{
		channels[channel-1] = label;
	}
	
	printf("channel labels:\n");
	for (unsigned int c=0; c<=numof_channels-2; c++)
	{
		printf("%i\t", channels[c]);
	}
	printf("eog: %i\n", channels[numof_channels-1]);

	/* ...and restarting. */

	if (restart == true)
	{
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

		if (read_h != NULL)
		{
			TerminateThread (read_h, status);
		}

		if (pHeader) free(pHeader);

		printf("Please restart Brain Recorder.\n");
		prepare();

		read_h = CreateThread(NULL, 0, reading_thread, 0, 0, NULL);
	}
}

					

void set_returning_speed(signed int level)
{
	printf("\nReset of returning speed requested.\n");

	if (reading == true)
	{
		reading = false;

		WaitForSingleObject(reading_stopped, INFINITE); /* finish reading the current data block */

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

	if (read_h != NULL)
	{
		TerminateThread (read_h, status);
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

	read_h = CreateThread(NULL, 0, reading_thread, 0, 0, NULL);
}



void end_bci()
{
	if (reading == true) /* avoid ending if Recorder is not running yet */
	{
		reading = false;

		WaitForSingleObject(reading_stopped, INFINITE); /* finish reading the current data block */

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
		if (channels != NULL)
		{
			delete[] channels;
			channels = NULL;
		}

		if (read_h != NULL)
		{
			TerminateThread (read_h, status);
		}

		if (sign_h != NULL)
		{
			TerminateThread (sign_h, status);
		}

		if (pHeader) free(pHeader);

		closesocket(hSocket);
		
		#ifdef BINOUT
			fclose(binoutfile);
		#endif

		/* Clear keyboard queue */
		if (_kbhit()) while(!getch());	
	}
}





BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}