#ifndef _INC_RECORDERRDA
#define _INC_RECORDERRDA
// MODULE: RDA.h
//: written by: Henning Nordholz
//+       date: 14-Nov-00
//+
//+ Description:
//+ 	Vision Recorder
//. 		Remote Data Access (RDA) structs and constants


#pragma pack(1)
#ifndef ULONG
typedef unsigned long ULONG;
#endif


//////    has to be tested yet   /////
/* GUID defines */
typedef struct __attribute__((__packed__)) _s_GUID {
    unsigned int	Data1;
    unsigned short	Data2;
    unsigned short	Data3;
    unsigned char	Data4[8];
} GUID;


/*typedef GUID*  LPGUID;

typedef GUID   IID;
typedef IID*   LPIID;

typedef GUID   CLSID;
typedef CLSID* LPCLSID;

typedef GUID   FMTID;
typedef FMTID* LPFMTID;

#define REFGUID  const GUID &
#define REFIID   const IID &
#define REFCLSID const CLSID &
#define REFFMTID const FMTID &

typedef GUID  UUID;	*/	


/* See initguid.h for the real defines */
#ifndef INITGUID
 # ifdef __cplusplus 
 #   define EXTERN_C extern "C" 
 # else 
 #   define EXTERN_C extern 
 # endif 
  #define GUID_EXT extern "C"
//#define DEFINE_GUID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) GUID_EXT const GUID n
#endif

//#define DEFINE_OLEGUID(n,l,w1,w2) DEFINE_GUID(n,l,w1,w2,0xC0,0,0,0,0,0,0,0x46)

// All numbers are sent in little endian format.

// Unique identifier for messages sent to clients
// {4358458E-C996-4C86-AF4A-98BBF6C91450}
// As byte array (16 bytes): 8E45584396C9864CAF4A98BBF6C91450

//DEFINE_GUID(GUID_RDAHeader, 0x4358458e, 0xc996, 0x4c86, 0xaf, 0x4a, 0x98, 0xbb, 0xf6, 0xc9, 0x14, 0x50);

//#define GUID_RDAHeader GUID_EXT GUID;
//extern "C" GUID GUID_RDAHeader  = { 0xcb88d973, 0x492c, 0x49b6, { 0xb5, 0x72, 0x94, 0x45, 0x88, 0xe3, 0xbe, 0x8d } };


struct RDA_Marker
//; A single marker in the marker array of RDA_MessageData
{
	ULONG				nSize;				// Size of this marker.
	ULONG				nPosition;			// Relative position in the data block.
	ULONG				nPoints;			// Number of points of this marker
	long				nChannel;			// Associated channel number (-1 = all channels).
	char				sTypeDesc[1];		// Type, description in ASCII delimited by '\0'.
};


struct RDA_MessageHeader
//; Message header
{
	GUID guid;	// Always GUID_RDAHeader
	ULONG nSize; 	// Size of the message block in bytes including this header
	ULONG nType;	// Message type.
};


// **** Messages sent by the RDA server to the clients. ****
struct RDA_MessageStart : RDA_MessageHeader
//; Setup / Start infos, Header -> nType = 1
{
	ULONG				nChannels;			// Number of channels
	double				dSamplingInterval;	// Sampling interval in microseconds
	double				dResolutions[1];	// Array of channel resolutions -> double dResolutions[nChannels]
											// coded in microvolts. i.e. RealValue = resolution * A/D value
	char 				sChannelNames[1];	// Channel names delimited by '\0'. The real size is
											// larger than 1.
};


struct RDA_MessageData : RDA_MessageHeader
//; Block of 16-bit data, Header -> nType = 2, sent only from port 51234
{
	ULONG				nBlock;				// Block number, i.e. acquired blocks since acquisition started.
	ULONG				nPoints;			// Number of data points in this block
	ULONG				nMarkers;			// Number of markers in this data block
	short				nData[1];			// Data array -> short nData[nChannels * nPoints], multiplexed
	RDA_Marker			Markers[1];			// Array of markers -> RDA_Marker Markers[nMarkers]
};

struct RDA_MessageStop : RDA_MessageHeader
//; Data acquisition has been stopped. // Header -> nType = 3
{
};



struct RDA_MessageData32 : RDA_MessageHeader
//; Block of 32-bit floating point data, Header -> nType = 4, sent only from port 51244
{
	ULONG				nBlock;				// Block number, i.e. acquired blocks since acquisition started.
	ULONG				nPoints;			// Number of data points in this block
	ULONG				nMarkers;			// Number of markers in this data block
	float				fData[1];			// Data array -> float fData[nChannels * nPoints], multiplexed
	RDA_Marker			Markers[1];			// Array of markers -> RDA_Marker Markers[nMarkers]
};


// **** End Messages sent by the RDA server to the clients. ****

#pragma pack()

#endif //_INC_RECORDERRDA
