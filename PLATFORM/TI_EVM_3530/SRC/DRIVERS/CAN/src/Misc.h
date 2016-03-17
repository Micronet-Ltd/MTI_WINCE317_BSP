#ifndef __MISC_H
#define __MISC_H

//#include "CAN.h"
//#include "..\..\applications\m_user\inc\can_api.h"
#include "can_api_ex.h"
#include "CANB.h"



// Definitions ====================================================================================

#define CANDRVREG		TEXT( "Drivers\\BuiltIn\\CANBD" )
#define CAN_BRP			TEXT( "BRP" )
#define CAN_PROPSEG		TEXT( "PropSeg" )
#define CAN_PHASESEG1	TEXT( "PhaseSeg1" )
#define CAN_PHASESEG2	TEXT( "PhaseSeg2" )
#define CAN_BTL			TEXT( "BTL" )
#define CAN_SJW			TEXT( "SJW" )
#define CAN_SAMPLES		TEXT( "Samples" )

#define CAN_WAKFIL		TEXT( "WAKFIL" )

#define CAN_RTO			TEXT( "RTO" )
#define CAN_WTO			TEXT( "WTO" )

#define CAN_RXMB0		TEXT( "RXMB0" )
#define CAN_RXMB1		TEXT( "RXMB1" )

#define CAN_SIDMASK0	TEXT( "SIDMASK0" )
#define CAN_EIDMASK0	TEXT( "EIDMASK0" )

#define CAN_SIDMASK1	TEXT( "SIDMASK1" )
#define CAN_EIDMASK1	TEXT( "SIDMASK1" )

#define CAN_SIDFILTER0	TEXT( "SIDFILTER0" )
#define CAN_FILSWITCH0	TEXT( "FILSWITCH0" )
#define CAN_EIDFILTER0	TEXT( "EIDFILTER0" )

#define CAN_SIDFILTER1	TEXT( "SIDFILTER1" )
#define CAN_FILSWITCH1	TEXT( "FILSWITCH1" )
#define CAN_EIDFILTER1	TEXT( "EIDFILTER1" )

#define CAN_SIDFILTER2	TEXT( "SIDFILTER2" )
#define CAN_FILSWITCH2	TEXT( "FILSWITCH2" )
#define CAN_EIDFILTER2	TEXT( "EIDFILTER2" )

#define CAN_SIDFILTER3	TEXT( "SIDFILTER3" )
#define CAN_FILSWITCH3	TEXT( "FILSWITCH3" )
#define CAN_EIDFILTER3	TEXT( "EIDFILTER3" )

#define CAN_SIDFILTER4	TEXT( "SIDFILTER4" )
#define CAN_FILSWITCH4	TEXT( "FILSWITCH4" )
#define CAN_EIDFILTER4	TEXT( "EIDFILTER4" )

#define CAN_SIDFILTER5	TEXT( "SIDFILTER5" )
#define CAN_FILSWITCH5	TEXT( "FILSWITCH5" )
#define CAN_EIDFILTER5	TEXT( "EIDFILTER5" )


// Misc. Functions ================================================================================

// Function name	: RegGetParameters
// Description	    : Get all values from registry in one call
// Return type		: BOOL RegGetParameters 
// Argument         : PCANREGSETTINGS

BOOL RegGetParameters ( PCANREGSETTINGS );

// ------------------------------------------------------------------------------------------------

// Function name	: ValidateCNFParameters
// Description	    : Checks Parameters Validity of CANREGSETTINGS structure
// Return type		: BOOL ValidateCNFParameters 
// Argument         : PCANREGSETTINGS

BOOL ValidateCNFParameters ( PCANREGSETTINGS );

// ------------------------------------------------------------------------------------------------

// Function name	: ValidateMasksAndFilters
// Description	    : Checks Masks and Filters Validity
// Return type		: BOOL ValidateMasksAndFilters 
// Argument         : PCANREGSETTINGS

BOOL ValidateMasksAndFilters ( PCANREGSETTINGS );

// ------------------------------------------------------------------------------------------------

// Function name	: CheckEID
// Description	    : Check the Standard Identifier Validity
// Return type		: BOOL CheckEID 
// Argument         : unsigned __int32 EID

BOOL CheckEID ( unsigned __int32 EID );

// ------------------------------------------------------------------------------------------------

// Function name	: CheckSID
// Description	    : Check the Standard Identifier Validity
// Return type		: BOOL CheckSID 
// Argument         : unsigned __int32 SID

BOOL CheckSID ( unsigned __int32 SID );

// ------------------------------------------------------------------------------------------------

// Function name	: SetTimeOuts
// Description	    : Set values for globals g_dwReadTimeout and g_dwWriteTimeout
// Return type		: BOOL SetTimeOuts 
// Argument         : PCANREGSETTINGS

BOOL SetTimeOuts ( PCANREGSETTINGS );

// ------------------------------------------------------------------------------------------------

// Function name	: ValidateRXMParameters
// Description	    : Checks Receive Buffer Operating Mode Parameters
// Return type		: BOOL ValidateRXMParameters 
// Argument         : PCANREGSETTINGS

BOOL ValidateRXMParameters ( PCANREGSETTINGS );

// ------------------------------------------------------------------------------------------------

void DelayMS(DWORD dwMilliSeconds, BOOL bInPowerHandler);

#endif //__MISC_H