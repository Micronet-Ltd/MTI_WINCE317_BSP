//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

#include "ceshpriv.h"

// Defines
#define IL_ALL                         ((UINT)-1)

// Flags
#define CEIDLIST_TYPEMASK              ((WORD)0xf000)
#define CEIDLIST_FILESYSTEM            ((WORD)0x1000)
#define CEIDLIST_NAMESPACE             ((WORD)0x2000)
#define CEIDLIST_FS_DIRECTORY          ((WORD)0x0001)
#define CEIDLIST_FS_LINK               ((WORD)0x0002)
#define CEIDLIST_FS_CACHEMASK          ((WORD)0x0f00)
#define CEIDLIST_FS_DATECACHED         ((WORD)0x0100)
#define CEIDLIST_FS_SIZECACHED         ((WORD)0x0200)
#define CEIDLIST_FS_TYPECACHED         ((WORD)0x0400)

// Access macros
#define IL_CB(p)                       (*((USHORT*) p))
#define IL_GET_FLAGS(p)                (HIWORD(*((DWORD*) p)))
#define IL_SET_FLAGS(p,f)              (*((DWORD*) p)) |= (MAKELONG(0,((WORD)(f))))
#define IL_GET_PARSINGNAME_OFFSET(p)   (sizeof(USHORT)+sizeof(WORD)+sizeof(WORD)+sizeof(WORD))
#define IL_GET_DISPLAYNAME_OFFSET(p)   (LOWORD((*((DWORD*)(((BYTE*)(p))+sizeof(USHORT)+sizeof(WORD))))))
#define IL_SET_DISPLAYNAME_OFFSET(p,o) (*((DWORD*)(((BYTE*)(p))+sizeof(USHORT)+sizeof(WORD)))) |= (MAKELONG(((WORD)(o)),0))
#define IL_GET_CACHE_OFFSET(p)         (HIWORD((*((DWORD*)(((BYTE*)(p))+sizeof(USHORT)+sizeof(WORD))))))
#define IL_SET_CACHE_OFFSET(p,o)       (*((DWORD*)(((BYTE*)(p))+sizeof(USHORT)+sizeof(WORD)))) |= (MAKELONG(0,((WORD)(o))))
#define IL_PARSINGNAME(p)              ((WCHAR*)(((BYTE*)(p))+IL_GET_PARSINGNAME_OFFSET(p)))
#define IL_DISPLAYNAME(p)              ((WCHAR*)(((BYTE*)(p))+IL_GET_DISPLAYNAME_OFFSET(p)))
#define IL_DATECACHED(p)               (((BYTE*)(p))+IL_GET_CACHE_OFFSET(p))
#define IL_SIZECACHED(p)               (((BYTE*)(p))+IL_GET_CACHE_OFFSET(p)+sizeof(FILETIME))
#define IL_TYPECACHED(p)               ((WCHAR*)(((BYTE*)(p))+IL_GET_CACHE_OFFSET(p)+sizeof(FILETIME)+sizeof(ULARGE_INTEGER)))


LPITEMIDLIST ILConcatenate(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
LPITEMIDLIST ILCopy(LPCITEMIDLIST pidl, UINT cbItems);
DWORD ILCount(LPCITEMIDLIST pidl);
LPITEMIDLIST ILCreate(LPCWSTR pwszParsingName, LPCWSTR pwszDisplayName, DWORD dwFlags);
LPITEMIDLIST ILCreateWithCache(LPCWSTR pwszParsingName, LPCWSTR pwszDisplayName, WORD flags,
                               LPCWSTR pwszFilePath, WIN32_FIND_DATA * pfd);
LPCWSTR ILDisplayName(LPCITEMIDLIST pidl);
LPITEMIDLIST ILFindChild(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild);
LPITEMIDLIST ILFindLast(LPCITEMIDLIST pidl);
void ILFree(LPCITEMIDLIST pidl);
BOOL ILGetDateCached(LPCITEMIDLIST pidl, FILETIME* ftDateOut);
BOOL ILGetSizeCached(LPCITEMIDLIST pidl, ULARGE_INTEGER* uliSizeOut);
LPCWSTR ILGetTypeCached(LPCITEMIDLIST pidl);
BOOL ILHasDateCached(LPCITEMIDLIST pidl);
BOOL ILHasSizeCached(LPCITEMIDLIST pidl);
BOOL ILHasTypeCached(LPCITEMIDLIST pidl);
BOOL ILIsEmpty(LPCITEMIDLIST pidl);
BOOL ILIsEqualParsingNames(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
BOOL ILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
BOOL ILIsFileSystemPidl(LPCITEMIDLIST pidl);
BOOL ILIsFileSystemDirectoryPidl(LPCITEMIDLIST pidl);
BOOL ILIsFileSystemLinkPidl(LPCITEMIDLIST pidl);
BOOL ILIsNameSpacePidl(LPCITEMIDLIST pidl);
BOOL ILIsNameSpace(LPCITEMIDLIST pidl, REFCLSID rclsid);
BOOL ILIsParent(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fImmediate);
LPITEMIDLIST ILNext(LPCITEMIDLIST pidl);
BOOL ILRemoveLast(LPITEMIDLIST pidl);
LPCWSTR ILParsingName(LPCITEMIDLIST pidl);
size_t ILSize(LPCITEMIDLIST pidl, UINT cbItems);

HRESULT CreateFileSystemPidl(LPCWSTR pwszPathPart, LPITEMIDLIST * ppidl);
HRESULT CreateNameSpacePidl(REFCLSID rclsid, LPITEMIDLIST * ppidl);
HRESULT RenameNamespacePidl(LPCWSTR pwszNewDisplayName, LPITEMIDLIST * ppidl);
HRESULT UpdateFileSystemPidl(LPCITEMIDLIST pidlParent, LPITEMIDLIST * ppidl);

class NameSpacePidlCache
{
private:
    enum CacheIDs
    {
        DESKTOP,
        DRIVES,
        BITBUCKET,
        NETWORK,
        CACHE_COUNT
    };

    struct CacheItem
    {
	    CLSID clsid;
	    LPITEMIDLIST pidl;
    };

    CacheItem rgCache[CACHE_COUNT];

public:
    NameSpacePidlCache();
    ~NameSpacePidlCache();

    LPCITEMIDLIST Get(REFCLSID rclsid);
    void Cache(REFCLSID rclsid, LPCITEMIDLIST pidl);
    void Update(LPCITEMIDLIST pidl);
};
