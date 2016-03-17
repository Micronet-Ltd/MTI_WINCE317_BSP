//#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include <string.h>

//#include "Modem.h"
//#include "Line.h"
#include "mdmparser.h"



#define  PARSER_DBG_PRN           0

typedef struct _VMODEM_RESPONSE
{
    const char      *pszResponse;
	size_t          lencmp;
    VMODEM_EVENTS   mdmRespCode;

} VMODEM_RESPONSE, *PVMODEMRESPONSE;


const VMODEM_RESPONSE mdmEvTable[] =
{
	// modem responses events

    { "OK",             2, VMODEM_OK },
    { "RING",           4, VMODEM_RING},
    { "NO CARRIER",     10, VMODEM_NO_CARRIER },
	{ "ERROR",          5, VMODEM_ERROR },
	{ "NO DIAL TONE",   12, VMODEM_NO_DIALTONE },
	{ "BUSY",           4, VMODEM_BUSY },
	{ "NO ANSWER",      9, VMODEM_NO_ANSWER },
	{ "+CLCC:",         6, (VMODEM_EVENTS)-1 },

	// modem commands events
	{ "ATD",            3, VMODEM_DIAL },
	{ "ATH",            3, VMODEM_HANGUP },
	{ "ATA",            3, VMODEM_ANSWER },
	{ "AT+CLCC",        7, VMODEM_CURRENT_CALL }

};

#define MDM_RESPONSE_TABLE_SIZE (sizeof(mdmEvTable)/sizeof(VMODEM_RESPONSE))


char seps[]   = "\r\n";
char *token, *next_token;



static VMODEM_EVENTS mdmParse(const char *mdmStr, VMODEM_STATES *pModemState);
static VMODEM_STATES mdmParseCLCC(const char *mdmStr);

static int dummyFunct(int param);

char *findEvent(char *str, VMODEM_EVENTS *pModemEvent, VMODEM_STATES *pModemState)
{
   VMODEM_EVENTS  ev;
   VMODEM_STATES  st;
    
   if( ( str == NULL )||(pModemEvent == NULL)||(pModemState == NULL) )
    return(NULL);
    

  // Establish string and get the first token: 
    token = strtok_s( str, seps, &next_token);
	if( token != NULL )
	{
	  ev = mdmParse(token, &st);
	  *pModemEvent = ev;
	  *pModemState = st;
	}
	else
     return(NULL);


  return(next_token);
}


static VMODEM_EVENTS mdmParse(const char *mdmStr, VMODEM_STATES *pModemState)
{
  int i;

    *pModemState = (VMODEM_STATES)-1;

    for( i=0; i<MDM_RESPONSE_TABLE_SIZE; i++ )
    {
        if( !strncmp( mdmStr, mdmEvTable[i].pszResponse, mdmEvTable[i].lencmp) ) 
        {
            // Found a match.
			if( mdmEvTable[i].mdmRespCode != -1 )
			{
              return( mdmEvTable[i].mdmRespCode );
			}
			else
			{
             if( !strncmp( mdmStr, "+CLCC:", mdmEvTable[i].lencmp ) )
			 {
               *pModemState =  mdmParseCLCC(mdmStr);
               return( (VMODEM_EVENTS)-1 );
			 }
			}
        }
    }

  return((VMODEM_EVENTS)-1);
}


// at+CLCC[\r][\n][\r][\n]+CLCC: 1,1,4,0,0,"035582962",129,""[\r][\n][\r][\n]OK[\r][\n]
// at+clcc[\r][\n][\r][\n]+CLCC: 1,0,3,0,0,"089418170",129,""[\r][\n][\r][\n]OK[\r][\n]
// at+clcc[\r][\n][\r][\n]+CLCC: 1,0,0,0,0,"0544677181",129,""[\r][\n][\r][\n]OK[\r][\n]
static VMODEM_STATES mdmParseCLCC(const char *mdmStr)
{
 //char *pStr;
 int  i, cnt, len;
 VMODEM_STATES  st;
 
 RETAILMSG(PARSER_DBG_PRN, (L"DIGVOICE:+%S, (%S)\r\n",__FUNCTION__, mdmStr));

 len = strlen(mdmStr);

	 cnt = 0;
	 // Search for <stat>, 3rd digit
	 for( i = 0; i < len; i++ )
	 {
	   if( mdmStr[i] == ',') 
		cnt++;

	   if( cnt == 3 )
		break;
	 }

	 if( cnt < 3 )
       return((VMODEM_STATES)-1);

	 i--;

     switch(mdmStr[i])
	 {
	  case  '0': st = VMODEM_CONNECTED; break;
      case  '2': st = VMODEM_DIALING;  break;
      case  '4': st = VMODEM_RINGING; break;
	  default:  st = (VMODEM_STATES)-1; break;
	 }

  //i = dummyFunct(i);

  RETAILMSG(1, (L"DIGVOICE:-%S, state = %d\r\n",__FUNCTION__, st));

  return(st);
}


static int dummyFunct(int param)
{
 return(param++);
}