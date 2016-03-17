/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This is the WinCE sockets abstraction layer.  It may be used as is, or
    as a prototype for your project.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify the
    make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osnet.c $
    Revision 1.5  2011/10/07 03:30:24Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.4  2009/09/02 16:34:27Z  johnbr
    Changes to correct coding standard problems:  removed gotos
    in error handling and corrected a variable naming error.
    Revision 1.3  2009/08/06 15:34:13Z  johnbr
    Changes for the power cycling tests.
    Revision 1.2  2009/07/15 07:28:59Z  keithg
    Corrected socket address type name for consistency.
    Revision 1.1  2009/07/15 04:47:04Z  keithg
    Initial revision
    Revision 1.1  2009/03/20 00:42:22Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <winsock.h>

#include <dcl.h>
#include <dlnet.h>


/*-------------------------------------------------------------------
    Public: DclNetSocket()

    Creates an endpoint for network communication.

    Parameters:
        af       - The address family for the call.  Currently, only
                   INET is supported.
        type     - The address type for the call.  Currently, Stream,
                   Datagram and Raw are supported.
        protocol - The protocol required for the socket.  Currently,
                   only IP, TCP and UDP are supported.

    Return Value:
        If the return value is less than 0, then an error has occured.
        Otherwise, the return value is a socket handle.
-------------------------------------------------------------------*/
DCLNETSOCKET DclNetSocket(
    int af,
    int type,
    int protocol)
{
    int os_af;
    int os_type;
    int os_protocol;
    SOCKET os_socket;
    DCLNETSOCKET iSocket = -1;

    /*  Convert the DCL network type to the OS equivalent
    */
    switch(af)
    {
        case DCLNET_AF_INET:
            os_af = AF_INET;
            break;
        default:
            DclPrintf("Unknown address family\n");
            return iSocket;
    }

    /*  Convert the DCL DclSocket type to the OS equivalent
    */
    switch(type)
    {
        case DCLNET_SOCK_STREAM:
            os_type = SOCK_STREAM;
            break;
        case DCLNET_SOCK_DGRAM:
            os_type = SOCK_DGRAM;
            break;
        case DCLNET_SOCK_RAW:
            os_type = SOCK_RAW;
            break;
        default:
            DclPrintf("Unknown socket type\n");
            return iSocket;
    }

    /*  Convert the DCL DclSocket protocol to the OS equivalent
    */
    switch(protocol)
    {
        case DCLNET_IPPROTO_IP:
            os_protocol = IPPROTO_IP;
            break;
        case DCLNET_IPPROTO_TCP:
            os_protocol = IPPROTO_TCP;
            break;
        case DCLNET_IPPROTO_UDP:
            os_protocol = IPPROTO_UDP;
            break;
        default:
            DclPrintf("Unknown protocol\n");
            return iSocket;
    }

    /*  Open a network DclSocket
    */
    DclPrintf("socket: af is %d, type is %d, protocol is %d\n",
            af,
            type,
            protocol);
    DclPrintf("socket: os_af is %d, os_type is %d, os_protocol is %d\n",
            os_af,
            os_type,
            os_protocol);
    os_socket = socket(os_af, os_type, os_protocol);
    if(os_socket < 0)
    {
        DclPrintf("socket: Error %d socket()\n", WSAGetLastError());
        return iSocket;
    }

    /*  Return the DclSocket descriptor
    */
    iSocket = os_socket;
    DclPrintf("socket:  Opened socket %d\n", os_socket );

    return iSocket;
}


/*-------------------------------------------------------------------
    Public: DclNetSetSockOpt()

    Sets a created network socket's option value.

    Parameters:
        socket - a socket created by DclNetSocket.
        level  - The level at which the option is defined.
        optname- The socket option for which the value is to be
                 set. Currently, only DCLNET_SO_REUSEADDR is
                 supported
        optval - A pointer to the buffer containing the option
                 value.
        optlen - The size, in bytes, of the buffer referred to by
                 the optval parameter.

    Return Value:
        If the function returns -1, then an error has occured. If
        the function returns 0, then the function succeeded.  All
        other values are undefined.
-------------------------------------------------------------------*/
int DclNetSetSockOpt(
    DCLNETSOCKET socket,
    int level,
    int optname,
    const char * optval,
    int optlen)
{
    SOCKET os_socket;
    int os_level;
    int os_optname;
    int iErr = -1;


    /*  Convert the socket level to the OS equivalent
    */
    switch(level)
    {
        case DCLNET_SOL_SOCKET:
            os_level = SOL_SOCKET;
            break;
        default:
            DclPrintf("Unknown socket level\n");
            return iErr;
    }


    /*  Convert the socket option to the OS equivalent
    */
    switch(optname)
    {
        case DCLNET_SO_REUSEADDR:
            os_optname = SO_REUSEADDR;
            break;
        default:
            DclPrintf("Unknown socket option\n");
            return iErr;
    }


    /*  Set the socket configuration option
    */
    os_socket = socket;
    iErr = setsockopt(os_socket, os_level, os_optname, optval, optlen);
    if(iErr)
    {
        DclPrintf("setsockopt: Error %d setsockopt() for os_socket %d\n",
                WSAGetLastError(),
                os_socket);
    }

    return iErr;
}


/*-------------------------------------------------------------------
    Public: DclNetCloseSocket()

    Close a previously created and open network socket.

    Parameters:
        socket - a socket created by DclNetSocket.

    Return Value:
        If the function returns -1, then an error has occured. If
        the function returns 0, then the function succeeded.  All
        other values are undefined.
-------------------------------------------------------------------*/
int DclNetCloseSocket(
    DCLNETSOCKET socket)
{
    SOCKET os_s;
    int iErr;


    /*  Close the socket
    */
    os_s = socket;
    iErr = closesocket(os_s);
    if(iErr)
    {
        DclPrintf("Error %d closesocket()\n", iErr);
        return -1;
    }

    DclPrintf("closesocket:  Closing socket %d\n", os_s);
    return 0;
}


/*-------------------------------------------------------------------
    Public: DclNetSend()

    Sends data across a connected network socket.

    Parameters:
        socket - a socket created by DclNetSocket or DclNetConnect.
        buf    - the buffer containing the data to be transmitted.
        len    - the byte-count for the data block being transmitted.
        flags  - miscellanious flags modifying the send.  Currently,
                 only DCLNET_MSG_DEFAULT is supported.

    Return Value:
        If the function returns -1, then an error has occured. Other
        values indicate the number of bytes transmitted.
-------------------------------------------------------------------*/
int DclNetSend(
    DCLNETSOCKET socket,
    const char * buf,
    int len,
    int flags)
{
    SOCKET os_s;
    int os_flags;
    int iResult = -1;


    /*  Convert the transmition flags to the OS equivalent
    */
    switch(flags)
    {
        case DCLNET_MSG_DEFAULT:
            os_flags = 0;
            break;
        default:
            DclPrintf("Unknown transmition flags\n");
            return iResult;
    }


    /*  Transmit the requested data
    */
    os_s = socket;
    iResult = send(os_s, buf, len, os_flags);
    if (iResult < 0)
    {
        DclPrintf("send: Error value is %d on os_socket %d, os_flags is %d\n",
                WSAGetLastError(),
                os_s,
                os_flags);
    }

    return iResult;
}

/*-------------------------------------------------------------------
    Public: DclNetRecv()

    Recieves data from a connected network socket.

    Parameters:
        socket - a socket created by DclNetSocket or DclNetConnect.
        buf    - the buffer containing the data to be transmitted.
        len    - the byte-count for the data block being transmitted.
        flags  - miscellanious flags modifying the send.  Currently,
                 only DCLNET_MSG_DEFAULT and DCLNET_MSG_PEEK are
                 supported.

    Return Value:
        If the function returns -1, then an error has occured. Other
        values indicate the number of bytes received.

    CAUTION:  A MSG_PEEK read does not do an actual read on WINCE
              targets.  WINCE does not have a PEEK-mode call.  It
              does have an ioctl that will return the number of
              bytes curently available.  This value is returned
              to the caller if DCLNET_MSG_PEEK used.  THE BUFFER
              WILL NOT BE MODIFIED IF DCLNET_MSG_PEEK IS USED.
-------------------------------------------------------------------*/
int DclNetRecv(
    DCLNETSOCKET socket,
    char * buf,
    int len,
    int flags)
{
    SOCKET   os_s;
    int      os_flags;
    int      iResult     = -1;
    D_UINT32 ulDataCount = 0;

    /* First, if we're using a PEEK read, use the ioctlsocket call instead.
    */

    if (flags == DCLNET_MSG_PEEK)
    {
        iResult = ioctlsocket(
                socket,
                FIONREAD,
                &ulDataCount );
        if (iResult < 0)
            return iResult;
        return ulDataCount;
    }

    /*  Convert the receive flags to the OS equivalent
    */
    switch(flags)
    {
        case DCLNET_MSG_DEFAULT:
            os_flags = 0;
            break;
        case DCLNET_MSG_PEEK:
            os_flags = MSG_PEEK;
            break;
        default:
            DclPrintf("Unknown receive flags\n");
            return iResult;
    }


    /*  Receive the requested data
    */
    os_s = socket;
    iResult = recv(os_s, buf, len, os_flags);
    if (iResult < 0)
    {
        DclPrintf( "recv: Error value is %d on os_socket %d, os_flags is %d\n",
                WSAGetLastError(),
                os_s,
                os_flags );
    }

    return iResult;
}


/*-------------------------------------------------------------------
    Public: DclNetConnect()

    Establishes a connection to the given network socket.

    Parameters:
        socket - a socket created by DclNetSocket.
        name   - the name of the protocol to use. Currently, only
                 DCLNET_AF_INET is supported.
        namelen- the length of the name field.

    Return Value:
        If the function returns -1, then an error has occured. If
        the function returns 0, then the function succeeded.  All
        other values are undefined.
-------------------------------------------------------------------*/
int DclNetConnect(
    DCLNETSOCKET socket,
    const DCLNETSOCKADDR * name,
    int namelen)
{
    SOCKET os_s;
    struct sockaddr os_name;
    int iErr;


    DclAssert(sizeof(os_name.sa_data) == sizeof(name->sa_data));
    DclAssert(namelen == sizeof(os_name));

    /*  Convert the DCL network type to the OS equivalent
    */
    switch(name->sa_family)
    {
        case DCLNET_AF_INET:
            os_name.sa_family = AF_INET;
            break;
        default:
            DclPrintf("Unknown network type\n");
            return -1;
    }


    DclMemCpy(os_name.sa_data, name->sa_data, sizeof(name->sa_data));
    os_s = (SOCKET)socket;

    iErr = connect(os_s, &os_name, namelen);
    if(iErr == SOCKET_ERROR)
    {
        iErr = WSAGetLastError();  /* debugging code, should be removed */
        DclPrintf( "connect: Error value is %d\n", WSAGetLastError() );
        return -1;
    }

    return 0;
}


/*-------------------------------------------------------------------
    Public: DclNetHtons()

    Converts a 16-bit host integer to network endian order.

    Parameters:
        hostshort - value to be converted.

    Return Value:
        hostshort translated to network byte order.
-------------------------------------------------------------------*/
D_UINT16 DclNetHtons(
    D_UINT16 hostshort)
{
    return htons(hostshort);
}


/*-------------------------------------------------------------------
    Public: DclNetGetHostByAddr()

    Retrieves host information for the given address.

    Parameters:
        addr    - the address to be examined.
        len     - the length of the add string.
        type    - the type of address specified.  Currently, only
                  DCLNET_AF_INET is supported.

    Return Value:
        A pointer to an item of type struct DclNet_hostent.  This
        is a static item, internal to the function and should not
        be freed.  This also means that it is not thread-safe.
-------------------------------------------------------------------*/
DCLNETHOSTENT * DclNetGetHostByAddr(
    const char * addr,
    int len,
    int type)
{
    static DCLNETHOSTENT  Dcl_h;
    int                   os_type;
    struct hostent      * h;


    /*  Wipe the contents of the hostent structue
    */
    DclMemSet(&Dcl_h, 0, sizeof(Dcl_h));


    /*  Convert the DCL network type to the OS equivalent
    */
    switch(type)
    {
        case DCLNET_AF_INET:
            os_type = AF_INET;
            break;
        default:
            DclPrintf("Unknown network type\n");
            return NULL;
    }


    /*  Get information about this network address
    */
    h = gethostbyaddr(
              (char *)addr,
              sizeof(struct hostent),
              os_type);
    if(h)
    {
        Dcl_h.h_name      = h->h_name;
        Dcl_h.h_aliases   = h->h_aliases;
        Dcl_h.h_addrtype  = h->h_addrtype;
        Dcl_h.h_length    = h->h_length;
        Dcl_h.h_addr_list = h->h_addr_list;

        return &Dcl_h;
    }

    return NULL;
}


/*-------------------------------------------------------------------
    Public: DclNetInetAddr()

    Converts a string to a network address.

    Parameters:
        cp    - The ip address in numeric format.

    Return Value:
        A D_UINT32 containing an IPv4 address.
-------------------------------------------------------------------*/
D_UINT32 DclNetInetAddr(
    const char * cp)
{
    return inet_addr((char *)cp);
}


