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
//
//  File: bq27000
//

#ifndef __BQ27000_H
#define __BQ27000_H

// Gas gauge registers
#define BQ_CTRL             0x00
#define BQ_MODE             0x01
#define BQ_ARL              0x02
#define BQ_ARH              0x03
#define BQ_ARTTEL           0x04
#define BQ_ARTTEH           0x05
#define BQ_TEMPL            0x06
#define BQ_TEMPH            0x07
#define BQ_VOLTL            0x08
#define BQ_VOLTH            0x09
#define BQ_FLAGS            0x0A
#define BQ_RSOC             0x0B
#define BQ_NACL             0x0C
#define BQ_NACH             0x0D
#define BQ_CACDL            0x0E
#define BQ_CACDH            0x0F
#define BQ_CACTL            0x10
#define BQ_CACTH            0x11
#define BQ_LMDL             0x12
#define BQ_LMDH             0x13
#define BQ_AIL              0x14
#define BQ_AIH              0x15
#define BQ_TTEL             0x16
#define BQ_TTEH             0x17
#define BQ_TTFL             0x18
#define BQ_TTFH             0x19
#define BQ_SIL              0x1A
#define BQ_SIH              0x1B
#define BQ_STTEL            0x1C
#define BQ_STTEH            0x1D
#define BQ_MLIL             0x1E
#define BQ_MLIH             0x1F
#define BQ_MLTTEL           0x20
#define BQ_MLTTEH           0x21
#define BQ_SAEL             0x22
#define BQ_SAEH             0x23
#define BQ_APL              0x24
#define BQ_APH              0x25
#define BQ_TTECPL           0x26
#define BQ_TTECPH           0x27
#define BQ_CYCLL            0x28
#define BQ_CYCLH            0x29
#define BQ_CYCTL            0x2A
#define BQ_CYCTH            0x2B
#define BQ_CSOC             0x2C

// 0x14-0x6D are reserved
#define BQ_EE_CMD           0x6E
#define BQ_EE_ILMD          0x76
#define BQ_EE_SEDVF         0x77
#define BQ_EE_SEDV1         0x78
#define BQ_EE_ISLC          0x79
#define BQ_EE_DMFSD         0x7A
#define BQ_EE_TAPER         0x7B
#define BQ_EE_PKCFG         0x7C
#define BQ_EE_ID3           0x7D
#define BQ_EE_DCOMP         0x7E
#define BQ_EE_TCOMP         0x7F

// BQ flags register bits
#define BQ_FLAGS_CHGS       (1 << 7)
#define BQ_FLAGS_NOACT      (1 << 6)
#define BQ_FLAGS_IMIN       (1 << 5)
#define BQ_FLAGS_CI         (1 << 4)
#define BQ_FLAGS_RSVD       (1 << 3)
#define BQ_FLAGS_VDQ        (1 << 2)
#define BQ_FLAGS_EDV1       (1 << 1)
#define BQ_FLAGS_EDVF       (1 << 0)

#endif
