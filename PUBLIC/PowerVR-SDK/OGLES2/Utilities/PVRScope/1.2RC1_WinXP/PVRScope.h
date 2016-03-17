
#ifndef _PVRSCOPE_H_
#define _PVRSCOPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(UNDER_CE)
	#define IMG_CALLCONV __cdecl
#else
	#if defined(_WIN32)
		#define IMG_CALLCONV __stdcall
	#else
		#define IMG_CALLCONV
	#endif
#endif

// 64-bit type
#if defined(_WIN32)
typedef unsigned __int64	IMG_UINT64;
typedef __int64				IMG_INT64;
#else
	#if defined(LINUX) || defined (__SYMBIAN32__) || defined(_UITRON_)
		typedef unsigned long long		IMG_UINT64;
		typedef long long 				IMG_INT64;
	#else
		#error("define an OS")
	#endif
#endif

/****************************************************************************
** Enums
****************************************************************************/
enum EPVRScopeTimingType
{
	ePVRScopeInvalid,
	ePVRScopeTransfer,
	ePVRScopeTA,
	ePVRScope3D,
	ePVRScopeTimeTypeCount	// used for array lengths
};

/****************************************************************************
** Structures
****************************************************************************/

// Internal implementation data
struct SPVRScopeData;

struct SPVRScopeCounter
{
	const char		*pszName;
	unsigned int	nMaximum;
	unsigned int	nGroup;
};

struct SPVRScopeTiming
{
	unsigned int		nFrameNumber;
	EPVRScopeTimingType	eType;
	IMG_UINT64			ui64StartTimeHW;
	IMG_UINT64			ui64EndTimeHW;
	unsigned int		nStartTimeHWUS;
	unsigned int		nEndTimeHWUS;
	unsigned int		nStartTimeHostUS;
	unsigned int		nEndTimeHostUS;
	unsigned int		nClockSpeed;
};

/****************************************************************************
** Declarations
****************************************************************************/

SPVRScopeData * IMG_CALLCONV PVRScopeInit();

void IMG_CALLCONV PVRScopeDeInit(
	SPVRScopeData		**ppsData,
	SPVRScopeCounter	**ppsCounters,
	unsigned int		**ppnValueBuf);

bool IMG_CALLCONV PVRScopeGetPerfCounters(
	SPVRScopeData		* const psData,
	unsigned int		*pnCount,
	SPVRScopeCounter	**ppsCounters,
	unsigned int		**ppnValueBuf);

bool IMG_CALLCONV PVRScopeReadPerfCountersThenSetGroup(
	SPVRScopeData		* const psData,
	unsigned int		* const pnValueCnt,
	unsigned int		* const pnValueBuf,
	const unsigned int	nValueBufLen,
	const unsigned int	nGroup);

bool IMG_CALLCONV PVRScopeTimingEnable(
	SPVRScopeData	* const psData,
	const bool		bEnable);

bool IMG_CALLCONV PVRScopeTimingRetrieve(
	SPVRScopeData		* const psData,
	SPVRScopeTiming		* const psTimings);

#ifdef __cplusplus
}
#endif

#endif /* _PVRSCOPE_H_ */

/*****************************************************************************
 End of file (PVRScope.h)
*****************************************************************************/
