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

    This module provides the prototypes and symbols that are required to
    use the DL Loader.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlloader.h $
    Revision 1.14  2009/05/21 02:37:59Z  garyp
    Updated documentation.  Updated to use standard type names.  Cleaned
    up some code formatting.  No functional changes.
    Revision 1.13  2009/02/13 01:09:53Z  garyp
    Partial migration to standard structure and type naming.
    Revision 1.12  2009/02/08 01:08:38Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.11  2007/12/18 05:26:35Z  brandont
    Updated comments.
    Revision 1.10  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.9  2007/10/05 22:55:17Z  brandont
    Changed DCL_DEFINED_sDlReaderIo to not use TRUE.  Disabled the use of
    DCLFUNC for this application interface module.
    Revision 1.8  2007/10/03 19:59:37Z  brandont
    Changed typedef for sDlReaderIo to only be defined once.
    Revision 1.7  2007/10/03 01:17:52Z  brandont
    Renamed FILEINFO to sDlLoaderFile.
    Revision 1.6  2007/10/03 01:10:37Z  brandont
    Updated the DlLoaderFileHandle typedef.
    Revision 1.5  2007/10/03 00:42:34Z  brandont
    Moved the ReaderInstance and ReaderFileHandle typedefs to dlreader.h.
    Revision 1.4  2007/10/02 23:13:13Z  brandont
    Renamed FILESTATE_SUCCESS to DL_LOADER_FILESTATE_SUCCESS.
    Renamed FILESTATE_EOF to DL_LOADER_FILESTATE_EOF.
    Renamed FILESTATE_IO_ERROR to DL_LOADER_FILESTATE_IO_ERROR.
    Revision 1.3  2007/10/02 17:58:01Z  brandont
    Removed the unused DlRelReaderFileOpen prototype.
    Revision 1.2  2007/10/01 23:30:35Z  brandont
    Updated formatting.  No functional changes.
    Revision 1.1  2007/09/27 01:26:28Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLLOADER_H_INCLUDED
#define DLLOADER_H_INCLUDED


/*-------------------------------------------------------------------
    Type: DCLLOADERHANDLE

    Type for a DL loader instance handle.
-------------------------------------------------------------------*/
typedef struct sDlLoaderInstance *  DCLLOADERHANDLE;


/*-------------------------------------------------------------------
    Type: DCLLOADERFILEHANDLE

    Type for a DL loader file handle.
-------------------------------------------------------------------*/
typedef struct sDCLREADERFILE *     DCLLOADERFILEHANDLE;


#ifndef DCL_DEFINED_sDlReaderIo
/*-------------------------------------------------------------------
    Type: DCLREADERIO

    Type for a DL loader block device instance handle.
-------------------------------------------------------------------*/
typedef struct sDCLREADERIO         DCLREADERIO;


/* Note: this cannot be defined as TRUE since an application can include
         this header directly.
*/
#define DCL_DEFINED_sDlReaderIo     1
#endif


/*  Name masquerading is not performed on these APIs since an application
    can include this header directly (NOT TRUE, any code properly including
    dcl_osname.h will have the DCLFUNC() macro defined).
*/
#if 0
/*  The Datalight loader interface.
*/
#define DclLoaderCreate         DCLFUNC(DclLoaderCreate)
#define DclLoaderDestroy        DCLFUNC(DclLoaderDestroy)
#define DclLoaderFileOpen       DCLFUNC(DclLoaderFileOpen)
#define DclLoaderFileClose      DCLFUNC(DclLoaderFileClose)
#define DclLoaderFileRead       DCLFUNC(DclLoaderFileRead)
#define DclLoaderFileState      DCLFUNC(DclLoaderFileState)
#endif

DCLLOADERHANDLE     DclLoaderCreate(struct sDCLREADERIO *psReaderDevice);
void                DclLoaderDestroy(  DCLLOADERHANDLE hLoader);
DCLLOADERFILEHANDLE DclLoaderFileOpen( DCLLOADERHANDLE hLoader, const char *pszName);
void                DclLoaderFileClose(DCLLOADERHANDLE hLoader, DCLLOADERFILEHANDLE hFile);
unsigned int        DclLoaderFileRead( DCLLOADERHANDLE hLoader, DCLLOADERFILEHANDLE hFile, unsigned nByteCount, unsigned char *pBuffer);
unsigned int        DclLoaderFileState(DCLLOADERHANDLE hLoader, DCLLOADERFILEHANDLE hFile);


/*  Values returned by DlLoaderFileState()
*/
#define DL_LOADER_FILESTATE_SUCCESS     0
#define DL_LOADER_FILESTATE_EOF         1
#define DL_LOADER_FILESTATE_IO_ERROR    2


#endif /* #ifndef DLLOADER_H_INCLUDED */
