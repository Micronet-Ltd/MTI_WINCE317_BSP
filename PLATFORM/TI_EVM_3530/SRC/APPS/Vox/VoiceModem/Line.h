/** =============================================================================
 *
 *  Copyright (c) 2011 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           H module template
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   7-Apr-2011
 *
 *  Revision Histiory:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

#ifndef _LINE_H
#define _LINE_H

/************** HEADER FILE INCLUDES ******************************************/
#include <windows.h>

/************** GLOBAL CONSTANTS **********************************************/
/************** STRUCTURES, ENUMS, AND TYPEDEFS *******************************/
/************** FUNCTION PROTOTYPES *******************************************/

#ifdef __cplusplus
	extern "C" {
#endif

 DWORD WINAPI LineThread(LPVOID lpParam);

#ifdef __cplusplus
		}
#endif	

/************** GLOBAL VARIABLES **********************************************/
/************** MACROS ********************************************************/



#endif /* _LINE_H */
