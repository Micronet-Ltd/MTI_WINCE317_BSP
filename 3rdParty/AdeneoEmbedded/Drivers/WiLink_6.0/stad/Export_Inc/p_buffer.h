/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/
#ifndef P_BUFFER
#define P_BUFFER


#define P_BUFFER_ADD_UINT8(_p_buffer, _uint8)                               \
        {                                                                   \
            *(TI_UINT8 *)(_p_buffer++) = _uint8;                            \
        }

#define P_BUFFER_ADD_UINT16(_p_buffer, _uint16)                             \
        {                                                                   \
            *(TI_UINT8 *)(_p_buffer++) = (TI_UINT8)(_uint16 & 0x00FF);                \
            *(TI_UINT8 *)(_p_buffer++) = (TI_UINT8)((_uint16 & 0xFF00) >> 8);         \
        }

#define P_BUFFER_ADD_UINT32(_p_buffer, _uint32)                             \
        {                                                                   \
            *(TI_UINT8 *)(_p_buffer++) = (TI_UINT8)(_uint32 & 0x000000FF);            \
            *(TI_UINT8 *)(_p_buffer++) = (TI_UINT8)((_uint32 & 0x0000FF00) >> 8);     \
            *(TI_UINT8 *)(_p_buffer++) = (TI_UINT8)((_uint32 & 0x00FF0000) >> 16);    \
            *(TI_UINT8 *)(_p_buffer++) = (TI_UINT8)((_uint32 & 0xFF000000) >> 24);    \
        }

#define P_BUFFER_ADD_DATA(_p_buffer, _p_data, _len)                         \
        {                                                                   \
            memcpy(_p_buffer, _p_data, _len);                               \
            _p_buffer += _len;                                              \
        }         

#define P_BUFFER_GET_UINT8(_p_buffer, _uint8)                               \
        {                                                                   \
            _uint8 = *(TI_UINT8 *)(_p_buffer++);                            \
        }

#define P_BUFFER_GET_UINT16(_p_buffer, _uint16)                             \
        {                                                                   \
            _uint16 = *(TI_UINT8 *)(_p_buffer++);                           \
            _uint16 |= (*(TI_UINT8 *)(_p_buffer++) << 8);                   \
        }


#define P_BUFFER_GET_UINT32(_p_buffer, _uint32)                             \
        {                                                                   \
            _uint32 = *(TI_UINT8 *)(_p_buffer++);                           \
            _uint32 |= (*(TI_UINT8 *)(_p_buffer++) << 8);                   \
            _uint32 |= (*(TI_UINT8 *)(_p_buffer++) << 16);                  \
            _uint32 |= (*(TI_UINT8 *)(_p_buffer++) << 24);                  \
        }

#define P_BUFFER_ADD_HDR_PARAMS(_p_buffer, _op, _status)                    \
        {                                                                   \
            *(TI_UINT8 *)(_p_buffer + 0) = (_op & 0x00FF);                  \
            *(TI_UINT8 *)(_p_buffer + 1) = ((_op & 0xFF00) >> 8);           \
            *(TI_UINT8 *)(_p_buffer + 2) = _status;                         \
            _p_buffer += 3;                                                 \
        }

#endif
