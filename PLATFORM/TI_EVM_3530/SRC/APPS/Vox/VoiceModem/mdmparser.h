#ifndef _MDM_PARSER_H_
#define _MDM_PARSER_H_

#include <windows.h>
#include "Line.h"
#include "Modem.h"

//BOOL findToken(const char *cStr);

#ifdef __cplusplus
	extern "C" {
#endif

  char *findEvent(char *str, VMODEM_EVENTS *pModemEvent, VMODEM_STATES *pModemState);

 #ifdef __cplusplus
		}
 #endif	

#endif //_MDM_PARSER_H_