/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
#include <windows.h>
#include <tchar.h>
#include <bsp.h>
#include <bus.h>
#include "utils.h"

//------------------------------------------------------------------------------

BOOL IsOptionChar(TCHAR cInp)
{
   const char acOptionChars[] = { _T('/'), _T('-') };
   const int nOptionChars = 2;

   for (int i = 0; i < nOptionChars; i++)   {
      if (acOptionChars[i] == cInp) break;
   }
   return (i < nOptionChars);
}

//------------------------------------------------------------------------------

BOOL GetOptionFlag(int *pargc, LPTSTR argv[], LPCTSTR szOption)
{
   BOOL bResult = FALSE;
   int i = 0;

   while (i < *pargc) {
      if (IsOptionChar(argv[i][0])) {
         if (_tcsicmp(&argv[i][1], szOption) == 0) {
            bResult = TRUE;
            break;
         }
      }
      i++;
   }

   if (bResult) {
      for (int j = i + 1; j < *pargc; j++) argv[j - 1] = argv[j];
      (*pargc)--;
   }
   return bResult;
}

//------------------------------------------------------------------------------

LPTSTR GetOptionString(int *pargc, LPTSTR argv[], LPCTSTR szOption)
{
   LPTSTR szResult = NULL;
   int i = 0;
   int nRemove = 1;

   while (i < *pargc) {
      if (IsOptionChar(argv[i][0])) {
         if (_tcsnicmp(&argv[i][1], szOption, _tcslen(szOption)) == 0) {
            szResult = &argv[i][_tcslen(szOption) + 1];
            if (*szResult == _T('\0') && (i + 1) < *pargc) {
               szResult = argv[i + 1];
               nRemove++;
            }
            break;
         }
      }
      i++;
   }

   if (szResult != NULL) {
      for (int j = i + nRemove; j < *pargc; j++) argv[j - nRemove] = argv[j];
      (*pargc) -= nRemove;
   }
   return szResult;
}

//------------------------------------------------------------------------------

LONG GetOptionLong(int *pargc, LPTSTR argv[], LONG lDefault, LPCTSTR szOption)
{
   LPTSTR szValue = NULL;
   LONG nResult = 0;
   TCHAR cDigit = _T('\0');
   INT nBase = 10;
   BOOL bMinus = FALSE;

   szValue = GetOptionString(pargc, argv, szOption);
   if (szValue == NULL) {
      nResult = lDefault;
      goto cleanUp;
   }

   // Check minus sign
   if (*szValue == _T('-')) {
      bMinus = TRUE;
      szValue++;
   }

   // And plus sign
   if (*szValue == _T('+')) szValue++;

   if (
      *szValue == _T('0') && 
      (*(szValue + 1) == _T('x') || *(szValue + 1) == _T('X'))
   ) {
      nBase = 16;
      szValue += 2;
   }

   nResult = 0;
   while (TRUE) {
      if (*szValue >= _T('0') && *szValue <= _T('9')) {
         nResult = nResult * nBase + (*szValue - _T('0'));
         szValue++;
      } else if (nBase == 16) {
         if (*szValue >= _T('a') && *szValue <= _T('f')) {
            nResult = nResult * nBase + (*szValue - _T('a') + 10);
            szValue++;
         } else if (*szValue >= _T('A') && *szValue <= _T('F')) {
            nResult = nResult * nBase + (*szValue - _T('A') + 10);
            szValue++;
         } else {
            break;
         }
      } else {
         break;
      }
   }

   if (bMinus) nResult = -nResult;

cleanUp:
   return nResult;
}

//------------------------------------------------------------------------------

void CommandLineToArgs(TCHAR szCommandLine[], int *argc, LPTSTR argv[]) 
{
   int i, nArgc = 0;
   BOOL bInQuotes, bEndFound = FALSE;
   TCHAR *p = szCommandLine;

   if (szCommandLine == NULL || argc == NULL || argv == NULL) return;

   for (i = 0; i < *argc; i++) {
        
      // Clear our quote flag
      bInQuotes = FALSE;

      // Move past and zero out any leading whitespace
      while (*p && _istspace(*p)) *(p++) = _T('\0');

      // If the next character is a quote, move over it and remember it
      if (*p == _T('\"')) {
        *(p++) = _T('\0');
        bInQuotes = TRUE;
      }

      // Point the current argument to our current location
      argv[i] = p;

      // If this argument contains some text, then update our argument count
      if (*p != _T('\0')) nArgc = i + 1;

      // Move over valid text of this argument.
      while (*p != _T('\0')) {
         if (_istspace(*p) || (bInQuotes && (*p == _T('\"')))) {
            *(p++) = _T('\0');
            break;
         }
         p++;
      }
      
      // If reach end of string break;
      if (*p == _T('\0')) break;
   }
   *argc = nArgc;
}

//------------------------------------------------------------------------------

WCHAR const* _rgDeviceClockNames[] =
{
    L"DPLL1_M2X2_CLK",
    L"DPLL2_M2_CLK",
    L"CORE_CLK",
    L"COREX2_CLK",
    L"EMUL_CORE_ALWON_CLK",
    L"PRM_96M_ALWON_CLK",
    L"DPLL4_M3X2_CLK",
    L"DSS1_ALWON_FCLK",
    L"CAM_MCLK",
    L"EMUL_PER_ALWON_CLK",
    L"120M_FCLK",
    L"32K_FCLK",
    L"SYS_CLK",
    L"SYS_ALTCLK",
    L"SECURE_32K_FCLK",
    L"MCBSP_CLKS",
    L"USBTLL_SAR_FCLK",
    L"USBHOST_SAR_FCLK",
    L"EFUSE_ALWON_FCLK",
    L"SR_ALWON_FCLK",
    L"DPLL1_ALWON_FCLK",
    L"DPLL2_ALWON_FCLK",
    L"DPLL3_ALWON_FCLK",
    L"DPLL4_ALWON_FCLK",
    L"DPLL5_ALWON_FCLK",
    L"CM_SYS_CLK",
    L"DSS2_ALWON_FCLK",
    L"WKUP_L4_ICLK",
    L"CM_32K_CLK",
    L"CORE_32K_FCLK",
    L"WKUP_32K_FCLK",
    L"PER_32K_ALWON_FCLK",
    L"CORE_120M_FCLK",
    L"CM_96M_FCLK",
    L"96M_ALWON_FCLK",
    L"L3_ICLK",
    L"L4_ICLK",
    L"USB_L4_ICLK",
    L"RM_ICLK",
    L"DPLL1_FCLK",
    L"DPLL2_FCLK",
    L"CORE_L3_ICLK",
    L"CORE_L4_ICLK",
    L"SECURITY_L3_ICLK",
    L"SECURITY_L4_ICLK1",
    L"SECURITY_L4_ICLK2",
    L"SGX_L3_ICLK",
    L"SSI_L4_ICLK",
    L"DSS_L3_ICLK",
    L"DSS_L4_ICLK",
    L"CAM_L3_ICLK",
    L"CAM_L4_ICLK",
    L"USBHOST_L3_ICLK",
    L"USBHOST_L4_ICLK",
    L"PER_L4_ICLK",
    L"SR_L4_ICLK",
    L"SSI_SSR_FCLK",
    L"SSI_SST_FCLK",
    L"CORE_96M_FCLK",
    L"DSS_96M_FCLK",
    L"CSI2_96M_FCLK",
    L"CORE_48M_FCLK",
    L"USBHOST_48M_FCLK",
    L"PER_48M_FCLK",
    L"12M_FCLK",
    L"CORE_12M_FCLK",
    L"DSS_TV_FCLK",
    L"USBHOST_120M_FCLK",
    L"CM_USIM_CLK",
    L"USIM_FCLK",
    L"96M_FCLK",
    L"48M_FCLK",
    L"54M_FCLK",
    L"SGX_FCLK",
    L"GPT1_FCLK",
    L"GPT2_ALWON_FCLK",
    L"GPT3_ALWON_FCLK",
    L"GPT4_ALWON_FCLK",
    L"GPT5_ALWON_FCLK",
    L"GPT6_ALWON_FCLK",
    L"GPT7_ALWON_FCLK",
    L"GPT8_ALWON_FCLK",
    L"GPT9_ALWON_FCLK",
    L"GPT10_FCLK",
    L"GPT11_FCLK",
    NULL
};

WCHAR const* GetDeviceClockText(int i)
{
    static WCHAR const* szUnknown = L"(UKNOWN DEV. CLOCK)";

    WCHAR const *szResult = szUnknown;
    if (i < sizeof(_rgDeviceClockNames)/sizeof(WCHAR const*))
        szResult = _rgDeviceClockNames[i];

    return szResult;
}


UINT FindClockIdByName(WCHAR const *szName)
{
    int i = 0;
    while (_rgDeviceClockNames[i])
        {
        if (_tcsicmp(_rgDeviceClockNames[i], szName) == 0)
            {         
            return i;
            }
        ++i;
        }
    return -1;
}

//------------------------------------------------------------------------------

WCHAR const* _rgDpllClockNames[] =
{
    L"EXT_32LHZ",
    L"DPLL1_CLLOUT_M2X2",
    L"DPLL2_CLLOUT_M2",
    L"DPLL3_CLLOUT_M2",
    L"DPLL3_CLLOUT_M2X2",
    L"DPLL3_CLLOUT_M3X2",
    L"DPLL4_CLLOUT_M2X2",
    L"DPLL4_CLLOUT_M3X2",
    L"DPLL4_CLLOUT_M4X2",
    L"DPLL4_CLLOUT_M5X2",
    L"DPLL4_CLLOUT_M6X2",
    L"DPLL5_CLLOUT_M2",
    L"EXT_SYS_CLL",
    L"EXT_ALT",
    L"EXT_MISC",
    L"INT_OSC",
};

WCHAR const* GetDpllClockText(int i)
{
    static WCHAR const* szUnknown = L"(UKNOWN DPLL CLOCK)";

    WCHAR const * szResult = szUnknown;
    if (i < sizeof(_rgDpllClockNames)/sizeof(WCHAR const*))
        szResult = _rgDpllClockNames[i];

    return szResult;
}

//------------------------------------------------------------------------------

WCHAR const* _rgDpllNames[] =
{
    L"External",
    L"DPLL1",
    L"DPLL2",
    L"DPLL3",
    L"DPLL4",
    L"DPLL5"
};

WCHAR const* GetDpllText(int i)
{
    static WCHAR const* szUnknown = L"(UKNOWN DPLL)";

    WCHAR const * szResult = szUnknown;
    if (i < sizeof(_rgDpllNames)/sizeof(WCHAR const*))
        szResult = _rgDpllNames[i];

    return szResult;
}

//------------------------------------------------------------------------------

WCHAR const* _rgVddNames[] =
{
    {L"External"},
    {L"VDD1"},
    {L"VDD2"},
    {L"VDD3"},
    {L"VDD4"},
    {L"VDD5"},
    {L"VDDS"},
    {L"VDDPLL"},
    {L"VDDADAC"}
};

WCHAR const* GetVddText(int i)
{
    static WCHAR const* szUnknown = L"(UKNOWN Vdd)";

    WCHAR const * szResult = szUnknown;
    if (i < sizeof(_rgVddNames)/sizeof(WCHAR const*))
        szResult = _rgVddNames[i];

    return szResult;
}

//------------------------------------------------------------------------------

typedef struct {
    OMAP_DEVICE      devId;
    WCHAR const    *pszDevice;
} OMAP_CLOCK_MAP;

OMAP_CLOCK_MAP _rgOmapDeviceMap[] = {
    {OMAP_DEVICE_SSI, L"SSI"},
    {OMAP_DEVICE_SDRC, L"SDRC"},
    {OMAP_DEVICE_D2D, L"D2D"},
    {OMAP_DEVICE_HSOTGUSB, L"HSOTGUSB"},
    {OMAP_DEVICE_OMAPCTRL, L"OMAPCTRL"},
    {OMAP_DEVICE_MAILBOXES, L"MAILBOXES"},
    {OMAP_DEVICE_MCBSP1, L"MCBSP1"},
    {OMAP_DEVICE_MCBSP5, L"MCBSP5"},
    {OMAP_DEVICE_GPTIMER10, L"GPTIMER10"},
    {OMAP_DEVICE_GPTIMER11, L"GPTIMER11"},
    {OMAP_DEVICE_UART1, L"UART1"},
    {OMAP_DEVICE_UART2, L"UART2"},
    {OMAP_DEVICE_I2C1, L"I2C1"},
    {OMAP_DEVICE_I2C2, L"I2C2"},
    {OMAP_DEVICE_I2C3, L"I2C3"},
    {OMAP_DEVICE_MCSPI1, L"MCSPI1"},
    {OMAP_DEVICE_MCSPI2, L"MCSPI2"},
    {OMAP_DEVICE_MCSPI3, L"MCSPI3"},
    {OMAP_DEVICE_MCSPI4, L"MCSPI4"},
    {OMAP_DEVICE_HDQ, L"HDQ"},
    {OMAP_DEVICE_MSPRO, L"MSPRO"},
    {OMAP_DEVICE_MMC1, L"MMC1"},
    {OMAP_DEVICE_MMC2, L"MMC2"},
    {OMAP_DEVICE_MMC3, L"MMC3"},
    {OMAP_DEVICE_DES2, L"DES2"},
    {OMAP_DEVICE_SHA12, L"SHA12"},
    {OMAP_DEVICE_AES2, L"AES2"},
    {OMAP_DEVICE_ICR, L"ICR"},
    {OMAP_DEVICE_DES1, L"DES1"},
    {OMAP_DEVICE_SHA11, L"SHA11"},
    {OMAP_DEVICE_RNG, L"RNG"},
    {OMAP_DEVICE_AES1, L"AES1"},
    {OMAP_DEVICE_PKA, L"PKA"},
    {OMAP_DEVICE_USBTLL, L"USBTLL"},
    {OMAP_DEVICE_TS, L"TS"},
    {OMAP_DEVICE_EFUSE, L"EFULSE"},
    {OMAP_DEVICE_GPTIMER1, L"GPTIMER1"},
    {OMAP_DEVICE_GPTIMER12, L"GPTIMER12"},
    {OMAP_DEVICE_32KSYNC, L"32KSYNC"},
    {OMAP_DEVICE_WDT1, L"WDT1"},
    {OMAP_DEVICE_WDT2, L"WDT2"},
    {OMAP_DEVICE_GPIO1, L"GPIO1"},
    {OMAP_DEVICE_SR1, L"SR1"},
    {OMAP_DEVICE_SR2, L"SR2"},
    {OMAP_DEVICE_USIM, L"USIM"},
    {OMAP_DEVICE_GPIO2, L"GPIO2"}, 
	{OMAP_DEVICE_GPIO3, L"GPIO3"},
    {OMAP_DEVICE_GPIO4, L"GPIO4"},
    {OMAP_DEVICE_GPIO5, L"GPIO5"},
    {OMAP_DEVICE_GPIO6, L"GPIO6"},
    {OMAP_DEVICE_MCBSP2, L"MCBSP2"},
    {OMAP_DEVICE_MCBSP3, L"MCBSP3"},
    {OMAP_DEVICE_MCBSP4, L"MCBSP4"},
    {OMAP_DEVICE_GPTIMER2, L"GPTIMER2"},
    {OMAP_DEVICE_GPTIMER3, L"GPTIMER3"},
    {OMAP_DEVICE_GPTIMER4, L"GPTIMER4"},
    {OMAP_DEVICE_GPTIMER5, L"GPTIMER5"},
    {OMAP_DEVICE_GPTIMER6, L"GPTIMER6"},
    {OMAP_DEVICE_GPTIMER7, L"GPTIMER7"},
    {OMAP_DEVICE_GPTIMER8, L"GPTIMER8"},
    {OMAP_DEVICE_GPTIMER9, L"GPTIMER9"},
    {OMAP_DEVICE_UART3, L"UART3"},
    {OMAP_DEVICE_WDT3, L"WDT3"},
    {OMAP_DEVICE_DSS, L"DSS"},
    {OMAP_DEVICE_DSS1, L"DSS1"},
    {OMAP_DEVICE_DSS2, L"DSS2"},
    {OMAP_DEVICE_TVOUT, L"TVOUT"},
    {OMAP_DEVICE_CAMERA, L"CAMERA"},
    {OMAP_DEVICE_CSI2, L"CSI2"},
    {OMAP_DEVICE_DSP, L"DSP"},
    {OMAP_DEVICE_2D, L"2D"},
    {OMAP_DEVICE_3D, L"3D"},
    {OMAP_DEVICE_SGX, L"SGX"},
    {OMAP_DEVICE_USBHOST1, L"USBHOST1"},
    {OMAP_DEVICE_USBHOST2, L"USBHOST2"},
    {OMAP_DEVICE_USBHOST3, L"USBHOST3"}, 
    {OMAP_DEVICE_UART4, L"UART4"},
	{OMAP_DEVICE_NONE, NULL}
};

OMAP_DEVICE FindDeviceIdByName(WCHAR const *szName)
{
    int i = 0;
    while (_rgOmapDeviceMap[i].pszDevice)
        {
        if (_tcsicmp(_rgOmapDeviceMap[i].pszDevice, szName) == 0)
            {         
            return _rgOmapDeviceMap[i].devId;
            }
        ++i;
        }
    return OMAP_DEVICE_NONE;
}

WCHAR const* FindDeviceNameById(OMAP_DEVICE devId)
{
    for (int i = 0; _rgOmapDeviceMap[i].pszDevice; ++i)
        {
        if (_rgOmapDeviceMap[i].devId == devId)
            return _rgOmapDeviceMap[i].pszDevice;
        }
    return NULL;
}

//------------------------------------------------------------------------------

WCHAR const* _rgVddMap[] = {    
    L"VDD1",
    L"VDD2",
    L"VDD3",
    L"VDD4",
    L"VDD5",
    L"VDD_EXT",
    L"VDDS",
    L"VDDPLL",
    L"VDDADAC",
    L"MMC_VDDS",
    NULL    
};

WCHAR const* _rgDpllMap[] = {    
    L"DPLL1",
    L"DPLL2",
    L"DPLL3",
    L"DPLL4",
    L"DPLL5",
    L"DPLL_EXT",
    NULL     
};

WCHAR const* _rgDpllClkOutMap[] = {
    L"EXT_32KHZ",
    L"DPLL1_CLKOUT_M2X2",
    L"DPLL2_CLKOUT_M2",
    L"DPLL3_CLKOUT_M2",
    L"DPLL3_CLKOUT_M2X2",
    L"DPLL3_CLKOUT_M3X2",
    L"DPLL4_CLKOUT_M2X2",
    L"DPLL4_CLKOUT_M3X2",
    L"DPLL4_CLKOUT_M4X2",
    L"DPLL4_CLKOUT_M5X2",
    L"DPLL4_CLKOUT_M6X2",
    L"DPLL5_CLKOUT_M2",
    L"EXT_SYS_CLK",    
    L"EXT_ALT",
    L"EXT_MISC",
    L"INT_OSC",
    NULL     
};

WCHAR const** _rgClockMap[] =
{
    _rgDeviceClockNames,
    _rgDpllClkOutMap,
    _rgDpllMap,
    _rgVddMap
};

BOOL 
FindClockIdByName(
    WCHAR const *szName, 
    UINT *pClockId, 
    UINT *pLevel
    )
{    
    UINT clockId;
    UINT nLevel = 0;
    while (nLevel < 4)
        {
        clockId = 0;
        while (_rgClockMap[nLevel][clockId] != NULL)
            {
            if (wcsicmp(_rgClockMap[nLevel][clockId], szName) == 0)
                {
                *pClockId = clockId;
                *pLevel = nLevel + 1;
                return TRUE;
                }
            clockId++;
            }
        nLevel++;
        }
    
    return FALSE;
}

WCHAR const* 
FindClockNameById(
    UINT clockId, 
    UINT nLevel
    )
{
    return _rgClockMap[nLevel - 1][clockId];
}

//------------------------------------------------------------------------------

