/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

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

/*---------------------------------------------------------------------------
                                Description

    This is a stubbed implementation of the DCL network services.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify the
    make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osnet.c $
    Revision 1.2  2009/07/15 07:28:57Z  keithg
    Corrected socket address type name for consistency.
    Revision 1.1  2009/07/15 04:47:04Z  keithg
    Initial revision
    Revision 1.1  2009/03/20 00:42:22Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlnet.h>


/*-------------------------------------------------------------------
    Protected: DclNetSocket()

    Creates an endpoint for network communication.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
DCLNETSOCKET DclNetSocket(
    int af,
    int type,
    int protocol)
{
    (void)af;
    (void)type;
    (void)protocol;
    DclProductionError();
    return -1;
}


/*-------------------------------------------------------------------
    Protected: DclNetSetSockOpt()

    Sets a created network socket's option value.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
int DclNetSetSockOpt(
    DCLNETSOCKET socket,
    int level,
    int optname,
    const char * optval,
    int optlen)
{
    (void)socket;
    (void)level;
    (void)optname;
    (void)optval;
    (void)optlen;
    DclProductionError();
    return 0;
}


/*-------------------------------------------------------------------
    Protected: DclNetCloseSocket()

    Close a previously created and open network socket.

    Parameters:

    Return Value:

-------------------------------------------------------------------*/
int DclNetCloseSocket(
    DCLNETSOCKET socket)
{
    (void)socket;
    DclProductionError();
    return 0;
}


/*-------------------------------------------------------------------
    Protected: DclNetSend()

    Sends data across a connected network socket.

    Parameters:

    Return Value:

-------------------------------------------------------------------*/
int DclNetSend(
    DCLNETSOCKET socket,
    const char * buf,
    int len,
    int flags)
{
    (void)socket;
    (void)buf;
    (void)len;
    (void)flags;
    DclProductionError();
    return 0;
}

/*-------------------------------------------------------------------
    Protected: DclNetRecv()

    Recieves data from a connected network socket.

    Parameters:

    Return Value:

-------------------------------------------------------------------*/
int DclNetRecv(
    DCLNETSOCKET socket,
    char * buf,
    int len,
    int flags)
{
    (void)socket;
    (void)buf;
    (void)len;
    (void)flags;
    DclProductionError();
    return 0;
}


/*-------------------------------------------------------------------
    Protected: DclNetConnect()

    Establishes a connection to the given network socket.

    Parameters:

    Return Value:

-------------------------------------------------------------------*/
int DclNetConnect(
    DCLNETSOCKET socket,
    const DCLNETSOCKADDR * name,
    int namelen)
{
    (void)socket;
    (void)name;
    (void)namelen;
    DclProductionError();
    return 0;
}


/*-------------------------------------------------------------------
    Protected: DclNetHtons()

    Converts a 16-bit host integer to network endian order.

    Parameters:

    Return Value:

-------------------------------------------------------------------*/
D_UINT16 DclNetHtons(
    D_UINT16 hostshort)
{
    /*  TODO: Use DCL Endian conversions if necessary
    */

    (void)hostshort;
    DclProductionError();
    return 0;
}


/*-------------------------------------------------------------------
    Protected: DclNetGetHostByAddr()

    Retrieves host information for the given address.

    Parameters:

    Return Value:

-------------------------------------------------------------------*/
DCLNETHOSTENT * DclNetGetHostByAddr(
    const char * addr,
    int len,
    int type)
{
    (void)addr;
    (void)len;
    (void)type;
    DclProductionError();
    return (void *)0;
}


/*-------------------------------------------------------------------
    Protected: DclNetInetAddr()

    Converts a string to a network address.

    Parameters:

    Return Value:

-------------------------------------------------------------------*/
D_UINT32 DclNetInetAddr(
    const char * cp)
{
    (void)cp;
    DclProductionError();
    return 0;
}


