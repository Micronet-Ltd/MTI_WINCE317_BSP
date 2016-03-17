/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/

#include <windows.h>

#include <ndis.h>
#include <Nuiouser.h> 

#include "cu_osapi.h"
#include "STADExternalIf.h"
#include "oserr.h"


#define OID_TI_WILINK_IOCTL         0xFF010200 
#define MAX_MEMORY_SIZE             (16*1024) 

#define WLAN_DEVICE_NAME 			(_T("TIWLNAPI1"))


THandle IpcSta_Create(const PS8 device_name)
{
	return (THandle) 1;
}


VOID IpcSta_Destroy(THandle hIpcSta)
{
}


S32 IpcSta_Send_Oid ( ULONG oid, VOID* pInBuffer, ULONG uInBufSize,
                        VOID* pOutBuffer, ULONG uOutBufSize, ULONG* puBytesReturned )
{
    BOOL  rc = 0;
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbBuffer;
    DWORD dwBytesReturned = 0;

    HANDLE hNdisuio;
    PNDISUIO_QUERY_OID pQueryOid;

    if (uInBufSize > MAX_MEMORY_SIZE || uOutBufSize > MAX_MEMORY_SIZE)
    {
        DEBUGMSG(1, (TEXT("tiIoCtrl: Buffer specified is too large to transport\n")));
        return ERROR_GEN_FAILURE;
    }

    hNdisuio = CreateFile( NDISUIO_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hNdisuio == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        DEBUGMSG(1, (TEXT("tiIoCtrl: Could not establish connection with NDISUIO, error=%08x\n"), dwError));
    }
    else
    {
        if (uInBufSize > uOutBufSize)
            cbBuffer = sizeof(NDISUIO_QUERY_OID) + uInBufSize;
        else
            cbBuffer = sizeof(NDISUIO_QUERY_OID) + uOutBufSize;

        if ((pQueryOid = (PNDISUIO_QUERY_OID)malloc(cbBuffer)) != NULL)
        {
            pQueryOid->Oid = oid;
            pQueryOid->ptcDeviceName = WLAN_DEVICE_NAME;

            // if there is data to pass to the driver, append it beyond QueryOid header
            if (uInBufSize != 0)
                memcpy((pQueryOid+1), pInBuffer, uInBufSize);

            SetLastError(ERROR_SUCCESS);

            dwBytesReturned = 0;

            rc = DeviceIoControl(hNdisuio, IOCTL_NDISUIO_QUERY_OID_VALUE,
                pQueryOid, cbBuffer, pQueryOid,  cbBuffer, 
                &dwBytesReturned, NULL);

            if (!rc)
            {
                dwError = GetLastError();
                DEBUGMSG(1, (TEXT("tiIoCtrl: DeviceIoControl() failed, error=%08x\n"), dwError));
            }
            else
            {

                // Get the response from the driver
                if (pOutBuffer)
                {
                    // NDISUIO returns the QUERY_OID header with an apparent sequence
                    // number in the ptcDeviceName field.
                    cbBuffer = dwBytesReturned - ((DWORD)(&pQueryOid->Data[0]) - (DWORD)pQueryOid);

                    // limit copy to what size caller requested
                    if (cbBuffer > uOutBufSize)
                        cbBuffer = uOutBufSize;

                    if (cbBuffer > 0)
                    {
                        memcpy(pOutBuffer, (void *)&pQueryOid->Data[0], cbBuffer);
                        *puBytesReturned = cbBuffer;
                    }
                }
            }

            free ((void *)pQueryOid);
        }
        else
            dwError = GetLastError();

        CloseHandle(hNdisuio);
    }

    return dwError;
} 


S32 IPC_STA_Private_Send(THandle hIpcSta, U32 ioctl_cmd, PVOID bufIn, U32 sizeIn, 
                                                PVOID bufOut, U32 sizeOut)

{
    BOOL  rc = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbBuffer;
    DWORD dwBytesReturned = 0;

    HANDLE hNdisuio;
    PNDISUIO_QUERY_OID pQueryOid;

	ti_private_cmd_t* pcmd;

    if (sizeIn > MAX_MEMORY_SIZE || sizeOut > MAX_MEMORY_SIZE)
    {
        DEBUGMSG(1, (TEXT("tiIoCtrl: Buffer specified is too large to transport\n")));
        return ERROR_GEN_FAILURE;
    }

    hNdisuio = CreateFile( NDISUIO_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hNdisuio == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        DEBUGMSG(1, (TEXT("tiIoCtrl: Could not establish connection with NDISUIO, error=%08x\n"), dwError));
    }
    else
    {
        if (sizeIn > sizeOut)
            cbBuffer = sizeof(NDISUIO_QUERY_OID) + sizeof(ti_private_cmd_t) + sizeIn;
        else
            cbBuffer = sizeof(NDISUIO_QUERY_OID) + sizeof(ti_private_cmd_t) + sizeOut;

        if ((pQueryOid = (PNDISUIO_QUERY_OID)malloc(cbBuffer)) != NULL)
        {
            pQueryOid->Oid = OID_TI_WILINK_IOCTL;
            pQueryOid->ptcDeviceName = WLAN_DEVICE_NAME;

			pcmd = (ti_private_cmd_t*) &pQueryOid->Data[0];

			pcmd->cmd = ioctl_cmd;

			if(bufOut == NULL)
				pcmd->flags = PRIVATE_CMD_SET_FLAG;
			else
				pcmd->flags = PRIVATE_CMD_GET_FLAG;

			pcmd->in_buffer = bufIn;
			pcmd->in_buffer_len = sizeIn;
			pcmd->out_buffer = bufOut;
			pcmd->out_buffer_len = sizeOut;

            // if there is data to pass to the driver, append it beyond QueryOid header
            if (sizeIn != 0)
                memcpy((PUCHAR)(pQueryOid + 1) + sizeof(ti_private_cmd_t) - sizeof(UINT32), bufIn, sizeIn);

            SetLastError(ERROR_SUCCESS);

            dwBytesReturned = 0;

            rc = DeviceIoControl(hNdisuio, IOCTL_NDISUIO_QUERY_OID_VALUE,
                pQueryOid, cbBuffer, pQueryOid,  cbBuffer, 
                &dwBytesReturned, NULL);

            if (!rc)
            {
                dwError = GetLastError();
                DEBUGMSG(1, (TEXT("tiIoCtrl: DeviceIoControl() failed, error=%08x\n"), dwError));
            }
            else
            {

                // Get the response from the driver
                if (bufOut)
                {
                    // NDISUIO returns the QUERY_OID header with an apparent sequence
                    // number in the ptcDeviceName field.
                    cbBuffer = dwBytesReturned - ((DWORD)(&pQueryOid->Data[0]) - (DWORD)pQueryOid);

                    // limit copy to what size caller requested
                    if (cbBuffer > sizeOut)
                        cbBuffer = sizeOut;

                    if (cbBuffer > 0)
                    {
                        memcpy(bufOut, (void *)&pQueryOid->Data[0], cbBuffer);
//                        *puBytesReturned = cbBuffer;
                    }
                }
            }

            free ((void *)pQueryOid);
        }
        else
            dwError = GetLastError();

        CloseHandle(hNdisuio);
    }

	if(dwError != ERROR_SUCCESS) {
		//os_error_printf(CU_MSG_ERROR, "ERROR. see driver prints or logs for details\n");
		return EOALERR_IPC_STA_ERROR_SENDING_WEXT;
	}

	return OK;
}
