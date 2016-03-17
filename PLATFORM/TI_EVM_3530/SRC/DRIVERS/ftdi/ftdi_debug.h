/*++

Copyright (c) 2000-2005  Future Technology Devices International Ltd.

Module Name:

    ftdi_debug.h

Abstract:

    Native USB device driver for FTDI FT8U232
    Debug macros

Environment:

    kernel mode

Revision History:

	15/03/05	st		Add debug functions.
	22/06/05	st		Changes the timed messages to display on the same line.


--*/

#if defined(LOG_DEBUG)
	extern TCHAR gtcBuf[1024];
	void WriteToDebugFile(TCHAR * pBuf);

#ifdef DEBUG
	#define TO_DEBUG(x)
#else	// definitions for RELEASE build
	#define ZONE_INIT		0x00000001
	#define ZONE_OPEN		0x00000002
	#define ZONE_READ		0x00000004
	#define ZONE_WRITE		0x00000008
	#define ZONE_CLOSE		0x00000010
	#define ZONE_IOCTL		0x00000020
	#define ZONE_THREAD		0x00000040
	#define ZONE_EVENTS		0x00000080
	#define ZONE_CRITSEC	0x00000100
	#define ZONE_FLOW		0x00000200
	#define ZONE_IR			0x00000400
	#define ZONE_USR_READ	0x00000800
	#define ZONE_ALLOC		0x00001000
	#define ZONE_FUNCTION	0x00002000
	#define ZONE_WARN		0x00004000
	#define ZONE_ERROR		0x00008000
	extern unsigned long ulDebugFlags;
	#define TO_DEBUG(x)	if(x & ulDebugFlags) 
#endif

	// our log to file debug - for both DEBUG and RELEASE builds
	#define DEBUGMSG1(x, y)  TO_DEBUG(x)  WriteToDebugFile(y) 
	#define DEBUGMSG2(x, y, z) TO_DEBUG(x)  { swprintf(gtcBuf, y, z) ; WriteToDebugFile(gtcBuf); }
	#define DEBUGMSG3(x, y, z, a ) TO_DEBUG(x) { swprintf(gtcBuf, y, z, a) ; WriteToDebugFile(gtcBuf); }
	#define DEBUGMSG4(x, y, z, a, b ) TO_DEBUG(x) { swprintf(gtcBuf, y, z, a, b) ; WriteToDebugFile(gtcBuf); }
	#define DEBUGMSG5(x, y, z, a, b, c ) TO_DEBUG(x) { swprintf(gtcBuf, y, z, a, b, c) ; WriteToDebugFile(gtcBuf); }
	#define DEBUGMSG6(x, y, z, a, b, c, d ) TO_DEBUG(x) { swprintf(gtcBuf, y, z, a, b, c, d) ; WriteToDebugFile(gtcBuf); }
	#define DEBUGMSG7(x, y, z, a, b, c, d, e) TO_DEBUG(x) { swprintf(gtcBuf, y, z, a, b, c, d, e) ; WriteToDebugFile(gtcBuf); }
	#define DEBUGMSG8(x, y, z, a, b, c, d, e, f) TO_DEBUG(x) { swprintf(gtcBuf, y, z, a, b, c, d, e, f) ; WriteToDebugFile(gtcBuf); }

#else

#ifdef DEBUG
	extern 
#ifdef __cplusplus
	"C" {
#endif

		TCHAR gtcBuf[1024];
#ifdef __cplusplus
	}
#endif	
	extern 
#ifdef __cplusplus
	"C" {
#endif
		TCHAR gtcBufTime[1024];
#ifdef __cplusplus
	}
#endif	
	DWORD t;
	#define DEBUGMSGT(x, y) t = timeGetTime() ; \
							swprintf(gtcBufTime, L"%d: %s", t, y) ; \
							DEBUGMSG(x, (gtcBufTime))

	#define DEBUGMSG1(x, y)  DEBUGMSGT(x, y)
	#define DEBUGMSG2(x, y, z)  swprintf(gtcBuf, y, z) ; DEBUGMSGT(x, (gtcBuf))
	#define DEBUGMSG3(x, y, z, a )  swprintf(gtcBuf, y, z, a) ; DEBUGMSGT(x, (gtcBuf))
	#define DEBUGMSG4(x, y, z, a, b )  swprintf(gtcBuf, y, z, a, b) ; DEBUGMSGT(x, (gtcBuf))
	#define DEBUGMSG5(x, y, z, a, b, c )  swprintf(gtcBuf, y, z, a, b, c) ; DEBUGMSGT(x, (gtcBuf))
	#define DEBUGMSG6(x, y, z, a, b, c, d )  swprintf(gtcBuf, y, z, a, b, c, d) ; DEBUGMSGT(x, (gtcBuf))
	#define DEBUGMSG7(x, y, z, a, b, c, d, e )  swprintf(gtcBuf, y, z, a, b, c, d, e) ; DEBUGMSGT(x, (gtcBuf))
	#define DEBUGMSG8(x, y, z, a, b, c, d, e, f )  swprintf(gtcBuf, y, z, a, b, c, d, e, f) ; DEBUGMSGT(x, (gtcBuf))

#else
// RELEASE debug - i.e none
	#define DEBUGMSG1(x, y)  
	#define DEBUGMSG2(x, y, z)  
	#define DEBUGMSG3(x, y, z, a )  
	#define DEBUGMSG4(x, y, z, a, b )  
	#define DEBUGMSG5(x, y, z, a, b, c )  
	#define DEBUGMSG6(x, y, z, a, b, c, d )  
	#define DEBUGMSG7(x, y, z, a, b, c, d, e )  
	#define DEBUGMSG8(x, y, z, a, b, c, d, e, f )  
	
#endif

#endif  //#if defined(LOG_DEBUG)