/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2012 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

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

    This module implements the NAND FIM, which is used by all NAND flash
    which FlashFX supports.  Support for a variety of different NAND flash
    and NAND flash controllers are further abstracted in the NAND Technology
    Module (NTM) interface.  The NAND FIM is responsible for loading and
    initializing NTMs.

    Notes on NAND flash...

    NAND flash is accessed serially and constructed of addressable pages.
    The pages actually consist of 2 parts, a common (aka normal, user) data
    section and a reserved (aka hidden, spare) section.  NAND is generally
    divided into two categories -- small-block, and large-block flash.  Small
    block flash generally has an erase zone size of 16KB or smaller, and a
    page size of 512 bytes.  Large block flash generally has erase zone sizes
    of 128KB or larger, and a page size of 2048 bytes.  The spare area sizes
    for the two types of flash are 16 and 64 bytes respectively.

    The Solid State Floppy Disk Card (SSFDC) Forum (aka SMART cards) specifies
    the format of the spare area within each page of flash.  The spare area
    includes Error Correcting Codes (ECCs) that are used to determine if a
    block of NAND flash has gone bad, as they tend to do over time.  Every
    time we read a page from a NAND part we validate its ECC.  The ECC code is
    actually a parity matrix that will detect double bit errors and correct
    single bit errors.  There is an ECC for every 256 bytes in a page.  The
    SSFDC specification dictates that a factory bad block is denoted by one
    or more zero bits in the byte at offset 5 in the spare areas for either
    of the first two pages in the block.

    Note that the SSFDC specification only handles small block flash with an
    8-bit interface.  Large block NAND, and NAND with interfaces wider than
    8-bits are not dealt with in the specification.  In particular, the method
    for marking bad blocks is different.  These differences are dealt with in
    the NTMs, and are hidden from other layers.

    BBM handles remapping bad blocks with good blocks contained in a region
    reserved when the flash is formatted.  This remapping dynamically replaces
    blocks of NAND flash that have gone bad.

    Many larger capacity NAND flash chip specifications require pages to be
    written sequentially.  This is because out of order writes increase the
    "write disturb" effect and can cause uncorrectable errors (more than one
    bit per page).  The FIM and BBM make the assumption that this restriction
    is observed.  When a block is being replaced due to a write error, only
    pages before the one where the error occurred are copied to the replacement
    block.

    Tag Characteristics and Implementation

    A tag is a mechanism for attaching metadata to each NAND page which is
    written.  This metadata is used by higher layers in FlashFX to implement
    the logical to physical mapping of client sectors onto physical NAND
    flash pages.

    The following behaviors must be observed when implementing tag support
    in an NTM:

    1) Tags MUST be written atomically with each NAND page.  This is critical
       in allowing FlashFX to recover properly from write interruptions.
    2) Tags must be protected from single bit errors in some fashion.  Some
       NAND controllers have a mechanism for applying ECC to data stored
       within the spare area.  If such a mechanism is not available, then the
       NTM must use a Hamming or ECC code to protect the tag.
    3) Any attempt to read a tag from a page which has never been programmed
       should return a tag value which is all 0xFFs.
    4) Any attempt to read a tag value which is not valid, or perhaps has been
       corrupted by a multi-bit error, must return a tag value of all 0x00s.
    5) In accordance with points 3 and 4, FlashFX will never write tag values
       of all 0xFFs, and will only write tags of all 0x00s when it needs to
       invalidate a page.

    FlashFX uses a standard 2-byte tag.  The typical NTM behavior is to use
    a third byte as a check byte (a ^ ~b), and then apply a Hamming code to
    those three bytes, storing the result in a fourth byte.  The Hamming code
    will correct single-bit errors, but provides no means for detecting multi-
    bit errors.  The check byte helps ensure the tag integrity.  Functions
    for encoding and decoding tags can be found in nthelp.c.

    Whatever mechanism is used to store the tag within the spare area should
    do so in a way that the tag is distinguishable from tags which have never
    been written and are still in an erased state, or within 1 bit of being
    erased (for SLC flash).  This means that an encoded tag, regardless what
    the original tag value was, must have at least two bits which are off.
    This way the encoded tag can be distinguished from an erased tag which
    has a single bit error.

    The GetRawPageStatus() NTM function, in addition to returning whether
    the page was written with ECC or not, must also return the length of
    the tag, if any.  The macros PAGESTATUS_GET/SET_TAG_WIDTH() can be used
    to facilitate this.  It is important that the tag size be returned as
    zero if the page was not written with a tag.

    Note that some NTMs may use the tag as a means to determine whether
    multiple pages where atomically written.  For example, an NTM might
    supports dual plane operations in a mode where a larger virtual page
    size is created (ntmicron.c), or an NTM may simply combine smaller
    pages to create a larger virtual page (ntpageio.c).  In these cases,
    there MUST be a way to ensure that the full virtual page is atomically
    written.  One way to do this is to use the tag field, ensuring that the
    same tag value is written to each page.  Since FlashFX uses the tag to
    determine whether a page is valid or not, the NTM can examine the tag
    values from each segment of the virtual page to ensure that all the
    tags match.  If they do, then the tag is returned to the caller, properly
    indicating that the page is normal and complete.  If they do not match,
    then a NULL tag value is returned, which as described above, is the
    indicator to FlashFX that the page is not valid.

    Variable Length Tags

    Some NTMs support variable length tags.  This is useful when running
    alternate software on top of FlashFX, which uses the FML interface.  It
    is possible that future versions of FlashFX will make use of other, most
    likely, larger tags.

    NTMs must set the NtmInfo.uMetaSize field to the size of tag that the
    NTM supports.  If variable length tags are supported, than the flag
    DEV_VARIABLE_LENGTH_TAGS must be set, and the NTM MUST support any tag
    size from 1 to the specified length.  If variable length tag sizes are
    not supported, then the DEV_VARIABLE_LENGTH_TAGS flag must not be set.

    The allocator layer (VBF) in FlashFX only works with two byte tags at this
    time.  Therefore, if an NTM has an NtmInfo.uMetaSize value of something
    other than two, and the DEV_VARIABLE_LENGTH_TAGS is NOT set, VBF will
    fail to initialize.

    When using variable length tags, the GetRawPageStatus() function must
    accurately report the tag length for the particular page in question, or
    report 0 if the page does not have a tag.

    The FMSLTEST unit test verifies the tag behaviors described herein,
    including exercising all the various tag lengths for those NTMs which
    support variable length tags.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nand.c $
    Revision 1.98  2012/03/02 19:14:42Z  garyp
    Minor documentation and debug message fixes -- no functional changes.
    Revision 1.97  2011/11/21 17:35:14Z  glenns
    Add support for standard and enhanced ClearNAND NTMs.
    Revision 1.96  2011/04/06 20:06:22Z  jimmb
    Added MX-35 NTM.
    Revision 1.95  2011/02/07 21:19:47Z  jimmb
    Added the nVidia Tegra 2.
    Revision 1.94  2010/12/14 00:44:46Z  glenns
    Remove temporary change from previous checkin; fix located in NTMs.
    Revision 1.93  2010/12/12 15:37:55Z  garyp
    Try that again...
    Revision 1.92  2010/12/12 12:58:50Z  garyp
    Temporarily hacked to get the nightly build to complete.
    Revision 1.91  2010/12/10 19:11:44Z  glenns
    Fix Bug 3213: Replaced patch in Create procedure that makes sure
    pFimInfo->uEdcSegmentSize is set with simple checks on the
    value. NTMs now handle this correctly.
    Revision 1.90  2010/09/28 21:24:00Z  glenns
    Back out previous change pending product release.
    Revision 1.89  2010/08/05 21:19:18Z  glenns
    Add [TRADE NAME TBA] NTM.
    Revision 1.88  2010/06/20 00:19:33Z  garyp
    Removed a bogus assert.
    Revision 1.87  2010/06/19 17:46:52Z  garyp
    Temporary workaround for Bug 3213.
    Revision 1.86  2010/06/19 04:31:28Z  garyp
    Updated to set the bit correction capabilities on a per segment basis,
    using default values if necessary.
    Revision 1.85  2009/08/27 19:35:50Z  jimmb
    Added support for the ads5120 (supplied by WindRiver)
    Revision 1.84  2009/08/04 19:42:52Z  garyp
    Merged from the v4.0 branch.  Added the "Read/WriteUncorrectedPages" IOCTLs
    and eliminated the "ReadCorrectedPages" IOCTL.  The old plain "Read" and 
    "Write" IOCTLs which used to do uncorrected operations, now do corrected 
    ones.  The internal PageWrite function has been modified to allow pages to 
    be written with no tags, yet still use ECCs.  Added OTP support.  Added 
    support for block locking and unlocking.  Added an optional IORequest() 
    interface into the NTMs.  The NAND FIM will call this first if it exists.
    If the NTM does not handle the request, then the standard logic in the
    NAND FIM is used. 
    Revision 1.83  2009/04/09 21:26:36Z  garyp
    Removed support for the Atlas NTM.
    Revision 1.82  2009/04/08 20:32:13Z  garyp
    Renamed the public functions so they fit smoothly into the automated
    documentation system.  No other functional changes.
    Revision 1.81  2009/04/03 20:43:42Z  davidh
    Function header error corrected for AutoDoc.
    Revision 1.80  2009/04/01 20:30:50Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.79  2009/03/25 23:14:48Z  glenns
    - Fix Bug 2464: Clarify commentary to explain difference between
      the NAND and NOR implementations of ErasePoll.
    - Fix minor commentary format and typo.
    Revision 1.78  2009/03/23 18:31:20Z  billr
    Resolve Bug 2538: FMSLTest causes assertion failure on Tx4938.
    Revision 1.77  2009/02/26 00:15:16Z  billr
    Resolve Bug 2448: NAND FIM returns incorrect status for some erase
    failures.  Resolve Bug 2455: NAND program failure may not cause block
    replacement.
    Revision 1.76  2009/02/13 21:41:57Z  glenns
    - Fixed by initializing the ioStat in fmsl/nand/nand.c's IORequest
      function to FIM_UNSUPPORTEDIOREQUEST rather than using
      using a DCL function to zero out the structure.
    Revision 1.75  2009/02/11 00:20:12Z  billr
    Resolve Bug 2364: NAND FIM comments are wrong.  Change some
    assertions to match corrected comments.
    Revision 1.74  2009/02/07 03:30:47Z  glenns
    - Added the MLC version of ntcad to the list of available NTMs.
    Revision 1.73  2009/02/06 02:13:35Z  keithg
    Updated to reflect new location of NAND header files and macros,.
    Revision 1.72  2009/02/02 12:53:07  billr
    Implement FXIOSUBFUNC_FIM_READ_NATIVEPAGES by calling a FIM
    NativePageRead() method if it is available, with a fallback to
    RawPageRead() if it is not.
    --- Added comments ---  billr [2009/02/02 21:23:58Z]
    Make that "calling an NTM NativePageRead() method."
    Revision 1.71  2009/01/31 01:19:33Z  billr
    - Remove dead code related to BBM, or needed only for the old FIM
      interface.
    - Use a consistent method of avoiding NTM calls that cross erase
      block boundaries (this is temporary until all NTMs are capable
      of tolerating this).  Do this without needing a modulo
      operation on each pass through the loop.
    - Remove spurious typecasts.
    - Remove macros that hid (some of) the calls to NTM methods.
    - Uniformly pass back the FFXIOSTATUS values returned by the NTM.
    - Resolve Bug 2330: NAND HiddenRead() does not return correct
      count of tags.
    - Corrected format specifications in some debug output.
    Revision 1.70  2009/01/28 20:08:25Z  billr
    Implement "write native pages" (which is the same as "raw pages"
    at the FIM level), and stub "read native pages" to "read raw
    pages" for now.
    Revision 1.69  2009/01/23 22:32:11Z  glenns
    - Modified to accomodate structure variable names that were
      updated to Datalight coding standards.
    Revision 1.68  2009/01/18 08:55:01Z  keithg
    Removed OLDNANDDEVICE and BBM v5 conditioned code.  BBM
    interaction and reserved space are now handled outside of the FIM.
    Revision 1.67  2009/01/16 05:24:05Z  glenns
    - Fixed up literal FFXIOSTATUS initialization in four places.
    - Modified HiddenRead to return an FFXIOSTATUS instead of
      boolean. Enables Error Manager encoding to be propagated
      upward if errors occur during HiddenRead.
    - Modified IORequest to return FFXIOSTATUS returned by
      HiddenRead to the caller rather than recreate it based on the
      status of a boolean.
    Revision 1.66  2009/01/07 01:06:36Z  billr
    Fix warning about GetBlockStatus() defined but not used.
    Revision 1.65  2008/12/18 17:13:05Z  keithg
    Replace the WriteNativePages functionality which as incorrectly
    removed in the previous check in.
    Revision 1.64  2008/12/18 08:39:13Z  keithg
    Removed unused IOCTLs, re-enabled status functions.
    Revision 1.63  2008/12/17 23:53:45Z  keithg
    Removed duplicated code, removed unused IOCTL.
    Revision 1.62  2008/12/12 07:33:14Z  keithg
    Re-added set block status; added stubs for read/write native pages.
    Revision 1.61  2008/12/02 23:48:21Z  keithg
    Conditioned inclusion of the old BBM header.  Corrected BBM v5
    switch statement.  Eliminated constant conditionals.
    Revision 1.60  2008/11/13 00:14:39Z  billr
    Fix warnings about GetPageStatus() and GetBlockStatus() defined
    but not used.  GetPageStatus() isn't needed at all.
    FXIOFUNC_FIM_GET_BLOCK_STATUS still needs to be implemented.
    Revision 1.59  2008/10/21 00:28:45Z  keithg
    Changed BBM_OBSOLESCENT to BBM_USE_V5 for clarity;
    Conditioned block and page status functions on use of the new BBM.
    Revision 1.58  2008/09/02 05:22:34Z  keithg
    Changed semantics to describe a media rather than a disk to
    avoid confusion with other 'disks'.  Raw block status is no longer
    dependant upon the inclusion of BBM functionality.
    Revision 1.57  2008/07/25 17:04:42Z  billr
    Consistent use of const on input buffers of write functions.
    Revision 1.56  2008/06/24 21:30:43Z  thomd
    Propagate fields, bringing info up from ntminfo
    Revision 1.55  2008/05/09 17:31:24Z  garyp
    Modified a message.
    Revision 1.54  2008/05/08 01:25:54Z  garyp
    Documenation update.
    Revision 1.53  2008/05/03 20:37:10Z  garyp
    Modified so that FFXCONF_NANDSUPPORT_MICRON controls whether the Micron NTM
    is included or not.
    Revision 1.52  2008/03/23 20:30:17Z  Garyp
    Updated the interfaces which use tags so that the tag size is specified.
    Renamed the "Get/SetBlockStatus" functions to "Get/SetRawBlockStatus" for
    interface consistency.  Added a GetBlockStatus interface which allows it
    to be easily determined whether a logical block has been remapped or not.
    Removed support for the byteio, mx21, and omap5912 NTMs.  Modified the BBM
    initialization process so that it can now be prevented from being used at
    run-time, rather than just at compile time.
    Revision 1.51  2008/02/03 01:42:46Z  keithg
    comment updates to support autodoc.
    Revision 1.50  2008/01/13 01:50:09Z  Garyp
    Clarified a debug message.
    Revision 1.49  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.48  2007/10/17 21:50:08Z  pauli
    Updated trace settings to reduce general output.
    Revision 1.47  2007/10/04 15:03:29Z  thomd
    Compare using pointer not structure element
    Revision 1.46  2007/10/04 02:28:07Z  Garyp
    Implement the default alignment based on DCL_CPU_ALIGNSIZE rather than
    DCL_ALIGNSIZE.  Fixed an issue with the previous rev where the wrong
    structure was being used in the alignment check.
    Revision 1.45  2007/09/28 01:20:59Z  pauli
    Resolved Bug 355: Added an alignment field to the NTMINFO structure.
    Revision 1.44  2007/09/21 04:35:46Z  pauli
    Report that BBM stats are disabled if BBM is disabled.
    Revision 1.43  2007/08/28 03:35:39Z  Garyp
    Added the Micron NTM.  Fixed so GetBlockStatus() is available even
    when BBM is turned off.
    Revision 1.42  2007/08/01 00:07:16Z  timothyj
    Added minor assertions that flash block size is evenly divisible by the
    page size.
    Revision 1.41  2007/07/16 20:24:18Z  Garyp
    Updated to use a corrected FXSTATUS value.
    Revision 1.40  2007/07/12 18:21:33Z  timothyj
    Updated assertions for Large Flash Array.
    Revision 1.39  2007/06/29 21:13:29Z  rickc
    Added GetBlockStatus to IORequest()
    Revision 1.38  2007/06/16 23:58:57Z  Garyp
    Resolved Bug 996: ValidatedRead() now returns an FFXIOSTATUS rather than
    D_BOOL.  Corrected a variety of documentation deficiencies.
    Revision 1.37  2007/04/15 18:01:37Z  Garyp
    Made the BBM stats support conditional on the symbol FFXCONF_STATS_BBM.
    Revision 1.36  2007/04/07 03:25:13Z  Garyp
    Modified the "ParameterGet" function so that the buffer size for a given
    parameter can be queried without actually retrieving the parameter.
    Revision 1.35  2007/02/28 00:54:46Z  timothyj
    Moved uPagesPerBlock into the FimInfo structure, where it can be used by
    other modules to which FimInfo is public, rather than computing or storing
    an additional copy of the same value.  Added reference to PXA320 NTM.
    Revision 1.34  2007/02/15 19:05:44Z  timothyj
    Changed OLDNANDDEVICE interfaces into nand.c, and some internal interfaces
    used by the IoRequest dispatch function from using linear byte offsets to
    using page numbers, for Large Flash Array support.  Added uPagesPerBlock to
    FIMDATA to reduce the number of places where this is re-calculated, for
    optimization and code simplification.  Changed D_UINT16 uCount parameters
    to D_UIN32 ulCount, to allow the call tree all the way up through the
    IoRequest to avoid having to range check (and/or split) requests.  Removed
    corresponding casts.  Modified BBM_MAP_BLOCK to take and return pages
    rather than blocks, for optimization and code simplification.
    Revision 1.33  2007/02/12 23:49:00Z  timothyj
    Updated function calls into NTMs to use blocks and pages instead of linear
    byte offsets.  Removed deprecated SpareWrite and ValidatedRead functions
    from OLDNANDDEVICE interface.
    Revision 1.32  2007/02/01 03:30:27Z  Garyp
    Updated to allow erase-suspend support to be compiled out of the product.
    Revision 1.31  2006/12/14 21:42:15Z  Garyp
    Added debug code -- no functional changes.
    Revision 1.30  2006/11/14 17:54:41Z  billr
    Add support for new parameter to get BBM information.
    Revision 1.29  2006/11/10 20:21:31Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.28  2006/08/30 19:06:25Z  Garyp
    Added the MX31 NTM.
    Revision 1.27  2006/07/19 19:59:21Z  Pauli
    Added NAND Simulator NTM.
    Revision 1.26  2006/07/10 23:57:30Z  Garyp
    Renamed fhcadio to fhcad.
    Revision 1.25  2006/06/16 14:29:55Z  johnb
    Added LSI NTM to list of NTMs.
    Revision 1.24  2006/05/17 22:02:48  Garyp
    Modified to set the ulReservedBlocks and ulChipBlocks fields in the
    FimInfo structure.
    Revision 1.23  2006/05/07 16:27:31Z  timothyj
    Added cadio NTM
    Revision 1.22  2006/03/22 07:41:29Z  Garyp
    Updated the function comments.
    Revision 1.21  2006/03/18 20:40:25Z  Garyp
    Removed some dead code.
    Revision 1.20  2006/03/12 22:16:40Z  Garyp
    Added the support for the "atlas" NTM.
    Revision 1.19  2006/03/10 07:26:54Z  Garyp
    Fixed to not call BBM_MAP_BLOCK for Get/SetBlockStatus() and
    GetPageStatus().
    Revision 1.18  2006/03/08 21:18:47Z  Garyp
    Changed the NTM init process back to using a list of possible NTMs, rather
    than a single fixed NTM per Device.
    Revision 1.17  2006/03/08 02:17:54Z  Garyp
    Updated to build cleanly if BBM is compiled out.
    Revision 1.16  2006/03/07 20:50:21Z  Garyp
    Added RawPageRead/Write() support.  Fixed ReadData() to increment the tag
    pointer by the tag size (was benign for current use cases).  Fixed some
    invalid asserts.
    Revision 1.15  2006/03/07 00:44:00Z  billr
    Add MX21 NTM.
    Revision 1.14  2006/03/03 07:37:47Z  Garyp
    Debug code updated.
    Revision 1.13  2006/02/27 06:40:13Z  Garyp
    Removed all knowledge of the tag format.  Modified PageRead() to return
    only the tag, rather than the entire spare area.  Added GetPageStatus().
    Revision 1.12  2006/02/16 02:06:22Z  Garyp
    Removed some dead code.
    Revision 1.11  2006/02/15 00:32:03Z  Garyp
    Corrected compiler warnings.
    Revision 1.10  2006/02/12 18:48:20Z  Garyp
    Updated to the new tools/tests model where device/disk numbers are
    used, rather than handles.
    Revision 1.9  2006/02/11 23:49:11Z  Garyp
    Tweaked to build cleanly.
    Revision 1.8  2006/02/11 03:19:00Z  Garyp
    No longer scan NTMs, but rather load the specific one configured for the
    device in ffxconf.h.
    Revision 1.7  2006/02/10 21:09:49Z  Garyp
    Renamed the FIMDEVICE structure instantiation for clarity.
    Revision 1.6  2006/02/09 22:25:21Z  Garyp
    Updated to no longer use the EXTMEDIAINFO structure.  Updated to the
    new IORequest model.
    Revision 1.5  2006/02/03 22:04:00Z  timothyj
    Added OneNAND extern declaration.
    Revision 1.4  2006/02/03 21:30:01Z  timothyj
    Added reference to OneNAND NTM
    Revision 1.3  2006/01/25 03:57:53Z  Garyp
    Updated to conditionally build only if NAND support is enabled.
    Revision 1.2  2006/01/11 02:12:00Z  Garyp
    Eliminated support for IsWriteProtected() and GetChipInfo().
    Revision 1.1  2005/12/02 01:13:28Z  Pauli
    Initial revision
    Revision 1.6  2005/12/02 01:13:28Z  Garyp
    Added the Get/SetBlockStatus() functions.  Commented out ValidatedWrite()
    since it is not used anywhere (any more).  Updated to work with the new BBM
    load process (at long last).  Modified so BBM usage can be compiled out of
    the system.  Modified to work with a number of NTM level functions that now
    return FFXIOSTATUS structures.  Removed dead code.
    Revision 1.5  2005/11/15 21:59:47Z  Garyp
    Debug code updated.
    Revision 1.4  2005/11/07 17:23:53Z  Garyp
    Major update to support run-time page size determination.
    Revision 1.3  2005/10/22 08:23:50Z  garyp
    Minor cleanup -- no functional changes.
    Revision 1.2  2005/10/21 03:47:14Z  garyp
    Fixed a benign problem where we were mapping blocks more often than was
    necessary.  Fixed a non-benign problem in HiddenRead where we were not
    mapping blocks correctly.
    Revision 1.1  2005/10/14 03:22:22Z  Garyp
    Initial revision
    Revision 1.2  2005/10/11 18:31:03Z  Garyp
    Modified the PageWrite() interface to take a page count.  Modified
    HiddenRead() to call the NTM with multiple sectors to read if possible.
    Revision 1.1  2005/10/05 23:12:18Z  Garyp
    Initial revision
    Revision 1.3  2005/09/09 03:07:31Z  garyp
    Updated to build cleanly.
    Revision 1.2  2005/09/07 04:25:09Z  Garyp
    Merge from the main code tree -- Bill's comments:
    A flash programming error in a redundant write no longer attempts
    block replacement.  Redundant write is only used during BBM
    formatting, and BBM obviously isn't able to replace a block then.
    Change bad block replacement policy.  Block replacement now occurs
    only when the NAND chip status indicates a program or erase error,
    never for an ECC error.  Correctable ECC errors are now quietly
    corrected without block replacement.  Uncorrectable ECC errors now
    cause read errors instead of copying the corrupted data to a
    replacement block and indicating success.  Read errors during block
    replacement that were previously ignored now stop the replacement and
    cause the write to fail instead of silently copying garbage.  Add
    block replacement when hidden write fails.  Eliminate some dead code
    (like unused loops in page read/write).  Fix spurious failure reading
    write-protected flash.  Fix problem in NANDRedundantWrite() that would
    cause data to be written to old (bad) block after replacement.  Fix
    possible buffer overrun with bad argument to NANDRedundantRead().
    Always report flash manufacturer and part (not just in debug build).
    Revision 1.1  2005/08/05 17:40:56Z  pauli
    Initial revision
    Revision 1.7  2005/08/05 17:40:56Z  Garyp
    Documentation and formatting cleanup -- no functional changes.
    Revision 1.6  2005/08/03 19:31:52Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.4  2005/07/31 05:41:17Z  Garyp
    Updated to use new profiler leave function which now takes a ulUserData
    parameter.
    Revision 1.3  2005/07/30 20:34:00Z  Garyp
    Updated to support read-ahead capability.
    Revision 1.2  2005/07/29 19:31:20Z  Garyp
    Updated to compile cleanly.
    Revision 1.1  2005/07/28 21:58:10Z  pauli
    Initial revision
    Revision 1.40  2005/06/30 22:39:28Z  billr
    Eliminate unnecessary waits for chip to be ready in cases where it is
    already known to be ready. This enables using a Read Status command
    in the NAND Control Module to check for ready (for hardware that doesn't
    allow software to read the Ready/Busy signal from the chip). Read Status
    isn't allowed in the middle of some command sequences that previously
    had superfluous checks for ready.
    Revision 1.39  2005/05/17 18:48:40Z  garyp
    Removed a duplicated device ID (Samsung x46).  Reordered some of the
    part lists for easier correlation with the FIM list.
    Revision 1.38  2005/05/02 22:39:23Z  billr
    Eliminate warning with Borland 5.2 compiler.
    Revision 1.37  2005/04/29 23:34:44Z  billr
    Rewrite manufacturer/device ID recognition to be table driven. Add
    table entries for all the parts I could find data sheets on, specifically
    including some Hynix and MIcron parts customers have been inquiring
    about recently.
    Revision 1.36  2005/04/01 18:13:45Z  billr
    Add ID for Samsung K9K1216Q0C (512 Mbit, 16-bit I/O).
    Revision 1.35  2005/03/27 19:26:35Z  GaryP
    Standardized the function headers.  General code formatting cleanup.
    Added some debug code.  Changed functions to static where possible.
    Renamed public but non-published functions to reduce name-space pollution.
    Revision 1.34  2005/03/24 23:09:35Z  GaryP
    Minor formatting changes.
    Revision 1.33  2004/12/30 23:17:53Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.32  2004/11/08 18:14:51Z  GaryP
    Added a warning message if the BBM format state is undefined.
    Revision 1.31  2004/08/12 01:23:22Z  GaryP
    Updated to use the renamed "nandctl" functions which are now part of the
    Project Hooks interface.
    Revision 1.30  2004/08/04 21:52:55Z  billr
    Eliminate ulInterleaved in ExtndMediaInfo, no longer used or needed.
    Revision 1.29  2004/07/22 19:06:26Z  jaredw
    In HiddenRead function added a check after misaligned reads to see if there
    are more pages it read, if not break (IR 4461)
    Revision 1.28  2004/07/22 03:25:55Z  GaryP
    Changed to query the project option code to determine the BBM format state.
    Revision 1.27  2004/07/21 23:00:54Z  jaredw
    Added fix for ST 1Gbit flash bug (refer to comment)
    Revision 1.26  2004/02/25 17:12:40Z  garys
    Merge from FlashFXMT
    Revision 1.23.1.4  2004/02/25 17:12:40  garyp
    No longer include hwecc.h.
    Revision 1.23.1.3  2003/11/21 23:30:28Z  garyp
    Reverted to the previous BBM interface that knows nothing of FML handles.
    Revision 1.23.1.2  2003/11/03 19:38:48Z  garyp
    Re-checked into variant sandbox.
    Revision 1.24  2003/11/03 19:38:48Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.23  2003/10/16 20:41:09Z  garys
    Added typecast to first parameter in calls to WRITE_SEEK_REDUNDANT (CE
    compiler warnings)
    Revision 1.22  2003/10/14 22:04:23  garys
    Added support for the Toshiba TH58NVG1S3AFT
    Revision 1.21  2003/09/19 15:49:33  garys
    removed the uLength parameter from WriteHidden()
    Revision 1.20  2003/09/19 00:27:47  dennis
    Avoid some compiler warnings and fix a problem in WriteRedundant.
    Revision 1.19  2003/09/11 21:30:31  billr
    Minor cleanup after merge.
    Revision 1.18  2003/09/11 21:12:14Z  billr
    Merged from [Integration_2K_NAND] branch. Major rewrite to support
    NAND parts with 2KB page size.
    Revision 1.1  2003/08/15 17:45:38Z  billr
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <fxdriverfwapi.h>
#include <fxdevapi.h>
#include <fimdev.h>

#include <ecc.h>
#include <nandconf.h>
#include <nandctl.h>
#include <nandid.h>
#include "nand.h"

struct tagFIMDATA
{
    FFXDEVHANDLE            hDev;
    NTMHANDLE               hNTM;
    FIMINFO                 FimInfo;    /* information public to upper layers */
    NANDTECHNOLOGYMODULE   *pNTM;
    const NTMINFO          *pNtmInfo;   /* needed only for some debug assertions */
};

static FFXIOSTATUS  ReadData(         FIMHANDLE hFim, D_UINT32 ulStartPage, D_UINT32 ulCount, D_BUFFER *pPage, D_BUFFER *pTags, unsigned nTagSize, D_BOOL fUseEcc);
static FFXIOSTATUS  WriteData(        FIMHANDLE hFim, D_UINT32 ulStartPage, D_UINT32 ulCount, const void *pBuffer);
static FFXIOSTATUS  ReadRawPages(     FIMHANDLE hFim, D_UINT32 ulStartPage, D_UINT32 ulCount, D_BUFFER *pPages, D_BUFFER *pSpares);
static FFXIOSTATUS  WriteRawPages(    FIMHANDLE hFim, D_UINT32 ulStartPage, D_UINT32 ulCount, const D_BUFFER *pPages, const D_BUFFER *pSpares);
static FFXIOSTATUS  ReadNativePages(  FIMHANDLE hFim, D_UINT32 ulStartPage, D_UINT32 ulCount, D_BUFFER *pPages, D_BUFFER *pSpares);
static FFXIOSTATUS  PageWrite(        FIMHANDLE hFim, D_UINT32 ulStartPage, const D_BUFFER *pBuffer, const D_BUFFER *pTags, D_UINT32 ulCount, unsigned nTagSize);
static FFXIOSTATUS  SpareRead(        FIMHANDLE hFim, D_UINT32 ulStartPage, D_UINT32 ulCount, void *pBuffer);
static FFXIOSTATUS  SpareWrite(       FIMHANDLE hFim, D_UINT32 ulStartPage, D_UINT32 ulCount, const void *pBuffer);
static FFXIOSTATUS  TagRead(          FIMHANDLE hFim, D_UINT32 ulStartPage, D_UINT32 ulCount, void *pBuffer, unsigned nTagSize);
static FFXIOSTATUS  TagWrite(         FIMHANDLE hFim, D_UINT32 ulStartPage, D_UINT32 ulCount, const D_BUFFER *pTags, unsigned nTagSize);
static FFXIOSTATUS  SetBlockStatus(   FIMHANDLE hFim, D_UINT32 ulBlock, D_UINT32 ulBlockStatus);
static FFXIOSTATUS  EraseStart(       FIMHANDLE hFim, D_UINT32 ulStartBlock, D_UINT32 ulCount);
static FFXSTATUS    ErasePoll(        FIMHANDLE hFim, D_UINT32 *pulCount);

#define ERASE_BLOCK(x)         (hFim->pNTM->BlockErase( hFim->hNTM, (x)))

extern NANDTECHNOLOGYMODULE FFXNTM_cad;
extern NANDTECHNOLOGYMODULE FFXNTM_cadmlc;
extern NANDTECHNOLOGYMODULE FFXNTM_lsi;
#if FFXCONF_NANDSUPPORT_MICRON
extern NANDTECHNOLOGYMODULE FFXNTM_micron;
extern NANDTECHNOLOGYMODULE FFXNTM_stdclrnand;
extern NANDTECHNOLOGYMODULE FFXNTM_eclrnand;
#endif
extern NANDTECHNOLOGYMODULE FFXNTM_mx31;
extern NANDTECHNOLOGYMODULE FFXNTM_mx35;
extern NANDTECHNOLOGYMODULE FFXNTM_mx51;
extern NANDTECHNOLOGYMODULE FFXNTM_tegra2;
#if FFXCONF_NANDSUPPORT_ONENAND
extern NANDTECHNOLOGYMODULE FFXNTM_flexonenand;
extern NANDTECHNOLOGYMODULE FFXNTM_onenand;
#endif
extern NANDTECHNOLOGYMODULE FFXNTM_pageio;
extern NANDTECHNOLOGYMODULE FFXNTM_pxa320;
extern NANDTECHNOLOGYMODULE FFXNTM_ram;
extern NANDTECHNOLOGYMODULE FFXNTM_sim;
extern NANDTECHNOLOGYMODULE FFXNTM_ads5121e;

#if FFX_MAX_DEVICES > 4
    #error "FFX: FFX_MAX_DEVICES is larger than what nand.c supports"

    /*  If you get this particular build error and you need to support
        more than 4 flash devices, add the appropriate logic to the
        code below, and increase the value used in this error check
        accordingly.
    */
#endif

#if FFX_MAX_DEVICES > 0 && defined(FFX_DEV0_NTMS)
    static NANDTECHNOLOGYMODULE *pNTMList_0[] = FFX_DEV0_NTMS;
#endif
#if FFX_MAX_DEVICES > 1 && defined(FFX_DEV1_NTMS)
    static NANDTECHNOLOGYMODULE *pNTMList_1[] = FFX_DEV1_NTMS;
#endif
#if FFX_MAX_DEVICES > 2 && defined(FFX_DEV2_NTMS)
    static NANDTECHNOLOGYMODULE *pNTMList_2[] = FFX_DEV2_NTMS;
#endif
#if FFX_MAX_DEVICES > 3 && defined(FFX_DEV3_NTMS)
    static NANDTECHNOLOGYMODULE *pNTMList_3[] = FFX_DEV3_NTMS;
#endif

NANDTECHNOLOGYMODULE **ppNTMDevList[FFX_MAX_DEVICES+1] =
{
  #if FFX_MAX_DEVICES > 0
  #ifdef FFX_DEV0_NTMS
    &pNTMList_0[0],
  #else
    NULL,
  #endif
  #endif
  #if FFX_MAX_DEVICES > 1
  #ifdef FFX_DEV1_NTMS
    &pNTMList_1[0],
  #else
    NULL,
  #endif
  #endif
  #if FFX_MAX_DEVICES > 2
  #ifdef FFX_DEV2_NTMS
    &pNTMList_2[0],
  #else
    NULL,
  #endif
  #endif
  #if FFX_MAX_DEVICES > 3
  #ifdef FFX_DEV3_NTMS
    &pNTMList_3[0],
  #else
    NULL,
  #endif
  #endif
    NULL
};


                    /*-----------------------------*\
                     *                             *
                     *      Public Interface       *
                     *                             *
                    \*-----------------------------*/


/*-------------------------------------------------------------------
    Public: FfxFimCreate()

    Create an instance of the FIM.  It iterates though the list of
    NTMs specified for this Device until one loads succesfully.

    Parameters:
        hDev      - The FFXDEVHANDLE for the device.
        ppFimInfo - A pointer to a pointer to the FIMINFO structure.

    Return Value:
        Returns a FIMHANDLE value if successful, or NULL otherwise.
-------------------------------------------------------------------*/
static FIMHANDLE FfxFimCreate(
    FFXDEVHANDLE            hDev,
    const FIMINFO         **ppFimInfo)
{
    D_BOOL                  fSuccess = FALSE;
    FIMDATA                *pFim = NULL;
    unsigned                nDevNum;
    NANDTECHNOLOGYMODULE  **ppNTMList;
    unsigned                i;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
        "NAND:FfxFimCreate() hDev=%P\n", hDev));

    DclAssert(hDev);
    DclAssert(ppFimInfo);

    if(!FfxHookOptionGet(FFXOPT_DEVICE_NUMBER, hDev, &nDevNum, sizeof nDevNum))
        goto CreateCleanup;

    DclAssert(nDevNum < FFX_MAX_DEVICES);

    ppNTMList = ppNTMDevList[nDevNum];
    if(!ppNTMList)
    {
        FFXPRINTF(1, ("An NTM was not specified for DEV%u\n", nDevNum));
        DclError();
        goto CreateCleanup;
    }

    pFim = DclMemAllocZero(sizeof *pFim);
    if(!pFim)
        goto CreateCleanup;

    /*  Call the create routine for each NTM until we find one that works
    */
    for(i = 0; ppNTMList[i]; i++)
    {
        /*  See if we can successfully create the NTM instance
        */
        DclAssert(ppNTMList[i]->Create);
        pFim->hNTM = ppNTMList[i]->Create(hDev, &pFim->pNtmInfo);
        if(pFim->hNTM)
        {
            pFim->pNTM = ppNTMList[i];
            break;
        }
    }

    if(!pFim->hNTM)
        goto CreateCleanup;

    DclAssert(pFim->pNtmInfo);
    DclAssert(pFim->pNtmInfo->ulTotalBlocks);
    DclAssert(pFim->pNtmInfo->ulBlockSize);
    DclAssert(pFim->pNtmInfo->uPageSize);
    DclAssert(pFim->pNtmInfo->uSpareSize);
    DclAssert(pFim->pNtmInfo->uMetaSize >= FFX_NAND_TAGSIZE);
    DclAssert(pFim->pNtmInfo->nLockFlags <= D_UINT16_MAX);

    pFim->FimInfo.uDeviceType         = DEVTYPE_NAND;
    pFim->FimInfo.uDeviceFlags        = pFim->pNtmInfo->uDeviceFlags;
    pFim->FimInfo.uLockFlags          = (D_UINT16)pFim->pNtmInfo->nLockFlags;
  #if FFXCONF_OTPSUPPORT
    pFim->FimInfo.nOTPPages           = pFim->pNtmInfo->nOTPPages;
  #endif
    pFim->FimInfo.ulPhysicalBlocks    = pFim->pNtmInfo->ulTotalBlocks;
    pFim->FimInfo.ulTotalBlocks       = pFim->pNtmInfo->ulTotalBlocks;
    pFim->FimInfo.ulChipBlocks        = pFim->pNtmInfo->ulChipBlocks;
    pFim->FimInfo.ulBlockSize         = pFim->pNtmInfo->ulBlockSize;
    pFim->FimInfo.uPageSize           = pFim->pNtmInfo->uPageSize;
    pFim->FimInfo.uPagesPerBlock      = pFim->pNtmInfo->uPagesPerBlock;
    DclAssert(pFim->FimInfo.uPagesPerBlock == pFim->FimInfo.ulBlockSize / pFim->FimInfo.uPageSize);
    DclAssert((pFim->FimInfo.ulBlockSize % pFim->FimInfo.uPageSize) == 0);
    pFim->FimInfo.uSpareSize          = pFim->pNtmInfo->uSpareSize;
    pFim->FimInfo.uMetaSize           = pFim->pNtmInfo->uMetaSize;

    pFim->FimInfo.uEdcRequirement     = pFim->pNtmInfo->uEdcRequirement;
    pFim->FimInfo.ulEraseCycleRating  = pFim->pNtmInfo->ulEraseCycleRating;
    pFim->FimInfo.ulBBMReservedRating = pFim->pNtmInfo->ulBBMReservedRating;
    pFim->FimInfo.uEdcSegmentSize     = pFim->pNtmInfo->uEdcSegmentSize;
    pFim->FimInfo.uEdcCapability      = pFim->pNtmInfo->uEdcCapability;

    /*  Sanity checks on EDC configuration
    */        
    if(pFim->pNtmInfo->uDeviceFlags & DEV_REQUIRES_ECC)
    {
        /*  These should always be set if the Device requires EDC.
        */
        DclAssert(pFim->FimInfo.uEdcCapability);
        DclAssert(pFim->FimInfo.uEdcSegmentSize);
    }
    else
    {
        /*  These should never be set if the Device does not require ECC
        */
        DclAssert(!pFim->FimInfo.uEdcSegmentSize);
        DclAssert(!pFim->FimInfo.uEdcCapability);
    }

    /*  Sanity check and report the minimum alignment allowed for this NTM.
        Preserve the old alignment requirements for any NTM which does
        not initialize the alignment setting.
    */
    if(!pFim->pNtmInfo->uAlignSize || !DCLISPOWEROF2(pFim->pNtmInfo->uAlignSize))
    {
        FFXPRINTF(1, ("The NTM alignment size (%U) is invalid, using DCL_CPU_ALIGNSIZE (%U) instead.\n",
                      pFim->pNtmInfo->uAlignSize, DCL_CPU_ALIGNSIZE));

        pFim->FimInfo.uAlignSize    = DCL_CPU_ALIGNSIZE;
    }
    else
    {
        pFim->FimInfo.uAlignSize    = pFim->pNtmInfo->uAlignSize;
    }

    *ppFimInfo = &pFim->FimInfo;
    fSuccess = TRUE;

  CreateCleanup:

    if(!fSuccess && pFim)
    {
        if(pFim->hNTM)
            pFim->pNTM->Destroy(pFim->hNTM);

        DclMemFree(pFim);

        pFim = NULL;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEUNDENT),
        "NAND:FfxFimCreate() returning hFim=%P\n", pFim));

    return pFim;
}


/*-------------------------------------------------------------------
    Public: FfxFimDestroy()

    Destroy a FIM instance and release any allocated resources.

    Parameters:
        hFim - The FIM instance handle.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void FfxFimDestroy(
    FIMHANDLE   hFim)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
        "NAND:FfxFimDestroy() hInst=%P\n", hFim));

    hFim->pNTM->Destroy(hFim->hNTM);

    DclMemFree(hFim);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEUNDENT),
        "NAND:FfxFimDestroy() completed \n"));

    return;
}


/*-------------------------------------------------------------------
    Public: FfxFimParameterGet()

    Get a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hFim      - The FIM handle.
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the buffer in which to store the
                    value.  May be NULL which causes this function
                    to return the size of buffer required to store
                    the parameter.
        ulBuffLen - The size of buffer.  May be zero if pBuffer is
                    NULL.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
        If pBuffer is NULL, but the id is valid, the returned
        FFXSTATUS value will be decodeable using the macro
        DCLSTAT_GETUINT20(), which will return the buffer length
        required to store the parameter.
-------------------------------------------------------------------*/
static FFXSTATUS FfxFimParameterGet(
    FIMHANDLE       hFim,
    FFXPARAM        id,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
        "NAND:FfxFimParameterGet() hFim=%P ID=%x pBuff=%P Len=%lU\n",
        hFim, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, so pass the request to
                the layer below us.
            */
            ffxStat = hFim->pNTM->ParameterGet(hFim->hNTM, id, pBuffer, ulBuffLen);
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEUNDENT),
        "NAND:FfxFimParameterGet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxFimParameterSet()

    Set a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hFim      - The FIM handle.
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the parameter data.
        ulBuffLen - The parameter data length.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS FfxFimParameterSet(
    FIMHANDLE       hFim,
    FFXPARAM        id,
    const void     *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
        "NAND:FfxFimParameterSet() hFim=%P ID=%x pBuff=%P Len=%lU\n",
        hFim, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, so pass the request to
                the layer below us.
            */
            ffxStat = hFim->pNTM->ParameterSet(hFim->hNTM, id, pBuffer, ulBuffLen);
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEUNDENT),
        "NAND:FfxFimParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxFimIORequest()

    Dispatch an I/O operation request.

    Parameters:
        hFim      - The FIM handle.
        pIOR      - A pointer to the FFXIOREQUEST structure.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.
-------------------------------------------------------------------*/
static FFXIOSTATUS FfxFimIORequest(
    FIMHANDLE           hFim,
    FFXIOREQUEST       *pIOR)
{
    FFXIOSTATUS         ioStat = INITIAL_UNKNOWNFLAGS_STATUS(FFXSTAT_FIM_UNSUPPORTEDIOREQUEST);

    DclAssert(pIOR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEINDENT),
        "NAND:FfxFimIORequest() hFim=%P Func=%x\n", hFim, pIOR->ioFunc));

    DclProfilerEnter("NAND:FfxFimIORequest", 0, 0);

    DclAssert(hFim);

    /*  If the NTM has an IORequest function, give the NTM first crack
        at handling the request.  (NOTE: For the time being, <only>
        requests which do not require BBM can be handled.)
    */
    if(hFim->pNTM->IORequest)
    {
        ioStat = hFim->pNTM->IORequest(hFim->hNTM, pIOR);
    }

    if(ioStat.ffxStat == FFXSTAT_FIM_UNSUPPORTEDIOREQUEST)
    {
        switch(pIOR->ioFunc)
        {
            case FXIOFUNC_FIM_ERASE_START:
            {
                FFXIOR_FIM_ERASE_START *pReq = (FFXIOR_FIM_ERASE_START*)pIOR;

                DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

                ioStat = EraseStart(hFim, pReq->ulStartBlock, pReq->ulCount);
                if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                {
                    FFXPRINTF(1, ("Erase start returned status %lX\n", ioStat.ffxStat));

                    /*  Some NTMs return FFXSTAT_FIM_ERASEFAILED when the
                        chip reports an erase failure, others report
                        FFXSTAT_FIMIOERROR.  Make this a uniform
                        FFXSTAT_FIM_ERASEFAILED so the Error Manager will
                        decode the correct recommendation.
                    */
                    if (ioStat.ffxStat == FFXSTAT_FIMIOERROR)
                        ioStat.ffxStat = FFXSTAT_FIM_ERASEFAILED;

                    /*  NTMs that return anything else are broken.
                    */

                    /*  NOT true in all cases.  Tests may intentionally try
                        to write/erase a locked block, and expect a status
                        code which is meaningful.
                    */                        
                    DclAssert(ioStat.ffxStat == FFXSTAT_FIM_ERASEFAILED
                              || ioStat.ffxStat == FFXSTAT_FIMTIMEOUT); 
                }

                break;
            }

            case FXIOFUNC_FIM_ERASE_POLL:
            {
                FFXIOR_FIM_ERASE_POLL  *pReq = (FFXIOR_FIM_ERASE_POLL*)pIOR;

                (void)pReq;
                DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

                ioStat.ulFlags = IOFLAGS_BLOCK;
                ioStat.ffxStat = ErasePoll(hFim, &ioStat.ulCount);

                break;
            }

          #if FFXCONF_ERASESUSPENDSUPPORT
            case FXIOFUNC_FIM_ERASE_SUSPEND:
            {
                FFXIOR_FIM_ERASE_SUSPEND *pReq = (FFXIOR_FIM_ERASE_SUSPEND*)pIOR;

                (void)pReq;
                DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

                ioStat.ulFlags = IOFLAGS_BLOCK;

                break;
            }

            case FXIOFUNC_FIM_ERASE_RESUME:
            {
                FFXIOR_FIM_ERASE_RESUME *pReq = (FFXIOR_FIM_ERASE_RESUME*)pIOR;

                (void)pReq;
                DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

                ioStat.ulFlags = IOFLAGS_BLOCK;

                break;
            }
          #endif

            case FXIOFUNC_FIM_READ_GENERIC:
            {
                FFXIOR_FIM_READ_GENERIC *pReq = (FFXIOR_FIM_READ_GENERIC*)pIOR;

                DclAssert(pReq->ulCount);

                /*  All these operations are page ops.
                */
                ioStat.ulFlags = IOFLAGS_PAGE;

                switch(pReq->ioSubFunc)
                {
                    case FXIOSUBFUNC_FIM_READ_PAGES:
                    {
                        DclAssert(pReq->pPageData);
                        DclAssert(pReq->ulCount);

                        ioStat = ReadData(hFim, pReq->ulStartPage, pReq->ulCount,
                                          pReq->pPageData, NULL, 0, TRUE);

                        break;
                    }

                    case FXIOSUBFUNC_FIM_READ_TAGGEDPAGES:
                    {
                        DclAssert(pReq->pPageData);
                        DclAssert(pReq->ulCount);

                        ioStat = ReadData(hFim, pReq->ulStartPage, pReq->ulCount,
                                          pReq->pPageData, pReq->pAltData,
                                          pReq->nAltDataSize, TRUE);
                        break;
                    }

                    case FXIOSUBFUNC_FIM_READ_UNCORRECTEDPAGES:
                    {
                        DclAssert(pReq->pPageData);
                        DclAssert(pReq->ulCount);
                        DclAssert(pReq->pAltData == NULL);

                        /*  The previous assert is necessary because the upper
                            layers allow a spare buffer pointer to be supplied,
                            but the lower layers from here on down do not
                            support this at this time.
                        */

                        ioStat = ReadData(hFim, pReq->ulStartPage, pReq->ulCount,
                                          pReq->pPageData, NULL, 0, FALSE);

                        break;
                    }

                    case FXIOSUBFUNC_FIM_READ_RAWPAGES:
                    {
                        DclAssert(pReq->pPageData);
                        DclAssert(pReq->ulCount);

                        ioStat = ReadRawPages(hFim, pReq->ulStartPage, pReq->ulCount,
                                              pReq->pPageData, pReq->pAltData);

                        break;
                    }

                    case FXIOSUBFUNC_FIM_READ_SPARES:
                        ioStat = SpareRead(hFim, pReq->ulStartPage, pReq->ulCount, pReq->pAltData);
                        break;

                    case FXIOSUBFUNC_FIM_READ_TAGS:
                        ioStat = TagRead(hFim, pReq->ulStartPage, pReq->ulCount,
                                            pReq->pAltData, pReq->nAltDataSize);
                        break;

                    case FXIOSUBFUNC_FIM_READ_NATIVEPAGES:
                        ioStat = ReadNativePages(hFim, pReq->ulStartPage, pReq->ulCount,
                                              pReq->pPageData, pReq->pAltData);

                        /*  If the NTM doesn't implement a NativePageRead
                            method, use a raw read as a fallback.
                        */
                        if (ioStat.ffxStat == FFXSTAT_FIM_UNSUPPORTEDFUNCTION)
                        {
                            ioStat = ReadRawPages(hFim, pReq->ulStartPage, pReq->ulCount,
                                                  pReq->pPageData, pReq->pAltData);

                            /*  READ_NATIVEPAGES should attempt to do
                                error correction, but RawPageRead returns
                                just the bits on the media with no
                                correction.  Allow code to detect this by
                                returning a special success status.
                            */
                            if (ioStat.ffxStat == FFXSTAT_SUCCESS)
                                ioStat.ffxStat = FFXSTAT_FIMUNCORRECTED;
                        }
                        break;

                    default:
                    {
                        FFXPRINTF(1, ("NAND:FfxFimIORequest() Bad Read Generic SubFunction %x\n", pReq->ioSubFunc));
                        break;
                    }
                }

                break;
            }

            case FXIOFUNC_FIM_WRITE_GENERIC:
            {
                FFXIOR_FIM_WRITE_GENERIC *pReq = (FFXIOR_FIM_WRITE_GENERIC*)pIOR;

                switch(pReq->ioSubFunc)
                {
                    case FXIOSUBFUNC_FIM_WRITE_PAGES:
                    {
                        DclAssert(pReq->pPageData);

                        ioStat = PageWrite(hFim,
                                            pReq->ulStartPage,
                                            pReq->pPageData,
                                            NULL,
                                            pReq->ulCount,
                                            0);
                        break;
                    }

                    case FXIOSUBFUNC_FIM_WRITE_TAGGEDPAGES:
                    {
                        DclAssert(pReq->pPageData);
                        DclAssert(pReq->pAltData);
                        DclAssert(pReq->nAltDataSize);

                        ioStat = PageWrite(hFim,
                                            pReq->ulStartPage,
                                            pReq->pPageData,
                                            pReq->pAltData,
                                            pReq->ulCount,
                                            pReq->nAltDataSize);
                        break;
                    }

                    case FXIOSUBFUNC_FIM_WRITE_UNCORRECTEDPAGES:
                    {
                        DclAssert(pReq->pPageData);
                        DclAssert(pReq->pAltData == NULL);

                        /*  The previous assert is necessary because the upper
                            layers allow a spare buffer pointer to be supplied,
                            but the lower layers from here on down do not
                            support this at this time.
                        */
                        ioStat = WriteData(hFim, pReq->ulStartPage, pReq->ulCount, pReq->pPageData);

                        break;
                    }

                    case FXIOSUBFUNC_FIM_WRITE_RAWPAGES:
                    case FXIOSUBFUNC_FIM_WRITE_NATIVEPAGES:
                    {
                        DclAssert(pReq->pPageData);
                        DclAssert(pReq->ulCount);

                        ioStat = WriteRawPages(hFim, pReq->ulStartPage, pReq->ulCount,
                                               pReq->pPageData, pReq->pAltData);

                        break;
                    }

                    case FXIOSUBFUNC_FIM_WRITE_SPARES:
                        ioStat = SpareWrite(hFim, pReq->ulStartPage, pReq->ulCount, pReq->pAltData);
                        break;

                    case FXIOSUBFUNC_FIM_WRITE_TAGS:
                        ioStat = TagWrite(hFim, pReq->ulStartPage, pReq->ulCount,
                                             pReq->pAltData, pReq->nAltDataSize);
                        break;

                    default:
                        FFXPRINTF(1, ("NAND:FfxFimIORequest() Bad Write Generic SubFunction %x\n", pReq->ioSubFunc));
                        break;
                }

                /*  Most (all?) NTMs return FFXSTAT_FIMIOERROR to indicate
                    the chip reported a failed program operation.  Make
                    this a uniform FFXSTAT_FIM_WRITEFAILED so the Error
                    Manager will decode the correct recommendation.
                */
                if (ioStat.ffxStat == FFXSTAT_FIMIOERROR)
                    ioStat.ffxStat = FFXSTAT_FIM_WRITEFAILED;

                break;
            }

            case FXIOFUNC_FIM_GET_PAGE_STATUS:
            {
                FFXIOR_FIM_GET_PAGE_STATUS *pReq = (FFXIOR_FIM_GET_PAGE_STATUS*)pIOR;

                DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

                ioStat = hFim->pNTM->GetPageStatus(hFim->hNTM, pReq->ulPage);

                break;
            }

            case FXIOFUNC_FIM_GET_BLOCK_STATUS:
            {
                FFXIOR_FIM_GET_BLOCK_STATUS *pReq = (FFXIOR_FIM_GET_BLOCK_STATUS*)pIOR;

                DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

                ioStat = hFim->pNTM->GetBlockStatus(hFim->hNTM, pReq->ulBlock);
                break;
            }

            case FXIOFUNC_FIM_SET_BLOCK_STATUS:
            {
                FFXIOR_FIM_SET_BLOCK_STATUS *pReq = (FFXIOR_FIM_SET_BLOCK_STATUS*)pIOR;

                DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

                ioStat = SetBlockStatus(hFim, pReq->ulBlock, pReq->ulBlockStatus);

                break;
            }

            default:
            {
                    FFXPRINTF(1, ("NAND:FfxFimIORequest() Unsupported Function %x\n", pIOR->ioFunc));
                break;
            }
        }
	}

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEUNDENT),
        "NAND:FfxFimIORequest() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}



                    /*-----------------------------*\
                     *                             *
                     *   Local Helper Functions    *
                     *                             *
                    \*-----------------------------*/



/*-------------------------------------------------------------------
    Local: PageWrite()

    Write pages and associated tags to the flash device.  The pages
    and tags are written with ECC.

    Parameters:
        hFim        - The FIM instance handle.
        ulStartPage - First page to write.
        pBuffer     - A pointer to the data to write.
        pTags       - A pointer to the tag data.
        ulCount     - The number of pages to write.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.
-------------------------------------------------------------------*/
static FFXIOSTATUS PageWrite(
    FIMHANDLE       hFim,
    D_UINT32        ulStartPage,
    const D_BUFFER *pBuffer,
    const D_BUFFER *pTags,
    D_UINT32        ulCount,
    unsigned        nTagSize)
{
    FFXIOSTATUS     ioTotal = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32        ulPageCount; /* never crosses an erase block boundary */

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
        "NANDPageWrite() Start Page=%lX Count=%lX pData=%P pTags=%P TagSize=%u\n",
        ulStartPage, ulCount, pBuffer, pTags, nTagSize));

    DclProfilerEnter("NANDPageWrite", 0, ulCount);

    /*  FimInfo.ulTotalBlocks should never change!
    */
    DclAssert(hFim->FimInfo.ulTotalBlocks == hFim->pNtmInfo->ulTotalBlocks);

    DclAssert(hFim);
    DclAssert(pBuffer);
    DclAssert(nTagSize <= hFim->FimInfo.uMetaSize);
    DclAssert(ulStartPage < (hFim->FimInfo.ulTotalBlocks * hFim->FimInfo.uPagesPerBlock));
    DclAssert(ulCount < D_UINT16_MAX); /* BOGUS! Why? */

    /*  The tag pointer and tag size are either always both set or both
        clear.  Assert it so.
    */
    DclAssert((pTags && nTagSize) || (!pTags && !nTagSize));

    /*  Don't cross an erase block boundary.
    */
    ulPageCount = hFim->FimInfo.uPagesPerBlock - (ulStartPage % hFim->FimInfo.uPagesPerBlock);
    while(ulCount)
    {
        FFXIOSTATUS ioStat;

        ulPageCount = DCLMIN(ulPageCount, ulCount);
        DclAssert((ulStartPage % hFim->FimInfo.uPagesPerBlock) + ulPageCount
                  <= hFim->FimInfo.uPagesPerBlock);

        ioStat = hFim->pNTM->PageWrite(hFim->hNTM, ulStartPage, pBuffer, pTags,
                                       ulPageCount, nTagSize, TRUE);

        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;
        ioTotal.ffxStat          = ioStat.ffxStat;

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ulStartPage     += ioStat.ulCount;
        pBuffer         += ioStat.ulCount * hFim->FimInfo.uPageSize;
        if(nTagSize)
            pTags       += ioStat.ulCount * nTagSize;
        ulCount         -= ioStat.ulCount;
        ulPageCount     -= ioStat.ulCount;

        /*  At end of one erase block, reset count for next.
        */
        if (ulPageCount == 0)
            ulPageCount = hFim->FimInfo.uPagesPerBlock;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEUNDENT),
        "NANDPageWrite() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: TagRead()

    Read just the tags associated with pages, starting at a given page
    number.

    Parameters:
        hFim        - The FIM instance handle.
        ulStartPage - First page to read.
        ulCount     - The number of tags to read.
        pBuffer     - A pointer to the buffer to fill
        nTagSize    - The number of tag bytes to read per page.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of the
        operation.  The count is the number of pages' tags (not tag
        bytes) read.
-------------------------------------------------------------------*/
static FFXIOSTATUS TagRead(
    FIMHANDLE       hFim,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    void           *pBuffer,
    unsigned        nTagSize)
{
    D_UINT32        ulPageNum = ulStartPage;
    D_UCHAR        *pcBuffer = pBuffer;
    FFXIOSTATUS     ioTotal = DEFAULT_PAGEIO_STATUS;
    D_UINT32        ulPageCount; /* never crosses an erase block boundary */

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
        "NANDHiddenRead() Start Page=%lX Count=%lX TagSize=%u\n",
        ulStartPage, ulCount, nTagSize));

    DclProfilerEnter("NANDTagRead", 0, ulCount);

    DclAssert(pBuffer);
    DclAssert(ulCount);

    /*  Don't cross an erase block boundary.
    */
    ulPageCount = hFim->FimInfo.uPagesPerBlock - (ulPageNum % hFim->FimInfo.uPagesPerBlock);
    while(ulCount)
    {
        FFXIOSTATUS ioStat;

        ulPageCount = DCLMIN(ulPageCount, ulCount);
        DclAssert((ulPageNum % hFim->FimInfo.uPagesPerBlock) + ulPageCount
                  <= hFim->FimInfo.uPagesPerBlock);

        ioStat = hFim->pNTM->HiddenRead(hFim->hNTM, ulPageNum, ulPageCount, pcBuffer, nTagSize);

        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;
        ioTotal.ffxStat          = ioStat.ffxStat;

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        DclAssert(ioStat.ulCount);

        ulPageNum       += ioStat.ulCount;
        pcBuffer        += ioStat.ulCount * nTagSize;
        ulCount         -= ioStat.ulCount;
        ulPageCount     -= ioStat.ulCount;

        /*  At end of one erase block, reset count for next.
        */
        if (ulPageCount == 0)
            ulPageCount = hFim->FimInfo.uPagesPerBlock;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEUNDENT),
        "NANDTagRead() returning status %lX\n", ioTotal.ffxStat));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: TagWrite()

    This function writes tags to the spare area of a page.

    Parameters:
        hFim        - The FIM instance handle.
        ulStartPage - First page to write.
        ulCount     - Number of tags to write.
        pTags       - A pointer to the buffer containing the tag data.
        nTagSize    - The number of tag bytes to write per page.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of the
        operation.  The count is the number of pages' tags (not tag
        bytes) written.
-------------------------------------------------------------------*/
static FFXIOSTATUS TagWrite(
    FIMHANDLE       hFim,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    const D_BUFFER *pTags,
    unsigned        nTagSize)
{
    FFXIOSTATUS     ioTotal = DEFAULT_PAGEIO_STATUS;
    D_UINT32        ulPageNum = ulStartPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
        "NANDTagWrite() Start Page=%lX Count=%lX TagSize=%u\n",
        ulStartPage, ulCount, nTagSize));

    DclProfilerEnter("NANDTagWrite", 0, ulCount);

    DclAssert(pTags);
    DclAssert(ulCount);

    /*  Start length loop
    */
    while(ulCount)
    {
        FFXIOSTATUS ioStat;

        ioStat = hFim->pNTM->HiddenWrite(hFim->hNTM, ulPageNum, pTags, nTagSize);

        ioTotal.ulCount += ioStat.ulCount;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;
        ioTotal.ffxStat  = ioStat.ffxStat;

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        DclAssert(ioStat.ulCount == 1);
        ulPageNum++;
        ulCount--;
        pTags += nTagSize;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEUNDENT),
        "NANDTagWrite() returning status %lX\n", ioTotal.ffxStat));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: SpareRead()

    Reads only the spare area of the given page.

    Parameters:
        hFim        - The FIM instance handle.
        ulStartPage - First page to read.
        ulCount     - The number of pages' spare areas to read.
        pBuffer     - Buffer to transfer the data into.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of the
        operation.  The count is the number of pages' spare areas (not
        spare area bytes bytes) read.
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareRead(
    FIMHANDLE       hFim,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    void           *pBuffer)
{
    FFXIOSTATUS     ioTotal = DEFAULT_PAGEIO_STATUS;
    D_UCHAR        *pcBuffer = pBuffer;
    D_UINT32        ulPageNum = ulStartPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
        "NANDSpareRead() Start Page=%lX Length=%lX\n", ulStartPage, ulCount));

    DclProfilerEnter("NANDSpareRead", 0, ulCount);

    DclAssert(hFim);
    DclAssert(pBuffer);
    DclAssert(ulCount);

    while(ulCount)
    {
        FFXIOSTATUS     ioStat;

        ioStat = hFim->pNTM->SpareRead(hFim->hNTM, ulPageNum, pcBuffer);

        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;
        ioTotal.ffxStat          = ioStat.ffxStat;

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Go to next address
        */
        DclAssert(ioStat.ulCount == 1);
        pcBuffer += hFim->FimInfo.uSpareSize;
        ulCount--;
        ulStartPage++;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEUNDENT),
        "NANDSpareRead() returning status %lX\n", ioTotal.ffxStat));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: SpareWrite()

    Writes only the spare area of the given page.

    Parameters:
        hFim        - The FIM instance handle.
        ulStartPage - First page to write.
        ulCount     - Number of pages' spare areas to write.
        pBuffer     - Buffer to transfer the data from.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of the
        operation.  The count is the number of pages' spare areas (not
        spare area bytes bytes) written.
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareWrite(
    FIMHANDLE       hFim,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    const void     *pBuffer)
{
    FFXIOSTATUS     ioTotal = DEFAULT_PAGEIO_STATUS;
    const D_UCHAR  *pcBuffer = pBuffer;
    D_UINT32        ulPageNum = ulStartPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
        "NANDSpareWrite() Start Page=%lX Count=%lX\n", ulStartPage, ulCount));

    DclProfilerEnter("NANDSpareWrite", 0, ulCount);

    DclAssert(hFim);
    DclAssert(pBuffer);
    DclAssert(ulCount);

    /*  Start length loop
    */
    while(ulCount)
    {
        FFXIOSTATUS ioStat;

        ioStat = hFim->pNTM->SpareWrite(hFim->hNTM, ulPageNum, pcBuffer);

        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;
        ioTotal.ffxStat          = ioStat.ffxStat;

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        DclAssert(ioStat.ulCount == 1);
        ulPageNum++;
        ulCount--;
        pcBuffer        += hFim->FimInfo.uSpareSize;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEUNDENT),
        "NANDSpareWrite() returning status %lX\n", ioTotal.ffxStat));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: EraseStart()

    Attempts to initiate an erase operation.  If it is started
    successfully, the only FIM functions that can then be called
    are EraseSuspend() and ErasePoll().  The operation must
    subsequently be monitored by calls to ErasePoll().

    If it is not started successfully, those functions may not
    be called.  The flash is restored to a readable state if
    possible, but this cannot always be guaranteed.

    Parameters:
        hFim         - The FIM instance handle.
        ulStartBlock - Starting block to begin the erase.
        ulCount      - Number of blocks to erase.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.
-------------------------------------------------------------------*/
static FFXIOSTATUS EraseStart(
    FIMHANDLE       hFim,
    D_UINT32        ulStartBlock,
    D_UINT32        ulCount)
{
    FFXIOSTATUS     ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
        "NANDEraseStart() Start Block=%lX Count=%lX\n", ulStartBlock, ulCount));

    DclProfilerEnter("NANDEraseStart", 0, ulCount);

    /*  TBD: Currently, this only erases one zone.  Modify to actually use
        the parameter passed in, let the caller account for the latency
        configurations.
    */
    DclAssert(ulCount);
    (void) ulCount;

    ioStat = hFim->pNTM->BlockErase(hFim->hNTM, ulStartBlock);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEUNDENT),
        "NANDEraseStart() returning status %lU\n", ioStat.ffxStat));

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: ErasePoll()

    Monitor the status of an erase begun with EraseStart().

    This function exists in the NAND FIM because the Device
    Manager has expectations on how erase procedures are called
    to erase a block, whether the device be NOR or NAND. In this
    FIM, the procedure EraseStart does all the work, and this
    function is something of a dummy function. Since its framework
    was borrowed from the NOR FIMs, and since it is possible that
    sometime in the future this function may perform more like the
    NOR FIM function, here is the text on what it is supposed to
    do in the NOR context:

        "If the erase fails, attempts to return the flash to its
        normal read mode.  Depending on the type of flash, this
        may or may not be possible.  If it is possible, it may be
        achieved by suspending the erase operation rather than by
        terminating it.  In this case, it may be possible to read
        the flash, but not to erase it further.

        This function may be called with the flash either in read
        array mode or in read status mode."

    Here is the NOR FIM's commentary on the return value:

        "If the erase is still in progress, returns
        FFXSTAT_FIM_ERASEINPROGRESS.  The only FIM functions that
        can then be called are EraseSuspend() and ErasePoll().

        If the erase completed successfully, returns FFXSTAT_SUCCESS,
        and *pulCount contains the number of erase zones actually
        erased.  This may be less than the ulCount value supplied
        to EraseStart().  The flash is in normal read mode.

        If the erase failed, returns FFXSTAT_FIM_ERASEFAILED. The flash
        is returned to normal read mode if possible, but this may not
        be possible in all cases (for example, if the flash does not
        support suspending an erase, and the operation times out)."

    Currently this FIM works on only one erase zone at a time, and
    so simply sets the return count to 1.

    Parameters:
        hFim     - The FIM instance handle.
        pulCount - pointer to storage into which to return the count

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS ErasePoll(
    FIMHANDLE       hFim,
    D_UINT32       *pulCount)
{
    DclAssert(hFim);
    DclAssert(pulCount);

    /*  Since EraseStart() will always erase exactly one erase zone at
        a time, and it will always erase the whole zone before returning,
        simply return one erase zone here.

        TBD: Modify to erase a range of blocks, rather than one
        (see EraseStart, above)
    */
    *pulCount = 1;
    return FFXSTAT_SUCCESS;
}

/*-------------------------------------------------------------------
    Local: SetBlockStatus()

    This function sets the block status information for the
    given erase block.

    Note that this function addresses absolute, physical blocks,
    not as might be remapped by BBM.

    Parameters:
        hFim          - The FIM instance handle.
        ulBlock       - The block for which to set the status.
        ulBlockStatus - The block status information to set.

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.
-------------------------------------------------------------------*/
static FFXIOSTATUS SetBlockStatus(
    FIMHANDLE       hFim,
    D_UINT32        ulBlock,
    D_UINT32        ulBlockStatus)
{
    DclAssert(hFim);
    return hFim->pNTM->SetBlockStatus(hFim->hNTM, ulBlock, ulBlockStatus);
}


/*-------------------------------------------------------------------
    Local: ReadData()

    Read a block of memory from the NAND flash device.

    Parameters:
        hFim        - The FIM instance handle.
        ulStartPage - The starting page number
        ulCount     - The number of pages to read.
        pPage       - A pointer to the buffer to fill with page data
        pTags       - A pointer to the buffer to fill with tag data.
                      This value may be NULL if the caller does not
                      want the tag to be returned.  This value MUST
                      be NULL if fUseEcc is FALSE.
        nTagSize    - The tag size.  Must be 0 of pTags is NULL.
        fUseEcc     - A flag indicating whether ECC should be used.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadData(
    FIMHANDLE       hFim,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    D_BUFFER       *pPage,
    D_BUFFER       *pTags,
    unsigned        nTagSize,
    D_BOOL          fUseEcc)
{
    D_UINT32        ulPageNum = ulStartPage;
    D_UINT32        ulPageCount;
    FFXIOSTATUS     ioTotal = DEFAULT_GOOD_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
        "NANDReadData() StartPage=%lX Count=%lX pPage=%P pTags=%P TagSize=%u UseECC=%U\n",
        ulStartPage, ulCount, pPage, pTags, nTagSize, fUseEcc));

    DclProfilerEnter("NANDReadData", 0, ulCount);

    /*  FimInfo.ulTotalBlocks should never change!
    */
    DclAssert(hFim->FimInfo.ulTotalBlocks == hFim->pNtmInfo->ulTotalBlocks);

    DclAssert(hFim);
    DclAssert(pPage);
    DclAssert(ulCount);
    DclAssert(ulStartPage < (hFim->FimInfo.ulTotalBlocks * hFim->FimInfo.uPagesPerBlock));

    /*  The tag pointer and tag size are either always both set or both clear
    */
    DclAssert((pTags && nTagSize) || (!pTags && !nTagSize));

    /*  If fUseEcc is FALSE, then the tag size (and the tag pointer, asserted
        above) must be clear.
    */
    DclAssert(fUseEcc || !nTagSize);

    /*  Don't cross an erase block boundary.
    */
    ulPageCount = hFim->FimInfo.uPagesPerBlock - (ulPageNum % hFim->FimInfo.uPagesPerBlock);
    while(ulCount)
    {
        FFXIOSTATUS ioStat;

        ulPageCount = DCLMIN(ulPageCount, ulCount);
        DclAssert((ulPageNum % hFim->FimInfo.uPagesPerBlock) + ulPageCount
                  <= hFim->FimInfo.uPagesPerBlock);

        ioStat = hFim->pNTM->PageRead(hFim->hNTM, ulPageNum, pPage, pTags,
                                      ulPageCount, nTagSize, fUseEcc);

        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;
        ioTotal.ffxStat          = ioStat.ffxStat;

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Update counts and pointer to reflect the pages that were
            succesfully read.
        */
        ulPageNum   += ioStat.ulCount;
        pPage       += ioStat.ulCount * hFim->FimInfo.uPageSize;

        if(pTags)
            pTags   += ioStat.ulCount * nTagSize;

        ulCount         -= ioStat.ulCount;
        ulPageCount     -= ioStat.ulCount;

        /*  At end of one erase block, reset count for next.
        */
        if (ulPageCount == 0)
            ulPageCount = hFim->FimInfo.uPagesPerBlock;
    }

    DclProfilerLeave(0UL);

    if(ioTotal.ffxStat != FFXSTAT_SUCCESS)
    {
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEUNDENT),
            "NANDReadData() failed! StartPage=%lU Returning %s\n",
            ulStartPage, FfxDecodeIOStatus(&ioTotal)));
    }
    else
    {
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEUNDENT),
            "NANDReadData() returning %s\n", FfxDecodeIOStatus(&ioTotal)));
    }

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: WriteData()

    Write to the flash device.

    NOTE: This is NOT the typical interface used to write NAND pages,
          as it does not use ECC (for production code, it is used
          only by BBM and FMSLTEST).

          As it has such limited use, it is NOT optimally imlemented,
          in that it breaks all long requests into single page
          writes.  Beware of this when doing performance tests.
          For example, the FMSL performance test without ECC <may>
          perform slower than the test with ECC for this reason.

    Parameters:
        hFim        - The FIM instance handle.
        ulStartPage - Page on which to start the write.
        ulCount     - Number of pages to write.
        pBuffer     - void pointer to the data to write.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.
-------------------------------------------------------------------*/
static FFXIOSTATUS WriteData(
    FIMHANDLE           hFim,
    D_UINT32            ulStartPage,
    D_UINT32            ulCount,
    const void         *pBuffer)
{
    FFXIOSTATUS         ioTotal = DEFAULT_PAGEIO_STATUS;
    D_UINT32            ulPage = ulStartPage;
    const D_UCHAR      *pcBuffer = pBuffer;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEINDENT),
        "NANDWriteData() Start Page=%lX Count=%lX\n", ulStartPage, ulCount));

    DclProfilerEnter("NANDWriteData", 0, ulCount);

    /*  FimInfo.ulTotalBlocks should never change!
    */
    DclAssert(hFim->FimInfo.ulTotalBlocks == hFim->pNtmInfo->ulTotalBlocks);

    DclAssert(hFim);
    DclAssert(pBuffer);
    DclAssert(ulCount);
    DclAssert(ulStartPage < hFim->FimInfo.ulTotalBlocks * hFim->FimInfo.uPagesPerBlock);

    /*  While data left...
    */
    while(ulCount)
    {
        FFXIOSTATUS ioStat;

        ioStat = hFim->pNTM->PageWrite(hFim->hNTM, ulPage, pcBuffer, NULL, 1, 0, FALSE);

        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;
        ioTotal.ffxStat          = ioStat.ffxStat;

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        DclAssert(ioStat.ulCount == 1);
        ulCount--;
        ulPage++;
        pcBuffer += hFim->FimInfo.uPageSize;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEUNDENT),
        "NANDWriteData() returning status %lU\n", ioTotal.ffxStat));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: ReadRawPages()

    This function reads raw pages and their associated spare
    areas from flash.  No ECC or BBM applies to this function.

    Parameters:
        hFim        - The FIM instance handle.
        ulStartPage - The starting page number
        ulCount     - The number of pages to read.
        pPages      - A pointer to the buffer to fill with page data
        pSpare      - A pointer to the buffer to fill with spare data.
                      This value may be NULL if the caller does not
                      want the spare data to be returned.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadRawPages(
    FIMHANDLE       hFim,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    D_BUFFER       *pPages,
    D_BUFFER       *pSpares)
{
    D_UINT32        ulPageCount; /* never crosses an erase block boundary */
    FFXIOSTATUS     ioTotal = DEFAULT_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
        "FfxNand:ReadRawPages() StartPage=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulStartPage, ulCount, pPages, pSpares));

    DclProfilerEnter("FfxNand:ReadRawPages", 0, ulCount);

    /*  FimInfo.ulTotalBlocks should never change!
    */
    DclAssert(hFim->FimInfo.ulTotalBlocks == hFim->pNtmInfo->ulTotalBlocks);

    DclAssert(hFim);
    DclAssert(pPages);
    DclAssert(ulCount);
    DclAssert(ulStartPage < (hFim->FimInfo.ulTotalBlocks * hFim->FimInfo.uPagesPerBlock));

    if(!hFim->pNTM->RawPageRead)
    {
        ioTotal.ffxStat = FFXSTAT_FIM_UNSUPPORTEDFUNCTION;
        goto ReadRawPagesCleanup;
    }

    /*  Don't cross an erase block boundary.
    */
    ulPageCount = hFim->FimInfo.uPagesPerBlock - (ulStartPage % hFim->FimInfo.uPagesPerBlock);
    while(ulCount)
    {
        FFXIOSTATUS ioStat;

        ulPageCount = DCLMIN(ulPageCount, ulCount);
        DclAssert((ulStartPage % hFim->FimInfo.uPagesPerBlock) + ulPageCount
                  <= hFim->FimInfo.uPagesPerBlock);

        ioStat = hFim->pNTM->RawPageRead(hFim->hNTM, ulStartPage, pPages, pSpares, ulPageCount);

        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;
        ioTotal.ffxStat          = ioStat.ffxStat;

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Update counts and pointer to reflect the pages that were
            succesfully read.
        */
        ulStartPage  += ioStat.ulCount;
        pPages       += ioStat.ulCount * hFim->FimInfo.uPageSize;

        if(pSpares)
            pSpares  += ioStat.ulCount * hFim->FimInfo.uSpareSize;

        ulCount         -= ioStat.ulCount;
        ulPageCount     -= ioStat.ulCount;

        /*  At end of one erase block, reset count for next.
        */
        if (ulPageCount == 0)
            ulPageCount = hFim->FimInfo.uPagesPerBlock;
    }

  ReadRawPagesCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEUNDENT),
        "FfxNand:ReadRawPages() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: WriteRawPages()

    This function writes raw pages and their associated spare
    areas to flash.  No ECC or BBM applies to this function.

    Parameters:
        hFim        - The FIM instance handle.
        ulStartPage - The starting page number
        ulCount     - The number of pages to write.
        pPages      - A pointer to the page data to write
        pSpare      - A pointer to the spare data to write.  If this
                      pointer is NULL, the behavior is defined by the
                      specific NTM being used.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.
-------------------------------------------------------------------*/
static FFXIOSTATUS WriteRawPages(
    FIMHANDLE       hFim,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    const D_BUFFER *pPages,
    const D_BUFFER *pSpares)
{
    D_UINT32        ulPageCount; /* never crosses an erase block boundary */
    FFXIOSTATUS     ioTotal = DEFAULT_GOOD_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
        "FfxNand:WriteRawPages() StartPage=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulStartPage, ulCount, pPages, pSpares));

    DclProfilerEnter("FfxNand:WriteRawPages", 0, ulCount);

    /*  FimInfo.ulTotalBlocks should never change!
    */
    DclAssert(hFim->FimInfo.ulTotalBlocks == hFim->pNtmInfo->ulTotalBlocks);

    DclAssert(hFim);
    DclAssert(pPages);
    DclAssert(ulCount);
    DclAssert(ulStartPage < (hFim->FimInfo.ulTotalBlocks * hFim->FimInfo.uPagesPerBlock));

    if(!hFim->pNTM->RawPageWrite)
    {
        ioTotal.ffxStat = FFXSTAT_FIM_UNSUPPORTEDFUNCTION;
        goto WriteRawPagesCleanup;
    }

    /*  Don't cross an erase block boundary.
    */
    ulPageCount = hFim->FimInfo.uPagesPerBlock - (ulStartPage % hFim->FimInfo.uPagesPerBlock);
    while(ulCount)
    {
        FFXIOSTATUS ioStat;

        ulPageCount = DCLMIN(ulPageCount, ulCount);
        DclAssert((ulStartPage % hFim->FimInfo.uPagesPerBlock) + ulPageCount
                  <= hFim->FimInfo.uPagesPerBlock);

        ioStat = hFim->pNTM->RawPageWrite(hFim->hNTM, ulStartPage, pPages, pSpares, ulPageCount);

        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;
        ioTotal.ffxStat          = ioStat.ffxStat;

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Update counts and pointer to reflect the pages that were
            succesfully read.
        */
        ulStartPage  += ioStat.ulCount;
        pPages       += ioStat.ulCount * hFim->FimInfo.uPageSize;

        if(pSpares)
            pSpares  += ioStat.ulCount * hFim->FimInfo.uSpareSize;

        ulCount         -= ioStat.ulCount;
        ulPageCount     -= ioStat.ulCount;

        /*  At end of one erase block, reset count for next.
        */
        if (ulPageCount == 0)
            ulPageCount = hFim->FimInfo.uPagesPerBlock;
    }

  WriteRawPagesCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEUNDENT),
        "FfxNand:WriteRawPages() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: ReadNativePages()

    This function reads pages and their associated spare areas from
    flash.  Any errors that can be corrected are; uncorrectable data
    is returned "as is".

    Unlike other read functions, this is guaranteed to return what
    was read from the media when an uncorrectable error occurs.

    Parameters:
        hFim        - The FIM instance handle.
        ulStartPage - The starting page number
        ulCount     - The number of pages to read.
        pPages      - A pointer to the buffer to fill with page data
        pSpare      - A pointer to the buffer to fill with spare data.
                      This value may be NULL if the caller does not
                      want the spare data to be returned.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadNativePages(
    FIMHANDLE       hFim,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    D_BUFFER       *pPages,
    D_BUFFER       *pSpares)
{
    D_UINT32        ulPageCount; /* never crosses an erase block boundary */
    FFXIOSTATUS     ioTotal = DEFAULT_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 1, TRACEINDENT),
                    "FfxNand:ReadNativePages() StartPage=%lX Count=%lU pPages=%P pSpares=%P\n",
                    ulStartPage, ulCount, pPages, pSpares));

    DclProfilerEnter("FfxNand:ReadNativePages", 0, ulCount);

    /*  FimInfo.ulTotalBlocks should never change!
    */
    DclAssert(hFim->FimInfo.ulTotalBlocks == hFim->pNtmInfo->ulTotalBlocks);

    DclAssert(hFim);
    DclAssert(pPages);
    DclAssert(ulCount);
    DclAssert(ulStartPage < (hFim->FimInfo.ulTotalBlocks * hFim->FimInfo.uPagesPerBlock));

    if(!hFim->pNTM->NativePageRead)
    {
        ioTotal.ffxStat = FFXSTAT_FIM_UNSUPPORTEDFUNCTION;
        goto cleanup;
    }

    /*  Don't cross an erase block boundary.
    */
    ulPageCount = hFim->FimInfo.uPagesPerBlock - (ulStartPage % hFim->FimInfo.uPagesPerBlock);
    while(ulCount)
    {
        FFXIOSTATUS ioStat;

        ulPageCount = DCLMIN(ulPageCount, ulCount);
        DclAssert((ulStartPage % hFim->FimInfo.uPagesPerBlock) + ulPageCount
                  <= hFim->FimInfo.uPagesPerBlock);

        ioStat = hFim->pNTM->NativePageRead(hFim->hNTM, ulStartPage, pPages, pSpares, ulPageCount);

        ioTotal.ulCount         += ioStat.ulCount;
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;
        ioTotal.ffxStat          = ioStat.ffxStat;

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Update counts and pointer to reflect the pages that were
            succesfully read.
        */
        ulStartPage  += ioStat.ulCount;
        pPages       += ioStat.ulCount * hFim->FimInfo.uPageSize;

        if(pSpares)
            pSpares  += ioStat.ulCount * hFim->FimInfo.uSpareSize;

        ulCount         -= ioStat.ulCount;
        ulPageCount     -= ioStat.ulCount;

        /*  At end of one erase block, reset count for next.
        */
        if (ulPageCount == 0)
            ulPageCount = hFim->FimInfo.uPagesPerBlock;
    }

  cleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NAND, 2, TRACEUNDENT),
        "FfxNand:ReadNativePages() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}




/*-------------------------------------------------------------------
    FIMDEVICE Declaration

    This structure declaration is used to define the entry points
    into the FIM.  This is declared at the end of the module to
    eliminate the need for what would be duplicated function
    prototypes in all the FIMs.
-------------------------------------------------------------------*/
FIMDEVICE FFXFIM_nand =
{
    /*  We're stuck with all these unused pointers until old style
        NOR FIMs go away, or stop using a FIMDEVICE structure.
    */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    FfxFimCreate,
    FfxFimDestroy,
    FfxFimIORequest,
    FfxFimParameterGet,
    FfxFimParameterSet
};


#endif  /* FFXCONF_NANDSUPPORT */


