/*++

Copyright (c) 2001  Future Technology Devices International Ltd.

Module Name:

    string.c

Abstract:

    Native USB device driver for FTDI FT8U245
    String routines

Environment:

    kernel mode only

Revision History:

    14/06/01    awm     Created.

--*/

#include <windows.h>


PCHAR
FT_StrCpy(
    PCHAR d,
    PCHAR s
    )
{
    PCHAR dest = d;


    while (*d++ = *s++)
        ;

    return dest;
}


ULONG
FT_StrLen(
    PCHAR s
    )
{
    ULONG i = 0;

    while (*s++)
        ++i;

    return i;
}

PCHAR
FT_CopyWStrToStr(
    PCHAR Dest,
    USHORT DestLen,
    PUSHORT WStr,
    USHORT Len
    )
{
    PUSHORT s = WStr;
    PCHAR d = Dest;
    CHAR c;

    while (Len && DestLen) {
        c = (CHAR) (*s++ & 0x00ff);
        *d++ = c;
        Len -= sizeof(USHORT);
        --DestLen;
    }

    *d = '\0';

    return Dest;

}

