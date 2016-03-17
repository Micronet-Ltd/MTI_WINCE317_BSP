// Audio.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>

#include <mmsystem.h>
//#include <mmddk.h>
#include "Mixer.h"


MIXER_DST_LINE_s  *g_pMixerCfg = NULL;


// --------------------- Static functions ------------------------------

static void enumListControls(HMIXER mix, MIXERLINE * ml, MIXER_CONTROL_s **pMixControl)
{
    MMRESULT error;
    MIXERLINECONTROLS controls;
    MIXERCONTROL *control;
	MIXER_CONTROL_s  *pMixCtrl;

	if( pMixControl == NULL )
     return;

    controls.cbStruct = sizeof(MIXERLINECONTROLS);
    controls.dwLineID = ml->dwLineID;  // from MIXERLINE
    controls.cControls = ml->cControls;// from MIXERLINE
    controls.cbmxctrl = sizeof(MIXERCONTROL);

    control = (MIXERCONTROL *)LocalAlloc(LPTR, sizeof(MIXERCONTROL) * ml->cControls );
	if( control == NULL )
     return;

    controls.pamxctrl = control;

	controls.dwControlType = 0;
    error = mixerGetLineControls((HMIXEROBJ)mix, &controls, MIXER_GETLINECONTROLSF_ALL);

	if( ml->cControls == 0 )
      return;

    if (error == MMSYSERR_NOERROR)
    {
        DWORD c;

		//dtprint(TEXT("//\r\n//\t   Num of Controls: %d\r\n//\r\n"), ml->cControls);

		*pMixControl = (MIXER_CONTROL_s *)LocalAlloc(LPTR, sizeof(MIXER_CONTROL_s) );
		pMixCtrl = *pMixControl;

        for (c = 0; c < ml->cControls; c++)
        {
         pMixCtrl->pszControlName = (TCHAR *)LocalAlloc(LPTR, sizeof(control[c].szName) );
		 memcpy(pMixCtrl->pszControlName, control[c].szName, sizeof(control[c].szName) );

		 pMixCtrl->pszShortControlName = (TCHAR *)LocalAlloc(LPTR, sizeof(control[c].szShortName) );
		 memcpy(pMixCtrl->pszShortControlName, control[c].szShortName, sizeof(control[c].szShortName) );

		 
		 if( c < ml->cControls-1 )
		  {
           pMixCtrl->pNextMixControl = (MIXER_CONTROL_s *)LocalAlloc(LPTR, sizeof(MIXER_CONTROL_s) ); 
		   pMixCtrl = pMixCtrl->pNextMixControl;
		  }
		 else
		  {
           pMixCtrl->pNextMixControl = NULL;
		  }
        }
    }
    else
    {
		//dtprint(TEXT("//  Error %ld in mixerGetLineControls()\r\n"), error);
    }

    LocalFree(control);
}


static void deEnumListControls(MIXER_CONTROL_s **pMixControl)
{
 MIXER_CONTROL_s  *pMixCtrl;
 MIXER_CONTROL_s  *pMixCtrlPrev;

	 if( pMixControl == NULL )
       return;
	
	 if( *pMixControl == NULL )
       return;

     do 
	 {
         pMixCtrl = *pMixControl;
		 pMixCtrlPrev = NULL;

		 while( pMixCtrl->pNextMixControl != NULL )
		 {
		  pMixCtrlPrev = pMixCtrl;
		  pMixCtrl = pMixCtrl->pNextMixControl;
		 }

		 LocalFree(pMixCtrl->pszControlName);
		 LocalFree(pMixCtrl->pszShortControlName);
		 if( pMixCtrlPrev != NULL )
		 {
		  pMixCtrlPrev->pNextMixControl = NULL;
		  LocalFree(pMixCtrl);
		 }
		 else
		 {
		   LocalFree(pMixCtrl);
		   pMixCtrl = NULL;
		 }

	 } while(pMixCtrl != NULL);

  *pMixControl = NULL;

  return;
}

static void deEnumSrcLine(MIXER_SRC_LINE_s **pMixSourceLine)
{
  MIXER_SRC_LINE_s  *pMixSrcLine;
  MIXER_SRC_LINE_s  *pMixSrcLinePrev;

  if( pMixSourceLine == NULL )
   return;

  if( *pMixSourceLine == NULL )
   return;

     do 
	 {
         pMixSrcLine = *pMixSourceLine;
		 pMixSrcLinePrev = NULL;

		 while( pMixSrcLine->pNextMixSrcLine != NULL )
		 {
		  pMixSrcLinePrev = pMixSrcLine;
		  pMixSrcLine = pMixSrcLine->pNextMixSrcLine;
		 }

		 LocalFree(pMixSrcLine->pszLineName);
		 LocalFree(pMixSrcLine->pszShortLineName);
		 // Release Controls
         deEnumListControls(&pMixSrcLine->pMixControl);

		 if( pMixSrcLinePrev != NULL )
		 {
		  pMixSrcLinePrev->pNextMixSrcLine = NULL;
		  LocalFree(pMixSrcLine);
		 }
		 else
		 {
		   LocalFree(pMixSrcLine);
		   pMixSrcLine = NULL;
		 }

	 } while(pMixSrcLine != NULL);

  *pMixSourceLine = NULL;
  return;

}


static void deEnumDstLine(MIXER_DST_LINE_s **pMixDestinationLine)
{
  MIXER_DST_LINE_s  *pMixDstLine;
  MIXER_DST_LINE_s  *pMixDstLinePrev;

  if( pMixDestinationLine == NULL )
   return;

  if( *pMixDestinationLine == NULL )
   return;

     do 
	 {
         pMixDstLine = *pMixDestinationLine;
		 pMixDstLinePrev = NULL;

		 while( pMixDstLine->pNextMixDstLine != NULL )
		 {
		  pMixDstLinePrev = pMixDstLine;
		  pMixDstLine = pMixDstLine->pNextMixDstLine;
		 }

		 LocalFree(pMixDstLine->pszLineName);
		 LocalFree(pMixDstLine->pszShortLineName);

		 // Release Controls
         deEnumListControls(&pMixDstLine->pMixControl);

		 // Release Source Lines
		 deEnumSrcLine(&pMixDstLine->pMixSrcLine);

		 if( pMixDstLinePrev != NULL )
		 {
		  pMixDstLinePrev->pNextMixDstLine = NULL;
		  LocalFree(pMixDstLine);
		 }
		 else
		 {
		   LocalFree(pMixDstLine);
		   pMixDstLine = NULL;
		 }

	 } while(pMixDstLine != NULL);

  *pMixDestinationLine = NULL;
  return;

}

static BOOL  verifyMixerLine(MIXER_DST_LINE_s **pMixerCfg, TCHAR *pszDstLine, TCHAR *pszSrcLine, DWORD *pDstLine, DWORD *pSrcLine)
{
 MIXER_DST_LINE_s  *pMixDstLine;
 MIXER_SRC_LINE_s  *pMixSrcLine;
 BOOL              bFound;

 if( pMixerCfg == NULL )
    return(FALSE);

 if( *pMixerCfg == NULL )
    return(FALSE);

 if( pszDstLine == NULL )
  return(FALSE);


 // Look for Dst line
 pMixDstLine = *pMixerCfg;
 bFound = FALSE;

 *pDstLine = 0;
 while( pMixDstLine )
 {
   if( !_tcscmp( pMixDstLine->pszLineName, pszDstLine ) )
   {
    bFound = TRUE;
	break;
   }

  *pDstLine = *pDstLine + 1;
   pMixDstLine = pMixDstLine->pNextMixDstLine;
 }

 if( bFound == FALSE )
  return(FALSE);

 if( pszSrcLine == NULL )
 {
  *pSrcLine = MIXERR_INVALLINE;
  return(TRUE);
 }

 if( pszSrcLine[0] == 0 )
 {
  *pSrcLine = MIXERR_INVALLINE;
  return(TRUE);
 }

 // Look for Src line
 bFound = FALSE;
 pMixSrcLine = pMixDstLine->pMixSrcLine;
 
 *pSrcLine = 0;
 while( pMixSrcLine )
 {
   if( !_tcscmp( pMixSrcLine->pszLineName, pszSrcLine ) )
   {
	bFound = TRUE;
	break;
   }

  *pSrcLine = *pSrcLine + 1;
   pMixSrcLine = pMixSrcLine->pNextMixSrcLine;
 }

 if( bFound == FALSE )
 return(FALSE);

 return(TRUE);
}


static HMIXER getMixerSourceLineControl
(
 MIXERCONTROL *pMixerControl, 
 MIXERCONTROLDETAILS *pMixerControlDetails, 
 BOOL  *pbLineActive,
 TCHAR *pszDstLine, 
 TCHAR *pszSrcLine, 
 DWORD dwControlType
 )
{
 HMIXER                        hMixer;
 MIXERCAPS                     mixercaps;
 MIXERLINE                     mixerline;
 MIXERLINECONTROLS             mixerLineControls;
 MMRESULT                      err;
 DWORD                         dwDestinationLine;
 DWORD                         dwSourceLine;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 BOOL                          bRet; 

 bRet =  verifyMixerLine(&g_pMixerCfg, pszDstLine, pszSrcLine, &dwDestinationLine, &dwSourceLine);
 if( bRet == FALSE )
  return(NULL);

 err = mixerOpen(&hMixer,0,0,NULL,MIXER_OBJECTF_MIXER /*| CALLBACK_WINDOW*/);
 if( err != MMSYSERR_NOERROR )
	 return(NULL);

 err = mixerGetDevCaps(0, &mixercaps, sizeof(MIXERCAPS));
 if( err != MMSYSERR_NOERROR )
 {
   mixerClose(hMixer);
   return(NULL);
 }

 // DISCONNECTED lines not counted ?
 /*
 if( dwDestinationLine >= mixercaps.cDestinations )
 {
   mixerClose(hMixer);
   return(NULL);
 }
 */

 mixerline.cbStruct = sizeof(MIXERLINE);
 mixerline.dwDestination = dwDestinationLine;
 err = mixerGetLineInfo((HMIXEROBJ)hMixer, &mixerline, MIXER_GETLINEINFOF_DESTINATION);
 if( err != MMSYSERR_NOERROR )
 {
   mixerClose(hMixer);
   return(NULL);
 }

 if( mixerline.fdwLine == MIXERLINE_LINEF_DISCONNECTED )
   *pbLineActive = FALSE;
 else
   *pbLineActive = TRUE;

 if( dwSourceLine != MIXERR_INVALLINE )
 {
  if( mixerline.cConnections <= dwSourceLine )
 {
   mixerClose(hMixer);
   return(NULL);
 }

  // get Source Line
  mixerline.dwSource = dwSourceLine;
  err = mixerGetLineInfo((HMIXEROBJ)hMixer, &mixerline, MIXER_GETLINEINFOF_SOURCE);
  if( err != MMSYSERR_NOERROR )
   {
    mixerClose(hMixer);
    return(NULL);
   }
 }

 mixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
 mixerLineControls.pamxctrl = pMixerControl;
 mixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);
 mixerLineControls.dwLineID = mixerline.dwLineID;
 /* We want to fetch info on only 1 control */
 mixerLineControls.cControls = 1;
 // Get ON_OFF control only
 mixerLineControls.dwControlType = dwControlType;

 err = mixerGetLineControls((HMIXEROBJ)hMixer, &mixerLineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(hMixer);
   return(NULL);
  }


// get Control detail, i.e. current status (switched On or Off)
 pMixerControlDetails->cbStruct = sizeof(MIXERCONTROLDETAILS);
 pMixerControlDetails->dwControlID = pMixerControl->dwControlID;
 /* This is always 1 for a MIXERCONTROL_CONTROLF_UNIFORM control */
 pMixerControlDetails->cChannels = 1;

 /* This is always 0 except for a MIXERCONTROL_CONTROLF_MULTIPLE control */
 pMixerControlDetails->cMultipleItems = 0;

/* pMixerControlDetails->paDetails = &value;

 // Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is
 pMixerControlDetails->cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)hMixer, pMixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
 }
*/
 return(hMixer);
}



// --------------------- Public functions -------------------------------------

MIXER_ERROR_CODES enumMixer(void)
{
	HMIXER hMixer;
	HRESULT hr;
	MIXER_DST_LINE_s  *pMixerDstLine;
	MIXER_SRC_LINE_s  *pMixSrcLine;
	
	if( mixerGetNumDevs()== 0 )
	{
     //_tprintf(TEXT("Mixer not found\r\n"));
	 return(MIXER_DEVICE_ERR);
	}

	hr = mixerOpen(&hMixer, 0, 0, 0, 0);
	if(FAILED(hr)) 
	 return(MIXER_OPEN_ERR);


	MIXERLINE mxl;
	//MIXERLINECONTROLS mxlc;
	DWORD count, item=-1;

	UINT id = 0;
	MIXERCAPS caps;

	mixerGetID((HMIXEROBJ)hMixer, &id, MIXER_OBJECTF_HMIXER);
	mixerGetDevCaps(id, &caps, sizeof(MIXERCAPS));

	//dtprint(TEXT("\r\n//   Mixer name: \"%s\" (szPname of 'MIXERCAPS')\r\n"), caps.szPname);

	int nDestinations = caps.cDestinations;

	if( nDestinations == 0 )
     return(MIXER_DEVICE_ERR);

	pMixerDstLine = (MIXER_DST_LINE_s *)LocalAlloc(LPTR, sizeof(MIXER_DST_LINE_s) );
	g_pMixerCfg = pMixerDstLine;

	pMixerDstLine->pNextMixDstLine = NULL;

	for (int d=0; d<nDestinations; d++)
	{
		mxl.cbStruct = sizeof(mxl);
//			mxl.dwComponentType = ComponentType;

		mxl.dwDestination = d;

		// get the title of the destination
		hr = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl, MIXER_GETLINEINFOF_DESTINATION);

		if( hr != MMSYSERR_NOERROR )
		{
          return(MIXER_DEVICE_ERR);
		}


		//dtprint(TEXT("//\r\n//\r\n//\r\n"));
		//dtprint(TEXT("//DESTINATION line # %i - \"%s\" (\"%s\")\r\n"), d, mxl.szName, mxl.szShortName);
		// Print status of Destination Line
		//pMixerDstLine->lineStatus = mxl.fdwLine;

		pMixerDstLine->pszLineName = (TCHAR *)LocalAlloc(LPTR, sizeof(mxl.szName) );
		memcpy(pMixerDstLine->pszLineName, mxl.szName, sizeof(mxl.szName) );

		pMixerDstLine->pszShortLineName = (TCHAR *)LocalAlloc(LPTR, sizeof(mxl.szShortName) );
		memcpy(pMixerDstLine->pszShortLineName, mxl.szShortName, sizeof(mxl.szShortName) );

		pMixerDstLine->pMixControl = NULL;

        //// Michael !!!!  Destination Line can has its own controls too !!!
		enumListControls(hMixer, &mxl, &pMixerDstLine->pMixControl);

		count = mxl.cConnections;
		//if( count > 0 )
		// dtprint(TEXT("//\r\n//   Num of SOURCE LINES: %d\r\n"), count);

		if( count > 0 )
		{
         pMixSrcLine = (MIXER_SRC_LINE_s *)LocalAlloc(LPTR, sizeof(MIXER_SRC_LINE_s) );  
		 pMixerDstLine->pMixSrcLine = pMixSrcLine;
		}
		else
		{
         pMixerDstLine->pMixSrcLine = NULL; // no Src lines
		}

		for(UINT i = 0; i < count; i++)
		{
			mxl.dwSource = i;
			hr = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl, MIXER_GETLINEINFOF_SOURCE);
			if (hr)
				break;

			pMixSrcLine->pszLineName = (TCHAR *)LocalAlloc(LPTR, sizeof(mxl.szName) );
			memcpy(pMixSrcLine->pszLineName, mxl.szName, sizeof(mxl.szName) );

			pMixSrcLine->pszShortLineName = (TCHAR *)LocalAlloc(LPTR, sizeof(mxl.szShortName) );
		    memcpy(pMixSrcLine->pszShortLineName, mxl.szShortName, sizeof(mxl.szShortName) );

			pMixSrcLine->pMixControl = NULL;

			// !!!! Call as a separate function.
			// It should be used for Source and Destination lines as well
			enumListControls(hMixer, &mxl, &pMixSrcLine->pMixControl);

			if( i < count-1 )
			{
             pMixSrcLine->pNextMixSrcLine = (MIXER_SRC_LINE_s *)LocalAlloc(LPTR, sizeof(MIXER_SRC_LINE_s) );
			 pMixSrcLine = pMixSrcLine->pNextMixSrcLine;
			}
			else
			{
             pMixSrcLine->pNextMixSrcLine = NULL; 
			}
		}

		// Next Dst Line ...
		if( d < nDestinations-1 )
		{ 
          pMixerDstLine->pNextMixDstLine = (MIXER_DST_LINE_s *)LocalAlloc(LPTR, sizeof(MIXER_DST_LINE_s) ); 
		  pMixerDstLine = pMixerDstLine->pNextMixDstLine;
		}
		else 
		 pMixerDstLine->pNextMixDstLine = NULL;

	}

	mixerClose(hMixer);
    return(MIXER_OK);
}


MIXER_ERROR_CODES deEnumMixer()
{
  if( g_pMixerCfg == NULL )
    return(MIXER_INVALID_PARAM);

  deEnumDstLine(&g_pMixerCfg);

  return(MIXER_OK);
}


// Turn Audio Codec Voice On
BOOL turnVoiceOnOff(BOOL OnOff, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MMRESULT                      err;
 BOOL                          bLineActive;

// unsigned long i, n, numSrc;

 // BT_VOICE_DEST_LINE
 if( btOn )
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Voice Modem", NULL, MIXERCONTROL_CONTROLTYPE_ONOFF);
 else
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Voice Modem", NULL, MIXERCONTROL_CONTROLTYPE_ONOFF);

 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }
 
 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


 if( value.fValue != OnOff )
 {
   value.fValue = OnOff;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    {
     mixerClose(m_hMixer);
     return(FALSE);
    }

   // Verify
   err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    {
     mixerClose(m_hMixer);
     return(FALSE);
    }

   if( value.fValue != OnOff )
   {
    mixerClose(m_hMixer);
    return(FALSE);
   }
 }

 mixerClose(m_hMixer);
 return(TRUE);
}


// Turn Audio Codec Bluetoth Playback On/Off
BOOL turnBTPlbOnOff(BOOL OnOff)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MMRESULT                      err;
 BOOL                          bLineActive;


 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Playback", NULL, MIXERCONTROL_CONTROLTYPE_ONOFF);
 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }


/* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }



 if( value.fValue != OnOff )
 {
   value.fValue = OnOff;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
   {
    mixerClose(m_hMixer);
    return(FALSE);
   }

   // Verify
   err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
     {
      mixerClose(m_hMixer);
      return(FALSE);
     }


   if( value.fValue != OnOff )
     {
      mixerClose(m_hMixer);
      return(FALSE);
     }
 }

  mixerClose(m_hMixer);
  return(TRUE);
}


BOOL turnExtAudioOnOff(BOOL OnOff)
{
 // EXT_AUDIO_DEST_LINE
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MMRESULT                      err;
 BOOL                          bLineActive;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"External Audio", NULL, MIXERCONTROL_CONTROLTYPE_ONOFF);
 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

/* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
   {
    mixerClose(m_hMixer);
    return(FALSE);
   }



 if( value.fValue != OnOff )
 {
   value.fValue = OnOff;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
     {
      mixerClose(m_hMixer);
      return(FALSE);
     }


   // Verify
   err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
     {
      mixerClose(m_hMixer);
      return(FALSE);
     }


   if( value.fValue != OnOff )
    {
      mixerClose(m_hMixer);
      return(FALSE);
    }
 }

 mixerClose(m_hMixer);
 return(TRUE);
}


BOOL queryExtAudio(BOOL *pOnOff)
{
 // EXT_AUDIO_DEST_LINE
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MMRESULT                      err;
 BOOL                          bLineActive;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"External Audio", NULL, MIXERCONTROL_CONTROLTYPE_ONOFF);
 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

/* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
   {
    mixerClose(m_hMixer);
    return(FALSE);
   }


   // Query
   err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
     {
      mixerClose(m_hMixer);
      return(FALSE);
     }

 *pOnOff = value.fValue;

 mixerClose(m_hMixer);
 return(TRUE);
}




BOOL getVoiceInGain(INT16 *pVoiceInGain, INT16 *pVoiceInGainMin, INT16 *pVoiceInGainMax, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;
 BOOL                          bLineActive;


 if( !btOn )
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Voice Modem", L"Voice Input", MIXERCONTROL_CONTROLTYPE_DECIBELS);
 }
 else
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Voice Modem", L"Input from BT", MIXERCONTROL_CONTROLTYPE_DECIBELS);
 }

 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }


	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &value;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  {
       mixerClose(m_hMixer);
       return(FALSE);
      }


	  *pVoiceInGain =    (INT16)value.lValue;
	  *pVoiceInGainMin = (INT16)mixerControl.Bounds.dwMinimum;
	  *pVoiceInGainMax = (INT16)mixerControl.Bounds.dwMaximum;

 mixerClose(m_hMixer);
 return(TRUE);
}


BOOL putVoiceInGain(INT16 voiceOutGain, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MMRESULT                      err;
 BOOL                          bLineActive;

 if( !btOn )
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Voice Modem", L"Voice Input", MIXERCONTROL_CONTROLTYPE_DECIBELS);
 }
 else
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Voice Modem", L"Input from BT", MIXERCONTROL_CONTROLTYPE_DECIBELS);
 }

 if( m_hMixer == NULL )
  return(FALSE);

   if( !bLineActive )
   {
    mixerClose(m_hMixer);
    return(FALSE);
   }


	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &value;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  {
       mixerClose(m_hMixer);
       return(FALSE);
      }


	 if( ( voiceOutGain <= mixerControl.Bounds.lMaximum) && ( voiceOutGain >= mixerControl.Bounds.lMinimum) )
	 {
	   value.lValue = voiceOutGain;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		{
         mixerClose(m_hMixer);
         return(FALSE);
        }
	 }
	 else
	 {
      mixerClose(m_hMixer);
      return(FALSE);
     }

 mixerClose(m_hMixer);
 return(TRUE);
}


BOOL getVoiceInMute(BOOL *pMute, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;
 BOOL                          bLineActive;

 if( !btOn )
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Voice Modem", L"Voice Input", MIXERCONTROL_CONTROLTYPE_MUTE);
 }
 else
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Voice Modem", L"Input from BT", MIXERCONTROL_CONTROLTYPE_MUTE);
 }


 if( m_hMixer == NULL )
  return(FALSE);

    if( !bLineActive )
    {
     mixerClose(m_hMixer);
     return(FALSE);
    }


	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &value;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  {
       mixerClose(m_hMixer);
       return(FALSE);
      }

	 *pMute = value.fValue;

   mixerClose(m_hMixer);
   return(TRUE);
}




BOOL setVoiceInMute(BOOL mute, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;
 BOOL                          bLineActive;

 if( !btOn )
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Voice Modem", L"Voice Input", MIXERCONTROL_CONTROLTYPE_MUTE);
 }
 else
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Voice Modem", L"Input from BT", MIXERCONTROL_CONTROLTYPE_MUTE);
 }

 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &value;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  {
       mixerClose(m_hMixer);
       return(FALSE);
      }



	 if( (mute <= (BOOL)mixerControl.Bounds.dwMaximum) && ( mute >= (BOOL)mixerControl.Bounds.dwMinimum) )
	 {
	   value.fValue = mute;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		{
         mixerClose(m_hMixer);
         return(FALSE);
        }

	 }
	 else
	  {
       mixerClose(m_hMixer);
       return(FALSE);
      }


   mixerClose(m_hMixer);
   return(TRUE);
}



BOOL getVoiceOutGain(INT16 *pVoiceOutGain, INT16 *pVoiceOutGainMin, INT16 *pVoiceOutGainMax, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MMRESULT                      err;
 BOOL                          bLineActive;


 if( !btOn )
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Voice Modem", L"Voice Output", MIXERCONTROL_CONTROLTYPE_DECIBELS);
 }
 else
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Voice Modem", L"Output from Codec", MIXERCONTROL_CONTROLTYPE_DECIBELS);
 }

 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }


	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &value;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  {
       mixerClose(m_hMixer);
       return(FALSE);
      }


	  *pVoiceOutGain    = (INT16)value.lValue;
	  *pVoiceOutGainMin = (INT16)mixerControl.Bounds.dwMinimum;
	  *pVoiceOutGainMax = (INT16)mixerControl.Bounds.dwMaximum;

  mixerClose(m_hMixer);
  return(TRUE);
}


BOOL putVoiceOutGain(INT16 voiceOutGain, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MMRESULT                      err;
 BOOL                          bLineActive;

 if( !btOn )
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Voice Modem", L"Voice Output", MIXERCONTROL_CONTROLTYPE_DECIBELS);
 }
 else
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Voice Modem", L"Output from Codec", MIXERCONTROL_CONTROLTYPE_DECIBELS);
 }

 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &value;

	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  {
       mixerClose(m_hMixer);
       return(FALSE);
      }


	if( (voiceOutGain <= mixerControl.Bounds.lMaximum) && ( voiceOutGain >= mixerControl.Bounds.lMinimum) )
	 {
	   value.lValue = voiceOutGain;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		{
         mixerClose(m_hMixer);
         return(FALSE);
        }

	 }
	 else
	 {
       mixerClose(m_hMixer);
       return(FALSE);
     }

 mixerClose(m_hMixer);
 return(TRUE);
}


BOOL getVoiceOutMute(BOOL *pMute, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;
 BOOL                          bLineActive;

 if( !btOn )
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Voice Modem", L"Voice Output", MIXERCONTROL_CONTROLTYPE_MUTE);
 }
 else
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Voice Modem", L"Output from Codec", MIXERCONTROL_CONTROLTYPE_MUTE);
 }

 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }


	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &value;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  {
       mixerClose(m_hMixer);
       return(FALSE);
      }


  *pMute = value.fValue;

  mixerClose(m_hMixer);
  return(TRUE);
}


BOOL setVoiceOutMute(BOOL mute, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;
 BOOL                          bLineActive;

 if( !btOn )
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Voice Modem", L"Voice Output", MIXERCONTROL_CONTROLTYPE_MUTE);
 }
 else
 {
  m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Voice Modem", L"Output from Codec", MIXERCONTROL_CONTROLTYPE_MUTE);
 }

 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &value;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  {
       mixerClose(m_hMixer);
       return(FALSE);
      }



	 if( (mute <= (BOOL)mixerControl.Bounds.dwMaximum) && ( mute >= (BOOL)mixerControl.Bounds.dwMinimum) )
	 {
	   value.fValue = mute;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		{
         mixerClose(m_hMixer);
         return(FALSE);
        }

	 }
	 else
	 {
      mixerClose(m_hMixer);
      return(FALSE);
     }

  mixerClose(m_hMixer);
  return(TRUE);
}



BOOL getBTPlaybackOutGain(INT16 *pBTPlaybackOutGain, INT16 *pBTPlaybackOutGainMin, INT16 *pBTPlaybackOutGainMax)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;


	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Playback", L"BT playback from Codec", MIXERCONTROL_CONTROLTYPE_DECIBELS);
	 if( m_hMixer == NULL )
	  return(FALSE);


 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  {
       mixerClose(m_hMixer);
       return(FALSE);
      }


	  *pBTPlaybackOutGain    = (INT16)btvalue.lValue;
	  *pBTPlaybackOutGainMin = (INT16)mixerControl.Bounds.dwMinimum;
	  *pBTPlaybackOutGainMax = (INT16)mixerControl.Bounds.dwMaximum;

  mixerClose(m_hMixer);
  return(TRUE);
}




BOOL getBTPlaybackOutMute(BOOL *pMute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_BOOLEAN   btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;
 BOOL                          bLineActive;

	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Playback", L"BT playback from Codec", MIXERCONTROL_CONTROLTYPE_MUTE);
	 if( m_hMixer == NULL )
	  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  {
       mixerClose(m_hMixer);
       return(FALSE);
      }


  *pMute = btvalue.fValue;

  mixerClose(m_hMixer);
  return(TRUE);
}




BOOL putBTPlaybackOutGain(INT16 btPlaybackOutGain)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MMRESULT                      err;
 BOOL                          bLineActive;


 	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Playback", L"BT playback from Codec", MIXERCONTROL_CONTROLTYPE_DECIBELS);
	 if( m_hMixer == NULL )
	  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  {
       mixerClose(m_hMixer);
       return(FALSE);
      }


	if( (btPlaybackOutGain <= mixerControl.Bounds.lMaximum) && ( btPlaybackOutGain >= mixerControl.Bounds.lMinimum) )
	 {
	   btvalue.lValue = btPlaybackOutGain;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		{
         mixerClose(m_hMixer);
         return(FALSE);
        }
	 }
	 else
	 {
       mixerClose(m_hMixer);
       return(FALSE);
     }

  mixerClose(m_hMixer);
  return(TRUE);
}



BOOL setBTPlaybackOutMute(BOOL mute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_BOOLEAN   btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;
 BOOL                          bLineActive;

	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Bluetooth Playback", L"BT playback from Codec", MIXERCONTROL_CONTROLTYPE_MUTE);
	 if( m_hMixer == NULL )
	  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  {
       mixerClose(m_hMixer);
       return(FALSE);
      }



	 if( (mute <= (BOOL)mixerControl.Bounds.dwMaximum) && ( mute >= (BOOL)mixerControl.Bounds.dwMinimum) )
	 {
	   btvalue.fValue = mute;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		{
         mixerClose(m_hMixer);
         return(FALSE);
        }
	 }
	 else
	 {
      mixerClose(m_hMixer);
      return(FALSE);
     }

  mixerClose(m_hMixer);
  return(TRUE);
}



BOOL getMicMute(BOOL *pMicMute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;
 

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Recorder Volume", L"Microphone", MIXERCONTROL_CONTROLTYPE_MUTE);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
 }


  *pMicMute = value.fValue;

   mixerClose(m_hMixer);
   return(TRUE);
}


BOOL getExtMicMute(BOOL *pMicMute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;
 

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"External Audio", L"External Microphone", MIXERCONTROL_CONTROLTYPE_MUTE);
 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
 }


  *pMicMute = value.fValue;

   mixerClose(m_hMixer);
   return(TRUE);
}


BOOL setMicMute(BOOL micMute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;


 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Recorder Volume", L"Microphone", MIXERCONTROL_CONTROLTYPE_MUTE);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


 if( (micMute <= (BOOL)mixerControl.Bounds.dwMaximum) && ( micMute >= (BOOL)mixerControl.Bounds.dwMinimum) )
 {
   value.fValue = micMute;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    {
     mixerClose(m_hMixer);
     return(FALSE);
    }
 }
 else
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

  mixerClose(m_hMixer);
  return(TRUE);
}


BOOL setExtMicMute(BOOL micMute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;
 

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"External Audio" , L"External Microphone", MIXERCONTROL_CONTROLTYPE_MUTE);
 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
 }


 if( (micMute <= (BOOL)mixerControl.Bounds.dwMaximum) && ( micMute >= (BOOL)mixerControl.Bounds.dwMinimum) )
 {
   value.fValue = micMute;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    {
     mixerClose(m_hMixer);
     return(FALSE);
    }
 }
 else
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

  mixerClose(m_hMixer);
  return(TRUE);
}



BOOL setSpkMute(BOOL mute)
{
HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;


 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Master Volume", NULL, MIXERCONTROL_CONTROLTYPE_MUTE);
 if( m_hMixer == NULL )
  return(FALSE);

if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


 if( (mute <= (BOOL)mixerControl.Bounds.dwMaximum) && ( mute >= (BOOL)mixerControl.Bounds.dwMinimum) )
 {
   value.fValue = mute;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    {
     mixerClose(m_hMixer);
     return(FALSE);
    }
 }
 else
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

  mixerClose(m_hMixer);
  return(TRUE);
}


BOOL getSpkMute(BOOL *pMute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;


 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Master Volume", NULL, MIXERCONTROL_CONTROLTYPE_MUTE);
 if( m_hMixer == NULL )
  return(FALSE);

if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }

 *pMute = value.fValue;

  mixerClose(m_hMixer);
  return(TRUE);
}


#if 0
BOOL setSpkVol(UINT16 vol)
{
 HMIXER                         m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;


 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Master Volume", NULL, MIXERCONTROL_CONTROLTYPE_VOLUME);
 if( m_hMixer == NULL )
  return(FALSE);

if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


 if( (vol <= (BOOL)mixerControl.Bounds.dwMaximum) && ( vol >= (BOOL)mixerControl.Bounds.dwMinimum) )
 {
   value.dwValue = vol;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    {
     mixerClose(m_hMixer);
     return(FALSE);
    }
 }
 else
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

  mixerClose(m_hMixer);
  return(TRUE);
}


BOOL getSpkVol(UINT16 *pVol, UINT16 *pVolMin, UINT16 *pVolMax)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;


 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Master Volume", NULL, MIXERCONTROL_CONTROLTYPE_VOLUME);
 if( m_hMixer == NULL )
  return(FALSE);

if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


 *pVol    = (UINT16)value.dwValue;
 *pVolMin = (UINT16)mixerControl.Bounds.dwMinimum;
 *pVolMax = (UINT16)mixerControl.Bounds.dwMaximum;

  mixerClose(m_hMixer);
  return(TRUE);
}

BOOL getRecVol(UINT16 *pRecVol, UINT16 *pRecVolMin, UINT16 *pRecVolMax)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;
 

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Recorder Volume", NULL, MIXERCONTROL_CONTROLTYPE_VOLUME);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


  *pRecVol = (UINT16)value.dwValue;
  *pRecVolMin = (UINT16)mixerControl.Bounds.dwMinimum;
  *pRecVolMax = (UINT16)mixerControl.Bounds.dwMaximum;

  mixerClose(m_hMixer);
  return(TRUE);
}


BOOL setRecVol(UINT16 recVol)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;
 

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Recorder Volume", NULL, MIXERCONTROL_CONTROLTYPE_VOLUME);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


if( (recVol <= mixerControl.Bounds.dwMaximum) && ( recVol >= mixerControl.Bounds.dwMinimum) )
 {
   value.dwValue = recVol;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    {
     mixerClose(m_hMixer);
     return(FALSE);
    }
 }
 else
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

  mixerClose(m_hMixer);
  return(TRUE);
}

#endif /* 0 */

BOOL getMicAnalogGain(UINT16 *pMicAnalogGain, UINT16 *pMicAnalogGainMin, UINT16 *pMicAnalogGainMax)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;
 

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Recorder Volume", L"Microphone", MIXERCONTROL_CONTROLTYPE_DECIBELS);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


  *pMicAnalogGain = (UINT16)value.lValue;
  *pMicAnalogGainMin = (UINT16)mixerControl.Bounds.dwMinimum;
  *pMicAnalogGainMax = (UINT16)mixerControl.Bounds.dwMaximum;

  mixerClose(m_hMixer);
  return(TRUE);
}


BOOL getExtMicDigitalGain(INT16 *pMicDigitalGain, INT16 *pMicDigitalGainMin, INT16 *pMicDigitalGainMax)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;
 

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"External Audio", L"External Microphone", MIXERCONTROL_CONTROLTYPE_DECIBELS);
 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
 }


  *pMicDigitalGain = (INT16)value.lValue;
  *pMicDigitalGainMin = (INT16)mixerControl.Bounds.dwMinimum;
  *pMicDigitalGainMax = (INT16)mixerControl.Bounds.dwMaximum;

  mixerClose(m_hMixer);
  return(TRUE);
}




BOOL putMicAnalogGain(UINT16 micAnalogGain)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;
 

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Recorder Volume", L"Microphone", MIXERCONTROL_CONTROLTYPE_DECIBELS);
 if( m_hMixer == NULL )
  return(FALSE);

 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


if( (micAnalogGain <= mixerControl.Bounds.dwMaximum) && ( micAnalogGain >= mixerControl.Bounds.dwMinimum) )
 {
   value.lValue = micAnalogGain;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    {
     mixerClose(m_hMixer);
     return(FALSE);
    }
 }
 else
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

 mixerClose(m_hMixer);
 return(TRUE);
}


BOOL putExtMicDigitalGain(INT16 micDigitalGain)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;
 

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"External Audio", L"External Microphone", MIXERCONTROL_CONTROLTYPE_DECIBELS);
 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


if( (micDigitalGain <= mixerControl.Bounds.lMaximum) && ( micDigitalGain >= mixerControl.Bounds.lMinimum) )
 {
   value.lValue = micDigitalGain;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    {
     mixerClose(m_hMixer);
     return(FALSE);
    }
 }
 else
  {
   mixerClose(m_hMixer);
   return(FALSE);
 }

  mixerClose(m_hMixer);
  return(TRUE);
}




BOOL getMicDigitalGain(UINT16 *pMicDigitalGain, UINT16 *pMicDigitalGainMin, UINT16 *pMicDigitalGainMax)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;
 

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Recorder Volume", L"Microphone", MIXERCONTROL_CONTROLTYPE_VOLUME);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


  *pMicDigitalGain    = (UINT16)value.dwValue;
  *pMicDigitalGainMin = (UINT16)mixerControl.Bounds.dwMinimum;
  *pMicDigitalGainMax = (UINT16)mixerControl.Bounds.dwMaximum;

  mixerClose(m_hMixer);
  return(TRUE);
}


BOOL getExtMicAnalogGain(UINT16 *pMicAnalogGain, UINT16 *pMicAnalogGainMin, UINT16 *pMicAnalogGainMax)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;
 

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"External Audio", L"External Microphone", MIXERCONTROL_CONTROLTYPE_VOLUME);
 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


  *pMicAnalogGain    = (UINT16)value.dwValue;
  *pMicAnalogGainMin = (UINT16)mixerControl.Bounds.dwMinimum;
  *pMicAnalogGainMax = (UINT16)mixerControl.Bounds.dwMaximum;

  mixerClose(m_hMixer);
  return(TRUE);
}



BOOL putMicDigitalGain(UINT16 micDigitalGain)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;
 

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"Recorder Volume", L"Microphone", MIXERCONTROL_CONTROLTYPE_VOLUME);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


if( (micDigitalGain <= mixerControl.Bounds.dwMaximum) && ( micDigitalGain >= mixerControl.Bounds.dwMinimum) )
 {
   value.dwValue = micDigitalGain;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    {
     mixerClose(m_hMixer);
     return(FALSE);
    }
 }
 else
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

  mixerClose(m_hMixer);
  return(TRUE);
}



BOOL putExtMicAnalogGain(UINT16 micAnalogGain)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 BOOL                          bLineActive;
 MMRESULT                      err;
 

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, &bLineActive, L"External Audio", L"External Microphone", MIXERCONTROL_CONTROLTYPE_VOLUME);
 if( m_hMixer == NULL )
  return(FALSE);

 if( !bLineActive )
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  {
   mixerClose(m_hMixer);
   return(FALSE);
  }


if( (micAnalogGain <= mixerControl.Bounds.dwMaximum) && ( micAnalogGain >= mixerControl.Bounds.dwMinimum) )
 {
   value.dwValue = micAnalogGain;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    {
     mixerClose(m_hMixer);
     return(FALSE);
    }
 }
 else
 {
   mixerClose(m_hMixer);
   return(FALSE);
 }

 mixerClose(m_hMixer);
 return(TRUE);
}


//////////////////////////////////////////////////////////
// For provide callbacks

 HMIXER         s_hMixer = NULL;

HMIXER openMixer(HWND hwnd)
{
 MMRESULT		err;

 if( s_hMixer == NULL )
 {
  err = mixerOpen(&s_hMixer,0,(DWORD)hwnd,NULL, MIXER_OBJECTF_MIXER | CALLBACK_WINDOW);
  if( err != MMSYSERR_NOERROR )
	 return(NULL);
 }
 
 return(s_hMixer); 
}


void closeMixer(void)
{
	// Close any open mixer device
	if(s_hMixer != NULL )
	{
		mixerClose(s_hMixer);
		s_hMixer = NULL;
	}
}
