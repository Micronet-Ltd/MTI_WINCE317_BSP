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
                                Revision History
    $Log: dlnet.h $
    Revision 1.2  2009/07/16 16:05:31Z  keithg
    Migrated network prototypes and structures from DLAPIINT.H into DLNET.H
    Revision 1.1  2009/07/15 06:35:16Z  keithg
    Initial revision
    Revision 1.1  2009/03/20 00:42:22Z  brandont
    Initial revision
---------------------------------------------------------------------------*/



#ifndef DLNET_H_INCLUDED
#define DLNET_H_INCLUDED

/*  DCLNET_AF_xxx

    DCL network address family
*/
#define DCLNET_AF_INET          0   /* internetwork: UDP, TCP, etc. */


/*  DCLNET_SOCK_xxx

    DCL network socket connection type
*/
#define DCLNET_SOCK_STREAM      0   /* stream socket */
#define DCLNET_SOCK_DGRAM       1   /* datagram socket */
#define DCLNET_SOCK_RAW         2   /* raw-protocol interface */


/*  DCLNET_IPPROTO_xxx

    DCL network socket protocol type
*/
#define DCLNET_IPPROTO_IP       0   /* default for DCLNET_SOCK_xxx type */
#define DCLNET_IPPROTO_TCP      1   /* tcp */
#define DCLNET_IPPROTO_UDP      2   /* user datagram protocol */


/*  DCLNET_SOL_xxx
*/
#define DCLNET_SOL_SOCKET       0   /* options for socket level */


/*  DCLNET_SO_xxx
*/
#define DCLNET_SO_REUSEADDR     0   /* allow local address reuse */


/*  DCLNET_MSG_xxx
*/
#define DCLNET_MSG_DEFAULT      0
#define DCLNET_MSG_PEEK         1   /* peek at incoming message */


/*  DCLNET_IPPORT_xxx

    Standard port numbers
*/
#define DCLNET_IPPORT_FTP       21
#define DCLNET_IPPORT_TFTP      69


typedef int DCLNETSOCKET;

typedef struct {
    D_UINT16    sa_family;      /* address family */
    char        sa_data[14];    /* up to 14 bytes of direct address */
} DCLNETSOCKADDR;

typedef struct {
    char *   h_name;            /* official name of host */
    char **  h_aliases;         /* alias list */
    D_UINT16 h_addrtype;        /* host address type */
    D_UINT16 h_length;          /* length of address */
    char **  h_addr_list;       /* list of addresses */
#define h_addr  h_addr_list[0]  /* address, for backward compatibility */
} DCLNETHOSTENT;


/*  DLCNETINADDR
*/
typedef struct  {
    union {
        struct {
            D_UINT8 s_b1;
            D_UINT8 s_b2;
            D_UINT8 s_b3;
            D_UINT8 s_b4;
        } S_un_b;

        struct {
            D_UINT16 s_w1;
            D_UINT16 s_w2;
        } S_un_w;

        D_UINT32 S_addr;
    } S_un;
} DLCNETINADDR;

#define s_addr  S_un.S_addr         /* can be used for most tcp & ip code */
#define s_host  S_un.S_un_b.s_b2    /* host on imp */
#define s_net   S_un.S_un_b.s_b1    /* network */
#define s_imp   S_un.S_un_w.s_w2    /* imp */
#define s_impno S_un.S_un_b.s_b4    /* imp # */
#define s_lh    S_un.S_un_b.s_b3    /* logical host */


/*  DCLNETSOCKADDRIN
*/
typedef struct {
    D_UINT16        sin_family;
    D_UINT16        sin_port;
    DLCNETINADDR    sin_addr;
    char            sin_zero[8];
} DCLNETSOCKADDRIN;


/*-------------------------------------------------------------------
    Network Socket Interfaces and types
-------------------------------------------------------------------*/
#define   DclNetSocket              DCLFUNC(DclNetSocket)
#define   DclNetSetSockOpt          DCLFUNC(DclNetSetSockOpt)
#define   DclNetCloseSocket         DCLFUNC(DclNetCloseSocket)
#define   DclNetSend                DCLFUNC(DclNetSend)
#define   DclNetRecv                DCLFUNC(DclNetRecv)
#define   DclNetConnect             DCLFUNC(DclNetConnect)
#define   DclNetHtons               DCLFUNC(DclNetHtons)
#define   DclNetGetHostByAddr       DCLFUNC(DclNetGetHostByAddr)
#define   DclNetInetAddr            DCLFUNC(DclNetInetAddr)


DCLNETSOCKET DclNetSocket(int af, int type, int protocol);
int DclNetSetSockOpt(DCLNETSOCKET socket, int level, int optname, const char * optval, int optlen);
int DclNetCloseSocket(DCLNETSOCKET socket);
int DclNetSend(DCLNETSOCKET socket, const char * buf, int len, int flags);
int DclNetRecv(DCLNETSOCKET socket, char * buf, int len, int flags);
int DclNetConnect(DCLNETSOCKET socket, const DCLNETSOCKADDR * name, int namelen);
D_UINT16 DclNetHtons(D_UINT16 hostshort);
DCLNETHOSTENT * DclNetGetHostByAddr(const char * addr, int len, int type);
D_UINT32 DclNetInetAddr(const char * cp);


#endif /* #ifndef _include_dlnet_h */

