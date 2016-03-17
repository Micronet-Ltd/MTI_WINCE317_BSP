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

    This header defines the Datalight POSIX interface.  Note that this
    header does not include the actual function calls at this time, as they
    are tied to Reliance.  Ultimately when things are fully abstracted, the
    Datalight POSIX redirector functions will be defined in this header.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlposix.h $
    Revision 1.6  2011/11/17 00:57:11Z  daniel.lewis
    Changed the private data member in DLP_DIR from a pointer to an
    integer. This matches how the member is used, and it avoids a benign
    set of compiler warnings arising from pointer-to-integer conversions.
    Revision 1.5  2011/07/24 04:36:29Z  garyp
    Comments updated -- no functional changes.
    Revision 1.4  2011/07/14 16:32:28Z  garyp
    Removed inaccurate comments.
    Revision 1.3  2011/04/27 17:11:15Z  garyp
    Moved the errno stuff to rlposix.h.
    Revision 1.2  2011/04/27 17:07:41Z  garyp
    Updated to include errno.h.
    Revision 1.1  2011/04/27 00:48:32Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLPOSIX_H_INCLUDED
#define DLPOSIX_H_INCLUDED


/*---------------------------------------------------------
    DLP_FS_MAXPATHLEN defines the absolute maximum path
    length which the DLP interfaces can support.

    DLP_FS_MAXFSNAMELEN defines the absolute maximum
    length of the file system name.

    ToDo: These should be defined in dclconf.h, and
          similar Reliance settings should be defined
          in terms of these values.
---------------------------------------------------------*/
#define DLP_FS_MAXPATHLEN             (1024)
#define DLP_FS_MAXFSNAMELEN             (32)


/*---------------------------------------------------------
    Seek Origin
---------------------------------------------------------*/
#define DLP_SEEK_SET        0
#define DLP_SEEK_CUR        1
#define DLP_SEEK_END        2


/*---------------------------------------------------------
    Open Flags
---------------------------------------------------------*/
#define DLP_O_RDONLY        0x0000  /* Open for read only */
#define DLP_O_WRONLY        0x0001  /* Open for write only */
#define DLP_O_RDWR          0x0002  /* Read/write access allowed. */
#define DLP_O_APPEND        0x0004  /* Seek to EOF on each write */
#define DLP_O_CREAT         0x0008  /* Create the file if it does not exist. */
#define DLP_O_TRUNC         0x0010  /* Truncate the file if it already exists */
#define DLP_O_EXCL          0x0020  /* Fail if creating and already exists */
#define DLP_O_NOSHAREANY    0x0040  /* Fail if already open.  Other opens will fail. */
#define DLP_O_NOSHAREWRITE  0x0080  /* Fail if already open for write.  Other open for write calls will fail. */


/*---------------------------------------------------------
    File permissions (in octal notation)
---------------------------------------------------------*/
#define DLP_S_IRUSR         0000400
#define DLP_S_IWUSR         0000200
#define DLP_S_IXUSR         0000100
#define DLP_S_IRWXU         (DLP_S_IRUSR | DLP_S_IWUSR | DLP_S_IXUSR)
#define DLP_S_IRGRP         0000040
#define DLP_S_IWGRP         0000020
#define DLP_S_IXGRP         0000010
#define DLP_S_IRWXG         (DLP_S_IRGRP | DLP_S_IWGRP | DLP_S_IXGRP)
#define DLP_S_IROTH         0000004
#define DLP_S_IWOTH         0000002
#define DLP_S_IXOTH         0000001
#define DLP_S_IRWXO         (DLP_S_IROTH | DLP_S_IWOTH | DLP_S_IXOTH)
#define DLP_S_ISUID         0004000
#define DLP_S_ISGID         0002000
#define DLP_S_ISVTX         0001000


/*---------------------------------------------------------
    File types (in octal notation)
---------------------------------------------------------*/
#define DLP_S_IFMT          0170000 /* File type mask */
#define DLP_S_IFBLK         0060000 /* Block device special */
#define DLP_S_IFCHR         0020000 /* Character special */
#define DLP_S_IFDIR         0040000 /* Directory */
#define DLP_S_IFIFO         0010000 /* FIFO special */
#define DLP_S_IFLNK         0120000 /* Symbolic link */
#define DLP_S_IFREG         0100000 /* Regular */
#define DLP_S_IFSOCK        0140000 /* Socket special */


/*---------------------------------------------------------
    File type macros to test the file type.
---------------------------------------------------------*/
#define DLP_S_ISBLK(m)      (m & DLP_S_IFBLK)
#define DLP_S_ISCHR(m)      (m & DLP_S_IFCHR)
#define DLP_S_ISDIR(m)      (m & DLP_S_IFDIR)
#define DLP_S_ISFIFO(m)     (m & DLP_S_IFIFO)
#define DLP_S_ISLNK(m)      (m & DLP_S_IFLNK)
#define DLP_S_ISREG(m)      (m & DLP_S_IFREG)
#define DLP_S_ISSOCK(m)     (m & DLP_S_IFSOCK)


/*---------------------------------------------------------
    File system attributes
---------------------------------------------------------*/
#define DLP_ST_RDONLY       0x0001  /*  Read-only */
#define DLP_ST_NOSUID       0x0002  /*  Does not support setuid/setgid-bit semantics. */
#define DLP_ST_QUOTA        0x0004  /*  Supports quotas. */
#define DLP_ST_NOTRUNC      0x0008  /*  Does not truncate files longer than NAME_MAX. */
#define DLP_ST_CASE_PRES    0x0010  /*  Preserves case of file names. */
#define DLP_ST_CASE_SENS    0x0020  /*  Supports case-sensitive file names. */
#define DLP_ST_UNICODE      0x0040  /*  Supports UNICODE path names. */
#define DLP_ST_ACLS         0x0080  /*  Supports persistent ACLs for security. */
#define DLP_ST_FILECOMP     0x0100  /*  Supports per-file compression. */
#define DLP_ST_COMPRESSED   0x0200  /*  Is compressed. */
#define DLP_ST_CRYPT        0x0400  /*  Supports encryption. */
#define DLP_ST_OBJ_IDS      0x0800  /*  Supports object IDs. */
#define DLP_ST_REPARSE      0x1000  /*  Supports reparse points. */
#define DLP_ST_SPARSE       0x2000  /*  Supports sparse files */


/*---------------------------------------------------------
    POSIX Error codes

    NOTE:  These values are generally similar to other
           system's POSIX error codes, but may not be
           identical in every instance.

    Codes which are not currently used by Datalight
    products are commented out.
---------------------------------------------------------*/
#define DLP_EFAIL              -1           /* General or unknown failure           */
#define DLP_ENONE               0           /* Everything's OK                      */
#define DLP_EPERM               1           /* Operation not permitted              */
#define DLP_ENOENT              2           /* No such file or directory            */
/* #define DLP_ESRCH            3       */  /* No such process                      */
#define DLP_EINTR               4           /* Interrupted function call            */
#define DLP_EIO                 5           /* Input/Output error                   */
#define DLP_ENXIO               6           /* No such device or address            */
/* #define DLP_E2BIG            7       */  /* Argument list too long               */
/* #define DLP_ENOEXEC          8       */  /* Executable file format error         */
#define DLP_EBADF               9           /* Bad file descriptor                  */
#define DLP_ECHILD              10          /* No child process                     */
#define DLP_EAGAIN              11          /* Resource is temporarily unavailable, try again */
/* #define DLP_ENOMEM           12      */  /* Not enough memory                    */
#define DLP_EACCES              13          /* Permission denied                    */
/* #define DLP_EFAULT           14      */  /* Bad address                          */
#define DLP_ENOTEMPTY           15          /* Directory not empty  (SHOULD BE 41?) */
#define DLP_EBUSY               16          /* Resource busy                        */
#define DLP_EEXIST              17          /* File exists                          */
/* #define DLP_EXDEV            18      */  /* Improper link                        */
/* #define DLP_ENODEV           19      */  /* No such device                       */
#define DLP_ENOTDIR             20          /* Not a directory                      */
#define DLP_EISDIR              21          /* Is a directory                       */
#define DLP_EINVAL              22          /* Invalid argument                     */
#define DLP_ENFILE              23          /* Too many files open in system        */
#define DLP_EMFILE              24          /* File descriptor value is too large, or too many open files */
/* #define DLP_ENOTTY           25      */  /* Inappropriate I/O control operation  */
#define DLP_ENAMETOOLONG        26          /* Filename too long    (SHOULD BE 38?) */
#define DLP_EFBIG               27          /* File too large                       */
#define DLP_ENOSPC              28          /* No space left on a device            */
#define DLP_ESPIPE              29          /* Invalid seek                         */
#define DLP_EROFS               30          /* Read-only file system                */
#define DLP_EMLINK              31          /* Too many links                       */
#define DLP_ELOOP               DLP_EMLINK  /* Symbolic link loop   (SHOULD BE 40?) */

/*  A partial list of other codes which are not used by Datalight.
*/
#if 0
#define DLP_EPIPE               32          /* Broken pipe                          */
#define DLP_EDOM                33          /* Math argument out of domain of func  */
#define DLP_ERANGE              34          /* Math result not representable        */
#define DLP_EDEADLK             36          /* Resource deadlock would occur        */
#define DLP_ENOLCK              39          /* No locks available                   */
#define DLP_ENOSYS              40          /* Function not supported               */
#define DLP_EILSEQ              42          /* Illegal byte sequence                */

#define DLP_ENOTSUP             ??          /* Not supported                        */
#define DLP_EMSGSIZE            ??          /* Message too large                    */
#define DLP_EADDRINUSE          ??          /* Address in use                       */
#define DLP_EADDRNOTAVAIL       ??          /* Address not available                */
#define DLP_EALREADY            ??          /* Connection already in progress       */
#define DLP_EBADMSG             ??          /* Bad message                          */
#define DLP_ECANCELLED          ??          /* Operation cancelled                  */
#define DLP_ECONNABORTED        ??          /* Connection refused                   */
#define DLP_ECONNRESET          ??          /* Connection reset                     */
#define DLP_EDESTADDRREQ        ??          /* Destination address required         */
#define DLP_EDQUOT              ??          /* Reserved                             */
#define DLP_EHOSTUNREACH        ??          /* Host is unreachable                  */
#define DLP_EIDRM               ??          /* Identifier removed                   */
#define DLP_EILSEQ              ??          /* Illegal byte sequence                */
#define DLP_EINPROGRESS         ??          /* Operation in progress                */
#define DLP_EISCONN             ??          /* Socket is connected                  */
#define DLP_EMULTIHOP           ??          /* Reserved                             */
#endif


/*---------------------------------------------------------
    Datalight POSIX data types.
---------------------------------------------------------*/
typedef int                 DLP_size_t;
typedef unsigned int        DLP_mode_t;
typedef long                DLP_off_t;
typedef unsigned short      DLP_attr_t;
typedef unsigned short      DLP_dev_t;
typedef unsigned long       DLP_ino_t;
typedef unsigned short      DLP_nlink_t;
typedef unsigned int        DLP_uid_t;
typedef unsigned int        DLP_gid_t;
typedef unsigned long       DLP_time_t;
typedef int                 DLP_blksize_t;
typedef unsigned long       DLP_blkcnt_t;


/*---------------------------------------------------------
    Structures
---------------------------------------------------------*/
typedef struct
{
    DLP_mode_t      st_mode;    /* file mode/type */
    DLP_attr_t      st_attr;    /* attributes */
    DLP_ino_t       st_ino;     /* file serial number */
    DLP_dev_t       st_dev;     /* device number where this file is located */
    DLP_nlink_t     st_nlink;   /* number of links to the file */
    DLP_uid_t       st_uid;     /* user ID */
    DLP_gid_t       st_gid;     /* group ID */
    DLP_off_t       st_size;    /* size of the file, in bytes */
    DLP_time_t      st_atime;   /* access time */
    DLP_time_t      st_mtime;   /* modified time */
    DLP_time_t      st_ctime;   /* creation time */
} DLP_STAT;

typedef struct
{
    unsigned long   f_bsize;    /* block size */
    unsigned long   f_frsize;   /* fundamental size (fragmentation size) */
    DLP_blkcnt_t    f_blocks;   /* total blocks */
    DLP_blkcnt_t    f_bfree;    /* blocks free */
    DLP_blkcnt_t    f_bavail;   /* blocks available for non-privileged processes */
    DLP_blkcnt_t    f_files;    /* total file nodes */
    DLP_blkcnt_t    f_ffree;    /* file nodes free */
    DLP_blkcnt_t    f_favail;   /* file nodes available for non-privileged processes */
    unsigned long   f_fsid;     /* file system ID */
    unsigned long   f_flag;     /* ?? */
    unsigned long   f_namemax;  /* maximum name length */
    unsigned long   f_type;     /* file system type */
    char            f_basetype[DLP_FS_MAXFSNAMELEN + 1];  /* file system name */
    char            f_str[DLP_FS_MAXPATHLEN + 1];         /* volume label */
} DLP_STATVFS;

typedef struct
{
    char            d_name[DLP_FS_MAXPATHLEN + 1];
} DLP_DIRENT;


#define DLP_DIR_SIG         0x33441122

typedef struct
{
    unsigned long   ulSignature;
    char            szName[DLP_FS_MAXPATHLEN + 1];
    unsigned long   ulPrivateData;              /* Private file system data */
} DLP_DIR;


#endif  /* DLPOSIX_H_INCLUDED */


