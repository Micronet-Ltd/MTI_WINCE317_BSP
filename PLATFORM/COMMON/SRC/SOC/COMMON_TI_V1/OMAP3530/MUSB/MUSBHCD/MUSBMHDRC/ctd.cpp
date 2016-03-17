// All rights reserved ADENEO EMBEDDED 2010
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
// 
// Module Name:  
//     CTd.cpp
// 
// Abstract: Provides interface to MUSBMHDRC host controller
// 
// Notes: 
//

#pragma warning(push)
#pragma warning(disable : 4510 4512 4610)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <pm.h>
#include <omap3530.h>
#include <oal_io.h>
#include <initguid.h>

#undef ZONE_INIT
#undef ZONE_WARNING
#undef ZONE_ERROR
#include "ctd.h"
#include "trans.h"
#include "cpipe.h"
#include "cmhcd.h"
#pragma warning(pop)

#undef ZONE_DEBUG
#define ZONE_DEBUG 0
//**************************************************************************************
CQTD::CQTD( CQTransfer * pTransfer, CQH * pQh)
: m_pTrans(pTransfer)
, m_pQh(pQh)
//
//  Purpose: Constructor for CQTD
//
//  Parameter:  pTransfer - CQTransfer object to be transferred
//              pQh - Queue Head
//
//**************************************************************************************
{
    m_dwTDType = TD_IDLE;
    m_pNext=NULL;    
    m_cbTransferred = 0;
    m_dwStatus = STATUS_IDLE;
    m_dwError = 0;
}

//**************************************************************************************
DWORD  CQTD::ReadDataIN(DWORD *pdwMore)
//
//  Purpose: Read the data from FIFO or DMA
//
//  Parameter: pdwMore - pointer to dword indicate the status
//                       refer to include to see list of available status
//
//  Return: TRUE
{
    PHSMUSB_T pOTG;
    CPipe *pPipe = m_pQh->GetPipe();            
    BYTE *pvData;
    BYTE *ppData;
    DWORD dwData;
    DWORD count;
    int iRetSize = 0;
    DWORD dwError = USB_NO_ERROR;    
    BOOL fClearRxPkt = TRUE;
    DWORD status = GetStatus();

    
    *pdwMore = MUSB_READ_SUCCESS;
    if (!pPipe)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CQTD::ReadDataIN -> not able to read the associated pipe\r\n")));        
        goto READ_EXIT;
    }
    pOTG = (PHSMUSB_T)pPipe->m_pCMhcd->GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CQTD::ReadDataIN: Get pOTG failed\r\n")));        
        goto READ_EXIT;
    }

    pvData = (BYTE *)GetVAData();
    pvData = pvData + GetTotTfrSize();
    ppData = (BYTE *)GetPAData();
    ppData = ppData + GetTotTfrSize();
    dwData = GetDataSize() - GetTotTfrSize();

    UCHAR endpoint = pPipe->GetMappedEndPoint();       

    DEBUGMSG(ZONE_HCD, (TEXT("Total=%d, Start=%d, Remain=%d\r\n"), GetDataSize(), GetTotTfrSize(), dwData));
    DEBUGMSG(ZONE_HCD, (TEXT("ReadDataIN: Total=%d, Start=%d, Remain=%d, CurTfrSize=%d\r\n"), 
        GetDataSize(), GetTotTfrSize(), dwData, GetCurTfrSize()));

    dwError = pPipe->m_pCMhcd->CheckRxCSR(endpoint);
    if (dwError != USB_NO_ERROR)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("Failed in checkRxCSR\r\n")));
        SetCurTfrSize(0);
        return dwError;
    }
    // Read the FIFO now
    if ((pPipe->GetTransferMode() != TRANSFER_FIFO) && (pPipe->GetTransferType() != USB_ENDPOINT_TYPE_CONTROL) && (dwData != 0))
    {
        if (status == STATUS_WAIT_RESPONSE)
        {
        	DWORD dwCount;
			UCHAR csrIndex = INDEX(endpoint);
			dwCount = INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].Count.RxCount);
			if (dwCount == 0){
//                RETAILMSG(1, (L"CQTD::ReadDataIN ZERO_LENGTH PACKET\r\n"));
				SetCurTfrSize(GetDataSize() - GetTotTfrSize());
				SetTotTfrSize(GetTotTfrSize() + (DWORD)GetCurTfrSize());
				goto READ_EXIT;
			}
			
		
            // Actually need to check if the RxCSR has error
            UCHAR channel = pPipe->m_pCMhcd->AcquireDMAChannel(pPipe);
            if (channel == 0xff) // Not able to get the DMA channel, use FIFO instead.
            {
                RETAILMSG(1, (TEXT("DMA Channel failed, use FIFO to read\r\n")));
                goto PROCESS_READ_FIFO;
            }


            if (pPipe->GetTransferMode() == TRANSFER_DMA1)
            {
                // This is a hack as mode 1 not working well.
                if ((dwData % GetPacketSize() == 0) && (dwData > GetPacketSize()))
                    dwData = dwData - 8; // Just minus 8 bytes i.e. it would have the last packet ACK
            }
            else
                dwData = GetPacketSize();

            *pdwMore = MUSB_WAIT_DMA_COMPLETE;
            count = pPipe->m_pCMhcd->ReadDMA((void *)pOTG, endpoint, channel, 
                         (void *)ppData, dwData, GetPacketSize(), this);       
            return USB_NO_ERROR;
        }
        else if ((status == STATUS_WAIT_DMA_0_RD_FIFO_COMPLETE) ||
            (status == STATUS_WAIT_DMA_1_RD_FIFO_COMPLETE))
        {           
            UCHAR channel = pPipe->m_pCMhcd->DeviceInfo2Channel(pPipe);                
            dwError = pPipe->m_pCMhcd->CheckDMAResult((void *)pOTG, channel);
            DEBUGMSG(ZONE_HCD, (TEXT("DMA IN success with status(0x%x), channel(0x%x), endpoint(%d)\r\n"),
                status, channel, USB_ENDPOINT(endpoint)));
            pPipe->m_pCMhcd->RestoreRxConfig((void *)pOTG, endpoint);            
        }
    }
    else
    {
PROCESS_READ_FIFO:
        DEBUGMSG(ZONE_TRANSFER, (TEXT("Read FIFO for MappedEP %d start\r\n"), USB_ENDPOINT(endpoint)));
        dwError = pPipe->m_pCMhcd->ReadFIFO((void *)pOTG, endpoint, (void *)pvData, dwData, &iRetSize);
        if (dwError == USB_NO_ERROR)
        {
            SetCurTfrSize(iRetSize);            
        }
        else
        {
            SetCurTfrSize(0);
        }
        DEBUGMSG(ZONE_HCD, (TEXT("Read FIFO for mapped EP %d success return size %d\r\n"), 
            USB_ENDPOINT(endpoint), iRetSize));
    }
    if (dwError != USB_NO_ERROR)
    {        
        DEBUGMSG(1, (TEXT("ReadFIFO error which suppose to read total %d bytes error 0x%x\r\n"), GetDataSize(), dwError));
        return dwError;
    }
    
    SetTotTfrSize(GetTotTfrSize() + (DWORD)GetCurTfrSize());
    // Control endpoint (message type), we can use exactly data buffer to wait
    // It seems that it doesn't send out zero data length to you ...
    // so, you should just check the data size.
    // transfer should be considered complete when short packets and zero length packets
    if (!((GetTotTfrSize() >= GetDataSize()) || (GetCurTfrSize() < GetPacketSize())))
      *pdwMore = MUSB_READ_MORE_DATA;

    // For mode 1, we need to check and see if there is more data
    if ((pPipe->GetTransferMode() != TRANSFER_FIFO) && (status == STATUS_WAIT_DMA_1_RD_FIFO_COMPLETE))
    {
        // Don't clear the RxPktRdy in case of RD 1 mode as it is auto clear already.
        fClearRxPkt = FALSE;

        // No need to worry about the RxCount at this point, since the DMA is too fast.
        // Just set data available and let the IssueTransfer check if RxCount.
        if (*pdwMore == MUSB_READ_MORE_DATA) // there is residue data, need to DMA again with mode 0
        {                 
            SetStatus(STATUS_WAIT_RESPONSE);
            ReadDataIN(pdwMore);            
            DEBUGMSG( ZONE_HCD|ZONE_DEBUG, (TEXT("Read the last block of data for ep %d, mapped %d\r\n"), 
                USB_ENDPOINT(endpoint), USB_ENDPOINT(pPipe->GetMappedEndPoint())));
        }
        else // Check and see if there is any thing inside the FIFO
        {
            pPipe->m_pCMhcd->SetRxDataAvail(USB_ENDPOINT(endpoint));
            DEBUGMSG(ZONE_HCD, (TEXT("DataAvail(%d) = 0x%x\r\n"), USB_ENDPOINT(endpoint), 
                    pPipe->m_pCMhcd->GetRxDataAvail(USB_ENDPOINT(endpoint))));        
        }
    }
    
    DEBUGMSG(ZONE_HCD, (TEXT("DataBuffer(0x%x), TfrSze(0x%x), Receive(0x%x), PacketSize(0x%x)\r\n"), 
        GetDataSize(),GetTotTfrSize(), iRetSize, GetPacketSize()));
    if (GetTotTfrSize() > GetDataSize())
        DEBUGMSG(ZONE_ERROR, (TEXT("Something wrong: receive more data than expected\r\n")));

READ_EXIT:
    if (fClearRxPkt) 
        ClearRxPktRdy();
    return USB_NO_ERROR;
}

//*************************************************************************************
void CQTD::ClearRxPktRdy(void)
//
//  Purpose: Clear the RxPktRdy bit for corresponding endpoint
//
//  Return: Nil
//
{
    PHSMUSB_T pOTG;
    UCHAR endpoint;
    UCHAR csrIndex;
    CPipe *pPipe = m_pQh->GetPipe();        
    if (!pPipe)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CQTD::ClearRxPktRdy -> not able to read the associated pipe\r\n")));
        return;
    }
    pOTG = (PHSMUSB_T)pPipe->m_pCMhcd->GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CQTD::ClearRxPktRdy: Get pOTG failed\r\n")));        
        return;
    }

    endpoint = pPipe->GetMappedEndPoint();
    // Set Index Register
    EnterCriticalSection(&pOTG->regCS);    
    OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));
    csrIndex = INDEX(endpoint);
    
    if (endpoint == 0) // Endpoint 0 handling
    {
        CLRREG16(&pOTG->pUsbCsrRegs->ep[0].CSR.CSR0, CSR0_H_RxPktRdy|CSR0_H_StatusPkt|CSR0_H_SetupPkt|CSR0_H_ReqPkt);        
    }
    else
    {
        DEBUGMSG(ZONE_HCD, (TEXT("ClearRxPktRdy bit for ep %d\r\n"), USB_ENDPOINT(endpoint)));
        SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_FlushFIFO);
        CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_RxPktRdy); 
        DEBUGMSG(ZONE_HCD|0, (TEXT("RxCSR[%d] = 0x%x\r\n"), csrIndex, INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR)));
    }    
    LeaveCriticalSection(&pOTG->regCS);
    return;
}
//*************************************************************************************
void CQTD::DumpReg()
//
//  Purpose: Dump the register
//
//  Parameter: Nil
//
//  Return : nil
{
    PHSMUSB_T pOTG;
    UCHAR endpoint;
    UCHAR csrIndex;
    DWORD dwType = GetTDType();        
    CPipe *pPipe = m_pQh->GetPipe();        
    if (!pPipe)
    {
        RETAILMSG(1, (TEXT("CQTD::ClearRxPktRdy -> not able to read the associated pipe\r\n")));
        return;
    }
    pOTG = (PHSMUSB_T)pPipe->m_pCMhcd->GetOTGContext();
    if (pOTG == NULL)
    {
        RETAILMSG(1, (TEXT("CQTD::ClearRxPktRdy: Get pOTG failed\r\n")));        
        return;
    }

    endpoint = pPipe->GetMappedEndPoint();
    // Set Index Register
    EnterCriticalSection(&pOTG->regCS);
    RETAILMSG(1, (TEXT("DumpReg CriticalSection: IN\r\n")));
    OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));
    csrIndex = INDEX(endpoint);
    if (USB_ENDPOINT(endpoint) == 0)
        RETAILMSG(1, (TEXT("DumpReg::CSR0 register = 0x%x\r\n"), INREG16(&pOTG->pUsbCsrRegs->ep[0].CSR.CSR0)));
    else {
        if (dwType == TD_DATA_IN)
        {
            RETAILMSG(1, (TEXT("DumpReg::RXCSR (%d) register = 0x%x\r\n"), endpoint, 
                INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR)));  
        }
        else
        {
            RETAILMSG(1, (TEXT("DumpReg::TXCSR (%d) register = 0x%x\r\n"), endpoint, 
                INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR)));
        }
    }
    RETAILMSG(1, (TEXT("DumpReg CriticalSection: OUT\r\n")));
    LeaveCriticalSection(&pOTG->regCS);
}
//**************************************************************************************
CQTD * CQTD::QueueNextTD(CQTD * pNextTD)
//
//  Purpose: Queue the TD for the next td
//
//  Parameter: pNextTD - TD to be queue up
//
//  Return:
//      pointer to the last TD before adding
//***************************************************************************************
{    
    CQTD * pReturn = m_pNext;    
    m_pNext = pNextTD;        
    return pReturn;
}

//**********************************************************************************
void CQTD::DeActiveTD()
//
//  Purpose:    Deactivate the TD
//
//  Parameter:  Nil
//
//  Return:     Nil
//
{    
    PHSMUSB_T pOTG;
    UCHAR endpoint;
    UCHAR csrIndex;
    DWORD dwType = GetTDType();        

    m_pQh->Lock();
    SetStatus(STATUS_ABORT);
    CPipe *pPipe = m_pQh->GetPipe();        
    if (!pPipe)
    {
        RETAILMSG(1, (TEXT("CQTD::ClearRxPktRdy -> not able to read the associated pipe\r\n")));
        return;
    }
    pOTG = (PHSMUSB_T)pPipe->m_pCMhcd->GetOTGContext();
    if (pOTG == NULL)
    {
        RETAILMSG(1, (TEXT("CQTD::ClearRxPktRdy: Get pOTG failed\r\n")));        
        return;
    }

    endpoint = pPipe->GetMappedEndPoint();
    // Set Index Register
    EnterCriticalSection(&pOTG->regCS);
    OUTREG8(&pOTG->pUsbGenRegs->Index, INDEX(endpoint));
    csrIndex = INDEX(endpoint);
    if (USB_ENDPOINT(endpoint) == 0)
    {
        RETAILMSG(1, (TEXT("CSR0 register = 0x%x\r\n"), INREG16(&pOTG->pUsbCsrRegs->ep[0].CSR.CSR0)));
        SETREG16(&pOTG->pUsbCsrRegs->ep[0].CSR.CSR0, CSR0_H_FlushFIFO);
    }
    else {
        if (dwType == TD_DATA_IN)
        {
            SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_FlushFIFO|RXCSR_H_ClrDataTog);
            CLRREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR, RXCSR_H_ReqPkt);
            DEBUGMSG(1, (TEXT("FlushFIFO::RXCSR (%d) register = 0x%x\r\n"), endpoint, 
                INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].RxCSR)));
            DEBUGMSG(1, (TEXT("FlushFIFO: RxCount(0x%x)\r\n"), INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].Count.RxCount)));


        }
        else
        {
            RETAILMSG(1, (TEXT("FlushFIFO::TXCSR (%d) register = 0x%x\r\n"), endpoint, 
                INREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR)));
            SETREG16(&pOTG->pUsbCsrRegs->ep[csrIndex].CSR.TxCSR, TXCSR_H_FlushFIFO|TXCSR_H_ClrDataTog);
        }
    }    
    LeaveCriticalSection(&pOTG->regCS);

    // Need to add FlushFIFO here....    
    m_pQh->UnLock();
}

//**********************************************************************************
CQH::CQH(CPipe *pPipe)
    : m_pPipe(pPipe)
    , m_pCQHTDList(NULL)    
//
//  Purpose: Constructor for the Queue Head class.
//
//  Parameter: pPipe - The associated pipe used (one queue head matches to one pipe
//
//  Returns: Nothing
// 
//  Notes:
//**********************************************************************************
{    
    USB_ENDPOINT_DESCRIPTOR endptDesc = pPipe->GetEndptDescriptor();
    m_bEndPointAddress = endptDesc.bEndpointAddress;
    InitLock();
}
//***********************************************************************************
BOOL CQH::IsActive()
//
//  Purpose: Check and see if the CQH has been completed
//
//  Parameter: Nil
//
//  Return values: TRUE - processing, FALSE - not active, no transaction
//
//***********************************************************************************
{
    // Loop through and see if there is any outstanding CQTD
    if (m_pCQHTDList == NULL) // there is nothing in the QH, we can return NOT active
        return FALSE;

    CQTD *pTmp = m_pCQHTDList;
    while (pTmp != NULL)
    {
        if ((pTmp->GetStatus() > STATUS_ACTIVE_START) && (pTmp->GetStatus() < STATUS_ACTIVE_END))
        {
            return TRUE;
        }

        pTmp = pTmp->GetNextTD();
    }

    return FALSE;
}
//***********************************************************************************
DWORD CQH::IssueTransfer(CQTD *pCurTD)
//
//  Purpose: Perform actual putting the data into FIFO and save it into the Queue Head
//
//  Parameter: pCurTD - TD to be processed and sent/received.
//
//  Return values: 
//      status
//
//************************************************************************************
{
    DWORD dwStatus = STATUS_ERROR;
    if (pCurTD)
    {        
        if ((m_pCQHTDList != NULL) && (m_pCQHTDList != pCurTD))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("There are something wrong with sequence\r\n")));
            dwStatus = STATUS_ERROR;
            goto EXIT;
        }
        else if (m_pCQHTDList == NULL)
            m_pCQHTDList = pCurTD;

        // Perform the FIFO or DMA transfer here
        // pCurTD should be linked-list of the TD to be transferred.
        CQTD *pTmp;

        pTmp = m_pCQHTDList;
        dwStatus = ERROR_SUCCESS;
        while (pTmp != NULL)
        {
            // 1. Find the TD that is not processed yet
            // 2. Put in the FIFO
            // 3. Set the status and tx or rx bit
            // 4. Exit and wait for packet tx or rx interrupt.
            DWORD dwPrevStatus;
            DEBUGMSG(ZONE_HCD, (TEXT("CQTD with type(%d), status(%d)\r\n"), pTmp->GetTDType(), pTmp->GetStatus()));
            if (pTmp->GetStatus() == STATUS_COMPLETE)
            {
                pTmp = pTmp->GetNextTD();
                continue;
            }

            dwPrevStatus = pTmp->GetStatus();
            if ((dwPrevStatus == STATUS_WAIT_TRANSFER) ||
                (dwPrevStatus == STATUS_WAIT_MORE_DATA) ||
                (dwPrevStatus == STATUS_SEND_MORE_DATA) ||
                (dwPrevStatus == STATUS_PROCESS_DMA))
            {                              
                if (((dwPrevStatus == STATUS_WAIT_TRANSFER) || 
                     (dwPrevStatus == STATUS_WAIT_MORE_DATA)||
                     (dwPrevStatus == STATUS_SEND_MORE_DATA)) && 
                    (USB_ENDPOINT(m_bEndPointAddress) != 0) &&
                    (USB_ENDPOINT_DIRECTION_OUT(m_bEndPointAddress)) &&
                    (m_pPipe->GetTransferMode() != TRANSFER_FIFO))
                        pTmp->SetStatus(STATUS_PROCESS_DMA);
                else                 
                        pTmp->SetStatus(STATUS_WAIT_RESPONSE);

                // Configure Endpoint here ... not sure if it should
                // be the case.
                // Put the data into FIFO       
                //RETAILMSG(1, (TEXT("IssueTransfer::EP %d\r\n"), USB_ENDPOINT((m_pPipe->GetEndptDescriptor()).bEndpointAddress)));
                DEBUGMSG(ZONE_HCD, (TEXT("IssueTransfer:EP %d, mapped %d\r\n"), 
                    USB_ENDPOINT((m_pPipe->GetEndptDescriptor()).bEndpointAddress), m_pPipe->GetMappedEndPoint()));
                m_pPipe->m_pCMhcd->InitFIFO(m_pPipe);
#pragma warning(push)
#pragma warning(disable : 4238)
                m_pPipe->m_pCMhcd->ConfigEP(&(m_pPipe->GetEndptDescriptor()), 
                    m_pPipe->GetMappedEndPoint(), m_pPipe->GetDeviceAddress(), m_pPipe->m_bHubAddress,
                    m_pPipe->m_bHubPort, m_pPipe->GetSpeed(), m_pPipe->GetTransferMode(), FALSE);
#pragma warning(pop)
                m_pPipe->m_pCMhcd->ProcessTD(m_pPipe->GetMappedEndPoint(),(void *)pTmp);                                      
                break; // jump out from the while loop
            }
            else 
            {
                //DEBUGMSG(ZONE_ERROR, (TEXT("GetStatus return %d for this\r\n"), pTmp->GetStatus()));
                // I guess we don't need to continue loop but break it.
                dwStatus = ERROR_NOT_READY;
                break;
            }
               
        }          
        // Finally, check if the current list in m_pQTDList has been completed.
        if (pTmp == NULL)
        {
            dwStatus = ERROR_SUCCESS;
            m_pCQHTDList = NULL;
        }
    }
EXIT:    
    return dwStatus;
}

//************************************************************************************
void CQH::SetDeviceAddress(UCHAR ucDeviceAddress, UCHAR outdir)
//
//  Purpose: Set the target device address
//
//  Parameter:  dwDeviceAddress - the device address
//              outdir - is it out endpoint?
//
//  Return: nil
//
//*************************************************************************************
{
    // Need to handle HUB case later on
    PHSMUSB_T pOTG;
    CPipe *pPipe = GetPipe();        
    if (!pPipe)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CQTD::ClearRxPktRdy -> not able to read the associated pipe\r\n")));
        return;
    }
    pOTG = (PHSMUSB_T)pPipe->m_pCMhcd->GetOTGContext();
    if (pOTG == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CQTD::ClearRxPktRdy: Get pOTG failed\r\n")));        
        return;
    }

    m_pPipe->m_pCMhcd->SetDeviceAddress(pPipe->GetMappedEndPoint(), ucDeviceAddress, 
        pPipe->m_bHubAddress,pPipe->m_bHubPort, outdir);     
    
}

//**************************************************************************************
void CQH::SetMaxPacketLength(DWORD dwMaxPacketSize)
//
//  Purpose: Setting the max packet length
//
//  Parameter: packet size
//
//  Return: nil
//***************************************************************************************
{
    // To be implemented
#ifdef DEBUG
    DEBUGMSG(ZONE_HCD, (TEXT("Handle SetMaxPacketLength request (%d)\r\n"), dwMaxPacketSize));
#else
    UNREFERENCED_PARAMETER(dwMaxPacketSize);
#endif
}
