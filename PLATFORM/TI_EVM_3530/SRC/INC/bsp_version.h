// Copyright (c) 2008 BSQUARE Corporation. All rights reserved.

//
//  File:  bsp_version.h
//
#ifndef __BSP_VERSION_H
#define __BSP_VERSION_H

// This BSP uses the following versioning scheme
// x.yy.zz
// x  -> major release number, typically CE version number
// yy -> minor release number, typically incremented on every BSP release
// zz -> additional incremental release number that can be used to version 
// patches to an incremental release

// NOTE: The BSP version is displayed in the IDE using the BSP Properties 
//  window.  The information in this window comes from the FileVersion field
//  in the pbcxml file.  The FileVersion field in the PBCXML file must be
//  synchronized with the contents of this file.

// Note: do not use a leading zero, compiler interprets leading 0 as octal number
#define MLDR_VERSION_MAJOR			6
#define MLDR_VERSION_MINOR			15

#define MLDR_VERSION_INCREMENTAL	310
#define MLDR_VERSION_BUILD			2

#define MLDR_VERSION_STRING			L"6.15.310.02"

#if PROJECT_LICENSE == 2
#define PRJ_LIC	L"PRO "
#elif PROJECT_LICENSE == 1
#define PRJ_LIC	L"CORE PLUS "
#else
#define PRJ_LIC	L"CORE "
#endif

#define BSP_VERSION_MAJOR       6
#define BSP_VERSION_MINOR       15

#define BSP_VERSION_INCREMENTAL	310
#define BSP_VERSION_BUILD		2

#define BSP_VERSION_STRING      PRJ_LIC L"6.15.310.02"

#define EBOOT_VERSION_MAJOR		    6
#define EBOOT_VERSION_MINOR		    15
#define EBOOT_VERSION_INCREMENTAL	310
#define EBOOT_VERSION_BUILD			3

#define EBOOT_VERSION_STRING		L"6.15.310.03"

#define XLDR_VERSION_MAJOR		    6
#define XLDR_VERSION_MINOR		    15
#define XLDR_VERSION_INCREMENTAL	310
#define XLDR_VERSION_BUILD			02

#define XLDR_VERSION_STRING		    L"6.15.310.02"

#define MARKER_LOW   0x55AA55AA
#define MARKER_HIGH  0xAA55AA55
#define MARKER_EXT	 0xA5A5A5A5

#define DATE_STRING		20
#define VERS_STRING_LEN	64

typedef struct _tagVersionPattern
{
	UINT32  marker_low; 
	UINT32  majorVersion;    
	UINT32  minorVersion;    
	UINT32  incrementalVersion; 
	char    dateString[DATE_STRING];
	TCHAR   versionString[VERS_STRING_LEN]; 
	UINT32  marker_hi;  
                     	
}VersionPattern;
typedef struct _tagVersionPatternExt
{
	VersionPattern ver;
	UINT32	Build;
	UINT32  marker_ext;  
}VersionPatternExt;
#endif
