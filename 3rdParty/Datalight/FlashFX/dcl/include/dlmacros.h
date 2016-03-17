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

    This header file contains generalized DCL level macros that are used
    on a multi-product-wide basis.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlmacros.h $
    Revision 1.41.1.2  2012/04/11 23:39:17Z  garyp
    Partial merge from the trunk.  Fixed DCLISALIGNED() so that arithmetic in
    the alignment argument works correctly. E.g., no longer does DCLISALSIGNED(p, 7+1)
    always yield TRUE.  Added the various "ISHEXDIGIT" macros.
    Revision 1.41  2011/03/09 01:42:26Z  daniel.lewis
    Added the DCLXOR() macro.
    Revision 1.40  2010/04/28 23:29:23Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.39  2010/04/18 20:41:31Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.38  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.37  2009/12/13 06:40:59Z  garyp
    Let's try that one more time...
    Revision 1.36  2009/12/13 05:51:34Z  garyp
    Fixed the non-debug versions of DCLBITSET() and DCLBITCLEAR().
    Revision 1.35  2009/12/12 19:09:03Z  garyp
    Added the DCLBITMAP(), DCLBITMAPDIMENSION(), and DCLALIGNEDBITMAP()
    macros.  Documentation updated.
    Revision 1.34  2009/11/10 19:39:55Z  garyp
    Conditionally include the Profiler, MemTracking, and HighRes timer services.
    Revision 1.33  2009/10/14 22:45:23Z  keithg
    Removed obsolete random seed value, added random service init call to the
    services list, and added DCLMEMGET and PUT macros here.
    Revision 1.32  2009/10/02 18:03:36Z  garyp
    Corrected some broken documentation -- no functional changes.
    Revision 1.31  2009/09/08 21:16:53Z  garyp
    Added DCLIOSTATUS.
    Revision 1.30  2009/06/28 01:21:10Z  garyp
    Added the DCL_DEFAULT_SERVICES setting.
    Revision 1.29  2009/05/29 22:14:53Z  johnbr
    Minor comment revision.
    Revision 1.28  2009/02/08 02:44:40Z  garyp
    Merged from the v4.0 branch.  Documentation updated.  Added D_UINT64_MIN.
    Added DCLABSDIFF().  Added DCLNEXTPOW2BOUNDARY().  Added DCL_MAX_THREADS.
    Revision 1.27  2008/05/21 02:14:08Z  garyp
    Merged from the WinMobile branch.
    Revision 1.26.1.2  2008/05/21 02:14:08Z  garyp
    Updated DCLISALIGNED to not use a modulus operator.
    Revision 1.26  2008/05/05 20:00:23Z  garyp
    Modified DCLISALIGNED() to avoid divide-by-zero problems.
    Revision 1.25  2008/05/02 21:05:23Z  garyp
    Updated to forcibly disable the command shell if output is disabled.
    Revision 1.24  2008/04/19 02:31:51Z  brandont
    Added default definition for DCLCONF_MINI_REDIRECTOR.
    Revision 1.23  2008/01/13 01:14:22Z  Garyp
    Documentation updated.
    Revision 1.22  2007/11/12 20:27:16Z  Garyp
    Documentation cleanup.
    Revision 1.21  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.20  2007/11/01 19:54:19Z  Garyp
    Eliminated the inclusion of limits.h.
    Revision 1.19  2007/10/18 17:07:07Z  pauli
    Added a set of verbose levels to allow for a consistent definition
    across all products, tests and tools.
    Revision 1.18  2007/10/07 05:30:19Z  pauli
    Added a DCLALIGNPAD to determine the number of bytes needed to make
    a value a multiple of DCL_ALIGNSIZE.
    Revision 1.17  2007/10/05 00:54:00Z  pauli
    Added extra parens to DCLISALIGNED so it will work as expected.
    Revision 1.16  2007/10/03 21:52:20Z  brandont
    Added default definitions for DCLCONF_RELIANCEREADERSUPPORT
    and DCLCONF_FATREADERSUPPORT.
    Revision 1.15  2007/09/26 22:41:15Z  pauli
    Added a new macro for determining if a value is aligned.
    Revision 1.14  2007/03/20 01:42:57Z  Garyp
    Added the DCLABS() macro.
    Revision 1.13  2007/03/06 18:32:11Z  Garyp
    Added the DCLMIN3() macro.
    Revision 1.12  2007/02/08 19:00:25Z  Garyp
    Added some file system symbols.
    Revision 1.11  2006/11/01 01:15:57Z  Garyp
    Added the DCLSIZEOFMEMBER() macro.
    Revision 1.10  2006/10/18 04:27:26Z  Garyp
    Added DCL_INVALID_THREADID.
    Revision 1.9  2006/10/10 01:00:53Z  Garyp
    Added a default global seed value.
    Revision 1.8  2006/08/18 18:16:53Z  Garyp
    Documentation fixes.
    Revision 1.7  2006/08/07 23:31:30Z  Garyp
    Added the DCLALIGNEDSTRUCT() macro.
    Revision 1.6  2006/05/03 01:21:15Z  Garyp
    Added the macros DCLBITCLEAR(), DCLBITGET(), and DCLBITSET().
    Revision 1.5  2006/03/07 22:15:22Z  brandont
    Added DCLSOURCELINE macro to replace __LINE__ in obfuscated source.
    Revision 1.4  2006/02/28 23:55:56Z  billr
    Correct typo.
    Revision 1.3  2006/02/28 02:06:07Z  brandont
    Added include limit.h to potentially resolve define for CHAR_BIT.
    Revision 1.2  2006/02/27 03:51:57Z  Garyp
    Added the CHAR_BIT and DCLALIGNEDBUFFER macros.
    Revision 1.1  2005/11/13 06:03:10Z  Pauli
    Initial revision
    Revision 1.4  2005/11/13 06:03:09Z  Garyp
    Added DCLSTRINGIZE().
    Revision 1.3  2005/11/07 15:05:40Z  Garyp
    Updated to use INT/UINT64SUFFIX.
    Revision 1.2  2005/10/21 02:39:51Z  garyp
    Added "min" limits for the standard signed types.
    Revision 1.1  2005/10/02 06:38:58Z  Garyp
    Initial revision
    Revision 1.3  2005/08/01 19:07:06Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.2  2005/07/31 02:57:28Z  Garyp
    Moved DCL_ALIGNSIZE definition into dlosconf.h.
    Revision 1.1  2005/07/10 04:50:20Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLMACROS_H_INCLUDED
#define DLMACROS_H_INCLUDED


#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif


/*-------------------------------------------------------------------
    Define the limits for the standard types.
-------------------------------------------------------------------*/
#define D_UINT8_MAX         (0xFF)
#define D_INT8_MAX          (0x7F)
#define D_INT8_MIN          (-D_INT8_MAX-1)
#define D_UINT16_MAX        (0xFFFFU)
#define D_INT16_MAX         (0x7FFF)
#define D_INT16_MIN         (-D_INT16_MAX-1)
#define D_UINT32_MAX        (0xFFFFFFFFU)
#define D_INT32_MAX         (0x7FFFFFFF)
#define D_INT32_MIN         (-D_INT32_MAX-1)

#if DCL_NATIVE_64BIT_SUPPORT
  #define D_UINT64_MIN      (UINT64SUFFIX(0x0))
  #define D_UINT64_MAX      (UINT64SUFFIX(0xFFFFFFFFFFFFFFFF))
  #define D_INT64_MAX       ( INT64SUFFIX(0x7FFFFFFFFFFFFFFF))
  #define D_INT64_MIN       (-D_INT64_MAX-1)
#endif


/*-------------------------------------------------------------------
    DCL_MAX_THREADS

    Define the maximum number of threads that any thread-aware code
    may need to deal with.  If this is not already defined, it will
    default to 10 for OS abstractions where threading is supported,
    and 1 for abstractions where it is not.

    Note that this defines the "maximum" number of threads that could
    be operating at once inside Datalight code.  Various products and
    subsystems may have restrictions which reduce that number inside
    that code.
-------------------------------------------------------------------*/
#ifndef DCL_MAX_THREADS
  #if DCL_OSFEATURE_THREADS
    #define DCL_MAX_THREADS   (10)
  #else
    #define DCL_MAX_THREADS   (1)
  #endif
#endif


/*-------------------------------------------------------------------
    Define the typical set of default services to initialize in most
    environments.
-------------------------------------------------------------------*/

#if DCLCONF_PROFILERENABLED
  #define DCLSVC_PROF       DclProfServiceInit,
#else
  #define DCLSVC_PROF
#endif

#if DCLCONF_MEMORYTRACKING
  #define DCLSVC_MEMTRACK   DclMemTrackServiceInit,
#else
  #define DCLSVC_MEMTRACK
#endif

#if DCLCONF_HIGHRESTIMESTAMP
  #define DCLSVC_HRTIMER    DclHighResTimeServiceInit,
#else
  #define DCLSVC_HRTIMER
#endif

#define DCL_DEFAULT_SERVICES        DclOutputServiceInit,       \
                                    DclAssertServiceInit,       \
                                    DclLogServiceInit,          \
                                    DCLSVC_PROF                 \
                                    DclInputServiceInit,        \
                                    DCLSVC_HRTIMER              \
                                    DclTimerServiceInit,        \
                                    DclRandomServiceInit,       \
                                    DCLSVC_MEMTRACK             \
                                    DclMemValServiceInit,       \
                                    NULL


/*-------------------------------------------------------------------
    DCL uses mutexes, counting semaphores, and threads that require
    unique identifying names on some operating systems.  DCL
    internally requires a minimum of 8 characters for these names,
    including a null terminator.  Even for those operating systems
    that don't require these objects to have real names, DCL uses
    the names to track synchronization objects during debugging.
-------------------------------------------------------------------*/
#define DCL_OSOBJECTNAMELEN             (8)


/*-------------------------------------------------------------------
    Define the invalid thread ID value.
-------------------------------------------------------------------*/
#define DCL_INVALID_THREADID            (0)


/*-------------------------------------------------------------------
    Calculate the proper DCL_MUTEXNAMELEN to use.
-------------------------------------------------------------------*/
#ifndef DCL_MUTEXNAMELEN
  #define DCL_MUTEXNAMELEN DCL_OSOBJECTNAMELEN
#else
  #if DCL_MUTEXNAMELEN < DCL_OSOBJECTNAMELEN
  #error "DCL_MUTEXNAMELEN is smaller than DCL_OSOBJECTNAMELEN"
  #endif
  #if DCL_MUTEXNAMELEN > DCL_OSOBJECTNAMELEN
  #undef DCL_MUTEXNAMELEN
  #define DCL_MUTEXNAMELEN DCL_OSOBJECTNAMELEN
  #endif
#endif
#if DCL_MUTEXNAMELEN == 0
  #error "DCL_MUTEXNAMELEN cannot be 0"
#endif


/*-------------------------------------------------------------------
    Calculate the proper DCL_SEMAPHORENAMELEN to use.
-------------------------------------------------------------------*/
#ifndef DCL_SEMAPHORENAMELEN
  #define DCL_SEMAPHORENAMELEN DCL_OSOBJECTNAMELEN
#else
  #if DCL_SEMAPHORENAMELEN < DCL_OSOBJECTNAMELEN
  #error "DCL_SEMAPHORENAMELEN is smaller than DCL_OSOBJECTNAMELEN"
  #endif
  #if DCL_SEMAPHORENAMELEN > DCL_OSOBJECTNAMELEN
  #undef DCL_SEMAPHORENAMELEN
  #define DCL_SEMAPHORENAMELEN DCL_OSOBJECTNAMELEN
  #endif
#endif
#if DCL_SEMAPHORENAMELEN == 0
  #error "DCL_SEMAPHORENAMELEN cannot be 0"
#endif


/*-------------------------------------------------------------------
    Calculate the proper DCL_THREADNAMELEN to use.
-------------------------------------------------------------------*/
#ifndef DCL_THREADNAMELEN
  #define DCL_THREADNAMELEN DCL_OSOBJECTNAMELEN
#else
  #if DCL_THREADNAMELEN < DCL_OSOBJECTNAMELEN
  #error "DCL_THREADNAMELEN is smaller than DCL_OSOBJECTNAMELEN"
  #endif
  #if DCL_THREADNAMELEN > DCL_OSOBJECTNAMELEN
  #undef DCL_THREADNAMELEN
  #define DCL_THREADNAMELEN DCL_OSOBJECTNAMELEN
  #endif
#endif
#if DCL_THREADNAMELEN == 0
  #error "DCL_THREADNAMELEN cannot be 0"
#endif


/*-------------------------------------------------------------------
    The following are generic file system ID values defined here
    for cross-product purposes.
-------------------------------------------------------------------*/
#define DCL_FILESYS_UNKNOWN                     (0)
#define DCL_FILESYS_FAT                         (1)
#define DCL_FILESYS_RELIANCE                    (2)
#define DCL_FILESYS_HRFS                        (3)


/*-------------------------------------------------------------------
    Macro: DCL_PAD()

    Pad an array to a total byte length that is some multiple of
    DCL_ALIGNSIZE.  This allows facilitates maintaining alignment
    of structure members even if they are declared as arrays of
    configurable length.  By keeping the fields DCL_ALIGNTYPE
    aligned and the array lengths as multiples of DCL_ALIGNTYPE
    then arrays of such structures will also be aligned.

    Example:
       Consider the standard array allocation:  char a[5];

       You would apply this macro as:  char a[DCL_PAD(5, char)];

       And the resulting allocation for the array would be 8 bytes
       (assuming DCL_ALIGNSIZE is 4).

    Parameters:
        n = the number of elements actually needed in the array
        t = the fundamental data type of each array element.  Make sure
            sizeof(t) <= DCL_ALIGNSIZE

    Return Value:
        Returns the count of necessary padding bytes.
-------------------------------------------------------------------*/
#define DCL_PAD(n, t) \
  ((((n) * sizeof(t)) + (DCL_ALIGNSIZE - (((n) * sizeof(t)) % DCL_ALIGNSIZE))) / sizeof(t))


/*-------------------------------------------------------------------
    Macro: DCLMAX()

    Determine the numerically greater of a set of 2 values.

    Parameters:
        val1 - The first value
        val2 - The second value

    Return Value:
        Returns the greater of the two values.
-------------------------------------------------------------------*/
#define DCLMAX(val1, val2)          ( (val1) > (val2) ? (val1) : (val2) )


/*-------------------------------------------------------------------
    Macro: DCLMIN()

    Determine the numerically smaller of a set of 2 values.

    Parameters:
        val1 - The first value
        val2 - The second value

    Return Value:
        Returns the lesser of the two values.
-------------------------------------------------------------------*/
#define DCLMIN(val1, val2)          ( (val1) < (val2) ? (val1) : (val2) )


/*-------------------------------------------------------------------
    Macro: DCLMIN3()

    Determine the numerically least of a set of 3 values.

    Parameters:
        val1 - The first value
        val2 - The second value
        val3 - The third value

    Return Value:
        Returns the lesser of the three values.
-------------------------------------------------------------------*/
#define DCLMIN3(val1, val2, val3)   ( DCLMIN((val1), DCLMIN((val2), (val3))) )


/*-------------------------------------------------------------------
    Macro: DCLXOR()

    Determine the logical exclusive-or evaluation of two values.

    Parameters:
        val1 - The first value
        val2 - The second value

    Return Value:
        Returns TRUE if exactly one value is nonzero or FALSE if not.
-------------------------------------------------------------------*/
#define DCLXOR(val1, val2)          ( ((val1) != 0) ^ ((val2) != 0) )


/*-------------------------------------------------------------------
    Macro: DCLABS()

    Determine the absolute value of a number.

    Parameters:
        val - The value

    Return Value:
        Returns the absolute value of the number.
-------------------------------------------------------------------*/
#define DCLABS(val)                 ( (val) < 0 ? -(val) : (val) )


/*-------------------------------------------------------------------
    Macro: DCLABSDIFF()

    Determine the absolute difference between two unsigned numbers.

    Parameters:
        val1 - The first value
        val2 - The second value

    Return Value:
        Returns the absolute difference between the values.
-------------------------------------------------------------------*/
#define DCLABSDIFF(val1, val2)      ( (val1) < (val2) ? ((val2)-(val1)) : ((val1)-(val2)) )


/*-------------------------------------------------------------------
    Macro: DCLNEXTPOW2BOUNDARY()

    Adjust a number up to the next value which is evenly divisible
    by a power-of-two value.

    Note that the result is not necessarily a power-of-two value.  If
    the boundary value is zero, the original value will be returned.

    Examples:
        DCLNEXTPOW2BOUNDARY(3, 0) == 3;
        DCLNEXTPOW2BOUNDARY(4, 2) == 4;
        DCLNEXTPOW2BOUNDARY(4, 4) == 4;
        DCLNEXTPOW2BOUNDARY(5, 2) == 6;
        DCLNEXTPOW2BOUNDARY(5, 4) == 8;

    Parameters:
        val    - The value to operate on
        bound  - The boundary value which must be a power-of-two

    Return Value:
        Returns the adjusted value.
-------------------------------------------------------------------*/
#define DCLNEXTPOW2BOUNDARY(val, bound)                             \
    ( (bound) ? (((val) + ((bound) - 1)) & ~((bound)-1)) : (val) )


/*-------------------------------------------------------------------
    Macro: DCLLOWESTSETBIT()

    Mask off all the bits except the lowest bit which is set.

    Parameters:
        val    - The value to operate on

    Return Value:
        Returns a value with only lowest original bit set.
-------------------------------------------------------------------*/
#define DCLLOWESTSETBIT(val)    ((val) & ((~(val)) + 1))


/*-------------------------------------------------------------------
    Macro: DCLISPOWEROF2()

    Determine whether a value is a power-of-two.

    Parameters:
        val    - The value to operate on

    Return Value:
        Returns TRUE if the value is a power-of-two or FALSE if not.
-------------------------------------------------------------------*/
#define DCLISPOWEROF2(val)      (DCLLOWESTSETBIT(val) == (val))


/*-------------------------------------------------------------------
    Macro: DCLDIMENSIONOF()

    Determine the number of elements in an array.  This is a very
    sharp-edged tool: use it on an array and it's fine; use it
    on a pointer and you'll hurt yourself with no warning.

    Parameters:
        obj - The object to process

    Return Value:
        Returns the number of elements in the array.
-------------------------------------------------------------------*/
#define DCLDIMENSIONOF(obj)     (sizeof(obj) / sizeof((obj)[0]))


/*-------------------------------------------------------------------
    Macro: DCLSTRINGIZE()

    It takes two levels of preprocessor macro substitution to turn
    a plain text sequence into a string literal.
-------------------------------------------------------------------*/
#define DCLSTRINGIZE(x) DCLSTRINGIZE2(x)
#define DCLSTRINGIZE2(x) #x


/*-------------------------------------------------------------------
    Macro: DCLALIGNEDBITMAP()

    Declare a general bitmap buffer of the type D_BUFFER, which is
    guaranteed to be aligned on a DCL_ALIGNTYPE boundary.

    Parameters:
        un  - The union name
        nam - The field name
        siz - The number of bytes in the field

    Return Value:
        None.
-------------------------------------------------------------------*/
#define DCLALIGNEDBITMAP(un, nam, siz)          \
    union                                       \
    {                                           \
        DCLBITMAP           (nam, siz);         \
        DCL_ALIGNTYPE       DummyAlign;         \
    } un


/*-------------------------------------------------------------------
    Macro: DCLALIGNEDBUFFER()

    Declare a DCL_ALIGNTYPE aligned byte buffer which is large enough
    to hold the specified number of bits.

    Parameters:
        un  - The union name
        nam - The field name
        siz - The number of bytes in the field

    Return Value:
        None.
-------------------------------------------------------------------*/
#define DCLALIGNEDBUFFER(un, nam, siz)          \
    union                                       \
    {                                           \
        D_BUFFER            nam[siz];           \
        DCL_ALIGNTYPE       DummyAlign;         \
    } un


/*-------------------------------------------------------------------
    Macro: DCLALIGNEDSTRUCT()

    Declare an instance of a structure which is guaranteed to start
    on a DCL_ALIGNTYPE aligned boundary.

    This macro uses a union of the specified structure and
    DCL_ALIGNTYPE, which is guaranteed to be aligned on a
    DCL_ALIGNTYPE boundary, as well has have an overall length
    which is evenly divisible by DCL_ALIGNTYPE.

    Parameters:
        un  - The union name
        str - The structure name
        nam - The name of the field

    Return Value:
        None.
-------------------------------------------------------------------*/
#define DCLALIGNEDSTRUCT(un, str, nam)          \
    union                                       \
    {                                           \
        str                 nam;                \
        DCL_ALIGNTYPE       DummyAlign;         \
    } un


/*-------------------------------------------------------------------
    Macro: DCLISALIGNED()

    Determine if the specified value is aligned on the specified
    byte boundary.  If "align" is 0, then "value" is considered to
    be aligned.

    Parameters:
        val   - The value to examine
        align - The alignment value

    Return Value:
        Returns TRUE if the value is aligned, or FALSE if not.
-------------------------------------------------------------------*/
#define DCLISALIGNED(val, align)    (!(align) || (((val) & ((align)-1)) == 0))


/*-------------------------------------------------------------------
    Macro: DCLALIGNPAD()

    Determines then number of bytes needed to make a value a multiple
    of DCL_ALIGNSIZE.

    Parameters:
        val - The value to examine

    Return Value:
        The amount required to pad the value.
-------------------------------------------------------------------*/
#define DCLALIGNPAD(val)                                    \
    ((DCLISALIGNED((val), DCL_ALIGNSIZE) ? 0                \
       : (DCL_ALIGNSIZE - ((val) % DCL_ALIGNSIZE))))


/*-------------------------------------------------------------------
    Macro: DCLBITMAP()

    Declare a byte oriented buffer which is large enough to hold the
    specified number of bits.

    Parameters:
        nam - The field name
        siz - The number of bits in the array

    Return Value:
        None.
-------------------------------------------------------------------*/
#define DCLBITMAP(nam, siz) D_BUFFER nam[DCLBITMAPDIMENSION(siz)]


/*-------------------------------------------------------------------
    Macro: DCLBITMAPDIMENSION()

    Determine the number of bytes required to contain the specified
    number of bits.

    Parameters:
        siz - The number of bits in the array

    Return Value:
        The number of bytes required.
-------------------------------------------------------------------*/
#define DCLBITMAPDIMENSION(n) (((n) + (CHAR_BIT-1)) / CHAR_BIT)


/*-------------------------------------------------------------------
    Macro: DCLBITGET()

    Get the value of a bit in an array of elements of the type
    pointed to by ptr.  The element type must not be wider than
    an int.

    *Note* -- The DCLBITGET(), DCLBITSET(), and DCLBITCLEAR() macros
              make no adjustments for byte-ordering.  This means that
              when using datatypes wider than a byte on a big-endian
              system, that a function such as DclBitCountArray() will
              not function properly unless the total count of bits is
              evenly divisible by the number of bits in the integral
              data type.

    Parameters:
        ptr - A pointer to the first element of the array.
        bit - The zero-based number of the bit to retrieve.

    Return Value:
        Returns the bit value, 0 or 1.
-------------------------------------------------------------------*/
#define DCLBITGET(ptr, bit)                                         \
    (((ptr)[(bit) / (sizeof (ptr)[0] * CHAR_BIT)] &                 \
            (1 << ((bit) % (sizeof (ptr)[0] * CHAR_BIT)))) != 0)


/*-------------------------------------------------------------------
    Macro: DCLBITSET()

    Set a bit in an array of elements of the type pointed to by ptr.
    The element type must not be wider than an int.  See the note
    regarding byte-ordering in the documentation for DCLBITGET().

    Parameters:
        ptr - A pointer to the first element of the array.
        bit - The zero-based number of the bit to set.

    Return Value:
        None.
 -------------------------------------------------------------------*/
#define DCLBITSET(ptr, bit)                                         \
    do                                                              \
    {                                                               \
        DclAssert(sizeof(*ptr) <= sizeof(int));                     \
        ((ptr)[(bit) / (sizeof (ptr)[0] * CHAR_BIT)] |=             \
                    1 << ((bit) % (sizeof (ptr)[0] * CHAR_BIT)));   \
    } while(FALSE)


/*-------------------------------------------------------------------
    Macro: DCLBITCLEAR()

    Clear a bit in an array of elements of the type pointed to by ptr.
    The element type must not be wider than an int.  See the note
    regarding byte-ordering in the documentation for DCLBITGET().

    Parameters:
        ptr - A pointer to the first element of the array.
        bit - The zero-based number of the bit to clear.

    Return Value:
        None.
-------------------------------------------------------------------*/
#define DCLBITCLEAR(ptr, bit)                                       \
    do                                                              \
    {                                                               \
        DclAssert(sizeof(*ptr) <= sizeof(int));                     \
        ((ptr)[(bit) / (sizeof (ptr)[0] * CHAR_BIT)] &=             \
                ~(1 << ((bit) % (sizeof (ptr)[0] * CHAR_BIT))));    \
    } while(FALSE)


/*-------------------------------------------------------------------
    Macro: DCLISHEXDIGIT()

    Determined if the specified character is in the class [a-fA-F].

    Parameters:
        c   - The character to examine.

    Return Value:
        Returns TRUE if the character is a hex digit, or FALSE if it
        is not.

    See Also:
        - DclIsDigit()
-------------------------------------------------------------------*/
#define DCLISHEXDIGITU(c)   (c >= 'A' && c <= 'F')
#define DCLISHEXDIGITL(c)   (c >= 'a' && c <= 'f')
#define DCLISHEXDIGIT(c)    (DCLISHEXDIGITL(c) || DCLISHEXDIGITU(c))


/*-------------------------------------------------------------------
    This macro is used in place of __LINE__ in obfuscated source.
    It is handled by our own HCL preprocesses to expand the source
    line number before the obfuscation process executes.
-------------------------------------------------------------------*/
#define DCLSOURCELINE()     (__LINE__)


/*-------------------------------------------------------------------
    Macro: DCLSIZEOFMEMBER()

    Determine the size of a structure member, without having to
    actually instantiate the structure.

    Parameters:
        typ - The structure type name
        mem - The member name

    Return Value:
        The size of the member.
-------------------------------------------------------------------*/
#define DCLSIZEOFMEMBER(typ, mem)   (sizeof((typ*)NULL)->mem)


/*-------------------------------------------------------------------
    Macro: DCLOFFSETOF()

    Given a structure type and the name of a member of the structure,
    derive the offset in bytes of that member relative to the
    beginning of the structure.

    This default definition will work for most processor
    architectures.  It may be overridden for OS environments,
    toolsets, or architectures that have special requirements.

    Parameters:
        type    - The structure type
        member  - The name of the member of the structure to which p
                  points

    Return Value:
        Returns a pointer of type (type *).
-------------------------------------------------------------------*/
#define DCLOFFSETOF(type, member) (((char *) &((type *) 0)->member) - (char *) 0)


/*-------------------------------------------------------------------
    Macro: DCLCONTAINEROF()

    Give a pointer to a member of a structure, the member name, and
    the type of the structure, derive a pointer to the structure.

    This default definition will work for most processor
    architectures.  It may be overridden for OS environments,
    toolsets, or architectures that have special requirements.

    Parameters:
        p       - Pointer to a structure member
        type    - The structure type
        member  - The name of the member of the structure to which p
                  points

    Return Value:
        Returns a pointer of type (type *).
-------------------------------------------------------------------*/
#define DCLCONTAINEROF(p, type, member) ((type *) ((char *) (p) - DCLOFFSETOF(type, member)))


/*-------------------------------------------------------------------
    Macro: DCLMEMGETn()

    Dereference the given memory address as an *n* bit width type.

    Parameters:
        addr - address to dereference

    Return Value:
        An n-bit wide int.
-------------------------------------------------------------------*/
#define  DCLMEMGET8(addr)         (*((volatile D_UINT8*)(addr)))
#define  DCLMEMGET16(addr)        (*((volatile D_UINT16*)(addr)))
#define  DCLMEMGET32(addr)        (*((volatile D_UINT32*)(addr)))
#define  DCLMEMGET64(addr)        (*((volatile D_UINT64*)(addr)))


/*-------------------------------------------------------------------
    Macro: DCLMEMPUTn()

    Write a given value to an address as an n-bit wide write.

    Parameters:
        addr - target address for the write
        data - data value to write to memory

    Return Value:
        None.
-------------------------------------------------------------------*/
#define  DCLMEMPUT8(addr, data)   ((*((volatile D_UINT8*)(addr)))  = (D_UINT8)(data))
#define  DCLMEMPUT16(addr, data)  ((*((volatile D_UINT16*)(addr))) = (D_UINT16)(data))
#define  DCLMEMPUT32(addr, data)  ((*((volatile D_UINT32*)(addr))) = (D_UINT32)(data))
#define  DCLMEMPUT64(addr, data)  ((*((volatile D_UINT64*)(addr))) = (D_UINT64)(data))


/*-------------------------------------------------------------------
    Default values for Reliance and FAT support in the DlLoader
    module.
-------------------------------------------------------------------*/
#ifndef DCLCONF_RELIANCEREADERSUPPORT
#define DCLCONF_RELIANCEREADERSUPPORT   TRUE
#endif
#ifndef DCLCONF_FATREADERSUPPORT
#define DCLCONF_FATREADERSUPPORT        TRUE
#endif


/*-------------------------------------------------------------------
    Verbosity Levels
-------------------------------------------------------------------*/
#define DCL_VERBOSE_QUIET       (0)
#define DCL_VERBOSE_NORMAL      (1)
#define DCL_VERBOSE_LOUD        (2)
#define DCL_VERBOSE_OBNOXIOUS   (3)


/*-------------------------------------------------------------------
    FS redirectory settings
-------------------------------------------------------------------*/
#ifndef DCLCONF_MINI_REDIRECTOR
#define DCLCONF_MINI_REDIRECTOR         FALSE
#endif


/*-------------------------------------------------------------------
    Validate shell settings
-------------------------------------------------------------------*/
#if DCLCONF_COMMAND_SHELL && ! DCLCONF_OUTPUT_ENABLED
#undef  DCLCONF_COMMAND_SHELL
#define DCLCONF_COMMAND_SHELL   (FALSE)
#endif



#endif /* DLMACROS_H_INCLUDED */

