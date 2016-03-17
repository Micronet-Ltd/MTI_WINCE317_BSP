/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
#include <windows.h>
#include <bsp.h>
#include <initguid.h>
#include <twl.h>
#include <bus.h>
#include <display.h>
#include <hdmi.h>
#include <omap35xx_guid.h>
#include <memtxapi.h>
#include "utils.h"
#include <i2cproxy.h>
#include <proxyapi.h>
#include <tps659xx_bci.h>

#include "pmxproxy.h"
#include "omap35xx_es20_obs.h"

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

// enables cacheinfo command to display OAL cache info, used for testing only
#define CACHEINFO_ENABLE        FALSE

#define MESSAGE_BUFFER_SIZE     280

EXTERN_C const GUID APM_SECURITY_GUID;

//-----------------------------------------------------------------------------

typedef VOID (*PFN_FmtPuts)(WCHAR *s, ...);
typedef WCHAR* (*PFN_Gets)(WCHAR *s, int cch);

//-----------------------------------------------------------------------------

WCHAR const* GetVddText(int i);
WCHAR const* GetDpllText(int i);
WCHAR const* GetDpllClockText(int i);
WCHAR const* GetDeviceClockText(int i);
UINT FindClockIdByName(WCHAR const *szName);
OMAP_DEVICE FindDeviceIdByName(WCHAR const *szName);
WCHAR const* FindDeviceNameById(OMAP_DEVICE clockId);
WCHAR const* FindClockNameById(UINT clockId, UINT nLevel);
BOOL FindClockIdByName(WCHAR const *szName, UINT *pId, UINT *pLevel);


static BOOL Help(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL InReg8(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL InReg16(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL InReg32(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL OutReg8(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL OutReg16(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL OutReg32(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL InRegTWL(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL OutRegTWL(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL InI2C(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL OutI2C(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL CpuIdle(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL Device_xxx(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL Observe(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL SetContrast(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL OPMode(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL SmartReflex(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL Battery(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);

static BOOL Dump_RegisterGroup(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL TouchScreenCalibrate(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL ScreenRotate(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
static BOOL Reboot(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts );
static BOOL Display(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts );
static BOOL TvOut(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts );
#if !BSP_DVI_NO_SWITCHING
static BOOL DVIControl(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts );
#endif
static BOOL InterruptLatency(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts );
static BOOL PowerDomain(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts );

extern BOOL ProfileDvfs(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts );
extern BOOL ProfileInterruptLatency(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts );
extern BOOL ProfileSdrcStall(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts );
extern BOOL ProfileWakeupAccuracy(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts );
static BOOL HalProfile(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts );

static LONG   GetDec(LPCWSTR string);
static UINT32 GetHex(LPCWSTR string);

static void Dump_PRCM(PFN_FmtPuts pfnFmtPuts);
static void Dump_PRCMStates(PFN_FmtPuts pfnFmtPuts);
static void Dump_GPMC(PFN_FmtPuts pfnFmtPuts);
static void Dump_SDRC(PFN_FmtPuts pfnFmtPuts);
static void Dump_DSS(PFN_FmtPuts pfnFmtPuts);
static void Dump_ContextRestore(PFN_FmtPuts pfnFmtPuts);
static void Dump_EFuse(PFN_FmtPuts pfnFmtPuts);

static BOOL WriteTWLRegs(HANDLE hTWL, DWORD address, VOID *pBuffer, DWORD size);
static BOOL ReadTWLRegs(HANDLE hTWL, DWORD address, VOID *pBuffer, DWORD size);

static BOOL SetSlaveAddress(HANDLE hI2C, DWORD address, DWORD mode);
static DWORD WriteI2C(HANDLE hI2C, UINT8  subaddr, VOID*  pBuffer, DWORD  count, DWORD *pWritten);
static DWORD ReadI2C(HANDLE hI2C, UINT8  subaddr, VOID*  pBuffer, DWORD  count, DWORD *pRead);

#if CACHEINFO_ENABLE
static BOOL ShowCacheInfo(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts );
#endif

static BOOL GetNEONStat(ULONG argc,LPWSTR args[],PFN_FmtPuts pfnFmtPuts);

//-----------------------------------------------------------------------------

HANDLE GetBusHandle()
{
    static HANDLE _hBus = NULL;
    if (_hBus == NULL)
        {
        _hBus = CreateFile(L"BUS1:", GENERIC_WRITE|GENERIC_READ,             
                    0, NULL, OPEN_EXISTING, 0, NULL
                    );
        if (_hBus == INVALID_HANDLE_VALUE)
            {
            _hBus = NULL;
            }
        }

    return _hBus;
}

HANDLE GetProxyDriverHandle()
{
    static HANDLE _hProxy = NULL;
    if (_hProxy == NULL)
        {
        _hProxy = CreateFile(L"PXY1:", GENERIC_WRITE|GENERIC_READ,
                    0, NULL, OPEN_EXISTING, 0, NULL
                    );
        if (_hProxy == INVALID_HANDLE_VALUE)
            {
            _hProxy = NULL;
            }
        }

    return _hProxy;
}

HANDLE GetTritonHandle()
{
    static HANDLE _hTwl = NULL;
    if (_hTwl == NULL)
        {
        _hTwl = CreateFile(L"TWL1:", GENERIC_WRITE|GENERIC_READ,
                    0, NULL, OPEN_EXISTING, 0, NULL
                    );
        if (_hTwl == INVALID_HANDLE_VALUE)
            {
            _hTwl = NULL;
            }
        }

    return _hTwl;
}

//-----------------------------------------------------------------------------

typedef struct {
    DWORD       dwStart;
    DWORD       dwSize;
    void       *pv;
} MEMORY_REGISTER_ENTRY;


static MEMORY_REGISTER_ENTRY _pRegEntries[] = {
    { OMAP_CONFIG_REGS_PA,          OMAP_CONFIG_REGS_SIZE,      NULL},
    { OMAP_WKUP_CONFIG_REGS_PA,     OMAP_WKUP_CONFIG_REGS_SIZE, NULL},
    { OMAP_PERI_CONFIG_REGS_PA,     OMAP_PERI_CONFIG_REGS_SIZE, NULL},
    { OMAP_CTRL_REGS_PA,            OMAP_CTRL_REGS_SIZE,        NULL},
    { OMAP_GPMC_REGS_PA,            OMAP_GPMC_REGS_SIZE,        NULL},
    { OMAP_SMS_REGS_PA,             OMAP_SMS_REGS_SIZE,         NULL},
    { OMAP_SDRC_REGS_PA,            OMAP_SDRC_REGS_SIZE,        NULL},
    { OMAP_SGX_REGS_PA,             OMAP_SGX_REGS_SIZE,         NULL},
    { 0,                            0,                          NULL}
};


void*
MmMapIoSpace_Proxy(
    PHYSICAL_ADDRESS PhysAddr,
    ULONG size,
    BOOL bNotUsed
    )
{
    MEMORY_REGISTER_ENTRY *pRegEntry = _pRegEntries;
    void *pAddress = NULL;
    UINT64 phSource;
    UINT32 sourceSize, offset;
    BOOL rc;
    VIRTUAL_COPY_EX_DATA data;


    // Check if we can use common mapping for device registers (test is
    // simplified as long as we know that we support only 32 bit addressing).
    //
    if (PhysAddr.HighPart == 0)
        {
        while (pRegEntry->dwSize > 0)
            {
            if (pRegEntry->dwStart <= PhysAddr.LowPart &&
                (pRegEntry->dwStart + pRegEntry->dwSize) > PhysAddr.LowPart)
                {

                // check if memory is already mapped to the current process space
                rc = TRUE;
                if (pRegEntry->pv == NULL)
                    {
                    // reserve virtual memory and map it to a physical address
                    //
                    pRegEntry->pv = VirtualAlloc(
                                        0, pRegEntry->dwSize, MEM_RESERVE,
                                        PAGE_READWRITE | PAGE_NOCACHE
                                        );

                    if (pRegEntry->pv == NULL)
                        {
                        DEBUGMSG(TRUE, (
                            L"ERROR: MmMapIoSpace_Proxy failed reserve registers memory\r\n"
                            ));
                        goto cleanUp;
                        }

                    // Populate IOCTL parameters
                    //
                    data.idDestProc = GetCurrentProcessId();
                    data.pvDestMem = pRegEntry->pv;
                    data.physAddr = pRegEntry->dwStart;
                    data.size = pRegEntry->dwSize;
                    rc = DeviceIoControl(GetProxyDriverHandle(), IOCTL_VIRTUAL_COPY_EX,
                            &data, sizeof(VIRTUAL_COPY_EX_DATA), NULL, 0,
                            NULL, NULL
                            );

                    }

                if (!rc)
                    {
                    DEBUGMSG(TRUE, (
                        L"ERROR: MmMapIoSpace_Proxy failed allocate registers memory\r\n"
                        ));
                    VirtualFree(pRegEntry->pv, 0, MEM_RELEASE);
                    pRegEntry->pv = NULL;
                    goto cleanUp;
                    }

                // Calculate offset
                offset = PhysAddr.LowPart - pRegEntry->dwStart;
                pAddress = (void*)((UINT32)pRegEntry->pv + offset);
                break;
                }

            // check next register map entry
            //
            ++pRegEntry;
            }
        }

    if (pAddress == NULL)
        {
        phSource = PhysAddr.QuadPart & ~(PAGE_SIZE - 1);
        sourceSize = size + (PhysAddr.LowPart & (PAGE_SIZE - 1));

        pAddress = VirtualAlloc(0, sourceSize, MEM_RESERVE, PAGE_READWRITE | PAGE_NOCACHE);
        if (pAddress == NULL)
            {
            DEBUGMSG(TRUE, (
                L"ERROR: MmMapIoSpace_Proxy failed reserve memory\r\n"
                ));
            goto cleanUp;
            }

        // Populate IOCTL parameters
        //
        data.idDestProc = GetCurrentProcessId();
        data.pvDestMem = pAddress;
        data.physAddr = (UINT)phSource;
        data.size = sourceSize;
        rc = DeviceIoControl(GetProxyDriverHandle(), IOCTL_VIRTUAL_COPY_EX,
                &data, sizeof(VIRTUAL_COPY_EX_DATA), NULL, 0,
                NULL, NULL
                );

        if (!rc)
            {
            DEBUGMSG(TRUE, (
                L"ERROR: MmMapIoSpace_Proxy failed allocate memory\r\n"
                ));
            VirtualFree(pAddress, 0, MEM_RELEASE);
            pAddress = NULL;
            goto cleanUp;
            }
        pAddress = (void*)((UINT)pAddress + (UINT)(PhysAddr.LowPart & (PAGE_SIZE - 1)));
        }

cleanUp:
    return pAddress;
}

//-----------------------------------------------------------------------------
void
MmUnmapIoSpace_Proxy(
	void *pAddress,
	UINT  count
	)
{
}


//-----------------------------------------------------------------------------

static struct {
    LPCWSTR cmd;
    BOOL (*pfnCommand)(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts);
} s_cmdTable[] = {
    { L"?", Help },
    { L"in8", InReg8 },
    { L"in16", InReg16 },
    { L"in32", InReg32 },
    { L"intwl", InRegTWL },
    { L"ini2c", InI2C },
    { L"out8", OutReg8 },
    { L"out16", OutReg16 },
    { L"out32", OutReg32 },
    { L"outtwl", OutRegTWL },
    { L"outi2c", OutI2C },
    { L"cpuidle", CpuIdle},
    { L"dump", Dump_RegisterGroup},
    { L"tscal", TouchScreenCalibrate},
    { L"rotate", ScreenRotate},
    { L"device", Device_xxx},
    { L"observe", Observe},
    { L"opm", OPMode},
    { L"setcontrast", SetContrast},
    { L"smartreflex", SmartReflex},
    { L"battery", Battery},
    { L"reboot", Reboot},
    { L"display", Display},
    { L"tvout", TvOut},
    { L"powerdomain", PowerDomain},
#if CACHEINFO_ENABLE
    { L"cacheinfo", ShowCacheInfo},
#endif
#if !BSP_DVI_NO_SWITCHING
    { L"dvi", DVIControl},
#endif
    { L"halprofile", HalProfile},
    { L"interruptlatency", InterruptLatency},
    { L"neonStat", GetNEONStat },    
};

//-----------------------------------------------------------------------------

BOOL
ParseCommand(
    LPWSTR cmd,
    LPWSTR cmdLine,
    PFN_FmtPuts pfnFmtPuts,
    PFN_Gets pfnGets
    )
{
    BOOL rc = FALSE;
    LPWSTR argv[64];
    int argc = 64;
    ULONG ix;

    // Look for command
    for (ix = 0; ix < dimof(s_cmdTable); ix++)
        {
        if (wcscmp(cmd, s_cmdTable[ix].cmd) == 0) break;
        }
    if (ix >= dimof(s_cmdTable)) goto cleanUp;

    // Divide command line to token
    if (cmdLine != NULL)
        {
        CommandLineToArgs(cmdLine, &argc, argv);
        }
    else
        {
        argc = 0;
        }

    // Call command
    pfnFmtPuts(L"\r\n");
    __try
        {
        if (!s_cmdTable[ix].pfnCommand(argc, argv, pfnFmtPuts))
            {
            pfnFmtPuts(L"\r\n");
            Help(0, NULL, pfnFmtPuts);
            }
        }
    __except (EXCEPTION_EXECUTE_HANDLER)
        {
        pfnFmtPuts(L"exception: please check addresses and values\r\n");
        }
    pfnFmtPuts(L"\r\n");

    rc = TRUE;

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
Help(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    pfnFmtPuts(L"  in8    address [size] -> read 8-bit registers\r\n");
    pfnFmtPuts(L"  in16   address [size] -> read 16-bit registers\r\n");
    pfnFmtPuts(L"  in32   address [size] -> read 32-bit registers\r\n");
    pfnFmtPuts(L"  out8   address value  -> write 8-bit register\r\n");
    pfnFmtPuts(L"  out16  address value  -> write 16-bit register\r\n");
    pfnFmtPuts(L"  out32  address value  -> write 32-bit register\r\n");
    pfnFmtPuts(L"  intwl  address subaddress [size] -> read TWL registers\r\n");
    pfnFmtPuts(L"  outtwl address subaddress value  -> write TWL register\r\n");
    pfnFmtPuts(L"  ini2c  [1,2,3] address subaddress [size] -> read TWL registers\r\n");
    pfnFmtPuts(L"  outi2c [1,2,3] address subaddress value  -> write TWL register\r\n");
    pfnFmtPuts(L"  cpuidle -> display amount of time spent in OEMIdle\r\n");
    pfnFmtPuts(L"  dump [prcm | sdrc | gpmc | dss | contextrestore | efuse | prcmstate] -> output values in register group\r\n");
    pfnFmtPuts(L"  tscal -> calibrate touchscreen\r\n");
    pfnFmtPuts(L"  rotate [0,90,180,270] -> rotates screen to next or given angle\r\n");
    pfnFmtPuts(L"  device [[enable | disable] peripheral] | [status]] | [source peripheral clockId]\r\n");
    pfnFmtPuts(L"  observe [signal] [enable | disable]\r\n");
    pfnFmtPuts(L"  setcontrast [level] -> changes the constrast level (0 to 6)\r\n");
    pfnFmtPuts(L"  opm [mode | ?] -> changes the operating mode (0 to 6)\r\n");
    pfnFmtPuts(L"  smartreflex [enable | disable] -> enables disables smartreflex monitoring\r\n");
    pfnFmtPuts(L"  battery [charge [usb | ac] | disable] -> enables disables battery charging\r\n");
    pfnFmtPuts(L"  reboot -> software reset of device\r\n");
    pfnFmtPuts(L"  display [on|off] -> controls display parameters\r\n");
    pfnFmtPuts(L"  tvout [on|off|settings|aspect|scale|offset] -> controls TV out parameters\r\n");
#if !BSP_DVI_NO_SWITCHING
    pfnFmtPuts(L"  dvi [on|off] -> controls DVI\r\n");
#endif
#if CACHEINFO_ENABLE
    pfnFmtPuts(L"  cacheinfo -> show CPU cache info\r\n");
#endif
    pfnFmtPuts(L"  powerdomain [core/mpu/iva2/neon/usbhost/dss/per/camera/sgx] [ D0 - D4 / -r ] -> Set power domain contraint\r\n");
    pfnFmtPuts(L"  halprofile [dvfs | interruptlatency | sdrcstall | wakeupaccuracy] -> profiles latency\r\n");
    pfnFmtPuts(L"  interruptlatency [time] -> applies an interrupt latency constraint\r\n");
    pfnFmtPuts(L"  neonStat [clear] -> Get NEON Statistics. If clear is specified, statistics are cleared after retrieving them\r\n");
    pfnFmtPuts(L"\r\n");
    pfnFmtPuts(L"    Address, size and value are hex numbers.\r\n");
    return TRUE;
}

//-----------------------------------------------------------------------------

BOOL
GetNEONStat(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = TRUE;
    int i = 0;
    IOCTL_HAL_GET_NEON_STAT_S neonStat;

    if ((argc>0) && (wcscmp(args[0], L"clear") == 0))
    {
	KernelIoControl(IOCTL_HAL_GET_NEON_STATS, 
                     (LPVOID)args[0], 
                     sizeof(args[0]), 
                     (LPVOID)&neonStat, 
                     sizeof(IOCTL_HAL_GET_NEON_STAT_S), 
                     NULL);

    }
    else 
    {    
	KernelIoControl(IOCTL_HAL_GET_NEON_STATS, 
                     (LPVOID)NULL, 
                     0, 
                     (LPVOID)&neonStat, 
                     sizeof(IOCTL_HAL_GET_NEON_STAT_S), 
                     NULL);
    }

    pfnFmtPuts(L"NEON STATS:\r\n");
    pfnFmtPuts(L" Correct Restore                   = %d\r\n", neonStat.dwNumRestore);        
    pfnFmtPuts(L" Saved using pArea                 = %d\r\n", neonStat.dwNumCorrectSave + neonStat.dwErrCond4);
    pfnFmtPuts(L"   Absolute Correct Save           = %d\r\n", neonStat.dwNumCorrectSave);
    pfnFmtPuts(L"   Last Restore doesnt match       = %d\r\n", neonStat.dwErrCond4);    
    pfnFmtPuts(L" Saved using Error Correction      = %d\r\n", neonStat.dwErrCond1+neonStat.dwErrCond2);
    pfnFmtPuts(L"   Back-to-Back Thread Switches    = %d\r\n", neonStat.dwErrCond1);
    pfnFmtPuts(L"   Multiple Thread Switches        = %d\r\n", neonStat.dwErrCond2);
    pfnFmtPuts(L" Save Skipped                      = %d\r\n", neonStat.dwErrCond3);       

    
    
cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
InReg8(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    UINT32 address, count;
    PHYSICAL_ADDRESS pa;
    UINT8 *pAddress = NULL;
    WCHAR line[80];
    UINT8 data;
    UINT32 ix, ip;

    if (argc < 1)
        {
        pfnFmtPuts(L"Missing address!\r\n");
        goto cleanUp;
        }

    address = GetHex(args[0]);
    if (argc > 1)
        {
        count = GetHex(args[1]);
        }
    else
        {
        count = 1;
        }

    pa.QuadPart = address;
    pAddress = (UINT8*)MmMapIoSpace_Proxy(pa, count * sizeof(UINT8), FALSE);
    if (pAddress == NULL)
        {
        pfnFmtPuts(
            L"Failed map physical address 0x%08x to virtual address!\r\n",
            address
            );
        goto cleanUp;
        }

    for (ix = 0, ip = 0; ix < count; ix++)
        {
        data = INREG8(&pAddress[ix]);
        if ((ix & 0x0F) == 0)
            {
            StringCchPrintf(
                &line[ip], dimof(line) - ip, L"%08x:", address + ix
                );
            ip += lstrlen(&line[ip]);
            }
        StringCchPrintf(
            &line[ip], dimof(line) - ip, L" %02x", data
            );
        ip += lstrlen(&line[ip]);
        if ((ix & 0x0F) == 0x0F)
            {
            pfnFmtPuts(line);
            pfnFmtPuts(L"\r\n");
            ip = 0;
            }
        }
    if (ip > 0)
        {
        pfnFmtPuts(line);
        pfnFmtPuts(L"\r\n");
        }

    rc = TRUE;

cleanUp:
    if (pAddress != NULL) MmUnmapIoSpace_Proxy(pAddress, count * sizeof(UINT8));
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
InReg16(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    UINT32 address, count;
    PHYSICAL_ADDRESS pa;
    UINT8 *pAddress = NULL;
    WCHAR line[80];
    UINT16 data;
    UINT32 ix, ip;

    if (argc < 1)
        {
        pfnFmtPuts(L"Missing address!\r\n");
        goto cleanUp;
        }

    address = GetHex(args[0]);
    if (argc > 1)
        {
        count = GetHex(args[1]);
        }
    else
        {
        count = 1;
        }

    pa.QuadPart = address;
    pAddress = (UINT8*)MmMapIoSpace_Proxy(pa, count * sizeof(UINT16), FALSE);
    if (pAddress == NULL)
        {
        pfnFmtPuts(
            L"Failed map physical address 0x%08x to virtual address!\r\n",
            address
            );
        goto cleanUp;
        }

    for (ix = 0, ip = 0; ix < count; ix += sizeof(UINT16))
        {
        data = INREG16((UINT32)pAddress + ix);
        if ((ix & 0x0F) == 0)
            {
            StringCchPrintf(
                &line[ip], dimof(line) - ip, L"%08x:", address + ix
                );
            ip += lstrlen(&line[ip]);
            }
        StringCchPrintf(
            &line[ip], dimof(line) - ip, L" %04x", data
            );
        ip += lstrlen(&line[ip]);
        if (((ix + sizeof(UINT16)) & 0x0F) == 0)
            {
            pfnFmtPuts(line);
            pfnFmtPuts(L"\r\n");
            ip = 0;
            }
        }
    if (ip > 0)
        {
        pfnFmtPuts(line);
        pfnFmtPuts(L"\r\n");
        }

    rc = TRUE;

cleanUp:
    if (pAddress != NULL) MmUnmapIoSpace_Proxy(pAddress, count * sizeof(UINT16));
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
InReg32(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    UINT32 address, count;
    PHYSICAL_ADDRESS pa;
    UINT8 *pAddress = NULL;
    WCHAR line[80];
    UINT32 data;
    UINT32 ix, ip;

    if (argc < 1)
        {
        pfnFmtPuts(L"Missing address!\r\n");
        goto cleanUp;
        }

    address = GetHex(args[0]);
    if (argc > 1)
        {
        count = GetHex(args[1]);
        }
    else
        {
        count = 1;
        }

    pa.QuadPart = address;
    pAddress = (UINT8*)MmMapIoSpace_Proxy(pa, count * sizeof(UINT32), FALSE);
    if (pAddress == NULL)
        {
        pfnFmtPuts(
            L"Failed map physical address 0x%08x to virtual address!\r\n",
            address
            );
        goto cleanUp;
        }

    for (ix = 0, ip = 0; ix < count; ix += sizeof(UINT32))
        {
        data = INREG32((UINT32)pAddress + ix);
        if ((ix & 0x0F) == 0)
            {
            StringCchPrintf(
                &line[ip], dimof(line) - ip, L"%08x:", address + ix
                );
            ip += lstrlen(&line[ip]);
            }
        StringCchPrintf(
            &line[ip], dimof(line) - ip, L" %08x", data
            );
        ip += lstrlen(&line[ip]);
        if (((ix + sizeof(UINT32)) & 0x0F) == 0)
            {
            pfnFmtPuts(line);
            pfnFmtPuts(L"\r\n");
            ip = 0;
            }
        }
    if (ip > 0)
        {
        pfnFmtPuts(line);
        pfnFmtPuts(L"\r\n");
        }

    rc = TRUE;

cleanUp:
    if (pAddress != NULL) MmUnmapIoSpace_Proxy(pAddress, count * sizeof(UINT32));
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
OutReg8(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    UINT32 address, value;
    PHYSICAL_ADDRESS pa;
    UINT8 *pAddress = NULL;

    if (argc < 2)
        {
        pfnFmtPuts(L"Address and value required!\r\n");
        goto cleanUp;
        }

    address = GetHex(args[0]);
    value = GetHex(args[1]);

    if (value > 0x0100)
        {
        pfnFmtPuts(L"Value must be in 0x00 to 0xFF range!\r\n");
        goto cleanUp;
        }

    pa.QuadPart = address;
    pAddress = (UINT8*)MmMapIoSpace_Proxy(pa, sizeof(UINT8), FALSE);
    if (pAddress == NULL)
        {
        pfnFmtPuts(
            L"Failed map physical address 0x%08x to virtual address!\r\n",
            address
            );
        goto cleanUp;
        }

    OUTREG8(pAddress, value);
    pfnFmtPuts(L"Done, read back: 0x%02x\r\n", INREG8(pAddress));

    rc = TRUE;

cleanUp:
    if (pAddress != NULL) MmUnmapIoSpace_Proxy(pAddress, sizeof(UINT8));
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
OutReg16(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    UINT32 address, value;
    PHYSICAL_ADDRESS pa;
    UINT16 *pAddress = NULL;

    if (argc < 2)
        {
        pfnFmtPuts(L"Address and value required!\r\n");
        goto cleanUp;
        }

    address = GetHex(args[0]);
    value = GetHex(args[1]);

    if (value > 0x00010000)
        {
        pfnFmtPuts(L"Value must be in 0x0000 to 0xFFFF range!\r\n");
        goto cleanUp;
        }

    pa.QuadPart = address;
    pAddress = (UINT16*)MmMapIoSpace_Proxy(pa, sizeof(UINT16), FALSE);
    if (pAddress == NULL)
        {
        pfnFmtPuts(
            L"Failed map physical address 0x%08x to virtual address!\r\n",
            address
            );
        goto cleanUp;
        }

    OUTREG16(pAddress, value);
    pfnFmtPuts(L"Done, read back: 0x%04x\r\n", INREG16(pAddress));

    rc = TRUE;

cleanUp:
    if (pAddress != NULL) MmUnmapIoSpace_Proxy(pAddress, sizeof(UINT16));
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
OutReg32(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    UINT32 address, value;
    PHYSICAL_ADDRESS pa;
    UINT32 *pAddress = NULL;

    if (argc < 2)
        {
        pfnFmtPuts(L"Address and value required!\r\n");
        goto cleanUp;
        }

    address = GetHex(args[0]);
    value = GetHex(args[1]);

    pa.QuadPart = address;
    pAddress = (UINT32*)MmMapIoSpace_Proxy(pa, sizeof(UINT32), FALSE);
    if (pAddress == NULL)
        {
        pfnFmtPuts(
            L"Failed map physical address 0x%08x to virtual address!\r\n",
            address
            );
        goto cleanUp;
        }

    OUTREG32(pAddress, value);
    pfnFmtPuts(L"Done, read back: 0x%08x\r\n", INREG32(pAddress));

    rc = TRUE;

cleanUp:
    if (pAddress != NULL) MmUnmapIoSpace_Proxy(pAddress, sizeof(UINT32));
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
InRegTWL(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    UINT32 address, offset, count;
    UCHAR buffer[MESSAGE_BUFFER_SIZE];
    WCHAR line[80];
    ULONG ix, ip;

    if (argc < 2)
        {
        pfnFmtPuts(L"Missing address and offset to read from!\r\n");
        goto cleanUp;
        }

    address = GetHex(args[0]);
    switch (address)
        {
        case 0x12:
            break;

        case 0x48:
            break;

        case 0x49:
            break;

        case 0x4A:
            break;

        case 0x4B:
            break;

        default:
            pfnFmtPuts(L"Address must be 0x12, 0x48, 0x49, 0x4A, or 0x4B!\r\n");
            goto cleanUp;
        }

    offset = GetHex(args[1]);
    if (offset >= 0x100)
        {
        pfnFmtPuts(L"Read can't cross page boundary!\r\n");
        goto cleanUp;
        }

    if (argc > 2)
        {
        count = GetHex(args[2]);
        }
    else
        {
        count = 1;
        }
    if (count == 0) count = 1;


    address = (address << 16) | offset;
    rc = ReadTWLRegs(GetTritonHandle(), address, buffer, count);
    if (!rc)
        {
        pfnFmtPuts(L"Can't read data from TWL1: device driver!\r\n");
        goto cleanUp;
        }

    PREFAST_SUPPRESS(12008, "No offerflow/underflow possible there.");
    for (ix = 0, ip = 0; ix < count; ix++)
        {
        if ((ix & 0x0F) == 0)
            {
            StringCchPrintf(
                &line[ip], dimof(line) - ip, L"%04x:", offset + ix
                );
            ip += lstrlen(&line[ip]);
            }
        StringCchPrintf(
            &line[ip], dimof(line) - ip, L" %02x", buffer[ix]
            );
        ip += lstrlen(&line[ip]);
        if ((ix & 0x0F) == 0x0F)
            {
            pfnFmtPuts(line);
            pfnFmtPuts(L"\r\n");
            ip = 0;
            }
        }
    if (ip > 0)
        {
        pfnFmtPuts(line);
        pfnFmtPuts(L"\r\n");
        }

    rc = TRUE;

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
OutRegTWL(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    UINT32 address, offset, data;

    if (argc < 3)
        {
        pfnFmtPuts(L"Address to write, offset and/or data required\r\n");
        goto cleanUp;
        }

    address = GetHex(args[0]);
    switch (address)
        {
        case 0x12:
            break;

        case 0x48:
            break;

        case 0x49:
            break;

        case 0x4A:
            break;

        case 0x4B:
            break;

        default:
            pfnFmtPuts(L"Address must be 0x12, 0x48, 0x49, 0x4A, or 0x4B!\r\n");
            goto cleanUp;
        }

    offset = GetHex(args[1]);
    if (offset >= 0x100)
        {
        pfnFmtPuts(L"Write can't cross page boundary!\r\n");
        goto cleanUp;
        }

    data = GetHex(args[2]);

    address = (address << 16) | offset;

    pfnFmtPuts(L"writing 0x%02X to 0x%08X\r\n", data, address);
    rc = WriteTWLRegs(GetTritonHandle(), address, &data, 1);
    if (!rc)
        {
        pfnFmtPuts(L"Can't write data to TWL1: device driver!\r\n");
        goto cleanUp;
        }

    //rc = ReadTWLRegs(hTWL, address, &data, 1);
    //pfnFmtPuts(L"Done, read back: 0x%02x\r\n", data);
    rc = TRUE;

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------

BOOL InI2C(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    TCHAR const szI2C1[] = _T("I2C1:");
    TCHAR const szI2C2[] = _T("I2C2:");
    TCHAR const szI2C3[] = _T("I2C3:");

    TCHAR const *szI2C;
    int i2cIndex;
    UINT32 address, offset, count;
    UINT32 mode = 1;
    DWORD readCount;
    HANDLE hI2C = INVALID_HANDLE_VALUE;
    UCHAR buffer[MESSAGE_BUFFER_SIZE];
    WCHAR line[80];
    ULONG ix, ip;

    if (argc < 3)
        {
        pfnFmtPuts(L"Missing i2c id, address, and/or offset to read from!\r\n");
        goto cleanUp;
        }

    i2cIndex = GetHex(args[0]);
    switch (i2cIndex)
        {
        case 1:
            szI2C = szI2C1;
            break;

        case 2:
            szI2C = szI2C2;
            break;

        case 3:
            szI2C = szI2C3;
            break;

        default:
            pfnFmtPuts(L"Invalid i2c identifier, must be 1,2,3!\r\n");
            goto cleanUp;
        }

    address = GetHex(args[1]);

    offset =  GetHex(args[2]);
    if (offset >= 0x100)
        {
        //  Special values for address mode setting:
        //
        //      0x1xx   Mode 0  Offset = xx
        //      0x2xx   Mode 8  Offset = xx
        //      0x3xx   Mode 16 Offset = xx
        //      0x4xx   Mode 24 Offset = xx
        //      0x5xx   Mode 32 Offset = xx

        if( offset > 0x504 )
        {
            pfnFmtPuts(L"Read can't cross page boundary!\r\n");
            goto cleanUp;
        }

        mode   = (offset >> 8) - 1;
        offset = offset & 0x000000FF;
        }

    if (argc > 3)
        {
        count = GetHex(args[3]);
        }
    else
        {
        count = 1;
        }
    if (count == 0) count = 1;

    hI2C = CreateFile(
        szI2C, GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL
        );
    if (hI2C == INVALID_HANDLE_VALUE)
        {
        pfnFmtPuts(L"Can't open %s: device driver!\r\n", szI2C);
        goto cleanUp;
        }


    // set slave address
    if (!SetSlaveAddress(hI2C, address, mode))
        {
        pfnFmtPuts(L"Failed set I2C slave address\r\n");
        goto cleanUp;
        }

    // read data
    SetFilePointer(hI2C, offset, NULL, FILE_BEGIN);
    if (!ReadFile(hI2C, buffer, count, &readCount, NULL))
        {
        pfnFmtPuts(L"Failed reading value(s)\r\n");
        goto cleanUp;
        }


    pfnFmtPuts(L"Read %d of %d byte(s)\r\n", count, readCount);
    PREFAST_SUPPRESS(12008, "No offerflow/underflow possible there.");
    for (ix = 0, ip = 0; ix < readCount; ix++)
        {
        if ((ix & 0x0F) == 0)
            {
            StringCchPrintf(
                &line[ip], dimof(line) - ip, L"%04x:", offset + ix
                );
            ip += lstrlen(&line[ip]);
            }
        StringCchPrintf(
            &line[ip], dimof(line) - ip, L" %02x", buffer[ix]
            );
        ip += lstrlen(&line[ip]);
        if ((ix & 0x0F) == 0x0F)
            {
            pfnFmtPuts(line);
            pfnFmtPuts(L"\r\n");
            ip = 0;
            }
        }
    if (ip > 0)
        {
        pfnFmtPuts(line);
        pfnFmtPuts(L"\r\n");
        }

    rc = TRUE;

cleanUp:
    if (hI2C != INVALID_HANDLE_VALUE) CloseHandle(hI2C);
    return rc;
}

//-----------------------------------------------------------------------------

BOOL OutI2C(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    TCHAR const szI2C1[] = _T("I2C1:");
    TCHAR const szI2C2[] = _T("I2C2:");
    TCHAR const szI2C3[] = _T("I2C3:");

    TCHAR const *szI2C;
    int i2cIndex;
    UINT32 address, offset, data;
    UINT32 mode = 1;
    DWORD readCount;
    HANDLE hI2C = INVALID_HANDLE_VALUE;

    if (argc < 3)
        {
        pfnFmtPuts(L"Missing i2c id, address, and/or offset to read from!\r\n");
        goto cleanUp;
        }

    i2cIndex = GetHex(args[0]);
    switch (i2cIndex)
        {
        case 1:
            szI2C = szI2C1;
            break;

        case 2:
            szI2C = szI2C2;
            break;

        case 3:
            szI2C = szI2C3;
            break;

        default:
            pfnFmtPuts(L"Invalid i2c identifier, must be 1 or 2!\r\n");
            goto cleanUp;
        }

    address = GetHex(args[1]);

    offset =  GetHex(args[2]);
    if (offset >= 0x100)
        {
        //  Special values for address mode setting:
        //
        //      0x1xx   Mode 0  Offset = xx
        //      0x2xx   Mode 8  Offset = xx
        //      0x3xx   Mode 16 Offset = xx
        //      0x4xx   Mode 24 Offset = xx
        //      0x5xx   Mode 32 Offset = xx

        if( offset > 0x504 )
        {
            pfnFmtPuts(L"Read can't cross page boundary!\r\n");
            goto cleanUp;
        }

        mode   = (offset >> 8) - 1;
        offset = offset & 0x000000FF;
        }

    data = GetHex(args[3]);

    hI2C = CreateFile(
        szI2C, GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL
        );
    if (hI2C == INVALID_HANDLE_VALUE)
        {
        pfnFmtPuts(L"Can't open %s: device driver!\r\n", szI2C);
        goto cleanUp;
        }

    // set slave address
    if (!SetSlaveAddress(hI2C, address, mode))
        {
        pfnFmtPuts(L"Failed set I2C slave address\r\n");
        goto cleanUp;
        }

    // write data
    if (!WriteI2C(hI2C, offset, &data, 1, &readCount))
        {
        pfnFmtPuts(L"Failed writing value(s)\r\n");
        goto cleanUp;
        }

    // verify data
    ReadI2C(hI2C, offset, &data, 1, &readCount);
    pfnFmtPuts(L"Done, read back value is: 0x%02x\r\n", data);
    rc = TRUE;

cleanUp:
    if (hI2C != INVALID_HANDLE_VALUE) CloseHandle(hI2C);
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
Dump_RegisterGroup(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    int i = 0;
    while (argc--)
        {
        if (wcscmp(args[i], L"sdrc") == 0)
            {
            Dump_SDRC(pfnFmtPuts);
            }
        else if (wcscmp(args[i], L"gpmc") == 0)
            {
            Dump_GPMC(pfnFmtPuts);
            }
        else if (wcscmp(args[i], L"prcm") == 0)
            {
            Dump_PRCM(pfnFmtPuts);
            }
        else if (wcscmp(args[i], L"dss") == 0)
            {
            Dump_DSS(pfnFmtPuts);
            }
        else if (wcscmp(args[i], L"contextrestore") == 0)
            {
            Dump_ContextRestore(pfnFmtPuts);
            }
        else if (wcscmp(args[i], L"efuse") == 0)
            {
            Dump_EFuse(pfnFmtPuts);
            }
        else if (wcscmp(args[i], L"prcmstate") == 0)
            {
            Dump_PRCMStates(pfnFmtPuts);
            }
        else
            {
            pfnFmtPuts(L"undefined registers(%s)\r\n", args[i]);
            }
        i++;
        }

    return TRUE;
}


//-----------------------------------------------------------------------------

BOOL
TouchScreenCalibrate(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    TouchCalibrate();
    return TRUE;
}

//-----------------------------------------------------------------------------

BOOL 
ScreenRotate(
    ULONG argc, 
    LPWSTR args[], 
    PFN_FmtPuts pfnFmtPuts
    )
{
    LONG    result;
    DEVMODE devMode;
    DWORD   angle;

    //  Get current rotation angle
    devMode.dmSize = sizeof(devMode);
    devMode.dmFields = DM_DISPLAYORIENTATION;
    devMode.dmDisplayOrientation = DMDO_0;
    result = ChangeDisplaySettingsEx( NULL, &devMode, NULL, CDS_TEST, NULL);
    
    //  See if new angle requested
    if( argc > 0 )
    {
        angle = GetDec( args[0] );
        
        switch( angle )
        {
            case 0:
                devMode.dmDisplayOrientation = DMDO_0;
                break;

            case 90:
                devMode.dmDisplayOrientation = DMDO_90;
                break;

            case 180:
                devMode.dmDisplayOrientation = DMDO_180;
                break;

            case 270:
                devMode.dmDisplayOrientation = DMDO_270;
                break;
        }
    }
    else
    {  
        //  Use next rotation angle
        switch( devMode.dmDisplayOrientation )
        {
            case DMDO_0:
                devMode.dmDisplayOrientation = DMDO_90;
                break;

            case DMDO_90:
                devMode.dmDisplayOrientation = DMDO_180;
                break;

            case DMDO_180:
                devMode.dmDisplayOrientation = DMDO_270;
                break;

            case DMDO_270:
                devMode.dmDisplayOrientation = DMDO_0;
                break;
        }
    }

    //  Change the screen rotation
    devMode.dmSize = sizeof(devMode);
    devMode.dmFields = DM_DISPLAYORIENTATION;
    result = ChangeDisplaySettingsEx( NULL, &devMode, NULL, CDS_RESET, NULL);
    
    return TRUE;
}


//-----------------------------------------------------------------------------

LONG
GetDec(
    LPCWSTR string
    )
{
    LONG result;
    UINT32 ix = 0;
    BOOL bNegative = FALSE;

    result = 0;
    while (string != NULL) 
        {
        if ((string[ix] >= L'0') && (string[ix] <= L'9'))
            {
            result = (result * 10) + (string[ix] - L'0');
            ix++;
            }
        else if( string[ix] == L'-')
            {
            bNegative = TRUE;
            ix++;
            }
        else
            {
            break;
            }
        }
    return (bNegative) ? -result : result;
}   

//-----------------------------------------------------------------------------

UINT32
GetHex(
    LPCWSTR string
    )
{
    UINT32 result;
    UINT32 ix = 0;

    result = 0;
    while (string != NULL)
        {
        if ((string[ix] >= L'0') && (string[ix] <= L'9'))
            {
            result = (result << 4) + (string[ix] - L'0');
            ix++;
            }
       else if ((string[ix] >= L'a') && (string[ix] <= L'f'))
            {
            result = (result << 4) + (string[ix] - L'a' + 10);
            ix++;
            }
       else if (string[ix] >= L'A' && string[ix] <= L'F')
            {
            result = (result << 4) + (string[ix] - L'A' + 10);
            ix++;
            }
        else
            {
            break;
            }
        }
    return result;
}

//-----------------------------------------------------------------------------

static BOOL
WriteTWLRegs(
    HANDLE hTWL,
    DWORD address,
    VOID *pBuffer,
    DWORD size
    )
{
    BOOL rc = FALSE;
    IOCTL_TWL_WRITEREGS_IN In;
    IOCTL_TWL_WRITEREGS_IN *pIn = &In;
    DWORD inSize = sizeof(IOCTL_TWL_WRITEREGS_IN) + size;

    if (size > 256) goto cleanUp;
    pIn->address = address;
    pIn->size = size;
    pIn->pBuffer = pBuffer;

    rc = DeviceIoControl(
        hTWL, IOCTL_TWL_WRITEREGS, pIn, inSize, NULL, 0, NULL, NULL
        );

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------

static BOOL
ReadTWLRegs(
    HANDLE hTWL,
    DWORD address,
    VOID *pBuffer,
    DWORD size
    )
{
    BOOL rc = FALSE;
    IOCTL_TWL_READREGS_IN in;

    if (size > 256) goto cleanUp;
    in.address = address;
    in.size = size;
    rc = DeviceIoControl(
        hTWL, IOCTL_TWL_READREGS, &in, sizeof(in), pBuffer, size, NULL, NULL
        );

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
SetSlaveAddress(
    HANDLE hI2C, DWORD address, DWORD mode
    )
{
    BOOL rc;

    rc = DeviceIoControl(
        hI2C, IOCTL_I2C_SET_SLAVE_ADDRESS, &address, sizeof(address), NULL, 0,
        NULL, NULL
        );

    rc = DeviceIoControl(
        hI2C, IOCTL_I2C_SET_SUBADDRESS_MODE, &mode, sizeof(mode), NULL, 0,
        NULL, NULL
        );

    return rc;
}

//-----------------------------------------------------------------------------

DWORD
WriteI2C(
    HANDLE hI2C,
    UINT8  subaddr,
    VOID*  pBuffer,
    DWORD  count,
    DWORD *pWritten
    )
{
    SetFilePointer(hI2C, subaddr, NULL, FILE_BEGIN);
    return WriteFile(hI2C, pBuffer, count, pWritten, NULL);
}

//-----------------------------------------------------------------------------

DWORD
ReadI2C(
    HANDLE hI2C,
    UINT8  subaddr,
    VOID*  pBuffer,
    DWORD  count,
    DWORD *pRead
    )
{
    SetFilePointer(hI2C, subaddr, NULL, FILE_BEGIN);
    return ReadFile(hI2C, pBuffer, count, pRead, NULL);
}

//-----------------------------------------------------------------------------

BOOL CpuIdle(ULONG argc, LPWSTR args[], PFN_FmtPuts pfnFmtPuts)
{
    static _idleLast = 0;
    static _tickLast = 0;
    DWORD idle;
    DWORD tick;
    WCHAR szBuffer[MESSAGE_BUFFER_SIZE];

    idle = GetIdleTime();
    tick = GetTickCount();

    StringCchPrintf(szBuffer, MESSAGE_BUFFER_SIZE, L"idle count=0x%08X, tick count=0x%08X\r\n", idle, tick);
    pfnFmtPuts(szBuffer);

    StringCchPrintf(szBuffer, MESSAGE_BUFFER_SIZE, L"The difference from last check is...\r\n", idle, tick);
    pfnFmtPuts(szBuffer);

    StringCchPrintf(szBuffer, MESSAGE_BUFFER_SIZE, L"idle delta=0x%08X, tick delta=0x%08X\r\n", idle - _idleLast, tick - _tickLast);
    pfnFmtPuts(szBuffer);

    StringCchPrintf(szBuffer, MESSAGE_BUFFER_SIZE, L"cpu load is %f%%\r\n", (1.0f - ((float)(idle - _idleLast)/(float)(tick - _tickLast))) * 100.0f);
    pfnFmtPuts(szBuffer);

    _idleLast = idle;
    _tickLast = tick;
    return TRUE;
}

//-----------------------------------------------------------------------------

DWORD DumpCurrentFrequencies(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    DWORD rc = -1;
    DWORD val;
    DWORD val2;
    float sys_clk;
    float mpu_freq;
    float core_freq;
    float iva_freq;
    PHYSICAL_ADDRESS pa;
    _TCHAR szBuffer[MAX_PATH];

    // get virtual address mapping to register
    pa.QuadPart = 0x48306D40;
    val = INREG32((UINT32)MmMapIoSpace_Proxy(pa, sizeof(DWORD), FALSE));
    switch (val)
        {
        case 0:
            sys_clk = 12.0f;
            break;

        case 1:
            sys_clk = 13.0f;
            break;

        case 2:
            sys_clk = 19.2f;
            break;

        case 3:
            sys_clk = 26.0f;
            break;

        case 4:
            sys_clk = 38.4f;
            break;

        case 5:
            sys_clk = 16.8f;
            break;

        default:
            goto cleanUp;                
        }

    // get mpu frequency
    pa.QuadPart = 0x48004940;
    val = INREG32((UINT32)MmMapIoSpace_Proxy(pa, sizeof(DWORD), FALSE));
    pa.QuadPart = 0x48004944;
    val2 = INREG32((UINT32)MmMapIoSpace_Proxy(pa, sizeof(DWORD), FALSE));
    mpu_freq = ((float)((val & DPLL_MULT_MASK)>>DPLL_MULT_SHIFT) * sys_clk) /
                    ((float)((val & DPLL_DIV_MASK) >> DPLL_DIV_SHIFT) + 1.0f);
    mpu_freq = mpu_freq / ((float)(val2 >> DPLL_MPU_CLKOUT_DIV_SHIFT));

    // get core frequency
    pa.QuadPart = 0x48004D40;
    val = INREG32((UINT32)MmMapIoSpace_Proxy(pa, sizeof(DWORD), FALSE)) >> 8;
    core_freq = ((float)((val & DPLL_MULT_MASK)>>DPLL_MULT_SHIFT) * sys_clk) /
                    ((float)((val & DPLL_DIV_MASK) >> DPLL_DIV_SHIFT) + 1.0f);
    core_freq = core_freq / ((float)(val >> (DPLL_CORE_CLKOUT_DIV_SHIFT - 8)));
    
    // get iva frequency
    pa.QuadPart = 0x48004040;
    val = INREG32((UINT32)MmMapIoSpace_Proxy(pa, sizeof(DWORD), FALSE));
    pa.QuadPart = 0x48004044;
    val2 = INREG32((UINT32)MmMapIoSpace_Proxy(pa, sizeof(DWORD), FALSE));
    iva_freq = ((float)((val & DPLL_MULT_MASK)>>DPLL_MULT_SHIFT) * sys_clk) /
                    ((float)((val & DPLL_DIV_MASK) >> DPLL_DIV_SHIFT) + 1.0f);
    iva_freq = iva_freq / ((float)(val2 >> DPLL_IVA2_CLKOUT_DIV_SHIFT));
    StringCchPrintf(szBuffer, MAX_PATH,
        _T("Current Frequencies: MPU-%.2f, IVA-%.2f, CORE-%.2f\r\n"),
        mpu_freq, iva_freq, core_freq);

    pfnFmtPuts(szBuffer);

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
OPMode(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL bForce = FALSE;
    UINT opm = _wtoi(args[0]);

    if (args == 0) return FALSE;
    if (wcsicmp(args[0], L"?") == 0)
        {
        // dump OPP settings
        pfnFmtPuts(L"Operating Mode Choices\r\n");
        pfnFmtPuts(L" 6 - MPU[720Mhz @ 1.35V],  IVA2[520Mhz @ 1.35V],  CORE[332Mhz @ 1.15V]\r\n");
        pfnFmtPuts(L" 5 - MPU[600Mhz @ 1.35V],  IVA2[430Mhz @ 1.35V],  CORE[332Mhz @ 1.15V]\r\n");
        pfnFmtPuts(L" 4 - MPU[550Mhz @ 1.27V],  IVA2[400Mhz @ 1.27V],  CORE[332Mhz @ 1.15V]\r\n");
        pfnFmtPuts(L" 3 - MPU[500Mhz @ 1.20V],  IVA2[360Mhz @ 1.20V],  CORE[332Mhz @ 1.15V]\r\n");
        pfnFmtPuts(L" 2 - MPU[250Mhz @ 1.05V],  IVA2[180Mhz @ 1.05V],  CORE[332Mhz @ 1.15V]\r\n");
        pfnFmtPuts(L" 1 - MPU[125Mhz @ 0.975V], IVA2[ 90Mhz @ 0.975V], CORE[332Mhz @ 1.15V]\r\n");
        pfnFmtPuts(L" 0 - MPU[125Mhz @ 0.975V], IVA2[ 90Mhz @ 0.975V], CORE[166Mhz @ 1.05V]\r\n");

        DumpCurrentFrequencies(argc, args, pfnFmtPuts);
        return TRUE;
        }
    else
        {
        if ((wcsicmp(args[0], L"0") != 0) &&
            (wcsicmp(args[0], L"1") != 0) &&
            (wcsicmp(args[0], L"2") != 0) &&
            (wcsicmp(args[0], L"3") != 0) &&
            (wcsicmp(args[0], L"4") != 0) &&
            (wcsicmp(args[0], L"5") != 0) &&
            (wcsicmp(args[0], L"6") != 0))
            {
            return FALSE;
            }

        if (argc > 1)
            {
            if (wcsicmp(args[1], L"-f") == 0)
                {
                bForce = TRUE;
                }
            }

        DeviceIoControl(
            GetProxyDriverHandle(), 
            bForce ? IOCTL_DVFS_FORCE : IOCTL_DVFS_REQUEST, 
            (void*)&opm, 
            sizeof(DWORD), 
            NULL, 
            0, 
            NULL, 
            NULL
            );
        }

    return TRUE;
}

//------------------------------------------------------------------------------

BOOL
SmartReflex(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc;
    BOOL bEnable;
    WCHAR szBuffer[MESSAGE_BUFFER_SIZE];

    // parse command
    if (wcsicmp(args[0], L"enable") == 0)
        {
        bEnable = TRUE;
        }
    else if (wcsicmp(args[0], L"disable") == 0)
        {
        bEnable = FALSE;
        }
    else
        {
        StringCchPrintf(szBuffer, MESSAGE_BUFFER_SIZE, L"syntax error!!!\r\n");
        pfnFmtPuts(szBuffer);
        return FALSE;
        }

    // send notification to kernel
    rc = DeviceIoControl(
            GetProxyDriverHandle(), 
            IOCTL_SMARTREFLEX_CONTROL, 
            &bEnable, 
            sizeof(bEnable), 
            NULL, 
            0, 
            NULL, 
            NULL
            );

    // output response
    if (rc == FALSE)
        {
        StringCchPrintf(szBuffer, 
            MESSAGE_BUFFER_SIZE, 
            L"Requst failed\r\n"
            );
        }
    else
        {
        if (bEnable)
            {
            StringCchPrintf(szBuffer, 
                MESSAGE_BUFFER_SIZE, 
                L"Smartreflex monitoring enabled\r\n"
                );
            }
        else
            {
            StringCchPrintf(szBuffer, 
                MESSAGE_BUFFER_SIZE, 
                L"Smartreflex monitoring disabled\r\n"
                );
            }
        }
    pfnFmtPuts(szBuffer);

    return TRUE;
}

//-----------------------------------------------------------------------------

#define DISPLAY_REGISTER_VALUE(x, y)  StringCchPrintf(szBuffer, MESSAGE_BUFFER_SIZE, L#y); \
                                      pfnFmtPuts(szBuffer); \
                                      StringCchPrintf(szBuffer, MESSAGE_BUFFER_SIZE, L"=0x%08X\r\n", INREG32(&x->y)); \
                                      pfnFmtPuts(szBuffer);

#define DISPLAY_REGISTER_VALUE16(x, y)  StringCchPrintf(szBuffer, MESSAGE_BUFFER_SIZE, L#y); \
                                      pfnFmtPuts(szBuffer); \
                                      StringCchPrintf(szBuffer, MESSAGE_BUFFER_SIZE, L"=0x%04X\r\n", INREG16(&x->y)); \
                                      pfnFmtPuts(szBuffer);


void
Dump_GPMC(
    PFN_FmtPuts pfnFmtPuts
    )
{
    OMAP_GPMC_REGS *pRegs;
    PHYSICAL_ADDRESS pa;
    WCHAR szBuffer[MESSAGE_BUFFER_SIZE];

    pa.QuadPart = OMAP_GPMC_REGS_PA;
    pRegs = (OMAP_GPMC_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_GPMC_REGS), FALSE);

    DISPLAY_REGISTER_VALUE(pRegs, GPMC_REVISION);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_SYSCONFIG);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_SYSSTATUS);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_IRQSTATUS);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_IRQENABLE);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_TIMEOUT_CONTROL);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ERR_ADDRESS);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ERR_TYPE);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_STATUS);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG1_0);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG2_0);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG3_0);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG4_0);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG5_0);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG6_0);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG7_0);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG1_1);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG2_1);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG3_1);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG4_1);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG5_1);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG6_1);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG7_1);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG1_2);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG2_2);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG3_2);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG4_2);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG5_2);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG6_2);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG7_2);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG1_3);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG2_3);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG3_3);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG4_3);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG5_3);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG6_3);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG7_3);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG1_4);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG2_4);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG3_4);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG4_4);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG5_4);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG6_4);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG7_4);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG1_5);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG2_5);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG3_5);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG4_5);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG5_5);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG6_5);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG7_5);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG1_6);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG2_6);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG3_6);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG4_6);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG5_6);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG6_6);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG7_6);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG1_7);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG2_7);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG3_7);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG4_7);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG5_7);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG6_7);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_CONFIG7_7);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_PREFETCH_CONFIG1);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_PREFETCH_CONFIG2);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_PREFETCH_CONTROL);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_PREFETCH_STATUS);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ECC_CONFIG);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ECC_CONTROL);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ECC_SIZE_CONFIG);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ECC1_RESULT);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ECC2_RESULT);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ECC3_RESULT);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ECC4_RESULT);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ECC5_RESULT);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ECC6_RESULT);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ECC7_RESULT);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ECC8_RESULT);
    DISPLAY_REGISTER_VALUE(pRegs, GPMC_ECC9_RESULT);
    //DISPLAY_REGISTER_VALUE(pRegs, GPMC_TESTMODE_CTRL);
    //DISPLAY_REGISTER_VALUE(pRegs, GPMC_PSA_LSB);
    //DISPLAY_REGISTER_VALUE(pRegs, GPMC_PSA_MSB);
}
    
void
Dump_SDRC(
    PFN_FmtPuts pfnFmtPuts
    )
{
    OMAP_SDRC_REGS *pRegs;
    PHYSICAL_ADDRESS pa;
    WCHAR szBuffer[MESSAGE_BUFFER_SIZE];

    pa.QuadPart = OMAP_SDRC_REGS_PA;
    pRegs = (OMAP_SDRC_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_SDRC_REGS), FALSE);

    DISPLAY_REGISTER_VALUE(pRegs, SDRC_REVISION);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_SYSCONFIG);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_STATUS);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_CS_CFG);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_SHARING);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_ERR_ADDR);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_ERR_TYPE);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_DLLA_CTRL);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_DLLA_STATUS);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_DLLB_CTRL);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_DLLB_STATUS);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_POWER);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_MCFG_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_MR_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_EMR1_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_EMR2_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_DCDL1_CTRL);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_DCDL2_CTRL);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_ACTIM_CTRLA_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_ACTIM_CTRLB_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_RFR_CTRL_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_MANUAL_0);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_MCFG_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_MR_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_EMR1_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_EMR2_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_ACTIM_CTRLA_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_ACTIM_CTRLB_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_RFR_CTRL_1);
    DISPLAY_REGISTER_VALUE(pRegs, SDRC_MANUAL_1);
}

void
Dump_PRCM(
    PFN_FmtPuts pfnFmtPuts
    )
{
    OMAP_PRCM_GLOBAL_PRM_REGS *pPrcmGlobalPrm;
    PHYSICAL_ADDRESS pa;
    WCHAR szBuffer[MESSAGE_BUFFER_SIZE];

    pa.QuadPart = OMAP_PRCM_GLOBAL_PRM_REGS_PA;
    pPrcmGlobalPrm = (OMAP_PRCM_GLOBAL_PRM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_GLOBAL_PRM_REGS), FALSE);

    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VC_SMPS_SA);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VC_SMPS_VOL_RA);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VC_SMPS_CMD_RA);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VC_CMD_VAL_0);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VC_CMD_VAL_1);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VC_CH_CONF);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VC_I2C_CFG);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VC_BYPASS_VAL);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_RSTCTRL);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_RSTTIME);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_RSTST);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VOLTCTRL);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_SRAM_PCHARGE);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_CLKSRC_CTRL);
    //DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_OBS);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VOLTSETUP1);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VOLTOFFSET);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_CLKSETUP);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_POLCTRL);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VOLTSETUP2);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VP1_CONFIG);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VP1_VSTEPMIN);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VP1_VSTEPMAX);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VP1_VLIMITTO);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VP1_VOLTAGE);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VP1_STATUS);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VP2_CONFIG);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VP2_VSTEPMIN);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VP2_VSTEPMAX);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VP2_VLIMITTO);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VP2_VOLTAGE);
    DISPLAY_REGISTER_VALUE(pPrcmGlobalPrm, PRM_VP2_STATUS);


}

void
Dump_DSS(
    PFN_FmtPuts pfnFmtPuts
    )
{
    OMAP_DSS_REGS   *pDssRegs;
    OMAP_DISPC_REGS *pDiscRegs;
    OMAP_VID_REGS   *pVidRegs;
    OMAP_VENC_REGS  *pVencRegs;
    PHYSICAL_ADDRESS pa;
    WCHAR szBuffer[MESSAGE_BUFFER_SIZE];
    DWORD i;


    pa.QuadPart = OMAP_DSS1_REGS_PA;
    pDssRegs = (OMAP_DSS_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_DSS_REGS), FALSE);

    pa.QuadPart = OMAP_DISC1_REGS_PA;
    pDiscRegs = (OMAP_DISPC_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_DISPC_REGS), FALSE);

    pa.QuadPart = OMAP_VENC1_REGS_PA;
    pVencRegs = (OMAP_VENC_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_VENC_REGS), FALSE);


    pfnFmtPuts(TEXT("\r\nDSS Registers:\r\n"));
    DISPLAY_REGISTER_VALUE(pDssRegs, DSS_REVISIONNUMBER);
    DISPLAY_REGISTER_VALUE(pDssRegs, DSS_SYSCONFIG);
    DISPLAY_REGISTER_VALUE(pDssRegs, DSS_SYSSTATUS);
    DISPLAY_REGISTER_VALUE(pDssRegs, DSS_CONTROL);
    DISPLAY_REGISTER_VALUE(pDssRegs, DSS_SDI_CONTROL);
    DISPLAY_REGISTER_VALUE(pDssRegs, DSS_PLL_CONTROL);
    DISPLAY_REGISTER_VALUE(pDssRegs, DSS_SDI_STATUS);

    pfnFmtPuts(TEXT("\r\nDISPC Registers:\r\n"));
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_REVISION);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_SYSCONFIG);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_SYSSTATUS);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_IRQSTATUS);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_IRQENABLE);

    pfnFmtPuts(TEXT("\r\nDISPC Registers (Control):\r\n"));
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_CONTROL);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_CONFIG);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_CAPABLE);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_DEFAULT_COLOR0);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_DEFAULT_COLOR1);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_TRANS_COLOR0);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_TRANS_COLOR1);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_GLOBAL_ALPHA);

    pfnFmtPuts(TEXT("\r\nDISPC Registers (LCD):\r\n"));
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_LINE_STATUS);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_LINE_NUMBER);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_TIMING_H);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_TIMING_V);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_POL_FREQ);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_DIVISOR);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_SIZE_DIG);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_SIZE_LCD);

    pfnFmtPuts(TEXT("\r\nDISPC Registers (GFX):\r\n"));
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_GFX_BA0);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_GFX_BA1);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_GFX_POSITION);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_GFX_SIZE);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_GFX_ATTRIBUTES);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_GFX_FIFO_THRESHOLD);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_GFX_FIFO_SIZE_STATUS);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_GFX_ROW_INC);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_GFX_PIXEL_INC);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_GFX_WINDOW_SKIP);
    DISPLAY_REGISTER_VALUE(pDiscRegs, DISPC_GFX_TABLE_BA);


    pfnFmtPuts(TEXT("\r\nDISPC Registers (VID 1):\r\n"));
    pVidRegs = &(pDiscRegs->tDISPC_VID1);

    DISPLAY_REGISTER_VALUE(pVidRegs, BA0);
    DISPLAY_REGISTER_VALUE(pVidRegs, BA1);
    DISPLAY_REGISTER_VALUE(pVidRegs, POSITION);
    DISPLAY_REGISTER_VALUE(pVidRegs, SIZE);
    DISPLAY_REGISTER_VALUE(pVidRegs, ATTRIBUTES);
    DISPLAY_REGISTER_VALUE(pVidRegs, FIFO_THRESHOLD);
    DISPLAY_REGISTER_VALUE(pVidRegs, FIFO_SIZE_STATUS);
    DISPLAY_REGISTER_VALUE(pVidRegs, ROW_INC);
    DISPLAY_REGISTER_VALUE(pVidRegs, PIXEL_INC);
    DISPLAY_REGISTER_VALUE(pVidRegs, FIR);
    DISPLAY_REGISTER_VALUE(pVidRegs, PICTURE_SIZE);
    DISPLAY_REGISTER_VALUE(pVidRegs, ACCU0);
    DISPLAY_REGISTER_VALUE(pVidRegs, ACCU1);

    pfnFmtPuts(TEXT("DISPC_VIDn_FIR_COEF_Hi _HVi _Vi:\r\n"));
    for( i = 0; i < 8; i++ )
    {
        StringCchPrintf(szBuffer, MESSAGE_BUFFER_SIZE,
        L"   %d: H = 0x%08x   HV = 0x%08x   V = 0x%08x\r\n",
        i, INREG32(&pVidRegs->aFIR_COEF[i].ulH), INREG32(&pVidRegs->aFIR_COEF[i].ulHV), INREG32(&pDiscRegs->DISPC_VID1_FIR_COEF_V[i]));
        pfnFmtPuts(szBuffer);
    }

    DISPLAY_REGISTER_VALUE(pVidRegs, CONV_COEF0);
    DISPLAY_REGISTER_VALUE(pVidRegs, CONV_COEF1);
    DISPLAY_REGISTER_VALUE(pVidRegs, CONV_COEF2);
    DISPLAY_REGISTER_VALUE(pVidRegs, CONV_COEF3);
    DISPLAY_REGISTER_VALUE(pVidRegs, CONV_COEF4);


    pfnFmtPuts(TEXT("\r\nDISPC Registers (VID 2):\r\n"));
    pVidRegs = &(pDiscRegs->tDISPC_VID2);

    DISPLAY_REGISTER_VALUE(pVidRegs, BA0);
    DISPLAY_REGISTER_VALUE(pVidRegs, BA1);
    DISPLAY_REGISTER_VALUE(pVidRegs, POSITION);
    DISPLAY_REGISTER_VALUE(pVidRegs, SIZE);
    DISPLAY_REGISTER_VALUE(pVidRegs, ATTRIBUTES);
    DISPLAY_REGISTER_VALUE(pVidRegs, FIFO_THRESHOLD);
    DISPLAY_REGISTER_VALUE(pVidRegs, FIFO_SIZE_STATUS);
    DISPLAY_REGISTER_VALUE(pVidRegs, ROW_INC);
    DISPLAY_REGISTER_VALUE(pVidRegs, PIXEL_INC);
    DISPLAY_REGISTER_VALUE(pVidRegs, FIR);
    DISPLAY_REGISTER_VALUE(pVidRegs, PICTURE_SIZE);
    DISPLAY_REGISTER_VALUE(pVidRegs, ACCU0);
    DISPLAY_REGISTER_VALUE(pVidRegs, ACCU1);

    pfnFmtPuts(TEXT("DISPC_VIDn_FIR_COEF_Hi _HVi _Vi:\r\n"));
    for( i = 0; i < 8; i++ )
    {
        StringCchPrintf(szBuffer, MESSAGE_BUFFER_SIZE,
        L"   %d: H = 0x%08x   HV = 0x%08x   V = 0x%08x\r\n",
        i, INREG32(&pVidRegs->aFIR_COEF[i].ulH), INREG32(&pVidRegs->aFIR_COEF[i].ulHV), INREG32(&pDiscRegs->DISPC_VID2_FIR_COEF_V[i]));
        pfnFmtPuts(szBuffer);
    }

    DISPLAY_REGISTER_VALUE(pVidRegs, CONV_COEF0);
    DISPLAY_REGISTER_VALUE(pVidRegs, CONV_COEF1);
    DISPLAY_REGISTER_VALUE(pVidRegs, CONV_COEF2);
    DISPLAY_REGISTER_VALUE(pVidRegs, CONV_COEF3);
    DISPLAY_REGISTER_VALUE(pVidRegs, CONV_COEF4);

/*
    pfnFmtPuts(TEXT("\r\nVENC Registers:\r\n"));
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_STATUS);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_F_CONTROL);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_VIDOUT_CTRL);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_SYNC_CTRL);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_LLEN);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_FLENS);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_HFLTR_CTRL);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_CC_CARR_WSS_CARR);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_C_PHASE);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_GAIN_U);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_GAIN_V);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_GAIN_Y);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_BLACK_LEVEL);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_BLANK_LEVEL);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_X_COLOR);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_M_CONTROL);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_BSTAMP_WSS_DATA);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_S_CARR);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_LINE21);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_LN_SEL);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_L21__WC_CTL);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_HTRIGGER_VTRIGGER);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_SAVID_EAVID);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_FLEN_FAL);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_LAL_PHASE_RESET);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_HS_INT_START_STOP_X);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_HS_EXT_START_STOP_X);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_VS_INT_START_X);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_VS_INT_STOP_X__VS_INT_START_Y);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_VS_INT_STOP_Y__VS_EXT_START_X);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_VS_EXT_STOP_X__VS_EXT_START_Y);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_VS_EXT_STOP_Y);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_AVID_START_STOP_X);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_AVID_START_STOP_Y);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_FID_INT_START_X__FID_INT_START_Y);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_FID_INT_OFFSET_Y__FID_EXT_START_X);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_FID_EXT_START_Y__FID_EXT_OFFSET_Y);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_TVDETGP_INT_START_STOP_X);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_TVDETGP_INT_START_STOP_Y);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_GEN_CTRL);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_OUTPUT_CONTROL);
    DISPLAY_REGISTER_VALUE(pVencRegs, VENC_OUTPUT_TEST);
*/
}

void 
Dump_ContextRestore(
    PFN_FmtPuts pfnFmtPuts
    )
{
    OMAP_CONTEXT_RESTORE_REGS  *pContextRestore;
    OMAP_PRCM_RESTORE_REGS     *pPrcmRestore;
    OMAP_SDRC_RESTORE_REGS     *pSdrcRestore;
    PHYSICAL_ADDRESS pa;
    WCHAR szBuffer[MESSAGE_BUFFER_SIZE];


    pa.QuadPart = OMAP_CONTEXT_RESTORE_REGS_PA;
    pContextRestore = (OMAP_CONTEXT_RESTORE_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_CONTEXT_RESTORE_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_RESTORE_REGS_PA;
    pPrcmRestore = (OMAP_PRCM_RESTORE_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_RESTORE_REGS), FALSE);

    pa.QuadPart = OMAP_SDRC_RESTORE_REGS_PA;
    pSdrcRestore = (OMAP_SDRC_RESTORE_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_SDRC_RESTORE_REGS), FALSE);


    pfnFmtPuts(TEXT("\r\nContext Restore Registers:\r\n"));
    DISPLAY_REGISTER_VALUE(pContextRestore, BOOT_CONFIG_ADDR);
    DISPLAY_REGISTER_VALUE(pContextRestore, PUBLIC_RESTORE_ADDR);
    DISPLAY_REGISTER_VALUE(pContextRestore, SECURE_SRAM_RESTORE_ADDR);
    DISPLAY_REGISTER_VALUE(pContextRestore, SDRC_MODULE_SEMAPHORE);
    DISPLAY_REGISTER_VALUE(pContextRestore, PRCM_BLOCK_OFFSET);
    DISPLAY_REGISTER_VALUE(pContextRestore, SDRC_BLOCK_OFFSET);
    DISPLAY_REGISTER_VALUE(pContextRestore, OEM_CPU_INFO_DATA_PA);
    DISPLAY_REGISTER_VALUE(pContextRestore, OEM_CPU_INFO_DATA_VA);

    pfnFmtPuts(TEXT("\r\nPrcm Restore Registers:\r\n"));
    DISPLAY_REGISTER_VALUE(pPrcmRestore, PRM_CLKSRC_CTRL);
    DISPLAY_REGISTER_VALUE(pPrcmRestore, PRM_CLKSEL);
    DISPLAY_REGISTER_VALUE(pPrcmRestore, CM_CLKSEL_CORE);
    DISPLAY_REGISTER_VALUE(pPrcmRestore, CM_CLKSEL_WKUP);
    DISPLAY_REGISTER_VALUE(pPrcmRestore, CM_CLKEN_PLL);
    DISPLAY_REGISTER_VALUE(pPrcmRestore, CM_AUTOIDLE_PLL);
    DISPLAY_REGISTER_VALUE(pPrcmRestore, CM_CLKSEL1_PLL);
    DISPLAY_REGISTER_VALUE(pPrcmRestore, CM_CLKSEL2_PLL);
    DISPLAY_REGISTER_VALUE(pPrcmRestore, CM_CLKSEL3_PLL);
    DISPLAY_REGISTER_VALUE(pPrcmRestore, CM_CLKEN_PLL_MPU);
    DISPLAY_REGISTER_VALUE(pPrcmRestore, CM_AUTOIDLE_PLL_MPU);
    DISPLAY_REGISTER_VALUE(pPrcmRestore, CM_CLKSEL1_PLL_MPU);
    DISPLAY_REGISTER_VALUE(pPrcmRestore, CM_CLKSEL2_PLL_MPU);

    pfnFmtPuts(TEXT("\r\nSDRC Restore Registers:\r\n"));
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, SYSCONFIG);
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, CS_CFG);
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, SHARING);
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, ERR_TYPE);
    DISPLAY_REGISTER_VALUE(pSdrcRestore,   DLLA_CTRL);
    DISPLAY_REGISTER_VALUE(pSdrcRestore,   DLLB_CTRL);
    DISPLAY_REGISTER_VALUE(pSdrcRestore,   POWER);
    DISPLAY_REGISTER_VALUE(pSdrcRestore,   MCFG_0);
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, MR_0);
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, EMR1_0);
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, EMR2_0);
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, EMR3_0);
    DISPLAY_REGISTER_VALUE(pSdrcRestore,   ACTIM_CTRLA_0);
    DISPLAY_REGISTER_VALUE(pSdrcRestore,   ACTIM_CTRLB_0);
    DISPLAY_REGISTER_VALUE(pSdrcRestore,   RFR_CTRL_0);
    DISPLAY_REGISTER_VALUE(pSdrcRestore,   MCFG_1);
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, MR_1);
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, EMR1_1);
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, EMR2_1);
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, EMR3_1);
    DISPLAY_REGISTER_VALUE(pSdrcRestore,   ACTIM_CTRLA_1);
    DISPLAY_REGISTER_VALUE(pSdrcRestore,   ACTIM_CTRLB_1);
    DISPLAY_REGISTER_VALUE(pSdrcRestore,   RFR_CTRL_1);
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, DCDL_1_CTRL);
    DISPLAY_REGISTER_VALUE16(pSdrcRestore, DCDL_2_CTRL);
}

void
Dump_EFuse(
    PFN_FmtPuts pfnFmtPuts
    )
{
    OMAP_SYSC_GENERAL_REGS     *pSyscGenralRegs;
    PHYSICAL_ADDRESS pa;
    WCHAR szBuffer[MESSAGE_BUFFER_SIZE];

    pa.QuadPart = OMAP_SYSC_GENERAL_REGS_PA;
    pSyscGenralRegs = (OMAP_SYSC_GENERAL_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_SYSC_GENERAL_REGS), FALSE);

    pfnFmtPuts(TEXT("\r\nSmartreflex EFuse Registers:\r\n"));
    DISPLAY_REGISTER_VALUE(pSyscGenralRegs, CONTROL_FUSE_OPP1_VDD1);
    DISPLAY_REGISTER_VALUE(pSyscGenralRegs, CONTROL_FUSE_OPP2_VDD1);
    DISPLAY_REGISTER_VALUE(pSyscGenralRegs, CONTROL_FUSE_OPP3_VDD1);
    DISPLAY_REGISTER_VALUE(pSyscGenralRegs, CONTROL_FUSE_OPP4_VDD1);
    DISPLAY_REGISTER_VALUE(pSyscGenralRegs, CONTROL_FUSE_OPP5_VDD1);
    DISPLAY_REGISTER_VALUE(pSyscGenralRegs, CONTROL_FUSE_OPP1_VDD2);
    DISPLAY_REGISTER_VALUE(pSyscGenralRegs, CONTROL_FUSE_OPP2_VDD2);
    DISPLAY_REGISTER_VALUE(pSyscGenralRegs, CONTROL_FUSE_OPP3_VDD2);
    DISPLAY_REGISTER_VALUE(pSyscGenralRegs, CONTROL_FUSE_SR);
}


void
Dump_PRCMStates(
    PFN_FmtPuts pfnFmtPuts
    )
{
    OMAP_PRCM_CLOCK_CONTROL_CM_REGS     *pPrcmClockControlCmRegs;
    OMAP_PRCM_IVA2_CM_REGS      *pPrcmIva2CmRegs;
    OMAP_PRCM_CORE_CM_REGS      *pPrcmCoreCmRegs;
    OMAP_PRCM_SGX_CM_REGS       *pPrcmSgxCmRegs;
    OMAP_PRCM_WKUP_CM_REGS      *pPrcmWkupCmRegs;
    OMAP_PRCM_DSS_CM_REGS       *pPrcmDssCmRegs;
    OMAP_PRCM_CAM_CM_REGS       *pPrcmCamCmRegs;
    OMAP_PRCM_PER_CM_REGS       *pPrcmPerCmRegs;
    OMAP_PRCM_USBHOST_CM_REGS   *pPrcmUsbhostCmRegs;
    OMAP_PRCM_IVA2_PRM_REGS     *pPrcmIva2PrmRegs;
    OMAP_PRCM_MPU_PRM_REGS      *pPrcmMpuPrmRegs;
    OMAP_PRCM_CORE_PRM_REGS     *pPrcmCorePrmRegs;
    OMAP_PRCM_SGX_PRM_REGS      *pPrcmSgxPrmRegs;
    OMAP_PRCM_DSS_PRM_REGS      *pPrcmDssPrmRegs;
    OMAP_PRCM_CAM_PRM_REGS      *pPrcmCamPrmRegs;
    OMAP_PRCM_PER_PRM_REGS      *pPrcmPerPrmRegs;
    OMAP_PRCM_NEON_PRM_REGS     *pPrcmNeonPrmRegs;
    OMAP_PRCM_USBHOST_PRM_REGS  *pPrcmUsbhostPrmRegs;
    PHYSICAL_ADDRESS pa;
    WCHAR szBuffer[MESSAGE_BUFFER_SIZE];

    pa.QuadPart = OMAP_PRCM_CLOCK_CONTROL_CM_REGS_PA;
    pPrcmClockControlCmRegs = (OMAP_PRCM_CLOCK_CONTROL_CM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_CLOCK_CONTROL_CM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_IVA2_CM_REGS_PA;
    pPrcmIva2CmRegs = (OMAP_PRCM_IVA2_CM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_IVA2_CM_REGS), FALSE);
    pa.QuadPart = OMAP_PRCM_CORE_CM_REGS_PA;
    pPrcmCoreCmRegs = (OMAP_PRCM_CORE_CM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_CORE_CM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_SGX_CM_REGS_PA;
    pPrcmSgxCmRegs = (OMAP_PRCM_SGX_CM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_SGX_CM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_WKUP_CM_REGS_PA;
    pPrcmWkupCmRegs = (OMAP_PRCM_WKUP_CM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_WKUP_CM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_DSS_CM_REGS_PA;
    pPrcmDssCmRegs = (OMAP_PRCM_DSS_CM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_DSS_CM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_CAM_CM_REGS_PA;
    pPrcmCamCmRegs = (OMAP_PRCM_CAM_CM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_CAM_CM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_PER_CM_REGS_PA;
    pPrcmPerCmRegs = (OMAP_PRCM_PER_CM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_PER_CM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_USBHOST_CM_REGS_PA;
    pPrcmUsbhostCmRegs = (OMAP_PRCM_USBHOST_CM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_USBHOST_CM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_IVA2_PRM_REGS_PA;
    pPrcmIva2PrmRegs = (OMAP_PRCM_IVA2_PRM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_IVA2_PRM_REGS), FALSE);
    pa.QuadPart = OMAP_PRCM_MPU_PRM_REGS_PA;
    pPrcmMpuPrmRegs = (OMAP_PRCM_MPU_PRM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_MPU_PRM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_CORE_PRM_REGS_PA;
    pPrcmCorePrmRegs = (OMAP_PRCM_CORE_PRM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_CORE_PRM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_SGX_PRM_REGS_PA;
    pPrcmSgxPrmRegs = (OMAP_PRCM_SGX_PRM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_SGX_PRM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_DSS_PRM_REGS_PA;
    pPrcmDssPrmRegs = (OMAP_PRCM_DSS_PRM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_DSS_PRM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_CAM_PRM_REGS_PA;
    pPrcmCamPrmRegs = (OMAP_PRCM_CAM_PRM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_CAM_PRM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_PER_PRM_REGS_PA;
    pPrcmPerPrmRegs = (OMAP_PRCM_PER_PRM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_PER_PRM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_NEON_PRM_REGS_PA;
    pPrcmNeonPrmRegs = (OMAP_PRCM_NEON_PRM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_NEON_PRM_REGS), FALSE);

    pa.QuadPart = OMAP_PRCM_USBHOST_PRM_REGS_PA;
    pPrcmUsbhostPrmRegs = (OMAP_PRCM_USBHOST_PRM_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_PRCM_USBHOST_PRM_REGS), FALSE);

    pfnFmtPuts(TEXT("\r\nPRCM check:\r\n"));

    pfnFmtPuts(TEXT("\r\nCLOCK_CONTROL_REG_CM:\r\n"));
    DISPLAY_REGISTER_VALUE(pPrcmClockControlCmRegs, CM_IDLEST_CKGEN);
    DISPLAY_REGISTER_VALUE(pPrcmClockControlCmRegs, CM_IDLEST2_CKGEN);

    pfnFmtPuts(TEXT("\r\nPower states:\r\n"));
    DISPLAY_REGISTER_VALUE(pPrcmIva2PrmRegs, PM_PWSTST_IVA2);
    DISPLAY_REGISTER_VALUE(pPrcmIva2PrmRegs, PM_PREPWSTST_IVA2);
    DISPLAY_REGISTER_VALUE(pPrcmMpuPrmRegs, PM_PWSTST_MPU);
    DISPLAY_REGISTER_VALUE(pPrcmMpuPrmRegs, PM_PREPWSTST_MPU);
    DISPLAY_REGISTER_VALUE(pPrcmCorePrmRegs, PM_PWSTST_CORE);
    DISPLAY_REGISTER_VALUE(pPrcmCorePrmRegs, PM_PREPWSTST_CORE);
    DISPLAY_REGISTER_VALUE(pPrcmSgxPrmRegs, PM_PWSTST_SGX);
    DISPLAY_REGISTER_VALUE(pPrcmSgxPrmRegs, PM_PREPWSTST_SGX);
    DISPLAY_REGISTER_VALUE(pPrcmDssPrmRegs, PM_PWSTST_DSS);
    DISPLAY_REGISTER_VALUE(pPrcmDssPrmRegs, PM_PREPWSTST_DSS);
    DISPLAY_REGISTER_VALUE(pPrcmCamPrmRegs, PM_PWSTST_CAM);
    DISPLAY_REGISTER_VALUE(pPrcmCamPrmRegs, PM_PREPWSTST_CAM);
    DISPLAY_REGISTER_VALUE(pPrcmPerPrmRegs, PM_PWSTST_PER);
    DISPLAY_REGISTER_VALUE(pPrcmPerPrmRegs, PM_PREPWSTST_PER);
    DISPLAY_REGISTER_VALUE(pPrcmNeonPrmRegs, PM_PWSTST_NEON);
    DISPLAY_REGISTER_VALUE(pPrcmNeonPrmRegs, PM_PREPWSTST_NEON);
    DISPLAY_REGISTER_VALUE(pPrcmUsbhostPrmRegs, PM_PWSTST_USBHOST);
    DISPLAY_REGISTER_VALUE(pPrcmUsbhostPrmRegs, PM_PREPWSTST_USBHOST);

    pfnFmtPuts(TEXT("\r\nFunctional clocks:\r\n"));
    DISPLAY_REGISTER_VALUE(pPrcmIva2CmRegs, CM_FCLKEN_IVA2);
    DISPLAY_REGISTER_VALUE(pPrcmCoreCmRegs, CM_FCLKEN1_CORE);
    DISPLAY_REGISTER_VALUE(pPrcmCoreCmRegs, CM_FCLKEN3_CORE);
    DISPLAY_REGISTER_VALUE(pPrcmSgxCmRegs, CM_FCLKEN_SGX);
    DISPLAY_REGISTER_VALUE(pPrcmWkupCmRegs, CM_FCLKEN_WKUP);
    DISPLAY_REGISTER_VALUE(pPrcmDssCmRegs, CM_FCLKEN_DSS);
    DISPLAY_REGISTER_VALUE(pPrcmCamCmRegs, CM_FCLKEN_CAM);
    DISPLAY_REGISTER_VALUE(pPrcmPerCmRegs, CM_FCLKEN_PER);
    DISPLAY_REGISTER_VALUE(pPrcmUsbhostCmRegs, CM_FCLKEN_USBHOST);

    pfnFmtPuts(TEXT("\r\nInterface clocks:\r\n"));
    DISPLAY_REGISTER_VALUE(pPrcmCoreCmRegs, CM_ICLKEN1_CORE);
    DISPLAY_REGISTER_VALUE(pPrcmCoreCmRegs, CM_ICLKEN3_CORE);
    DISPLAY_REGISTER_VALUE(pPrcmSgxCmRegs, CM_ICLKEN_SGX);
    DISPLAY_REGISTER_VALUE(pPrcmWkupCmRegs, CM_ICLKEN_WKUP);
    DISPLAY_REGISTER_VALUE(pPrcmDssCmRegs, CM_ICLKEN_DSS);
    DISPLAY_REGISTER_VALUE(pPrcmCamCmRegs, CM_ICLKEN_CAM);
    DISPLAY_REGISTER_VALUE(pPrcmPerCmRegs, CM_ICLKEN_PER);
    DISPLAY_REGISTER_VALUE(pPrcmUsbhostCmRegs, CM_ICLKEN_USBHOST);

}


//-----------------------------------------------------------------------------

void
DisplayClockInfo(
    UINT        clockId,
    UINT        nLevel,
    PFN_FmtPuts pfnFmtPuts
    )
{
    int i;
    WCHAR *psz;
    int indentation = 0;
    WCHAR szBuffer[MAX_PATH];
    IOCTL_PRCM_CLOCK_GET_SOURCECLOCKINFO_IN clockIn;
    IOCTL_PRCM_CLOCK_GET_SOURCECLOCKINFO_OUT clockOut;

    clockIn.clockId = clockId;
    clockIn.clockLevel = nLevel;
    do
        {
        if (DeviceIoControl(GetProxyDriverHandle(), IOCTL_PRCM_CLOCK_GET_SOURCECLOCKINFO,
                &clockIn, sizeof(IOCTL_PRCM_CLOCK_GET_SOURCECLOCKINFO_IN),
                &clockOut, sizeof(IOCTL_PRCM_CLOCK_GET_SOURCECLOCKINFO_OUT), NULL, 0) == FALSE)
            {
            StringCchPrintf(szBuffer, MAX_PATH,
                L"Unable to get clock if for %s",
                FindClockNameById(clockId, nLevel));
            pfnFmtPuts(szBuffer);
            break;
            }

        // output clock info
        psz = szBuffer;
        for (i = 0; i < indentation; ++i)
            {
            *psz++ = L'-';
            }

        // format and output text
        StringCchPrintf(psz, MAX_PATH, L"%s(%d)\r\n",
            FindClockNameById(clockIn.clockId, clockIn.clockLevel),
            clockOut.refCount);
        pfnFmtPuts(szBuffer);

        indentation++;
        clockIn.clockId = clockOut.parentId;
        clockIn.clockLevel = clockOut.parentLevel;

        } while (clockOut.parentId != 0);
}

//-----------------------------------------------------------------------------

void
Device_Enable(
    OMAP_DEVICE devId,
    BOOL bEnable,
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    if (bEnable)
        {
        rc = DeviceIoControl(GetBusHandle(), IOCTL_BUS_REQUEST_CLOCK, &devId,
                sizeof(OMAP_DEVICE), NULL, 0, NULL, 0
                );
        }
    else
        {
        rc = DeviceIoControl(GetBusHandle(), IOCTL_BUS_RELEASE_CLOCK, &devId,
                sizeof(OMAP_DEVICE), NULL, 0, NULL, 0
                );
        }

    if (rc == TRUE)
        {
        IOCTL_PRCM_DEVICE_GET_SOURCECLOCKINFO_OUT clockInfo;
        // get the clock information for the device
        if (DeviceIoControl(GetProxyDriverHandle(), IOCTL_PRCM_DEVICE_GET_SOURCECLOCKINFO,
                &devId, sizeof(devId),
                &clockInfo, sizeof(IOCTL_PRCM_DEVICE_GET_SOURCECLOCKINFO_OUT), NULL, 0))
            {
            UINT i = clockInfo.count;
            while (i--)
                {
                DisplayClockInfo(clockInfo.rgSourceClocks[i].clockId,
                    clockInfo.rgSourceClocks[i].nLevel, pfnFmtPuts);
                pfnFmtPuts(L"\r\n");
                }
            }
        }
}

//-----------------------------------------------------------------------------

void
Device_Status(
    OMAP_DEVICE devId,
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    WCHAR szBuffer[MAX_PATH];
    IOCTL_PRCM_DEVICE_GET_DEVICESTATUS_OUT info;
    rc = DeviceIoControl(GetProxyDriverHandle(), IOCTL_PRCM_DEVICE_GET_DEVICESTATUS,
                &devId, sizeof(devId),
                &info, sizeof(IOCTL_PRCM_DEVICE_GET_DEVICESTATUS_OUT), NULL, 0);

    if (rc == TRUE)
        {
        StringCchPrintf(szBuffer, MAX_PATH, L"%s - Enabled(%d), AutoIdle(%d)\r\n",
            FindDeviceNameById(devId), info.bEnabled, info.bAutoIdle
            );
        pfnFmtPuts(szBuffer);
        pfnFmtPuts(L"\r\n");

        IOCTL_PRCM_DEVICE_GET_SOURCECLOCKINFO_OUT clockInfo;
        // get the clock information for the device
        if (DeviceIoControl(GetProxyDriverHandle(), IOCTL_PRCM_DEVICE_GET_SOURCECLOCKINFO,
                &devId, sizeof(devId),
                &clockInfo, sizeof(IOCTL_PRCM_DEVICE_GET_SOURCECLOCKINFO_OUT), NULL, 0))
            {
            UINT i = clockInfo.count;
            while (i--)
                {
                DisplayClockInfo(clockInfo.rgSourceClocks[i].clockId,
                    clockInfo.rgSourceClocks[i].nLevel, pfnFmtPuts);
                pfnFmtPuts(L"\r\n");
                }
            }
        }
    else
        {
        pfnFmtPuts(L"Unknown device\r\n");
        }
}

//-----------------------------------------------------------------------------

void
Device_SourceClock(
    OMAP_DEVICE  devId,
    UINT        count,
    UINT        rgSourceClocks[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    UINT i;
    BOOL rc = FALSE;
    CE_BUS_DEVICE_SOURCE_CLOCKS clockInfo;

    clockInfo.devId = devId;
    clockInfo.count = count;
    for (i = 0; i < count; ++i)
        {
        clockInfo.rgSourceClocks[i] = rgSourceClocks[i];
        }

    rc = DeviceIoControl(
        GetBusHandle(), IOCTL_BUS_SOURCE_CLOCKS, &clockInfo, sizeof(clockInfo),
        NULL, 0, NULL, 0
        );

    if (rc == TRUE)
        {
        Device_Status(devId, pfnFmtPuts);
        }
    else
        {
        pfnFmtPuts(L"Unknown device\r\n");
        }
}

//-----------------------------------------------------------------------------

BOOL 
Battery(
    ULONG argc, 
    LPWSTR args[], 
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    WCHAR szBuffer[MESSAGE_BUFFER_SIZE];
    UINT8 input, output;

    HANDLE hTwl = GetTritonHandle();

    // parse command
    if (wcsicmp(args[0], L"charge") != 0 || argc < 2)
        {
        StringCchPrintf(szBuffer, MESSAGE_BUFFER_SIZE, L"syntax error\r\n");
        pfnFmtPuts(szBuffer);
        goto cleanUp;
        }

    if (wcsicmp(args[1], L"usb") == 0)
        {
        // DISABLE ALL AUTO charging
        output = CONFIG_DONE;
        TWLWriteRegs(hTwl, TWL_BOOT_BCI, &output, 1);

        // disable monitors
        TWLReadRegs(hTwl, TWL_BCIMFEN4, &input, 1);

        output = KEY_BCIMFEN2;
        TWLWriteRegs(hTwl, TWL_BCIMFKEY, &output, 1);

        output = input & ~(VBUSOVEN | ACCHGOVEN);
        TWLWriteRegs(hTwl, TWL_BCIMFEN2, &output, 1);

        // disable battery overcharge monitor
        TWLReadRegs(hTwl, TWL_BCIMFEN4, &input, 1);

        output = KEY_BCIMFEN4;
        TWLWriteRegs(hTwl, TWL_BCIMFKEY, &output, 1);

        output = input & ~VBATOVEN;
        TWLWriteRegs(hTwl, TWL_BCIMFEN4, &output, 1);

        // set charge current
        output = KEY_BCIMFTH9;
        TWLWriteRegs(hTwl, TWL_BCIMFKEY, &output, 1);

        output = 0x2C;
        TWLWriteRegs(hTwl, TWL_BCIIREF1, &output, 1);

        output = KEY_BCIMFTH9;
        TWLWriteRegs(hTwl, TWL_BCIMFKEY, &output, 1);

        output = 0x3;
        TWLWriteRegs(hTwl, TWL_BCIIREF2, &output, 1);

        // Disable charge mode
        output = KEY_BCIOFF;
        TWLWriteRegs(hTwl, TWL_BCIMDKEY, &output, 1);

        //Set USB charge mode
        Sleep(50);
        output = USBSLOWMCHG;
        TWLWriteRegs(hTwl, TWL_BCIMFSTS4, &output, 1);

        //Enable USB Linear charging
        output = KEY_BCIUSBLINEAR;
        TWLWriteRegs(hTwl, TWL_BCIMDKEY, &output, 1);

        //Disable watchdog
        output = KEY_BCIWDKEY5;
        TWLWriteRegs(hTwl, TWL_BCIWDKEY, &output, 1);

        //Enable VBUS resistive divider
        TWLReadRegs(hTwl, TWL_BCICTL1, &input, 1);
        output = input | MESVBUS;
        TWLWriteRegs(hTwl, TWL_BCICTL1, &output, 1);
        }
    else if (wcsicmp(args[1], L"ac") == 0)
        {
        // disable charge mode
        output = KEY_BCIOFF;
        TWLWriteRegs(hTwl, TWL_BCIMDKEY, &output, 1);

        // enable auto charge
        output = CONFIG_DONE | BCIAUTOAC;
        TWLWriteRegs(hTwl, TWL_BOOT_BCI, &output, 1);
        }
    else if (wcsicmp(args[1], L"disable") == 0)
        {
        // disable charge mode
        output = KEY_BCIOFF;
        TWLWriteRegs(hTwl, TWL_BCIMDKEY, &output, 1);
        }
    else
        {
        StringCchPrintf(szBuffer, MESSAGE_BUFFER_SIZE, L"syntax error\r\n");
        pfnFmtPuts(szBuffer);
        goto cleanUp;
        }

    rc = TRUE;
    
cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
Device_xxx(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    int i = 0;
    int j = 0;
    UINT rgSourceClocks[MAX_PATH];
    BOOL bEnable = -1;
    BOOL bDisplayStatus = FALSE;
    OMAP_DEVICE devId = OMAP_DEVICE_NONE;
    while (argc--)
        {
        if (wcsicmp(args[i], L"enable") == 0)
            {
            bEnable = TRUE;
            }
        else if (wcsicmp(args[i], L"disable") == 0)
            {
            bEnable = FALSE;
            }
        else if (wcsicmp(args[i], L"status") == 0)
            {
            bDisplayStatus = TRUE;
            }
        else if (wcsicmp(args[i], L"source") == 0)
            {
            // get device id
            if (argc-- == 0) break;
            devId = FindDeviceIdByName(args[++i]);
            if (devId == OMAP_DEVICE_NONE)
                {
                pfnFmtPuts(L"undefined parameter(%s)\r\n", args[i]);
                return TRUE;
                }

            j = 0;
            while (argc--)
                {
                rgSourceClocks[j] = FindClockIdByName(args[++i]);
                if (rgSourceClocks[j] == -1)
                    {
                    pfnFmtPuts(L"undefined parameter(%s)\r\n", args[i]);
                    return TRUE;
                    }
                ++j;
                }


            // call api
            Device_SourceClock(devId, j, rgSourceClocks, pfnFmtPuts);
            break;

            }
        else
            {
            devId = FindDeviceIdByName(args[i]);
            if (devId == OMAP_DEVICE_NONE)
                {
                pfnFmtPuts(L"undefined parameter(%s)\r\n", args[i]);
                return TRUE;
                }
            }
        i++;
        }

    if (bDisplayStatus == TRUE)
        {
        Device_Status(devId, pfnFmtPuts);
        }
    else if (devId != OMAP_DEVICE_NONE && bEnable != -1)
        {
        Device_Enable(devId, bEnable, pfnFmtPuts);
        }

    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL
Observe(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    UINT val;
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS pa;    
    OMAP_SYSC_GENERAL_REGS *pGenRegs;    
    OMAP_SYSC_GENERAL_WKUP_REGS *pGenWkupRegs;

    OMAP_SYSC_PADCONFS_REGS *pPadCfgRegs;

    pa.QuadPart = OMAP_SYSC_PADCONFS_REGS_PA;
    pPadCfgRegs = (OMAP_SYSC_PADCONFS_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_SYSC_PADCONFS_REGS), FALSE);
    
    // setup pointers
    pa.QuadPart = OMAP_SYSC_GENERAL_REGS_PA;
    pGenRegs = (OMAP_SYSC_GENERAL_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_SYSC_GENERAL_REGS), FALSE);

    pa.QuadPart = OMAP_SYSC_GENERAL_WKUP_REGS_PA;
    pGenWkupRegs = (OMAP_SYSC_GENERAL_WKUP_REGS*)MmMapIoSpace_Proxy(pa, sizeof(OMAP_SYSC_GENERAL_WKUP_REGS), FALSE);

    for (int i = 0; i < sizeof(g_rgObservabilityList)/sizeof(OBSERVE_ITEM); ++i)
        {
        if (wcsicmp(args[0], g_rgObservabilityList[i].szEntry) == 0)
            {
            switch (g_rgObservabilityList[i].pEntry->domain)
                {
                case WKUP_OBSERVABILITY:
                    val = INREG32(((UCHAR*)pGenWkupRegs + g_rgObservabilityList[i].pEntry->pWkup->offset));
                    val &= g_rgObservabilityList[i].pEntry->pWkup->mask;
                    val |= g_rgObservabilityList[i].pEntry->val << g_rgObservabilityList[i].pEntry->pWkup->shift;
                    OUTREG32(((UCHAR*)pGenWkupRegs + g_rgObservabilityList[i].pEntry->pWkup->offset), val);
                    break;

                case CORE_OBSERVABILITY:
                    // put wkup mux into CORE_OBSMUXn mode
                    val = INREG32(((UCHAR*)pGenWkupRegs + g_rgObservabilityList[i].pEntry->pWkup->offset));
                    val &= g_rgObservabilityList[i].pEntry->pWkup->mask;
                    OUTREG32(((UCHAR*)pGenWkupRegs + g_rgObservabilityList[i].pEntry->pWkup->offset), val);

                    // mux core
                    val = INREG32(((UCHAR*)pGenRegs + g_rgObservabilityList[i].pEntry->pCore->offset));
                    val &= g_rgObservabilityList[i].pEntry->pCore->mask;
                    val |= g_rgObservabilityList[i].pEntry->val << g_rgObservabilityList[i].pEntry->pCore->shift;
                    OUTREG32(((UCHAR*)pGenRegs + g_rgObservabilityList[i].pEntry->pCore->offset), val);
                    break;
                }

            // mux pad
            val = INREG32(((UCHAR*)pPadCfgRegs + g_rgObservabilityList[i].pEntry->pEtk->offset));
            val &= g_rgObservabilityList[i].pEntry->pEtk->mask;
            val |= ETK_OBSMUX << g_rgObservabilityList[i].pEntry->pEtk->shift;
            OUTREG32(((UCHAR*)pPadCfgRegs + g_rgObservabilityList[i].pEntry->pEtk->offset), val);

            pfnFmtPuts(L"configured for %s\r\n", args[0]);
            rc = TRUE;
            goto cleanUp;
            }
        }

    pfnFmtPuts(L"unknown configuration '%s'\r\n", args[0]);

cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------

#define CONTRAST_MAX_LEVEL 6

BOOL
SetContrast(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    int level;
    BOOL rc = FALSE;
    level = GetHex(args[0]);
    ContrastCmdInputParm  ContrastLevel;

    if( level <= CONTRAST_MAX_LEVEL )
        {
        ContrastLevel.command = CONTRAST_CMD_SET;
        ContrastLevel.parm = level;

        if(ExtEscape(GetDC(NULL), CONTRASTCOMMAND,sizeof(ContrastLevel), (LPCSTR)&ContrastLevel, 0, NULL ) > 0)
            {
            rc = TRUE;
            goto cleanUp;
            }
        }

    pfnFmtPuts(L"SetContrast wrong args '%s' error :%d\r\n", args[0], GetLastError());

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

BOOL
Reboot(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;

    pfnFmtPuts( TEXT("Rebooting device\r\n") );    

    rc = KernelIoControl( IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL );
    if( rc == FALSE )
    {
        pfnFmtPuts(L"IOCTL_HAL_REBOOT  failed\r\n");
        goto cleanUp;
    }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

BOOL
Display(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = TRUE;
    int i = 0;
    HDC hDC;
    CEDEVICE_POWER_STATE  dx;
            
    
    //  Get handle to display driver
    hDC = GetDC(NULL);
    if( hDC == NULL )
    {
        pfnFmtPuts(L"Error getting display driver handle\r\n");
        goto cleanUp;
    }
    

    while (argc--)
    {
        if (wcsicmp(args[i], L"on") == 0)
        {
            //  Enable display
            dx = D0;
            
            ExtEscape( hDC, IOCTL_POWER_SET, 0, NULL, sizeof(CEDEVICE_POWER_STATE), (LPSTR)&dx );
            argc = 0;
        }
        else if (wcsicmp(args[i], L"off") == 0)
        {
            //  Disable display
            dx = D4;
            
            ExtEscape( hDC, IOCTL_POWER_SET, 0, NULL, sizeof(CEDEVICE_POWER_STATE), (LPSTR)&dx );
            argc = 0;
        }
        else
        {
            pfnFmtPuts(L"Unknown command\r\n");
            argc = 0;
        }

        i++;
    }


    //  Release the handle
    ReleaseDC( NULL, hDC );
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

BOOL
TvOut(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = TRUE;
    int i = 0;
    HDC hDC;
    DRVESC_TVOUT_SETTINGS   tvOutSettings;
    BOOL                    bUpdate = FALSE;
            
    
    //  Get handle to display driver
    hDC = GetDC(NULL);
    if( hDC == NULL )
    {
        pfnFmtPuts(L"Error getting display driver handle\r\n");
        goto cleanUp;
    }
    

    //  Get the current TV out settings
    ExtEscape( hDC, DRVESC_TVOUT_GETSETTINGS, 0, NULL, sizeof(DRVESC_TVOUT_SETTINGS), (LPSTR) &tvOutSettings );
            
    
    while (argc--)
    {
        if (wcsicmp(args[i], L"on") == 0)
        {
            //  Enable TV out
            ExtEscape( hDC, DRVESC_TVOUT_ENABLE, 0, NULL, 0, NULL );
            argc = 0;
        }
        else if (wcsicmp(args[i], L"off") == 0)
        {
            //  Disable TV out
            ExtEscape( hDC, DRVESC_TVOUT_DISABLE, 0, NULL, 0, NULL );
            argc = 0;
        }
        else if (wcsicmp(args[i], L"settings") == 0)
        {
                //  Dump the settings
                pfnFmtPuts(L"  TV Out Enable       : %d\r\n", tvOutSettings.bEnable );
            pfnFmtPuts(L"  TV Out Filter Level : %d\r\n", tvOutSettings.dwFilterLevel );
                pfnFmtPuts(L"  TV Out Aspect Ratio : %d %d\r\n", tvOutSettings.dwAspectRatioW, tvOutSettings.dwAspectRatioH );
                pfnFmtPuts(L"  TV Out Scaling %%    : %d %d\r\n", tvOutSettings.dwResizePercentW, tvOutSettings.dwResizePercentH );
                pfnFmtPuts(L"  TV Out Offset       : %d %d\r\n", tvOutSettings.lOffsetW, tvOutSettings.lOffsetH );
                argc = 0;
        }
        else if (wcsicmp(args[i], L"filter") == 0 && argc >= 1)
        {
            tvOutSettings.dwFilterLevel = GetDec( args[++i] );
            bUpdate = TRUE;
            argc -= 1;
        }
        else if (wcsicmp(args[i], L"aspect") == 0 && argc >= 2)
        {
            tvOutSettings.dwAspectRatioW = GetDec( args[++i] );
            tvOutSettings.dwAspectRatioH = GetDec( args[++i] );
            bUpdate = TRUE;
            argc -= 2;
        }
        else if (wcsicmp(args[i], L"scale") == 0 && argc >= 2)
        {
            tvOutSettings.dwResizePercentW = GetDec( args[++i] );
            tvOutSettings.dwResizePercentH = GetDec( args[++i] );
            bUpdate = TRUE;
            argc -= 2;
        }
        else if (wcsicmp(args[i], L"offset") == 0 && argc >= 2)
        {
            tvOutSettings.lOffsetW = GetDec( args[++i] );
            tvOutSettings.lOffsetH = GetDec( args[++i] );
            bUpdate = TRUE;
            argc -= 2;
        }
        else
        {
            pfnFmtPuts(L"Unknown command\r\n");
            argc = 0;
        }

        i++;
    }

    //  Update the current TV out settings
    if( bUpdate )
    {
        ExtEscape( hDC, DRVESC_TVOUT_SETSETTINGS, sizeof(DRVESC_TVOUT_SETTINGS), (LPSTR) &tvOutSettings, 0, NULL );

        //  Dump the new settings
        pfnFmtPuts(L"Updated TV Out Settings -\r\n");
        pfnFmtPuts(L"  TV Out Enable       : %d\r\n", tvOutSettings.bEnable );
        pfnFmtPuts(L"  TV Out Filter Level : %d\r\n", tvOutSettings.dwFilterLevel );
        pfnFmtPuts(L"  TV Out Aspect Ratio : %d %d\r\n", tvOutSettings.dwAspectRatioW, tvOutSettings.dwAspectRatioH );
        pfnFmtPuts(L"  TV Out Scaling %%    : %d %d\r\n", tvOutSettings.dwResizePercentW, tvOutSettings.dwResizePercentH );
        pfnFmtPuts(L"  TV Out Offset       : %d %d\r\n", tvOutSettings.lOffsetW, tvOutSettings.lOffsetH );
    }

    //  Release the handle
    ReleaseDC( NULL, hDC );
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

#if !BSP_DVI_NO_SWITCHING
BOOL
DVIControl(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = TRUE;
    int i = 0;
    HDC hDC;
    BOOL bUpdate = FALSE;
    
    //  Get handle to display driver
    hDC = GetDC(NULL);
    if( hDC == NULL )
    {
        pfnFmtPuts(L"Error getting display driver handle\r\n");
        goto cleanUp;
    }
    
    while (argc--)
    {
        if (wcsicmp(args[i], L"on") == 0)
        {
            //  Enable DVI
            ExtEscape( hDC, DRVESC_DVI_ENABLE, 0, NULL, 0, NULL );
            argc = 0;
        }
        else if (wcsicmp(args[i], L"off") == 0)
        {
            //  Disable DVI
            ExtEscape( hDC, DRVESC_DVI_DISABLE, 0, NULL, 0, NULL );
            argc = 0;
        }
        else
        {
            pfnFmtPuts(L"Unknown command\r\n");
            argc = 0;
        }

        i++;
    }

    //  Release the handle
    ReleaseDC( NULL, hDC );
    
cleanUp:
    return rc;
}

#endif

//-----------------------------------------------------------------------------
BOOL
PowerDomain (
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    POWERDOMAIN_CONSTRAINT_INFO   constriantInfo;

    if (argc--)
        {
        if (wcsicmp(args[0], L"core") == 0)
            {
            constriantInfo.powerDomain = POWERDOMAIN_CORE;
            }
        else if (wcsicmp(args[0], L"mpu") == 0)
            {
            constriantInfo.powerDomain = POWERDOMAIN_MPU;
            }
        else if (wcsicmp(args[0], L"iva2") == 0)
            {
            constriantInfo.powerDomain = POWERDOMAIN_IVA2;
            }
        else if (wcsicmp(args[0], L"neon") == 0)
            {
            constriantInfo.powerDomain = POWERDOMAIN_NEON;
            }
        else if (wcsicmp(args[0], L"usbhost") == 0)
            {
            constriantInfo.powerDomain = POWERDOMAIN_USBHOST;
            }
        else if (wcsicmp(args[0], L"dss") == 0)
            {
            constriantInfo.powerDomain = POWERDOMAIN_DSS;
            }
        else if (wcsicmp(args[0], L"camera") == 0)
            {
            constriantInfo.powerDomain = POWERDOMAIN_CAMERA;
            }
        else if (wcsicmp(args[0], L"sgx") == 0)
            {
            constriantInfo.powerDomain = POWERDOMAIN_SGX;
            }
        else if (wcsicmp(args[0], L"per") == 0)
            {
            constriantInfo.powerDomain = POWERDOMAIN_PERIPHERAL;
            }
        else
            {
            goto cleanUp;
            }
        }
    else
        {
        goto cleanUp;
        }

     if(argc)
        {
        if (wcsicmp(args[1], L"D0") == 0)
            {
            constriantInfo.state = D0;
            }
        else if (wcsicmp(args[1], L"D1") == 0)
            {
            constriantInfo.state = D1;
            }
        else if (wcsicmp(args[1], L"D2") == 0)
            {
            constriantInfo.state = D2;
            }
        else if (wcsicmp(args[1], L"D3") == 0)
            {
            constriantInfo.state = D3;
            }
        else if (wcsicmp(args[1], L"D4") == 0)
            {
            constriantInfo.state = D4;
            }
        else if (wcsicmp(args[1], L"-r") == 0)
            {
            constriantInfo.state = CONSTRAINT_STATE_NULL;
            }
        else
            {
            goto cleanUp;
            }
        }
     else
        {
        goto cleanUp;
        }

        constriantInfo.size = sizeof(POWERDOMAIN_CONSTRAINT_INFO);

        rc = DeviceIoControl(GetProxyDriverHandle(), IOCTL_POWERDOMAIN_REQUEST,
                            &constriantInfo, sizeof(constriantInfo),
                            NULL, 0, NULL, 0);
        return rc;

cleanUp:
    pfnFmtPuts(L"powerdomain [core/mpu/iva2/neon/usbhost/dss/per/camera/sgx] [ D0 - D4 / -r ] -> Set power domain contraint\r\n");
    return rc;
}

//-----------------------------------------------------------------------------

BOOL
HalProfile(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    if (argc)
        {
        if (wcsicmp(args[0], L"DVFS") == 0)
            {
            if (ProfileDvfs(argc, args, pfnFmtPuts) == TRUE) return TRUE;
            }
        else if (wcsicmp(args[0], L"interruptlatency") == 0)
            {
            if (ProfileInterruptLatency(argc, args, pfnFmtPuts) == TRUE) return TRUE;
            }
        else if (wcsicmp(args[0], L"sdrcstall") == 0)
            {
            if (ProfileSdrcStall(argc, args, pfnFmtPuts) == TRUE) return TRUE;
            }
        else if (wcsicmp(args[0], L"wakeupaccuracy") == 0)
            {
            if (ProfileWakeupAccuracy(argc, args, pfnFmtPuts) == TRUE) return TRUE;
            }
        }

    //  Dump the settings
    pfnFmtPuts(L"halprofile [dvfs|wakeupaccuracy|sdrcstall|interruptlatency] [{option parameters}]\r\n");
    pfnFmtPuts(L"  dvfs - \r\n");
    pfnFmtPuts(L"    -t : Time in ms between opm changes\r\n");
    pfnFmtPuts(L"    -d : Test duration in seconds\r\n");
    pfnFmtPuts(L"    -r : If exists then random mode\r\n");
    pfnFmtPuts(L"    -h : High opm (1-5) (default: 1)\r\n");
    pfnFmtPuts(L"    -l : Low opm (0-4) (default: 0)\r\n");
    pfnFmtPuts(L"\r\n");

    pfnFmtPuts(L"  interruptlatency - \r\n");
    pfnFmtPuts(L"    -s : number of times per sleep states (default: 100)\r\n");
    pfnFmtPuts(L"\r\n");

    pfnFmtPuts(L"  sdrcstall - \r\n");
    pfnFmtPuts(L"    -s : number of transitions between opm 0 and opm 1 (default: 100)\r\n");
    pfnFmtPuts(L"\r\n");

    pfnFmtPuts(L"  wakeupaccuracy - \r\n");
    pfnFmtPuts(L"    -s : number of samples (default: 100)\r\n");
    pfnFmtPuts(L"    -t : Time in ms between captures\r\n");
    pfnFmtPuts(L"\r\n");

    return TRUE;
}

//-----------------------------------------------------------------------------

BOOL
InterruptLatency(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    float time;
    int ticks = _wtoi(args[0]);

    if (argc == 0) return FALSE;
    if (wcsicmp(args[0], L"?") == 0)
        {
        // dump OPP settings
        pfnFmtPuts(L"  InterruptLatency [time] \r\n");
        pfnFmtPuts(L"    time - in ticks of 32khz clock, -1 releases constraint\r\n");
        return TRUE;
        }
    else
        {
        if (ticks < 0)
            {
            time = -1.0f;
            }
        else
            {
            time = (float)ticks/32768.0f;
            }

        DeviceIoControl(
            GetProxyDriverHandle(),
            IOCTL_INTERRUPT_LATENCY_CONSTRAINT,
            (void*)&time,
            sizeof(float),
            NULL,
            0,
            NULL,
            NULL
            );
        }

    return TRUE;
}

#if CACHEINFO_ENABLE

//------------------------------------------------------------------------------

BOOL
ShowCacheInfo(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = TRUE;
    int i = 0;
    CacheInfo CpuCacheInfo;
    
    if (!CeGetCacheInfo(sizeof(CpuCacheInfo), &CpuCacheInfo))
    {
        pfnFmtPuts(L"Can't get cache info!\r\n");
        goto cleanUp;
    }

    pfnFmtPuts(L"CPU cache info:\r\n");
    if (CpuCacheInfo.dwL1Flags & CF_UNIFIED)
        pfnFmtPuts(L" L1 cache is unified\r\n");
    pfnFmtPuts(L" L1 ICacheSize     = %d\r\n", CpuCacheInfo.dwL1ICacheSize);
    pfnFmtPuts(L" L1 ICacheLineSize = %d\r\n", CpuCacheInfo.dwL1ICacheLineSize);
    pfnFmtPuts(L" L1 ICacheNumWays  = %d\r\n", CpuCacheInfo.dwL1ICacheNumWays);
    pfnFmtPuts(L" L1 DCacheSize     = %d\r\n", CpuCacheInfo.dwL1DCacheSize);
    pfnFmtPuts(L" L1 DCacheLineSize = %d\r\n", CpuCacheInfo.dwL1DCacheLineSize);
    pfnFmtPuts(L" L1 DCacheNumWays  = %d\r\n", CpuCacheInfo.dwL1DCacheNumWays);
    if (CpuCacheInfo.dwL2Flags & CF_UNIFIED)
        pfnFmtPuts(L" L2 cache is unified\r\n");
    pfnFmtPuts(L" L2 ICacheSize     = %d\r\n", CpuCacheInfo.dwL2ICacheSize);
    pfnFmtPuts(L" L2 ICacheLineSize = %d\r\n", CpuCacheInfo.dwL2ICacheLineSize);
    pfnFmtPuts(L" L2 ICacheNumWays  = %d\r\n", CpuCacheInfo.dwL2ICacheNumWays);
    pfnFmtPuts(L" L2 DCacheSize     = %d\r\n", CpuCacheInfo.dwL2DCacheSize);
    pfnFmtPuts(L" L2 DCacheLineSize = %d\r\n", CpuCacheInfo.dwL2DCacheLineSize);
    pfnFmtPuts(L" L2 DCacheNumWays  = %d\r\n", CpuCacheInfo.dwL2DCacheNumWays);
    
cleanUp:
    return rc;
}

#endif
