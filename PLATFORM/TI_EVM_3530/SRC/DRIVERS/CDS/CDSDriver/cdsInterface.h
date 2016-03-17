#ifndef __CDSINTERFACE_H
#define __CDSINTERFACE_H

#include <windows.h>

#define IOCTL_CPUDYNAMICSPEED_GETSTATUS     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CPUDYNAMICSPEED_SET_SPEED     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CPUDYNAMICSPEED_GET_SPEED     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0802, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define OPM_1GHz   4
#define OPM_800MHz 3
#define OPM_600MHz 2

/* This is the mean value of each entry in AM37x technical reference manual table 13-11 */
int BgaptsTsAdcToTemp[128] =  {
-40 ,-40 ,-40 ,-40 ,-40 ,-40 ,-40 ,-40 ,-40 ,-40 ,-40 ,-40 ,-40 , //0	12
-39 ,-37 ,-35 ,-33 ,-31 ,-29 ,-27 ,-25 ,-23 ,-21 ,-19 ,-18 ,-16 , //13	25
-14 ,-13 ,-11 ,-9  ,-7  ,-6  ,-4  ,-2  ,-1  ,1   ,3   ,4   ,6   , //26	38
7   ,9   ,11  ,13  ,14  ,16  ,18  ,20  ,22  ,24  ,26  ,28  ,29  , //39	51
31  ,33  ,34  ,36  ,38  ,39  ,41  ,43  ,44  ,46  ,48  ,49  ,51  , //52	64
53  ,54  ,56  ,58  ,59  ,61  ,63  ,65  ,67  ,69  ,71  ,73  ,74  , //65	77
76  ,78  ,79  ,81  ,83  ,84  ,86  ,88  ,89  ,91  ,93  ,94  ,96  , //78	90
98  ,99  ,101 ,103 ,105 ,106 ,108 ,110 ,112 ,114 ,116 ,118 ,120 , //91	103
121 ,123 ,125 ,125 ,125 ,125 ,125 ,125 ,125 ,125 ,125 ,125 ,125 , //104	116
125 ,125 ,125 ,125 ,125 ,125 ,125 ,125 ,125 ,125 ,125             //117	127
};

#endif __CDS_H
