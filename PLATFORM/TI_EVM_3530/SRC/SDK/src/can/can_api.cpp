//-----------------------------------------------------------------------------
// Copyright 2009 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  File:  can_api.c
//
//  This file CAN API code.
//
//  Created by Michael Streshinsky    2010
//-----------------------------------------------------------------------------

#include <windows.h>
#include <MicUserSdk.h>
#include "CanOpenLib_HW.h"
#include "can_api_ex.h"
#include "canb.h"

#define   CAN_API_DBG  0
#define   CAN_IDENTIFIER                          L"CAN1:"

/* ----------- static functions ---------------------------------------------- */
static DWORD CANSetCNFG(HANDLE hDevice, PCANCONFIG pCNFG_IN);
static DWORD CANGetCNFG (HANDLE hDevice, PCANCONFIG pCNFG_OUT);
static DWORD CANSetMASK(HANDLE hDevice, PCANMASK pMASK_IN);
static DWORD CANGetMASK(HANDLE hDevice, PCANMASK pMASK_OUT);
static DWORD CANSetFILTER(HANDLE hDevice, PCANFILTER pFILTER_IN);
static DWORD CANGetFILTER(HANDLE hDevice, PCANFILTER pFILTER_OUT);
static DWORD CANSetRWTO(HANDLE hDevice, PCANRWTIMEOUT pRWTO_IN);
static DWORD CANGetRWTO(HANDLE hDevice, PCANRWTIMEOUT pRWTO_OUT);
static DWORD CANSetRXBOPMODE(HANDLE hDevice, PCANRXBOPMODE pRXBOPMODE_IN);
static DWORD CANGetRXBOPMODE(HANDLE hDevice, PCANRXBOPMODE pRXBOPMODE_OUT);
static DWORD MCP2515Reset(HANDLE hDevice);
static DWORD CANGetERROR(HANDLE hDevice);
static CAN_OPMODE  CANGetOpMode(HANDLE hDevice);
static BOOL        CANSetOpMode(HANDLE hDevice, CAN_OPMODE OpMode);
//------------------------------------------------------------------------------


CANOPENLIB_HW_API canOpenStatus __stdcall canPortLibraryInit(void)
{
    return(CANOPEN_OK);
}


CANOPENLIB_HW_API canOpenStatus  __stdcall canPortOpen( int port, canPortHandle *handle )
{
    HANDLE                  hDevice;
    BOOL                    ret = FALSE;
    DWORD                   canDrvErrs;
    DWORD                   dwActualOut;

    RETAILMSG(CAN_API_DBG, (L"+canPortOpen\r\n"));

    if ( ( port != 0 ) || (handle == NULL) )
        return CANOPEN_ARG_ERROR;

    hDevice = CreateFile(CAN_IDENTIFIER, GENERIC_READ | GENERIC_WRITE, 
                         0, 0, OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);


    if ( ( hDevice != NULL )&&( hDevice != INVALID_HANDLE_VALUE ) )
    {
        *handle = (canPortHandle)hDevice;

        RETAILMSG(CAN_API_DBG, (L"+canPortOpen, CreateFile Ok, handle = 0x%08X\r\n", hDevice ));

        ret = DeviceIoControl( hDevice, IOCTL_CAN_GET_ERRORS, NULL, 0,
                               (LPVOID)&canDrvErrs, sizeof(canDrvErrs), &dwActualOut, NULL ); 

        if ( ( ret != TRUE )||(canDrvErrs != 0 ) )
        {
            RETAILMSG(CAN_API_DBG, (L"+canPortOpen, IOCTL_CAN_GET_ERRORS: ret = %d, canDrvErrs = %d\r\n", ret, canDrvErrs ));
            return(CANOPEN_ERROR_DRIVER);
        }
        else
        {
            // Do not set some default parameters such as bit rate, masks, filters etc ?
            // because it set on Init, taken from Registry.
            // Remain in Configuration mode, bus On function will set operational mode.
            return(CANOPEN_OK);
        }
    }
    else
    {
        RETAILMSG(CAN_API_DBG, (L"canPortOpen: Fail, err = %d\r\n", GetLastError() ));
        *handle = (canPortHandle)INVALID_HANDLE_VALUE;
        return CANOPEN_ERROR_DRIVER;
    }

}



CANOPENLIB_HW_API canOpenStatus  __stdcall canPortClose( canPortHandle handle )
{
    BOOL                    ret = FALSE;

    if ( ( (HANDLE)handle != NULL )&&( (HANDLE)handle != INVALID_HANDLE_VALUE ) )
    {
        if ( !CloseHandle((HANDLE)handle) )
        {
            RETAILMSG(CAN_API_DBG, (L"canPortClose: CloseHandle for Device err = %d\r\n", GetLastError() ));
            return CANOPEN_ERROR_DRIVER;
        }

    }
    else
    {
        RETAILMSG(CAN_API_DBG, (L"canPortClose: Fail, Invalid Handler\r\n"));
        return CANOPEN_ARG_ERROR;;
    }

    return(CANOPEN_OK);
}



// Call canPortGoBusOff  before for setting CAN controller in Configuration mode.

CANOPENLIB_HW_API canOpenStatus   __stdcall canPortBitrateSet( canPortHandle handle, int bitrate )
{
    CANCONFIG    canCNF;
    UINT8        PropSeg, PS1, PS2;
    INT8         k;
    BOOL         updateFlag, ret;
    DWORD        dwActualOut;


    if ( ( (HANDLE)handle != NULL )&&( (HANDLE)handle != INVALID_HANDLE_VALUE ) )
    {

        //  Set Configuration mode

        ret = DeviceIoControl( (HANDLE)handle, IOCTL_CAN_GET_BITTIMING, NULL, 0,
                               (LPVOID)&canCNF, sizeof(CANCONFIG), &dwActualOut, NULL ); 

        if ( ret != TRUE )
        {
            RETAILMSG(CAN_API_DBG, (L"canPortBitrateSet: Fail calling IOCTL_CAN_GET_BITTIMING\r\n"));
            return(CANOPEN_ERROR_DRIVER);
        }

        k = -1;

        if ( canCNF.BTLMODE != 0 ) // 1
        {
            // ;  PropSeg = 1, PS1 = 4 , PS2 = 2
            PropSeg = 1;
            PS1 = 4;
            PS2 = 2;
        }
        else
        {
            PropSeg = 1;
            PS1 = 3;
            //PS2 = 3;
        }

        switch ( bitrate )
        {
        // ;     NBR = 1000000 / k    , where k = 1 ... 64;  k = BRP+1
        case CANBR_1MBPS:    k = 1; break;  // 1
        case CANBR_500KBPS:  k = 2; break; // 2
        case CANBR_250KBPS:  k = 4; break; // 4
        case CANBR_125KBPS:  k = 8; break; // 8 
        case CANBR_100KBPS:  k = 10; break; // 10
        case CANBR_50KBPS:   k = 20; break;  // 20
        case CANBR_20KBPS:   k = 50; break;  // 50
        case CANBR_10KBPS:   k = 100; break;  // 100
        case CANBR_800KBPS:  
            {
                k = 1;
                // Increase bit timing to 10 TQ
                if ( canCNF.BTLMODE != 0 ) // 1
                {
                    PropSeg = 2;
                    PS1 = 4;
                    PS2 = 3;
                }
                else
                {
                    PropSeg = 3;
                    PS1 = 3;
                    //PS2 = 3;
                }

            } break; // 

        default: 
            {
                RETAILMSG(CAN_API_DBG, (L"canPortBitrateSet: Fail, UNSUPPORTED BITRATE\r\n"));
                return CANOPEN_UNSUPPORTED_BITRATE;
            } break;

        } // switch

        // Update baud rate if needed to update
        updateFlag = FALSE;
        if ( (canCNF.PRSEG  + 1) != PropSeg )
        {
            canCNF.PRSEG = PropSeg - 1;
            updateFlag = TRUE;
        }

        if ( (canCNF.PHSEG1 + 1) != PS1 )
        {
            canCNF.PHSEG1 = PS1 - 1;
            updateFlag = TRUE;
        }

        if ( canCNF.BTLMODE &( (canCNF.PHSEG2 + 1) != PS2 ) )
        {
            canCNF.PHSEG2 = PS2 - 1;
            updateFlag = TRUE;
        }

        if ( k != (canCNF.BRP + 1 ) )
        {
            canCNF.BRP = k - 1;
            updateFlag = TRUE;
        }

        if ( updateFlag == TRUE )
        {
            ret = DeviceIoControl( (HANDLE)handle, IOCTL_CAN_SET_BITTIMING, (LPVOID)&canCNF, sizeof(CANCONFIG),
                                   NULL, 0 , &dwActualOut, NULL ); 

            if ( ret != TRUE )
            {
                RETAILMSG(CAN_API_DBG, (L"canPortBitrateSet: Fail calling IOCTL_CAN_SET_BITTIMING\r\n"));
                return(CANOPEN_ERROR_DRIVER);
            }

        }

    }
    else
    {
        RETAILMSG(CAN_API_DBG, (L"canPortBitrateSet: Fail, Invalid Handler\r\n"));
        return CANOPEN_ARG_ERROR;
    }

    return(CANOPEN_OK);
}



// Set CAN bus in Normal Operational Mode

CANOPENLIB_HW_API canOpenStatus __stdcall canPortGoBusOn( canPortHandle handle )
{
    CAN_OPMODE  OpMode;
    DWORD       dwBytesReturned   = 0;
    BOOL        ret; 

    if ( ( (HANDLE)handle != NULL )&&( (HANDLE)handle != INVALID_HANDLE_VALUE ) )
    {
        // Set CAN operational mode to Normal
        OpMode = CAN_OPMODE_NORMAL;
        ret = DeviceIoControl( (HANDLE)handle, IOCTL_CAN_SET_OPMODE, &OpMode, sizeof(CAN_OPMODE), 0, 0, &dwBytesReturned, NULL);
        if ( ret != TRUE )
        {
            RETAILMSG(CAN_API_DBG, (L"canPortGoBusOn: Fail calling IOCTL_CAN_SET_OPMODE\r\n"));
            return(CANOPEN_ERROR_DRIVER);
        }

    }
    else
    {
        RETAILMSG(CAN_API_DBG, (L"canPortGoBusOn: Fail, Invalid Handler\r\n"));
        return CANOPEN_ARG_ERROR;
    }

    return(CANOPEN_OK);
}



// Set CAN bus in Configuration Mode

CANOPENLIB_HW_API canOpenStatus __stdcall canPortGoBusOff( canPortHandle handle )
{
    CAN_OPMODE  OpMode;
    DWORD       dwBytesReturned   = 0;
    BOOL        ret; 

    if ( ( (HANDLE)handle != NULL )&&( (HANDLE)handle != INVALID_HANDLE_VALUE ) )
    {
        // Set CAN operational mode to Normal
        OpMode = CAN_OPMODE_CONFIG;
        ret = DeviceIoControl( (HANDLE)handle, IOCTL_CAN_SET_OPMODE, &OpMode, sizeof(CAN_OPMODE), 0, 0, &dwBytesReturned, NULL);
        if ( ret != TRUE )
        {
            RETAILMSG(CAN_API_DBG, (L"canPortGoBusOff: Fail calling IOCTL_CAN_SET_OPMODE\r\n"));
            return(CANOPEN_ERROR_DRIVER);
        }

    }
    else
    {
        RETAILMSG(CAN_API_DBG, (L"canPortGoBusOff: Fail, Invalid Handler\r\n"));
        return CANOPEN_ARG_ERROR;
    }

    return(CANOPEN_OK);
}



CANOPENLIB_HW_API canOpenStatus __stdcall canPortWrite(canPortHandle handle, long id, void *msg, unsigned int dlc, unsigned int flags)
{
    CANSENDPACKET      tCANSENDPACKET;
    DWORD              dwBytesNumWritten;
    canOpenStatus      canRet = CANOPEN_OK; 

    if ( ( (HANDLE)handle != NULL )&&( (HANDLE)handle != INVALID_HANDLE_VALUE )&&
         ( dlc <= 8 )&&(msg != NULL)
       )
    {
        memset ( &tCANSENDPACKET, 0x00, sizeof ( tCANSENDPACKET ) );

//#define CAN_MSG_RTR              0x0001      // Message is a remote request
//#define CAN_MSG_EXT              0x0002      // Message has a standard ID

        tCANSENDPACKET.TxBP         = TXB_PRIORITY_LOWEST;
        tCANSENDPACKET.bIsRTR       = FALSE;
        tCANSENDPACKET.bIsEXT       = FALSE;
        tCANSENDPACKET.DataLength   = dlc;

       tCANSENDPACKET.SID         	= (id & 0x000007FF);

        if ( flags & CAN_MSG_EXT )
        {
            tCANSENDPACKET.bIsEXT     = TRUE;
            tCANSENDPACKET.EID        = (id >> 11) & 0x0003FFFF;
        }

        if ( flags & CAN_MSG_RTR )
            tCANSENDPACKET.bIsRTR     = TRUE;

        memcpy ( tCANSENDPACKET.DATA, msg, dlc );
        dwBytesNumWritten = 0;
        if ( WriteFile( (HANDLE)handle, &tCANSENDPACKET, sizeof(tCANSENDPACKET), &dwBytesNumWritten, NULL ) == 0 )
        {
            // DeviceIoControl( hDevice, IOCTL_CAN_GET_ERRORS, NULL, 0,
            //					       (LPVOID)&canDrvErrs, sizeof(canDrvErrs),0,NULL ); 

            switch ( GetLastError() )
            {
            case  ERROR_INVALID_PARAMETER: canRet = CANOPEN_ARG_ERROR; break;
            case  ERROR_TIMEOUT: canRet = CANOPEN_TIMEOUT; break;
            case  ERROR_OPERATION_ABORTED: canRet = CANOPEN_ERROR; break;
            case  ERROR_IO_DEVICE: canRet = CANOPEN_ERROR; break;
            default: canRet = CANOPEN_ERROR; break;
            }
            RETAILMSG(CAN_API_DBG, (L"canPortWrite: Fail, Invalid params\r\n"));
            return canRet;
        }

    }
    else
    {
        RETAILMSG(CAN_API_DBG, (L"canPortWrite: Fail, Invalid params\r\n"));
        return CANOPEN_ARG_ERROR;
    }

    return(canRet);
}



CANOPENLIB_HW_API canOpenStatus __stdcall canPortRead(canPortHandle handle, long *id, void *msg, unsigned int *dlc, unsigned int *flags)
{
    CANRECEIVEPACKET   tCANREADPACKET;
    DWORD              dwNumOfBytesRead;
    canOpenStatus      canRet = CANOPEN_OK;


    if ( ( (HANDLE)handle != NULL )&&( (HANDLE)handle != INVALID_HANDLE_VALUE )&&
         ( msg != NULL)&&(id != NULL)&&(dlc != NULL)&&(flags != NULL)
       )
    {
        memset(&tCANREADPACKET, 0x00, sizeof(tCANREADPACKET));

        if ( ReadFile( (HANDLE)handle, &tCANREADPACKET, sizeof(tCANREADPACKET), &dwNumOfBytesRead, NULL ) == 0 )
        {
            // DeviceIoControl( hDevice, IOCTL_CAN_GET_ERRORS, NULL, 0,
            //					       (LPVOID)&canDrvErrs, sizeof(canDrvErrs),0,NULL ); 

            switch ( GetLastError() )
            {
            case  ERROR_TIMEOUT: canRet = CANOPEN_TIMEOUT; break;
            case  ERROR_OPERATION_ABORTED: canRet = CANOPEN_ERROR; break;
            case  ERROR_INTERNAL_ERROR: canRet = CANOPEN_ERROR; break;
            default: canRet = CANOPEN_ERROR; break;
            }

            RETAILMSG(CAN_API_DBG, (L"canPortRead: Fail, Invalid params\r\n"));
            return canRet;
        }

        if ( tCANREADPACKET.bIsRTR )
            *flags |= CAN_MSG_RTR;
        else
            *flags &= (~CAN_MSG_RTR); 

        *id = tCANREADPACKET.SID;

        if ( tCANREADPACKET.bIsEXT )
        {
            *flags |= CAN_MSG_EXT;
            *id |= (tCANREADPACKET.EID << 11) & 0x1FFFF800;
        }
        else
        {
            *flags &= (~CAN_MSG_EXT);
        }

        *dlc = tCANREADPACKET.DataLength;

        if ( *dlc > 8 )
        {
            RETAILMSG(CAN_API_DBG, (L"canPortRead: Fail, Invalid data length\r\n"));
            return CANOPEN_ERROR_DRIVER;
        }

        memcpy ( msg, tCANREADPACKET.DATA, *dlc );
    }
    else
    {
        RETAILMSG(CAN_API_DBG, (L"canPortRead: Fail, Invalid params\r\n"));
        return CANOPEN_ARG_ERROR;
    }

    return(CANOPEN_OK);

}


/*
   - bit timing;
   - masks;
   - filters;
   - timeouts;
   - rx buffer operation modes;
*/
/*
typedef struct
{
  CANCONFIG     CANcfg;
  CANMASK       CANmask;
  CANFILTER     CANfilter;
  CANRWTIMEOUT  CANtimeouts;
  CANRXBOPMODE  CANrxb0opmode;
  CANRXBOPMODE  CANrxb1opmode;
   
}GENCANCONFIG, *PGENCANCONFIG;
*/

CANOPENLIB_HW_API canOpenStatus __stdcall  canPortGetConfig(canPortHandle handle, PGENCANCONFIG pGENCANCONFIG)
{
    CAN_OPMODE  opMode;
    int i;

    if ( ( (HANDLE)handle != NULL )&&( (HANDLE)handle != INVALID_HANDLE_VALUE )&&
         ( pGENCANCONFIG!= NULL)
       )
    {

        // set configuration mode before
        opMode = CANGetOpMode((HANDLE)handle);
        if ( opMode != CAN_OPMODE_CONFIG )
            CANSetOpMode( (HANDLE)handle, CAN_OPMODE_CONFIG );

        if ( CANGetCNFG( (HANDLE)handle, &pGENCANCONFIG->CANcfg) == 0 )
            goto _leave_get_cfg;

        RETAILMSG(CAN_API_DBG, (L"canPortGetConfig: CANcfg:\r\n"));

        RETAILMSG(CAN_API_DBG, (L"PRSEG=0x%02X, PHSEG1=0x%02X, PHSEG2=0x%02X, SAM=0x%02X\r\n",
                                pGENCANCONFIG->CANcfg.PRSEG,
                                pGENCANCONFIG->CANcfg.PHSEG1,
                                pGENCANCONFIG->CANcfg.PHSEG2,
                                pGENCANCONFIG->CANcfg.SAM
                               ));

        RETAILMSG(CAN_API_DBG, (L"BTLMODE=0x%02X, BRP=0x%02X, SJW=0x%02X, WAKFIL=0x%02X\r\n",
                                pGENCANCONFIG->CANcfg.BTLMODE,
                                pGENCANCONFIG->CANcfg.BRP,
                                pGENCANCONFIG->CANcfg.SJW,
                                pGENCANCONFIG->CANcfg.WAKFIL
                               ));

        RETAILMSG(CAN_API_DBG, (L"\r\n"));


        pGENCANCONFIG->CANmask.Mask[0].bUpdate = TRUE;
        pGENCANCONFIG->CANmask.Mask[1].bUpdate = TRUE;
        if ( CANGetMASK( (HANDLE)handle, &pGENCANCONFIG->CANmask) == 0 )
            goto _leave_get_cfg;

        RETAILMSG(CAN_API_DBG, (L"canPortGetConfig: CANmask 0:\r\n"));
        RETAILMSG(CAN_API_DBG, (L"SID=0x%04X, EID=0x%08X, EX=%d\r\n", 
                                pGENCANCONFIG->CANmask.Mask[0].SID, 
                                pGENCANCONFIG->CANmask.Mask[0].EID,
                                pGENCANCONFIG->CANmask.Mask[0].bFISATO
                               ));

        RETAILMSG(CAN_API_DBG, (L"canPortGetConfig: CANmask 1:\r\n"));

        RETAILMSG(CAN_API_DBG, (L"SID=0x%04X, EID=0x%08X, EX=%d\r\n", 
                                pGENCANCONFIG->CANmask.Mask[1].SID, 
                                pGENCANCONFIG->CANmask.Mask[1].EID,
                                pGENCANCONFIG->CANmask.Mask[1].bFISATO
                               ));

        RETAILMSG(CAN_API_DBG, (L"\r\n"));

        for ( i = 0; i < 6; i++ )
            pGENCANCONFIG->CANfilter.Filter[i].bUpdate = TRUE;

        if ( CANGetFILTER( (HANDLE)handle, &pGENCANCONFIG->CANfilter) == 0 )
            goto _leave_get_cfg;

        for ( i = 0; i < 6; i++ )
        {
            RETAILMSG(CAN_API_DBG, (L"canPortGetConfig: CANfilter # i:\r\n", i)); 
            RETAILMSG(CAN_API_DBG, (L"SID=0x%04X, EID=0x%08X, EX=%d\r\n", 
                                    pGENCANCONFIG->CANfilter.Filter[0].SID, 
                                    pGENCANCONFIG->CANfilter.Filter[0].EID,
                                    pGENCANCONFIG->CANfilter.Filter[0].bFISATO
                                   ));
        }

        RETAILMSG(CAN_API_DBG, (L"\r\n"));


        if ( CANGetRWTO( (HANDLE)handle, &pGENCANCONFIG->CANtimeouts) == 0 )
            goto _leave_get_cfg;

        RETAILMSG(CAN_API_DBG, (L"canPortGetConfig: CANtimeouts:\r\n"));
        RETAILMSG(CAN_API_DBG, (L"read timeout = %d, write timeout = %d\r\n", 
                                pGENCANCONFIG->CANtimeouts.dwReadTimeOut, 
                                pGENCANCONFIG->CANtimeouts.dwWriteTimeOut
                               ));

        RETAILMSG(CAN_API_DBG, (L"\r\n"));


        pGENCANCONFIG->CANrxb0opmode.RxBufferID = RXB0;
        if ( CANGetRXBOPMODE( (HANDLE)handle, &pGENCANCONFIG->CANrxb0opmode) == 0 )
            goto _leave_get_cfg;

        RETAILMSG(CAN_API_DBG, (L"canPortGetConfig: CANrx0bopmode for RXB0:\r\n"));
        RETAILMSG(CAN_API_DBG, (L"mode = %d\r\n", 
                                pGENCANCONFIG->CANrxb0opmode.Mode
                               ));

        pGENCANCONFIG->CANrxb1opmode.RxBufferID = RXB1;
        if ( CANGetRXBOPMODE( (HANDLE)handle, &pGENCANCONFIG->CANrxb1opmode) == 0 )
            goto _leave_get_cfg;

        RETAILMSG(CAN_API_DBG, (L"canPortGetConfig: CANrxb1opmode for RXB1:\r\n"));
        RETAILMSG(CAN_API_DBG, (L"mode = %d\r\n", 
                                pGENCANCONFIG->CANrxb1opmode.Mode
                               ));

        RETAILMSG(CAN_API_DBG, (L"\r\n"));
    }
    else
    {
        RETAILMSG(CAN_API_DBG, (L"canPortGetConfig: Fail, Invalid params\r\n"));
        //CANSetOpMode((HANDLE)handle, opMode); 
        return CANOPEN_ARG_ERROR;
    }

    CANSetOpMode((HANDLE)handle, opMode); 
    return(CANOPEN_OK);

    _leave_get_cfg:

    RETAILMSG(CAN_API_DBG, (L"canPortGetConfig: Fail\r\n"));
    CANSetOpMode((HANDLE)handle, opMode); 
    return CANOPEN_ERROR;

}

CANOPENLIB_HW_API canOpenStatus __stdcall  canPortSetConfig(canPortHandle handle, PGENCANCONFIG pGENCANCONFIG)
{
    int  i;

    if ( ( (HANDLE)handle != NULL )&&( (HANDLE)handle != INVALID_HANDLE_VALUE )&&
         ( pGENCANCONFIG!= NULL)
       )
    {
        if ( CANSetCNFG( (HANDLE)handle, &pGENCANCONFIG->CANcfg) == 0 )
            goto _leave_set_cfg;

        pGENCANCONFIG->CANmask.Mask[0].bUpdate = TRUE;
        pGENCANCONFIG->CANmask.Mask[1].bUpdate = TRUE;

        if ( CANSetMASK( (HANDLE)handle, &pGENCANCONFIG->CANmask) == 0 )
            goto _leave_set_cfg;

        for ( i = 0; i < 6; i++ )
            pGENCANCONFIG->CANfilter.Filter[i].bUpdate = TRUE;

        if ( CANSetFILTER( (HANDLE)handle, &pGENCANCONFIG->CANfilter) == 0 )
            goto _leave_set_cfg;

        if ( CANSetRWTO( (HANDLE)handle, &pGENCANCONFIG->CANtimeouts) == 0 )
            goto _leave_set_cfg;

        pGENCANCONFIG->CANrxb0opmode.RxBufferID = RXB0;
        if ( CANSetRXBOPMODE( (HANDLE)handle, &pGENCANCONFIG->CANrxb0opmode) == 0 )
            goto _leave_set_cfg;

        pGENCANCONFIG->CANrxb1opmode.RxBufferID = RXB1;
        if ( CANSetRXBOPMODE( (HANDLE)handle, &pGENCANCONFIG->CANrxb1opmode) == 0 )
            goto _leave_set_cfg;

    }
    else
    {
        RETAILMSG(CAN_API_DBG, (L"canPortSetConfig: Fail, Invalid params\r\n"));
        return CANOPEN_ARG_ERROR;
    }

    return(CANOPEN_OK);

    _leave_set_cfg:

    RETAILMSG(CAN_API_DBG, (L"canPortSetConfig: Fail\r\n"));
    return CANOPEN_ERROR;
}


CANOPENLIB_HW_API canOpenStatus __stdcall  canPortReset(canPortHandle handle)
{
    if ( ( (HANDLE)handle != NULL )&&( (HANDLE)handle != INVALID_HANDLE_VALUE ) )
    {
        MCP2515Reset((HANDLE)handle); 
    }
    else
    {
        RETAILMSG(CAN_API_DBG, (L"canPortReset: Fail, Invalid params\r\n"));
        return CANOPEN_ARG_ERROR;
    }

    return(CANOPEN_OK);

}


CANOPENLIB_HW_API canOpenStatus __stdcall canPortEcho( canPortHandle handle, bool enabled )
{
    DWORD     dwRetVal = 0;
    DWORD     dwBytesReturned = 0;
    BOOL      bEcho;
    // IOCTL_CAN_SET_ECHO

    if ( ( (HANDLE)handle != NULL )&&( (HANDLE)handle != INVALID_HANDLE_VALUE ) )
    {
        bEcho = enabled; 
        dwRetVal = DeviceIoControl((HANDLE)handle, IOCTL_CAN_SET_ECHO, &bEcho, sizeof(BOOL), 0, 0, &dwBytesReturned, NULL);   
        if ( dwRetVal == 0 )
        {
            RETAILMSG(CAN_API_DBG, (L"canPortEcho: Fail\r\n"));
            return(CANOPEN_ERROR);
        }
    }
    else
    {
        RETAILMSG(CAN_API_DBG, (L"canPortEcho: Fail, Invalid params\r\n"));
        return CANOPEN_ARG_ERROR;
    }

    return(CANOPEN_OK);

}

/* ----------- static functions ---------------------------------------------- */

// Function name	: CANSetCNFG
// Description	    : Configures Bit Timing according CAN Bus Specification, and
//					  enables wake-up filter ( WAKFIL ) feature
// Return type		: DWORD CANSetCNFG 
// Argument         : HANDLE hDevice
// Argument         : PCANCONFIG pCNFG_IN

static DWORD CANSetCNFG(HANDLE hDevice, PCANCONFIG pCNFG_IN)
{
    DWORD       dwRetVal = 0, dwBytesReturned   = 0;
    CANCONFIG   tCNFG;


    memcpy(&tCNFG, pCNFG_IN, sizeof(CANCONFIG));

    dwRetVal = DeviceIoControl(hDevice, IOCTL_CAN_SET_BITTIMING, &tCNFG, sizeof(CANCONFIG), 0, 0, &dwBytesReturned, NULL);

    return dwRetVal;
}


// Function name	: CANGetCNFG
// Description	    : Get Configuration
// Return type		: DWORD CANGetCNFG 
// Argument         : HANDLE hDevice
// Argument         : PCANCONFIG pCNFG_OUT

static DWORD CANGetCNFG (HANDLE hDevice, PCANCONFIG pCNFG_OUT)
{
    DWORD       dwRetVal = 0, dwBytesReturned   = 0;
    CANCONFIG   tCNFG;


    memset(&tCNFG,0x00, sizeof(CANCONFIG));

    dwRetVal = DeviceIoControl(hDevice, IOCTL_CAN_GET_BITTIMING, 0, 0, &tCNFG, sizeof(CANCONFIG), &dwBytesReturned, NULL);
    if ( dwRetVal )
        memcpy(pCNFG_OUT, &tCNFG, sizeof(CANCONFIG));

    return dwRetVal;
}

// Function name	: CANSetMASK
// Description	    : Configures Mask for Rx Filters of specific Rx Buffer,
//					  achieves additional functionality of Rx Filters in packet filtering.
// Return type		: DWORD CANSetMASK 
// Argument         : HANDLE hDevice
// Argument         : PCANMASK pMASK_IN

static DWORD CANSetMASK(HANDLE hDevice, PCANMASK pMASK_IN)
{
    DWORD       dwRetVal = 0, dwBytesReturned = 0;
    CANMASK     tMASK;


    memcpy(&tMASK, pMASK_IN, sizeof(CANMASK));

    dwRetVal = DeviceIoControl(hDevice, IOCTL_CAN_SET_MASK, &tMASK, sizeof(CANMASK), 0, 0, &dwBytesReturned, NULL);

    return dwRetVal;
}


// Function name	: CANGetMASK
// Description	    : Get Mask
// Return type		: DWORD CANGetMASK 
// Argument         : HANDLE hDevice
// Argument         : PCANMASK pMASK_OUT

static DWORD CANGetMASK(HANDLE hDevice, PCANMASK pMASK_OUT)
{
    DWORD       dwRetVal = 0, dwBytesReturned   = 0;
    CANMASK     tMASK;

    memcpy(&tMASK, pMASK_OUT, sizeof(CANMASK));

    dwRetVal = DeviceIoControl(hDevice, IOCTL_CAN_GET_MASK, 0, 0, &tMASK, sizeof(CANMASK), &dwBytesReturned, NULL);
    if ( dwRetVal )
        memcpy(pMASK_OUT, &tMASK, sizeof(CANMASK));

    return dwRetVal;
}

// Function name	: CANSetFILTER
// Description	    : Configures Rx Filters of specific Rx Buffer,
//					  before incoming packet excepted it should pass this Filters with Mask.
// Return type		: DWORD CANSetFILTER 
// Argument         : HANDLE hDevice
// Argument         : PCANFILTER pFILTER_IN

static DWORD CANSetFILTER(HANDLE hDevice, PCANFILTER pFILTER_IN)
{
    DWORD       dwRetVal = 0, dwBytesReturned   = 0;
    CANFILTER   tFILTER;


    memcpy(&tFILTER, pFILTER_IN, sizeof(CANFILTER));

    dwRetVal = DeviceIoControl(hDevice, IOCTL_CAN_SET_FILTERS, &tFILTER, sizeof(CANFILTER), 0, 0, &dwBytesReturned, NULL);

    return dwRetVal;
}


// Function name	: CANGetFILTER
// Description	    : Get Rx Filter
// Return type		: DWORD CANGetFILTER 
// Argument         : HANDLE hDevice
// Argument         : PCANFILTER pFILTER_OUT

static DWORD CANGetFILTER(HANDLE hDevice, PCANFILTER pFILTER_OUT)
{
    DWORD       dwRetVal = 0, dwBytesReturned = 0;
    CANFILTER   tFILTER;

    memcpy(&tFILTER, pFILTER_OUT, sizeof(CANFILTER));

    dwRetVal = DeviceIoControl(hDevice, IOCTL_CAN_GET_FILTERS, 0, 0, &tFILTER, sizeof(CANFILTER), &dwBytesReturned, NULL);
    if ( dwRetVal )
        memcpy(pFILTER_OUT, &tFILTER, sizeof(CANFILTER));

    return dwRetVal;
}

// Function name	: CANSetRWTO
// Description	    : Configures Time Outs for ReadFile/WriteFile functions,
//					  achieves functionality similar to Serial COM port using
// Return type		: DWORD CANSetRWTO 
// Argument         : HANDLE hDevice
// Argument         : PCANRWTIMEOUT pRWTO_IN

static DWORD CANSetRWTO(HANDLE hDevice, PCANRWTIMEOUT pRWTO_IN)
{
    DWORD           dwRetVal = 0, dwBytesReturned   = 0;
    CANRWTIMEOUT    tTO;


    memcpy(&tTO, pRWTO_IN, sizeof(CANRWTIMEOUT));

    dwRetVal = DeviceIoControl(hDevice, IOCTL_CAN_SET_RWTIMEOUT, &tTO, sizeof(CANRWTIMEOUT), 0, 0, &dwBytesReturned, NULL);

    return dwRetVal;
}


// Function name	: CANGetRWTO
// Description	    : Get Time Outs
// Return type		: DWORD CANGetRWTO 
// Argument         : HANDLE hDevice
// Argument         : PCANRWTIMEOUT pRWTO_OUT

static DWORD CANGetRWTO(HANDLE hDevice, PCANRWTIMEOUT pRWTO_OUT)
{
    DWORD           dwRetVal = 0, dwBytesReturned   = 0;
    CANRWTIMEOUT    tTO;


    memset(&tTO, 0x00, sizeof(CANRWTIMEOUT));

    dwRetVal = DeviceIoControl(hDevice, IOCTL_CAN_GET_RWTIMEOUT, 0, 0, &tTO, sizeof(CANRWTIMEOUT), &dwBytesReturned, NULL);
    if ( dwRetVal )
        memcpy(pRWTO_OUT, &tTO, sizeof(CANRWTIMEOUT));

    return dwRetVal;
}

// Function name	: CANSetRXBOPMODE
// Description	    : Configure Rx Buffer Operation Mode,
//					  specifies to MCP2515 how Mask and Filters should be applied on
//					  Standard ID and Extended ID so only valid messages will be excepted
// Return type		: DWORD CANSetRXBOPMODE 
// Argument         : HANDLE hDevice
// Argument         : PCANRXBOPMODE pRXBOPMODE_IN

static DWORD CANSetRXBOPMODE(HANDLE hDevice, PCANRXBOPMODE pRXBOPMODE_IN)
{
    DWORD           dwRetVal = 0, dwBytesReturned   = 0;
    CANRXBOPMODE    tRXM;


    memcpy ( &tRXM, pRXBOPMODE_IN, sizeof ( CANRXBOPMODE ) );

    dwRetVal = DeviceIoControl(hDevice, IOCTL_CAN_SET_RXBOPMODE, &tRXM, sizeof(CANRXBOPMODE), 0, 0, &dwBytesReturned, NULL);

    return( dwRetVal );
}


// Function name	: CANGetRXBOPMODE
// Description	    : Get Rx Buffer Operational Mode
// Return type		: DWORD CANSetRXBOPMODE 
// Argument         : PCANRXBOPMODE pRXBOPMODE_OUT

static DWORD CANGetRXBOPMODE(HANDLE hDevice, PCANRXBOPMODE pRXBOPMODE_OUT)
{
    DWORD           dwRetVal = 0, dwBytesReturned = 0;
    CANRXBOPMODE    tRXM;

    memset(&tRXM, 0x00, sizeof(CANRXBOPMODE));

    dwRetVal = DeviceIoControl(hDevice, IOCTL_CAN_GET_RXBOPMODE, 0, 0, &tRXM, sizeof(CANRXBOPMODE), &dwBytesReturned, NULL);
    if ( dwRetVal )
        memcpy(pRXBOPMODE_OUT, &tRXM, sizeof(CANRXBOPMODE));

    return dwRetVal;
}

// Function name	: MCP2515Reset
// Description	    : Reset to MCP2515
//					: Reset internal Registers to Default State,
//					: sets Configuration Mode
// Return type		: DWORD MCP2515Reset 
// Argument         : HANDLE hDevice

static DWORD MCP2515Reset(HANDLE hDevice)
{
    DWORD           dwRetVal = 0, dwBytesReturned = 0;


    dwRetVal = DeviceIoControl(hDevice, IOCTL_CAN_MCP2515_RESET, 0, 0, 0, 0, &dwBytesReturned, NULL);

    return dwRetVal;
}

// Function name	: CANGetERROR
// Description	    : Returns CAN Specific Errors
// Return type		: DWORD CANGetERROR
// Argument         : HANDLE hDevice

static DWORD CANGetERROR(HANDLE hDevice)
{
    DWORD   dwBytesReturned = 0, dwERRORS = -1;


    if ( !DeviceIoControl(hDevice, IOCTL_CAN_GET_ERRORS, 0, 0, &dwERRORS, sizeof(dwERRORS), &dwBytesReturned, NULL) )
        dwERRORS = -1;

    return dwERRORS;
}

static CAN_OPMODE  CANGetOpMode(HANDLE hDevice)
{
    //return( (CAN_OPMODE)MCP2515GetModeSPI() );
    BOOL        ret;
    DWORD       dwBytesReturned;
    CAN_OPMODE  OpMode;

    ret = DeviceIoControl( hDevice, IOCTL_CAN_GET_OPMODE, NULL, 0, &OpMode,  sizeof(CAN_OPMODE), &dwBytesReturned, NULL);

    return OpMode;

}

static BOOL CANSetOpMode(HANDLE hDevice, CAN_OPMODE OpMode)
{
    BOOL    ret;
    DWORD   dwBytesReturned;

    ret = DeviceIoControl( hDevice, IOCTL_CAN_SET_OPMODE, &OpMode, sizeof(CAN_OPMODE), 0, 0, &dwBytesReturned, NULL);

    return ret;
}

int WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    return 1;
} 

