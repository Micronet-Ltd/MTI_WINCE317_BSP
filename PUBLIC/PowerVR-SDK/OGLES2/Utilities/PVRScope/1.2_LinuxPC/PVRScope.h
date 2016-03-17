/*****************************************************************************
$Revision: 1.1 $
*****************************************************************************/

#ifndef _PVRSCOPE_H_
#define _PVRSCOPE_H_

#ifdef __cplusplus
extern "C" {
#endif


			#if defined(__linux__)

				#define IMG_CALLCONV
				#define IMG_EXPORT
				#define IMG_IMPORT

			#else
					#error("define an OS")
			#endif

// 64-bit type
	#if defined(LINUX)
		typedef unsigned long long		IMG_UINT64;
		typedef long long 				IMG_INT64;
	#else
		#error("define an OS")
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

IMG_IMPORT
SPVRScopeData * IMG_CALLCONV PVRScopeInit();

IMG_IMPORT
void IMG_CALLCONV PVRScopeDeInit(
	SPVRScopeData		**ppsData,
	SPVRScopeCounter	**ppsCounters,
	unsigned int		**ppnValueBuf);

IMG_IMPORT
bool IMG_CALLCONV PVRScopeGetPerfCounters(
	SPVRScopeData		* const psData,
	unsigned int		*pnCount,
	SPVRScopeCounter	**ppsCounters,
	unsigned int		**ppnValueBuf);

IMG_IMPORT
bool IMG_CALLCONV PVRScopeReadPerfCountersThenSetGroup(
	SPVRScopeData		* const psData,
	unsigned int		* const pnValueCnt,
	unsigned int		* const pnValueBuf,
	const unsigned int	nValueBufLen,
	const unsigned int	nGroup);

IMG_IMPORT
bool IMG_CALLCONV PVRScopeTimingEnable(
	SPVRScopeData	* const psData,
	const bool		bEnable);

IMG_IMPORT
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
