/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2007 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

  In addition to civil penalties for infringement of copyright under appli-
  cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
  of (a) the restrictions on circumvention of copyright protection systems
  found in 17 U.S.C. 1201 and (b) the protections for the integrity of
  copyright management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
  IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
  OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
  BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
  PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.

  IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
  CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
---------------------------------------------------------------------------*/

/************************************************************************
    PCI.H              Created 09/10/97 PKG
    Copyright (c) 1996-2002 Datalight, Inc.
    Confidential and Proprietary
    All Rights Reserved

    $DOCMODULE
    "Real Mode PCI BIOS header and define"
    Provides the include file for the PCI module.

    $DOCHISTORY
    02/07/02 DE  Updated copyright notice for 2002.
  ************************************************************************/
#ifndef __PCI_H__
#define __PCI_H__

#define PCIMEMORY    (0)
#define PCIIO        (1)

#define LOCATEIN32BIT      (0)
#define LOCATEBELOW1MB     (1)
#define LOCATEIN64BIT      (2)


#define PCI_SUCCESSFUL              (0x00)
#define PCI_FUNCTION_NOT_SUPPORTED  (0x81)
#define PCI_BAD_VENDOR_ID           (0x82)
#define PCI_DEVICE_NOT_FOUND        (0x83)
#define PCI_BAD_REGISTER_NUMBER     (0x84)
#define PCI_SET_FAILED              (0x85)
#define PCI_BUFFER_TOO_SMALL        (0x86)


#define PCIFUNCTIONID            (0xB1)
#define GETPCIBIOSPRESENT        (0x01)
#define FINDPCIDEVICE            (0x02)
#define FINDPCICLASS             (0x03)
#define GENERATEPCICYCLE         (0x06)
#define READPCIBYTE              (0x08)
#define READPCIWORD              (0x09)
#define READPCIDWORD             (0x0a)
#define WRITEPCIBYTE             (0x0b)
#define WRITEPCIWORD             (0x0c)
#define WRITEPCIDWORD            (0x0d)
#define GETPCIINTROUTING         (0x0e)
#define SETPCIHARDWAREINTERRUPT  (0x0f)

#define VENDORID_WORD            (0)
#define DEVICEID_WORD            (2)
#define PCICOMMANDREGISTER       (4)
#define CLASSREVISION_DWORD      (8)
#define TIMERCACHE_WORD          (12)
#define BISTHEADER_WORD          (14)
#define BASE0_DWORD              (16)
#define BASE1_DWORD              (20)
#define BASE2_DWORD              (24)
#define BASE3_DWORD              (28)
#define BASE4_DWORD              (32)
#define BASE5_DWORD              (34)
#define CIS_DWORD                (30)
#define SUBSYSTEMVID_WORD        (44)
#define SUBSYSTEMID_WORD         (46)
#define ROMBASE_DWORD            (48)
#define INTERRUPT_WORD           (60)
#define MAXMIN_WORD              (62)

#define MEMORYACCESSENABLE       (0x2)

D_UCHAR         FindPciBios(
    D_UINT16 * version,
    D_UCHAR * lastbus,
    D_UCHAR * hwcharacteristics);
D_UCHAR         FindPciDevice(
    D_UINT16 vendorid,
    D_UINT16 deviceid,
    D_UINT16 deviceindex,
    D_UCHAR * busnumber,
    D_UCHAR * devicefn);
D_UCHAR         ReadPciConfigDWord(
    D_UCHAR devicefn,
    D_UCHAR busnumber,
    D_UINT16 registernumber,
    D_UINT32 * datadword);
D_UCHAR         WritePciConfigDWord(
    D_UCHAR devicefn,
    D_UCHAR busnumber,
    D_UINT16 registernumber,
    D_UINT32 datadword);

#endif /* __PCI_H__ */
