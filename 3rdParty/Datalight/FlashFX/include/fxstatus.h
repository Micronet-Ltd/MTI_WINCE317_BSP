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
  jurisdictions. 

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

  Notwithstanding the foregoing, Licensee acknowledges that the software may
  be distributed as part of a package containing, and/or in conjunction with
  other source code files, licensed under so-called "open source" software 
  licenses.  If so, the following license will apply in lieu of the terms set
  forth above:

  Redistribution and use of this software in source and binary forms, with or
  without modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions, and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions, and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY DATALIGHT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
  CHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
  DISCLAIMED.  IN NO EVENT SHALL DATALIGHT BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEG-
  LIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header file contains the standard status codes used throughout
    FlashFX.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxstatus.h $
    Revision 1.62  2011/12/02 18:22:49Z  glenns
    Added comments to deprecate the use of codes that try to 
    delineate between main page and spare area for bit error
    indications. No functional changes.
    Revision 1.61  2011/11/22 18:12:43Z  glenns
    Add ONFI indications for EZ-NAND.
    Revision 1.60  2010/11/13 01:00:12Z  glenns
    Added new VBF error code for disk format not supporting
    QuickMount.
    Revision 1.59  2010/04/30 23:40:44Z  garyp
    Added a status code.
    Revision 1.58  2010/04/13 17:19:31Z  glenns
    Fix spelling of FFXSTAT_DEVMGR_UNKNOWNBLOCKSTATUS.
    Revision 1.57  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.56  2010/01/23 21:39:41Z  garyp
    Added a status code.
    Revision 1.55  2009/11/25 23:03:47Z  garyp
    Added status codes.
    Revision 1.54  2009/08/02 18:13:53Z  garyp
    Added some new VBF status codes.
    Revision 1.53  2009/04/08 01:55:14Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.52  2009/03/30 21:06:23Z  keithg
    Corrected spelling error in BBM status not formatted codes.
    Revision 1.51  2009/03/26 06:35:36Z  glenns
    Addendum to previous checkin: also added FIM_UNEXPECTEDHWSTATE used to
    describe conditions in which something has put hardware into a state
    FlashFX never expects to see.
    Revision 1.50  2009/03/26 06:35:36Z  glenns
    Fix Bugzilla #2464: Added FFXSTAT_INTERNAL_ERROR.  This status code is
    to be used when FlashFX execution reaches a code path we believe to be
    impossible to get to.
    Revision 1.49  2009/02/26 00:07:29Z  billr
    Resolve bug 2448: NAND FIM returns incorrect status for some erase failures.
    Resolve bug 2455: NAND program failure may not cause block replacement.
    Eliminate deprecated status FFXSTAT_FIMERASEFAILED.
    Revision 1.48  2009/02/19 04:43:45Z  keithg
    Added device manager status code to indicate if there are no more
    replacement blocks available within a NAND device.
    Revision 1.47  2009/02/09 03:39:53Z  garyp
    Merged from the v4.0 branch.  Redefined the meaning of the "general" locale
    to indicate that those status codes are identical to their DCL counterparts.
    Makes transferring some codes which must be explicitly recongnized in
    multiple product code bases much easier.  Added a variety of new status
    codes.
    Revision 1.46  2009/01/28 20:07:38Z  billr
    Implement "write native pages" (which is the same as "raw pages" at the FIM
    level), and stub "read native pages" to "read raw pages" for now.
    Revision 1.45  2009/01/06 23:58:36Z  glenns
    Fixed funky editing mistake in values assigned to new macros added in last
    check-in.
    Revision 1.44  2009/01/06 23:51:15Z  glenns
    After discussion, decided it was better not to change values for existing
    macros and adjusted the last check-in to reflect a new value for the new
    macro. Also added two new macros, FFXSTAT_FIMCORRECTABLESPARE and
    FFXSTAT_FIMCORRECTABLEMAIN, for future use.
    Revision 1.43  2009/01/06 20:57:45Z  glenns
    Added "FFXSTAT_FIMCORRECTABLEDATA " for indicating
    existence of correctable bit errors.
    Revision 1.42  2008/12/31 00:56:45Z  keithg
    Added BBM status code to describe mappings that already exist.
    Revision 1.41  2008/12/11 23:05:26Z  keithg
    Added error code for the device manager to indicate a failed attempted
    operation to a BBM system related block.
    Revision 1.40  2008/10/23 23:15:49Z  keithg
    Added general status codes and UNKNOWN_BLOCK_STATUS.
    Revision 1.39  2008/10/01 22:25:58Z  glenns
    Added status indications to cover devices with integrated ECC that can
    report on uncorrectable ECC faults on the main page and spare area
    separately.
    Revision 1.38  2008/07/10 23:04:07Z  keithg
    Updated to include new BBM error values.
    Revision 1.37  2008/06/19 16:19:15Z  keithg
    Added BBM reserved block status.
    Revision 1.36  2008/06/02 05:34:40Z  keithg
    Added BBM status codes.
    Revision 1.35  2008/05/07 03:01:21Z  keithg
    Replaced inadvertantly deleted media manager error code.
    Revision 1.34  2008/05/07 02:40:38Z  keithg
    Added BBM locale, BBM error status values; Made all VBF error codes
    semantically consistent; Minor typo corrections.
    Revision 1.33  2008/03/26 02:53:32Z  Garyp
    Added status codes.
    Revision 1.32  2007/12/26 01:46:55Z  Garyp
    Added a status code.
    Revision 1.31  2007/12/15 01:04:05Z  Garyp
    Added status codes.
    Revision 1.30  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.29  2007/11/01 02:05:20Z  Garyp
    Eliminated a bunch of obsolete VBF status codes, and added several
    new ones for VBF and the Media Manager.
    Revision 1.28  2007/09/28 21:31:41Z  pauli
    Resolved Bug 355: Added a status code for a misaligned buffer.
    Revision 1.27  2007/06/22 16:24:31Z  Garyp
    Added new FFXSTATUS values.
    Revision 1.26  2007/05/23 16:40:05Z  Garyp
    Added status codes.
    Revision 1.25  2007/04/30 20:59:38Z  billr
    Add new status code.
    Revision 1.24  2007/04/21 20:45:34Z  Garyp
    Added BBM stats support.
    Revision 1.23  2007/04/19 18:57:54Z  timothyj
    Added FFXSTAT_VBF_READONLY, to be returned by the FfxVbfWritePages() and
    FfxVbfDiscardPages() functions when the are called with a read-only VBF
    instance.
    Revision 1.22  2007/04/06 23:10:25Z  Garyp
    New status codes.
    Revision 1.21  2007/04/03 20:12:05Z  Garyp
    Added status codes.
    Revision 1.20  2007/02/01 02:42:42Z  Garyp
    Added status codes.
    Revision 1.19  2006/12/28 20:04:38Z  rickc
    Added new status code
    Revision 1.18  2006/11/10 03:25:47Z  Garyp
    Added new status codes.
    Revision 1.17  2006/10/17 00:31:34Z  Garyp
    Added status values.
    Revision 1.16  2006/10/11 20:13:03Z  Garyp
    Modified to use the new DCL_MAKESTATUS macro.
    Revision 1.15  2006/10/07 03:42:07Z  Garyp
    Added status codes.
    Revision 1.14  2006/05/17 19:21:03Z  Garyp
    Added several new status codes.
    Revision 1.13  2006/03/08 01:26:48Z  Garyp
    Added status codes.
    Revision 1.12  2006/03/04 21:36:56Z  Garyp
    Added status codes.
    Revision 1.11  2006/03/03 16:44:14Z  Garyp
    Added status codes.
    Revision 1.10  2006/02/25 01:22:30Z  timothyj
    Added FFXSTAT_PROJMAPMEMFAILED status code, indicates that a memory map
    operation in the project hooks code failed.
    Revision 1.9  2006/02/20 02:49:41Z  Garyp
    New status codes.
    Revision 1.8  2006/02/15 03:00:58Z  Garyp
    More status codes.
    Revision 1.7  2006/02/12 19:41:12Z  Garyp
    New status codes.
    Revision 1.6  2006/02/11 04:06:35Z  Garyp
    New status codes.
    Revision 1.5  2006/02/09 00:04:01Z  Garyp
    Added a number of new status codes.
    Revision 1.4  2006/01/12 03:57:08Z  Garyp
    Added new status codes.
    Revision 1.3  2006/01/05 00:03:33Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.2  2005/12/30 17:52:21Z  Garyp
    Added thread status codes.
    Revision 1.1  2005/11/25 22:20:06Z  Pauli
    Initial revision
    Revision 1.6  2005/11/25 22:20:05Z  Garyp
    Tweaked some status values.
    Revision 1.5  2005/10/30 03:14:31Z  Garyp
    Added status codes.
    Revision 1.4  2005/10/22 12:41:57Z  garyp
    New status codes.
    Revision 1.3  2005/10/12 04:12:32Z  Garyp
    New status codes.
    Revision 1.2  2005/10/11 02:10:35Z  Garyp
    Added new status values.
    Revision 1.1  2005/08/05 17:40:56Z  Garyp
    Initial revision
    Revision 1.15  2005/05/11 00:56:16Z  garyp
    Added more status codes.
    Revision 1.14  2004/12/30 23:13:59Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.13  2004/11/29 18:45:04Z  GaryP
    Added more status codes.
    Revision 1.12  2004/11/19 01:25:56Z  GaryP
    Added status codes.
    Revision 1.11  2004/11/09 03:48:29Z  GaryP
    Added a number of new status codes.
    Revision 1.10  2004/09/22 17:52:18Z  GaryP
    More status codes.
    Revision 1.9  2004/08/24 04:20:54Z  GaryP
    Added status codes.
    Revision 1.8  2004/08/22 17:05:46Z  GaryP
    Added status codes.
    Revision 1.7  2004/08/15 22:44:59Z  GaryP
    Added status codes.
    Revision 1.6  2004/08/10 07:20:37Z  garyp
    New status codes.
    Revision 1.5  2004/08/06 23:39:00Z  GaryP
    Added more status codes.
    Revision 1.4  2004/07/14 23:13:09Z  BillR
    Add newline at end of file.
    Revision 1.3  2004/04/28 18:31:52Z  garyp
    Updated to avoid multiple inclusions.
    Revision 1.2  2004/02/02 05:56:58Z  garys
    Revision 1.1.1.7  2004/02/02 05:56:58  garyp
    Added a number of new statuses.
    Revision 1.1.1.6  2004/01/05 01:13:24Z  garyp
    Added FFXSTAT_FAILURE.
    Revision 1.1.1.5  2003/12/15 23:11:08Z  billr
    Initial implementation of OESL support for threads.
    Revision 1.1.1.4  2003/12/05 06:38:22Z  garyp
    Added new status codes.
    Revision 1.1.1.3  2003/11/18 03:05:06Z  garyp
    Added a bunch of new FFXSTAT values.
    Revision 1.1.1.2  2003/11/06 21:57:16Z  garyp
    Added more status codes.
    Revision 1.1  2003/10/04 19:10:44Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FXSTATUS_H_INCLUDED
#define FXSTATUS_H_INCLUDED


/*-------------------------------------------------------------------
    Type: FFXSTATUS

    A structured status code used to return error/status information.

    FFXSTATUS is a 32-bit structured status code which is based on
    DCLSTATUS.  See the DCLSTATUS definition for a detailed description
    of the layout.

    The following Locales are defined for FlashFX:

        FFXLOCALE_GENERAL        -  1 (reserved, same as DCLLOCALE_GENERAL)
        FFXLOCALE_OESERV         -  2
        FFXLOCALE_VBF            -  3
        FFXLOCALE_FIM            -  4
        FFXLOCALE_EXTAPI         -  5
        FFXLOCALE_DRIVERFW       -  6
        FFXLOCALE_OSDRIVER       -  7
        FFXLOCALE_PROJHOOKS      -  8
        FFXLOCALE_DOSDEV         -  9
        FFXLOCALE_LOADER         - 10
        FFXLOCALE_FML            - 11
        FFXLOCALE_MISC           - 12
        FFXLOCALE_DEVMGR         - 13
        FFXLOCALE_MEDIAMGR       - 14
        FFXLOCALE_BBM            - 15
-------------------------------------------------------------------*/
typedef DCLSTATUS FFXSTATUS;


/*  Note that FFXSTAT_SUCCESS must be zero for compatibility
    purposes.
*/
#define FFXSTAT_SUCCESS               DCLSTAT_SUCCESS
#define FFXSTAT_FAILURE               DCLSTAT_FAILURE

/*  Update the documentation above when adding items to this list
*/
/* #define FFXLOCALE_GENERAL          1 - Reserved, same as DCL */
#define FFXLOCALE_OESERV              2
#define FFXLOCALE_VBF                 3
#define FFXLOCALE_FIM                 4
#define FFXLOCALE_EXTAPI              5
#define FFXLOCALE_DRIVERFW            6
#define FFXLOCALE_OSDRIVER            7
#define FFXLOCALE_PROJHOOKS           8
#define FFXLOCALE_DOSDEV              9
#define FFXLOCALE_LOADER             10
#define FFXLOCALE_FML                11
#define FFXLOCALE_MISC               12
#define FFXLOCALE_DEVMGR             13
#define FFXLOCALE_MEDIAMGR           14
#define FFXLOCALE_BBM                15


/*  This is a general macro for combining all the elements together to
    make a FFXSTATUS value.
*/
#define MAKEFFXERR(loc, val) DCL_MAKESTATUS(PRODUCTNUM_FLASHFX, DCLSTATUSCLASS_ERROR, loc, val)

/*  General status codes.  Any status codes with the locale of "General"
    are required to be identical across all products.  Therefore, they
    are all defined in DCL.  The product specific names are only for
    convenience.
*/
#define FFXSTAT_BADSTRUCLEN               DCLSTAT_BADSTRUCLEN
#define FFXSTAT_BADPARAMETER              DCLSTAT_BADPARAMETER
#define FFXSTAT_BADHANDLE                 DCLSTAT_BADHANDLE
#define FFXSTAT_UNSUPPORTEDFUNCTION       DCLSTAT_UNSUPPORTEDFUNCTION
#define FFXSTAT_BADSYNTAX                 DCLSTAT_BADSYNTAX
#define FFXSTAT_ARGUMENTSTOOLONG          DCLSTAT_ARGUMENTSTOOLONG
#define FFXSTAT_BADPARAMETERLEN           DCLSTAT_BADPARAMETERLEN
#define FFXSTAT_OUTOFMEMORY               DCLSTAT_OUTOFMEMORY
#define FFXSTAT_UNSUPPORTEDREQUEST        DCLSTAT_UNSUPPORTEDREQUEST
#define FFXSTAT_OUTOFRANGE                DCLSTAT_OUTOFRANGE
#define FFXSTAT_CATEGORYDISABLED          DCLSTAT_CATEGORYDISABLED
#define FFXSTAT_SUBTYPECOMPLETE           DCLSTAT_SUBTYPECOMPLETE
#define FFXSTAT_SUBTYPEUNUSED             DCLSTAT_SUBTYPEUNUSED
#define FFXSTAT_UNSUPPORTEDFEATURE        DCLSTAT_UNSUPPORTEDFEATURE
#define FFXSTAT_INTERNAL_ERROR            DCLSTAT_INTERNAL_ERROR

/*  VBF status codes
*/
#define FFXSTAT_VBF_FMTBADCUSHIONSIZE     (MAKEFFXERR(FFXLOCALE_VBF, 100))
#define FFXSTAT_VBF_FMTBADSPARECOUNT      (MAKEFFXERR(FFXLOCALE_VBF, 101))
#define FFXSTAT_VBF_FMTBADGEOMETRY        (MAKEFFXERR(FFXLOCALE_VBF, 102))
#define FFXSTAT_VBF_FMTBADMEDIASIZE       (MAKEFFXERR(FFXLOCALE_VBF, 103))
#define FFXSTAT_VBF_FMTBADUNITCOUNT       (MAKEFFXERR(FFXLOCALE_VBF, 104))
#define FFXSTAT_VBF_FMTBADBLOCKSIZE       (MAKEFFXERR(FFXLOCALE_VBF, 105))
#define FFXSTAT_VBF_FMTDISKTOOSMALL       (MAKEFFXERR(FFXLOCALE_VBF, 106))
#define FFXSTAT_VBF_FMTWHILELOADED        (MAKEFFXERR(FFXLOCALE_VBF, 107))
#define FFXSTAT_VBF_NOTHINGTOCOMPACT      (MAKEFFXERR(FFXLOCALE_VBF, 108))
#define FFXSTAT_VBF_NODIRTYMETADATA       (MAKEFFXERR(FFXLOCALE_VBF, 109))
#define FFXSTAT_VBF_REGIONBUSY            (MAKEFFXERR(FFXLOCALE_VBF, 110))
#define FFXSTAT_VBF_COMPKEYNOTINITIALIZED (MAKEFFXERR(FFXLOCALE_VBF, 111))
#define FFXSTAT_VBF_SYSTEMTOOBUSY         (MAKEFFXERR(FFXLOCALE_VBF, 112))
#define FFXSTAT_VBF_ALLOCNOTFOUND         (MAKEFFXERR(FFXLOCALE_VBF, 113))
#define FFXSTAT_VBF_NOSPARE               (MAKEFFXERR(FFXLOCALE_VBF, 120))
#define FFXSTAT_VBF_BADUNITNUMBER         (MAKEFFXERR(FFXLOCALE_VBF, 122))
#define FFXSTAT_VBF_REGIONMOUNTFAILED     (MAKEFFXERR(FFXLOCALE_VBF, 123))
#define FFXSTAT_VBF_READONLY              (MAKEFFXERR(FFXLOCALE_VBF, 125))
#define FFXSTAT_VBF_OUTOFFLASH            (MAKEFFXERR(FFXLOCALE_VBF, 126))
#define FFXSTAT_VBF_BADUNITLIST           (MAKEFFXERR(FFXLOCALE_VBF, 127))
#define FFXSTAT_VBF_UNWRITTENDATA         (MAKEFFXERR(FFXLOCALE_VBF, 128))
#define FFXSTAT_VBF_ALREADYMOUNTED        (MAKEFFXERR(FFXLOCALE_VBF, 129))
#define FFXSTAT_VBF_MEDIATOOBIG           (MAKEFFXERR(FFXLOCALE_VBF, 130))
#define FFXSTAT_VBF_MOUNTFAILED           (MAKEFFXERR(FFXLOCALE_VBF, 131))
#define FFXSTAT_VBF_EUHNOTFOUND           (MAKEFFXERR(FFXLOCALE_VBF, 132))
#define FFXSTAT_VBF_BADMAXUNITS           (MAKEFFXERR(FFXLOCALE_VBF, 133))
#define FFXSTAT_VBF_IOERROR               (MAKEFFXERR(FFXLOCALE_VBF, 134))
#define FFXSTAT_VBF_UNITNOTFOUND          (MAKEFFXERR(FFXLOCALE_VBF, 135))
#define FFXSTAT_VBF_REGIONCACHECREATEFAILED (MAKEFFXERR(FFXLOCALE_VBF, 136))
#define FFXSTAT_VBF_FORMATNOTQUICKMOUNT   (MAKEFFXERR(FFXLOCALE_VBF, 137))

/*  Media Mgr status codes
*/
#define FFXSTAT_MEDIAMGR_VERIFYFAILED     (MAKEFFXERR(FFXLOCALE_MEDIAMGR, 1))
#define FFXSTAT_MEDIAMGR_NOTBLANK         (MAKEFFXERR(FFXLOCALE_MEDIAMGR, 2))
#define FFXSTAT_MEDIAMGR_NOEUH            (MAKEFFXERR(FFXLOCALE_MEDIAMGR, 3))
#define FFXSTAT_MEDIAMGR_BADEUH           (MAKEFFXERR(FFXLOCALE_MEDIAMGR, 4))
#define FFXSTAT_MEDIAMGR_CREATEFAIL       (MAKEFFXERR(FFXLOCALE_MEDIAMGR, 5))

/*  FML status codes
*/
#define FFXSTAT_FML_INFOFAILED            (MAKEFFXERR(FFXLOCALE_FML,   1))
#define FFXSTAT_FML_UNSUPPORTEDIOREQUEST  (MAKEFFXERR(FFXLOCALE_FML,   2))
#define FFXSTAT_FML_RAWACCESSONLY         (MAKEFFXERR(FFXLOCALE_FML,   3))
#define FFXSTAT_FML_DEVICENOTVALID        (MAKEFFXERR(FFXLOCALE_FML,   4))
#define FFXSTAT_FML_NOBOOTBLOCKS          (MAKEFFXERR(FFXLOCALE_FML,   5))
#define FFXSTAT_FML_OUTOFRANGE            (MAKEFFXERR(FFXLOCALE_FML,   6))
#define FFXSTAT_FML_SUSPENDPARTIAL        (MAKEFFXERR(FFXLOCALE_FML,   7))
#define FFXSTAT_FML_STILLSUSPENDED        (MAKEFFXERR(FFXLOCALE_FML,   8))
#define FFXSTAT_FML_ALREADYSUSPENDED      (MAKEFFXERR(FFXLOCALE_FML,   9))
#define FFXSTAT_FML_ALREADYRESUMED        (MAKEFFXERR(FFXLOCALE_FML,  10))
#define FFXSTAT_FML_UNSUPPORTEDLOCKRANGE  (MAKEFFXERR(FFXLOCALE_FML,  11))

/*  DevMgr status codes
*/
#define FFXSTAT_DEVMGR_READBACKVERIFYFAILED (MAKEFFXERR(FFXLOCALE_DEVMGR, 1))
#define FFXSTAT_DEVMGR_INFOFAILED           (MAKEFFXERR(FFXLOCALE_DEVMGR, 2))
#define FFXSTAT_DEVMGR_NOBOOTBLOCKS         (MAKEFFXERR(FFXLOCALE_DEVMGR, 3))
#define FFXSTAT_DEVMGR_BOOTBLOCKRANGE       (MAKEFFXERR(FFXLOCALE_DEVMGR, 4))
#define FFXSTAT_DEVMGR_MISALIGNED           (MAKEFFXERR(FFXLOCALE_DEVMGR, 5))
#define FFXSTAT_DEVMGR_UNKNOWNBLOCKSTATUS   (MAKEFFXERR(FFXLOCALE_DEVMGR, 6))
#define FFXSTAT_DEVMGR_BLOCK_RESERVED       (MAKEFFXERR(FFXLOCALE_DEVMGR, 7))
#define FFXSTAT_DEVMGR_ERASESUSPENDFAILED   (MAKEFFXERR(FFXLOCALE_DEVMGR, 8))
#define FFXSTAT_DEVMGR_UNSUPPORTEDIOREQUEST (MAKEFFXERR(FFXLOCALE_DEVMGR, 9))
#define FFXSTAT_DEVMGR_NOMORERESERVEDBLOCKS (MAKEFFXERR(FFXLOCALE_DEVMGR, 10))

/*  BBM status codes
*/
#define FFXSTAT_BBM_NOT_FORMATTED          (MAKEFFXERR(FFXLOCALE_BBM, 1))
#define FFXSTAT_BBM_OVERRIDE_ACTION        (MAKEFFXERR(FFXLOCALE_BBM, 2))
#define FFXSTAT_BBM_BAD_FORMAT             (MAKEFFXERR(FFXLOCALE_BBM, 3))
#define FFXSTAT_BBM_NOT_MOUNTED            (MAKEFFXERR(FFXLOCALE_BBM, 4))
#define FFXSTAT_BBM_NO_GOOD_BLOCKS         (MAKEFFXERR(FFXLOCALE_BBM, 5))
#define FFXSTAT_BBM_INVALID_BLOCK          (MAKEFFXERR(FFXLOCALE_BBM, 6))
#define FFXSTAT_BBM_DUPLICATE              (MAKEFFXERR(FFXLOCALE_BBM, 7))
#define FFXSTAT_BBM_ZERO_LIST              (MAKEFFXERR(FFXLOCALE_BBM, 8))
#define FFXSTAT_BBM_NO_SPACE               (MAKEFFXERR(FFXLOCALE_BBM, 9))
#define FFXSTAT_BBM_DEV_BUSY               (MAKEFFXERR(FFXLOCALE_BBM, 10))
#define FFXSTAT_BBM_BAD_GEOMETRY           (MAKEFFXERR(FFXLOCALE_BBM, 11))
#define FFXSTAT_BBM_MEDIA_MOUNTED          (MAKEFFXERR(FFXLOCALE_BBM, 12))
#define FFXSTAT_BBM_REMAP_NOT_FOUND        (MAKEFFXERR(FFXLOCALE_BBM, 13))
#define FFXSTAT_BBM_BLOCK_NOT_FOUND        (MAKEFFXERR(FFXLOCALE_BBM, 14))
#define FFXSTAT_BBM_END_OF_LIST            (MAKEFFXERR(FFXLOCALE_BBM, 15))
#define FFXSTAT_BBM_IO_ERROR               (MAKEFFXERR(FFXLOCALE_BBM, 16))
#define FFXSTAT_BBM_NO_HEADER              (MAKEFFXERR(FFXLOCALE_BBM, 17))
#define FFXSTAT_BBM_LIST_CHANGED           (MAKEFFXERR(FFXLOCALE_BBM, 18))
#define FFXSTAT_BBM_BAD_HEADER             (MAKEFFXERR(FFXLOCALE_BBM, 19))
#define FFXSTAT_BBM_RESERVED_BLOCK         (MAKEFFXERR(FFXLOCALE_BBM, 20))
#define FFXSTAT_BBM_BAD_REMAP              (MAKEFFXERR(FFXLOCALE_BBM, 21))
#define FFXSTAT_BBM_BLOCK_ALREADY_MAPPED   (MAKEFFXERR(FFXLOCALE_BBM, 22))


/*  FIM status codes
*/
#define FFXSTAT_FIMMOUNTFAILED              (MAKEFFXERR(FFXLOCALE_FIM,  1))
#define FFXSTAT_FIMBOUNDSINVALID            (MAKEFFXERR(FFXLOCALE_FIM,  2))
#define FFXSTAT_FIMIOERROR                  (MAKEFFXERR(FFXLOCALE_FIM,  3))
#define FFXSTAT_FIMTIMEOUT                  (MAKEFFXERR(FFXLOCALE_FIM,  4))
#define FFXSTAT_FIMRANGEINVALID             (MAKEFFXERR(FFXLOCALE_FIM,  5))
#define FFXSTAT_FIMREADSTARTFAILED          (MAKEFFXERR(FFXLOCALE_FIM,  6))
#define FFXSTAT_FIMREADCOMPLETEFAILED       (MAKEFFXERR(FFXLOCALE_FIM,  7))
#define FFXSTAT_FIMWRITESTARTFAILED         (MAKEFFXERR(FFXLOCALE_FIM,  8))
#define FFXSTAT_FIMWRITECOMPLETEFAILED      (MAKEFFXERR(FFXLOCALE_FIM,  9))
/*      FFXSTAT_FIMERASEFAILED              (MAKEFFXERR(FFXLOCALE_FIM, 10)) obsolete */
#define FFXSTAT_FIMUNCORRECTABLEDATA        (MAKEFFXERR(FFXLOCALE_FIM, 11))
#define FFXSTAT_FIM_ERASEINPROGRESS         (MAKEFFXERR(FFXLOCALE_FIM, 12))
#define FFXSTAT_FIM_ERASEFAILED             (MAKEFFXERR(FFXLOCALE_FIM, 13))
#define FFXSTAT_FIM_ERASESUSPENDFAILED      (MAKEFFXERR(FFXLOCALE_FIM, 14))
#define FFXSTAT_FIM_UNSUPPORTEDIOREQUEST    (MAKEFFXERR(FFXLOCALE_FIM, 15))
#define FFXSTAT_FIM_UNSUPPORTEDFUNCTION     (MAKEFFXERR(FFXLOCALE_FIM, 16))
#define FFXSTAT_FIM_ERASERESUMEFAILED       (MAKEFFXERR(FFXLOCALE_FIM, 17))
#define FFXSTAT_FIMUNCORRECTABLEMAIN        (MAKEFFXERR(FFXLOCALE_FIM, 18)) /* Deprecated- do not use */
#define FFXSTAT_FIMUNCORRECTABLESPARE       (MAKEFFXERR(FFXLOCALE_FIM, 19)) /* Deprecated- do not use */
#define FFXSTAT_FIMCORRECTABLEDATA          (MAKEFFXERR(FFXLOCALE_FIM, 20))
#define FFXSTAT_FIMCORRECTABLEMAIN          (MAKEFFXERR(FFXLOCALE_FIM, 21)) /* Deprecated- do not use */
#define FFXSTAT_FIMCORRECTABLESPARE         (MAKEFFXERR(FFXLOCALE_FIM, 22)) /* Deprecated- do not use */
#define FFXSTAT_FIMUNCORRECTED              (MAKEFFXERR(FFXLOCALE_FIM, 23))
#define FFXSTAT_FIM_NOTERASING              (MAKEFFXERR(FFXLOCALE_FIM, 24))
#define FFXSTAT_FIM_WRITEFAILED             (MAKEFFXERR(FFXLOCALE_FIM, 25))
#define FFXSTAT_FIM_UNEXPECTEDHWSTATE       (MAKEFFXERR(FFXLOCALE_FIM, 26))
#define FFXSTAT_FIM_WRITEPROTECTEDBLOCK     (MAKEFFXERR(FFXLOCALE_FIM, 30))
#define FFXSTAT_FIM_WRITEPROTECTEDPAGE      (MAKEFFXERR(FFXLOCALE_FIM, 31))
#define FFXSTAT_FIM_WRITEPROTECTEDSOFTLOCK  (MAKEFFXERR(FFXLOCALE_FIM, 32))
#define FFXSTAT_FIM_LOCKFAILED              (MAKEFFXERR(FFXLOCALE_FIM, 50))
#define FFXSTAT_FIM_UNLOCKFAILED            (MAKEFFXERR(FFXLOCALE_FIM, 51))
#define FFXSTAT_FIM_LOCKFREEZEFAILED        (MAKEFFXERR(FFXLOCALE_FIM, 52))
#define FFXSTAT_FIM_LOCKFROZEN              (MAKEFFXERR(FFXLOCALE_FIM, 53))
#define FFXSTAT_FIM_OTPREADFAILED           (MAKEFFXERR(FFXLOCALE_FIM, 100))
#define FFXSTAT_FIM_OTPWRITEFAILED          (MAKEFFXERR(FFXLOCALE_FIM, 101))
#define FFXSTAT_FIM_OTPLOCKFAILED           (MAKEFFXERR(FFXLOCALE_FIM, 102))
#define FFXSTAT_FIM_OTPALREADYLOCKED        (MAKEFFXERR(FFXLOCALE_FIM, 103))
#define FFXSTAT_FIM_OTPACCESSFAILED         (MAKEFFXERR(FFXLOCALE_FIM, 104))
#define FFXSTAT_FIM_OTPPAGERANGEINVALID     (MAKEFFXERR(FFXLOCALE_FIM, 105))

/*  External API
*/
#define FFXSTAT_VERSIONMISMATCH           (MAKEFFXERR(FFXLOCALE_EXTAPI, 1))
#define FFXSTAT_DEVBADHANDLE              (MAKEFFXERR(FFXLOCALE_EXTAPI, 2))
#define FFXSTAT_DEVNOTOPEN                (MAKEFFXERR(FFXLOCALE_EXTAPI, 3))
#define FFXSTAT_DEVWONTOPEN               (MAKEFFXERR(FFXLOCALE_EXTAPI, 4))
#define FFXSTAT_DEVIOFAILED               (MAKEFFXERR(FFXLOCALE_EXTAPI, 5))
#define FFXSTAT_DEVREQUESTFAILED          (MAKEFFXERR(FFXLOCALE_EXTAPI, 6))
#define FFXSTAT_DEVCLOSEFAILED            (MAKEFFXERR(FFXLOCALE_EXTAPI, 7))
#define FFXSTAT_DRIVERNOTINSTALLED        (MAKEFFXERR(FFXLOCALE_EXTAPI, 8))
#define FFXSTAT_APIBADREQUESTPOINTER      (MAKEFFXERR(FFXLOCALE_EXTAPI, 10))
#define FFXSTAT_APIBADREQUESTBLOCKLEN     (MAKEFFXERR(FFXLOCALE_EXTAPI, 11))
#define FFXSTAT_APIBADREQUESTSIGNATURE    (MAKEFFXERR(FFXLOCALE_EXTAPI, 12))
#define FFXSTAT_APIUNKNOWNFUNCTION        (MAKEFFXERR(FFXLOCALE_EXTAPI, 13))
#define FFXSTAT_APIUNKNOWNFMLFUNCTION     (MAKEFFXERR(FFXLOCALE_EXTAPI, 14))
#define FFXSTAT_APIUNKNOWNVBFFUNCTION     (MAKEFFXERR(FFXLOCALE_EXTAPI, 15))
#define FFXSTAT_APIUNKNOWNDEBUGFUNCTION   (MAKEFFXERR(FFXLOCALE_EXTAPI, 16))
#define FFXSTAT_APIUNKNOWNDRVRFWFUNCTION  (MAKEFFXERR(FFXLOCALE_EXTAPI, 17))
#define FFXSTAT_APIUNKNOWNGENERICFUNCTION (MAKEFFXERR(FFXLOCALE_EXTAPI, 18))
#define FFXSTAT_APIUNHANDLEDFUNCTION      (MAKEFFXERR(FFXLOCALE_EXTAPI, 19))

/*  Driver Framework
*/
#define FFXSTAT_DRVDEVAUTOMOUNTFAILED     (MAKEFFXERR(FFXLOCALE_DRIVERFW,   1))
#define FFXSTAT_FORMATFAILED              (MAKEFFXERR(FFXLOCALE_DRIVERFW,   2))
#define FFXSTAT_DEVPARAMSFAILED           (MAKEFFXERR(FFXLOCALE_DRIVERFW,   3))
#define FFXSTAT_DEVINITFAILED             (MAKEFFXERR(FFXLOCALE_DRIVERFW,   4))
#define FFXSTAT_DRVLOADCANCELLED          (MAKEFFXERR(FFXLOCALE_DRIVERFW,   5))
#define FFXSTAT_DRVALREADYLOADED          (MAKEFFXERR(FFXLOCALE_DRIVERFW,   6))
#define FFXSTAT_DRVINITFAILED             (MAKEFFXERR(FFXLOCALE_DRIVERFW,   7))
#define FFXSTAT_DRVDRIVECREATEFAILED      (MAKEFFXERR(FFXLOCALE_DRIVERFW,   8))
#define FFXSTAT_DRVDEVMOUNTFAILED         (MAKEFFXERR(FFXLOCALE_DRIVERFW,   9))
#define FFXSTAT_BADSECTORSIZE             (MAKEFFXERR(FFXLOCALE_DRIVERFW,  10))
#define FFXSTAT_DRVCOMPACTSUSPENDFAILED   (MAKEFFXERR(FFXLOCALE_DRIVERFW, 100))
#define FFXSTAT_DRVCOMPACTRESUMEFAILED    (MAKEFFXERR(FFXLOCALE_DRIVERFW, 101))
#define FFXSTAT_DRVCOMPACTSUSPENDED       (MAKEFFXERR(FFXLOCALE_DRIVERFW, 102))
#define FFXSTAT_DRVCOMPACTALREADYSUSPENDED (MAKEFFXERR(FFXLOCALE_DRIVERFW, 103))
#define FFXSTAT_DRVCOMPACTNOTENABLED      (MAKEFFXERR(FFXLOCALE_DRIVERFW, 104))
#define FFXSTAT_FATMONDISABLED            (MAKEFFXERR(FFXLOCALE_DRIVERFW, 200))
#define FFXSTAT_FATMONPARTITIONNOTFOUND   (MAKEFFXERR(FFXLOCALE_DRIVERFW, 201))
#define FFXSTAT_FATMONBADSECTORSIZE       (MAKEFFXERR(FFXLOCALE_DRIVERFW, 202))
#define FFXSTAT_DRV_FORMATDISABLED        (MAKEFFXERR(FFXLOCALE_DRIVERFW, 250))
#define FFXSTAT_DRV_NOFILESYSTOFORMAT     (MAKEFFXERR(FFXLOCALE_DRIVERFW, 251))
#define FFXSTAT_DRV_UNKNOWNALLOCATOR      (MAKEFFXERR(FFXLOCALE_DRIVERFW, 252))
#define FFXSTAT_DRV_NOFORMATSETTINGS      (MAKEFFXERR(FFXLOCALE_DRIVERFW, 253))
#define FFXSTAT_DRV_SECTORRANGEINVALID    (MAKEFFXERR(FFXLOCALE_DRIVERFW, 254))
#define FFXSTAT_DRV_NOALLOCATOR           (MAKEFFXERR(FFXLOCALE_DRIVERFW, 255))
#define FFXSTAT_DRV_LOCKFAILED            (MAKEFFXERR(FFXLOCALE_DRIVERFW, 256))

#define FFXSTAT_INVALIDDEVICE             (MAKEFFXERR(FFXLOCALE_DRIVERFW, 300))
#define FFXSTAT_HOOKDISKCREATEFAILED      (MAKEFFXERR(FFXLOCALE_DRIVERFW, 301))
#define FFXSTAT_DISKRANGEINVALID          (MAKEFFXERR(FFXLOCALE_DRIVERFW, 302))
#define FFXSTAT_DISKRANGENOTMODBLOCKSIZE  (MAKEFFXERR(FFXLOCALE_DRIVERFW, 303))
#define FFXSTAT_DISKMAPPINGFAILED         (MAKEFFXERR(FFXLOCALE_DRIVERFW, 304))
#define FFXSTAT_DISK_DEVICENOTINITIALIZED (MAKEFFXERR(FFXLOCALE_DRIVERFW, 305))
#define FFXSTAT_DEVICENUMBERINVALID       (MAKEFFXERR(FFXLOCALE_DRIVERFW, 306))
#define FFXSTAT_DISKNUMBERINVALID         (MAKEFFXERR(FFXLOCALE_DRIVERFW, 307))
#define FFXSTAT_INVALIDFORMATSTATE        (MAKEFFXERR(FFXLOCALE_DRIVERFW, 308))
#define FFXSTAT_INVALIDFORMATTYPE         (MAKEFFXERR(FFXLOCALE_DRIVERFW, 309))
#define FFXSTAT_DISK_ZEROLENGTH           (MAKEFFXERR(FFXLOCALE_DRIVERFW, 310))
#define FFXSTAT_DISK_SETTINGSMISSING      (MAKEFFXERR(FFXLOCALE_DRIVERFW, 311))
#define FFXSTAT_DISK_FMTSETTINGSMISSING   (MAKEFFXERR(FFXLOCALE_DRIVERFW, 312))
#define FFXSTAT_DISK_ALREADYINITIALIZED   (MAKEFFXERR(FFXLOCALE_DRIVERFW, 313))
#define FFXSTAT_DISK_FATUNSUPPORTED       (MAKEFFXERR(FFXLOCALE_DRIVERFW, 314))
#define FFXSTAT_DISK_RELIANCEUNSUPPORTED  (MAKEFFXERR(FFXLOCALE_DRIVERFW, 315))
#define FFXSTAT_DISK_BADMBRSETTING        (MAKEFFXERR(FFXLOCALE_DRIVERFW, 316))
#define FFXSTAT_DISK_BADFILESYSSETTING    (MAKEFFXERR(FFXLOCALE_DRIVERFW, 317))
#define FFXSTAT_DISK_MBRSUPPORTDISABLED   (MAKEFFXERR(FFXLOCALE_DRIVERFW, 318))
#define FFXSTAT_DISK_MBRFORMATDISABLED    (MAKEFFXERR(FFXLOCALE_DRIVERFW, 319))
#define FFXSTAT_DISK_HRFSUNSUPPORTED      (MAKEFFXERR(FFXLOCALE_DRIVERFW, 320))
#define FFXSTAT_DISK_ALLOCATORINITED      (MAKEFFXERR(FFXLOCALE_DRIVERFW, 321))
#define FFXSTAT_DISK_CREATEFAILED         (MAKEFFXERR(FFXLOCALE_DRIVERFW, 322))
#define FFXSTAT_DISK_FLAGSINVALID         (MAKEFFXERR(FFXLOCALE_DRIVERFW, 323))
#define FFXSTAT_DISK_OUTOFSLOTS           (MAKEFFXERR(FFXLOCALE_DRIVERFW, 324))

#define FFXSTAT_DEVICE_CREATEFAILED       (MAKEFFXERR(FFXLOCALE_DRIVERFW, 400))

/*  Project Hooks
*/
#define FFXSTAT_PROJBADACCESSTYPE         (MAKEFFXERR(FFXLOCALE_PROJHOOKS, 1))
#define FFXSTAT_PROJUNSUPPORTEDOPTION     (MAKEFFXERR(FFXLOCALE_PROJHOOKS, 2))
#define FFXSTAT_PROJMAPMEMFAILED          (MAKEFFXERR(FFXLOCALE_PROJHOOKS, 3))
#define FFXSTAT_HOOKS_OUTOFENTRIES        (MAKEFFXERR(FFXLOCALE_PROJHOOKS, 4))

/*  OS Driver Specific
*/
#define FFXSTAT_NOTFORMATTED              (MAKEFFXERR(FFXLOCALE_OSDRIVER, 1))
#define FFXSTAT_READFAILED                (MAKEFFXERR(FFXLOCALE_OSDRIVER, 2))
#define FFXSTAT_WRITEFAILED               (MAKEFFXERR(FFXLOCALE_OSDRIVER, 3))
#define FFXSTAT_BADDRIVEID                (MAKEFFXERR(FFXLOCALE_OSDRIVER, 4))
#define FFXSTAT_SECTOROUTOFRANGE          (MAKEFFXERR(FFXLOCALE_OSDRIVER, 5))

/*  NOTE! Status codes in this locale must have values
          in the low bytes that exactly match the standard
          DOS device driver return values.
*/
#define FFXSTAT_DOSDEVERR_WRITEPROTECTED  (MAKEFFXERR(FFXLOCALE_DOSDEV, 0))
#define FFXSTAT_DOSDEVERR_BADUNIT         (MAKEFFXERR(FFXLOCALE_DOSDEV, 1))
#define FFXSTAT_DOSDEVERR_NOTREADY        (MAKEFFXERR(FFXLOCALE_DOSDEV, 2))
#define FFXSTAT_DOSDEVERR_BADCMD          (MAKEFFXERR(FFXLOCALE_DOSDEV, 3))
#define FFXSTAT_DOSDEVERR_CRC             (MAKEFFXERR(FFXLOCALE_DOSDEV, 4))
#define FFXSTAT_DOSDEVERR_BADSTRUCT       (MAKEFFXERR(FFXLOCALE_DOSDEV, 5))
#define FFXSTAT_DOSDEVERR_SEEK            (MAKEFFXERR(FFXLOCALE_DOSDEV, 6))
#define FFXSTAT_DOSDEVERR_BADMEDIA        (MAKEFFXERR(FFXLOCALE_DOSDEV, 7))
#define FFXSTAT_DOSDEVERR_SECTORNOTFOUND  (MAKEFFXERR(FFXLOCALE_DOSDEV, 8))
#define FFXSTAT_DOSDEVERR_NOPAPER         (MAKEFFXERR(FFXLOCALE_DOSDEV, 9))
#define FFXSTAT_DOSDEVERR_WRITEFAULT      (MAKEFFXERR(FFXLOCALE_DOSDEV, 10))
#define FFXSTAT_DOSDEVERR_READFAULT       (MAKEFFXERR(FFXLOCALE_DOSDEV, 11))
#define FFXSTAT_DOSDEVERR_GENERALFAIL     (MAKEFFXERR(FFXLOCALE_DOSDEV, 12))

/*  Loader
*/
#define FFXSTAT_LOADER_PARTITIONNOTFOUND  (MAKEFFXERR(FFXLOCALE_LOADER, 1))
#define FFXSTAT_LOADER_NOALLOCATOR        (MAKEFFXERR(FFXLOCALE_LOADER, 2))
#define FFXSTAT_FATRDR_UNSUPPORTEDFORMAT  (MAKEFFXERR(FFXLOCALE_LOADER, 100))
#define FFXSTAT_RELRDR_UNSUPPORTEDFORMAT  (MAKEFFXERR(FFXLOCALE_LOADER, 200))
#define FFXSTAT_RELRDR_MOUNTFAILED        (MAKEFFXERR(FFXLOCALE_LOADER, 201))

/*  Miscellaneous
*/
#define FFXSTAT_BADDISKGEOMETRY              (MAKEFFXERR(FFXLOCALE_MISC, 1))
#define FFXSTAT_PERFLOG_OPENFAILED           (MAKEFFXERR(FFXLOCALE_MISC, 2))
#define FFXSTAT_FATFMT_UNSUPPORTEDSECTORSIZE (MAKEFFXERR(FFXLOCALE_MISC, 100))
#define FFXSTAT_FATFMT_INVALIDGEOMETRY       (MAKEFFXERR(FFXLOCALE_MISC, 101))
#define FFXSTAT_FATFMT_BPBCREATIONFAILED     (MAKEFFXERR(FFXLOCALE_MISC, 102))
#define FFXSTAT_SHELL_NOCURRENTDISK          (MAKEFFXERR(FFXLOCALE_MISC, 120))
#define FFXSTAT_INVALIDSTATE                 (MAKEFFXERR(FFXLOCALE_MISC, 140))
#define FFXSTAT_NOTMOUNTED                   (MAKEFFXERR(FFXLOCALE_MISC, 141))
#define FFXSTAT_NOT_ONFI_COMPLIANT           (MAKEFFXERR(FFXLOCALE_PROJHOOKS, 142))
#define FFXSTAT_NO_EZ_NAND_SUPPORT           (MAKEFFXERR(FFXLOCALE_PROJHOOKS, 143))


#endif
