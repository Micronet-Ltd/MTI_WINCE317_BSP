#pragma once
#include "resource.h"

#ifdef WIN32_PLATFORM_PSPC
	#define SecureZeroMemory(a,b) memset(a, 0, b)
#endif

#define MAX_LOADSTRING 100

#define DIR_UPLOAD_ONLY		0
#define DIR_DOWNLOAD_ONLY	1
#define DIR_BOTH			2

#define MSG_END_OF_TEST			1
#define MSG_TRANSFER_STARTED	2
#define MSG_RECEIVE_STARTED		3
#define MSG_PROGRESS_CHANGE		4
#define MSG_ITERATION_CHANGE	5

struct PacketHeader
{
	int			_type;
	DWORD		_size;
};

struct PacketHeaderData
{
	int			_nBufferSize;
	int			_nStreamSize;
	int			_nIterations;
	int			_nDirection;
};

struct PacketAcknowlege
{
	int			_nAck;
	int			_nReceived;
};

struct PacketBeginIterationHeader
{
	int			_nIterationNumber;
	int			_nCurrentDirection;
};
