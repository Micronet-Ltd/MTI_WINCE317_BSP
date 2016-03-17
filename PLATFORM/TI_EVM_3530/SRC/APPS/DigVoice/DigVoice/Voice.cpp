// Audio.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>

#include <mmsystem.h>
//#include <mmddk.h>
#include "Voice.h"

static HMIXER getMixerSourceLineControl(MIXERCONTROL *pMixerControl, MIXERCONTROLDETAILS *pMixerControlDetails, DWORD dwDestinationLine, DWORD dwSourceLine, DWORD dwControlType);

UINT ListMixerDevices(void);
BOOL InitMixerDev(HMIXER *m_hMixer, HWND hwnd);
void enumMixerLines(UINT mixDevNum);
BOOL turnVoiceOnOff(BOOL OnOff);



BOOL InitMixerDev(HMIXER *m_hMixer, HWND hwnd)
{
    MIXERCAPS mxcaps;
    ZeroMemory(&mxcaps, sizeof(MIXERCAPS));
    if (mixerGetNumDevs() != 0)
    {
        if (mixerOpen(m_hMixer,0,(DWORD)hwnd,NULL,MIXER_OBJECTF_MIXER | CALLBACK_WINDOW)!= MMSYSERR_NOERROR)
            return FALSE;
        //The mixerGetDevCaps function queries a specified mixer device to determine its capabilities
        if (mixerGetDevCaps((UINT)*m_hMixer, &mxcaps, sizeof(MIXERCAPS))!= MMSYSERR_NOERROR)
            return FALSE;
    }
    if (*m_hMixer == NULL) 
        return FALSE;
    return TRUE;
}

UINT ListMixerDevices(void)
{
    _tprintf(TEXT("Mixer Devices:\n"));

    UINT nMixerDevices = mixerGetNumDevs();
    UINT i;
    for (i = 0; i < nMixerDevices; i++) 
	{
        MIXERCAPS caps;
        MMRESULT mr = mixerGetDevCaps(i, &caps, sizeof(caps));
		if( mr == MMSYSERR_NOERROR )
		{
         //MRCHECK(mr, TEXT("mixerGetDevCaps"));
			_tprintf(TEXT("Device %s # %d, Mid = %X, Pid = %X, destLines = %d\n"), 
				          caps.szPname, i, caps.wMid, caps.wPid, caps.cDestinations );
		}
		else
		{
		 _tprintf(TEXT("%d: Device Error\n"), i);
		}
    }
    return nMixerDevices;
}



void enumMixerLines(UINT mixDevNum)
{
 HMIXER             m_hMixer;
 MIXERCAPS          mixercaps;
 MIXERLINE          mixerline;
 MIXERCONTROL       mixerControlArray[3];
 MIXERLINECONTROLS  mixerLineControls;
 MMRESULT      err;
 unsigned long i, n, numSrc;

 err = mixerOpen(&m_hMixer,0,0,NULL,MIXER_OBJECTF_MIXER | CALLBACK_WINDOW);

 mixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
 //mixerLineControls.cControls = 3;
 mixerLineControls.pamxctrl = &mixerControlArray[0];
 mixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);


	/* Get info about the first Mixer Device */
	if (!(err = mixerGetDevCaps((UINT)mixDevNum, &mixercaps, sizeof(MIXERCAPS))))
	{
		/* Print out the name of each destination line */
		for (i = 0; i < mixercaps.cDestinations; i++)
		{
			mixerline.cbStruct = sizeof(MIXERLINE);
			mixerline.dwDestination = i;

			if (!(err = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mixerline, MIXER_GETLINEINFOF_DESTINATION)))
			{
				_tprintf(TEXT("Destination #%lu = %s\n"), i, mixerline.szName);

				mixerLineControls.dwLineID = mixerline.dwLineID;
				mixerLineControls.cControls = mixerline.cControls;

				if ((err = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mixerLineControls, MIXER_GETLINECONTROLSF_ALL)))
                 {
                   /* An error */
                   _tprintf(TEXT("Error #%d calling mixerGetLineControls()\n"), err);
                 }

				// Enumerate all Controls


				/* Print out the name of each source line in this destination */
				numSrc = mixerline.cConnections;
				for (n = 0; n < numSrc; n++)
				{
					mixerline.cbStruct = sizeof(MIXERLINE);
					mixerline.dwDestination = i;
					mixerline.dwSource = n;

					if (!(err = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mixerline, MIXER_GETLINEINFOF_SOURCE)))
					{
						_tprintf(TEXT("\tSource #%lu = %s\n"), i, mixerline.szName);
					}
				}
			}
		}
	}

}


// Turn Audio Codec Voice On
BOOL turnVoiceOnOff(BOOL OnOff, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;
 DWORD                         dwDestinationLine;
 MMRESULT                      err;

// unsigned long i, n, numSrc;

 // BT_VOICE_DEST_LINE
 if( btOn )
  dwDestinationLine = BT_VOICE_DEST_LINE;
 else
  dwDestinationLine = VOICE_OUT_DEST_LINE;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, dwDestinationLine, MIXERR_INVALLINE, MIXERCONTROL_CONTROLTYPE_ONOFF);
 if( m_hMixer == NULL )
  return(FALSE);

 
 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);


 if( value.fValue != OnOff )
 {
   value.fValue = OnOff;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);

   // Verify
   err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);

   if( value.fValue != OnOff )
    return(FALSE);
 }

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

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, BT_PLAYBACK_DEST_LINE, MIXERR_INVALLINE, MIXERCONTROL_CONTROLTYPE_ONOFF);
 if( m_hMixer == NULL )
  return(FALSE);

/* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);


 if( value.fValue != OnOff )
 {
   value.fValue = OnOff;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);

   // Verify
   err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);

   if( value.fValue != OnOff )
    return(FALSE);
 }

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

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, EXT_AUDIO_DEST_LINE, MIXERR_INVALLINE, MIXERCONTROL_CONTROLTYPE_ONOFF);
 if( m_hMixer == NULL )
  return(FALSE);

/* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);


 if( value.fValue != OnOff )
 {
   value.fValue = OnOff;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);

   // Verify
   err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);

   if( value.fValue != OnOff )
    return(FALSE);
 }

 return(TRUE);
}



static HMIXER getMixerSourceLineControl(MIXERCONTROL *pMixerControl, MIXERCONTROLDETAILS *pMixerControlDetails, DWORD dwDestinationLine, DWORD dwSourceLine, DWORD dwControlType)
{
 HMIXER                        hMixer;
 MIXERCAPS                     mixercaps;
 MIXERLINE                     mixerline;
 MIXERLINECONTROLS             mixerLineControls;
 MMRESULT      err;


 err = mixerOpen(&hMixer,0,0,NULL,MIXER_OBJECTF_MIXER | CALLBACK_WINDOW);
 if( err != MMSYSERR_NOERROR )
	 return(NULL);

 err = mixerGetDevCaps(0, &mixercaps, sizeof(MIXERCAPS));
 if( err != MMSYSERR_NOERROR )
  return(NULL);

 if( dwDestinationLine >= mixercaps.cDestinations )
  return(NULL);

 mixerline.cbStruct = sizeof(MIXERLINE);
 mixerline.dwDestination = dwDestinationLine;
 err = mixerGetLineInfo((HMIXEROBJ)hMixer, &mixerline, MIXER_GETLINEINFOF_DESTINATION);
 if( err != MMSYSERR_NOERROR )
  return(NULL);

 if( dwSourceLine != MIXERR_INVALLINE )
 {
  if( mixerline.cConnections <= dwSourceLine )
   return(NULL);

  // get Source Line
  mixerline.dwSource = dwSourceLine;
  err = mixerGetLineInfo((HMIXEROBJ)hMixer, &mixerline, MIXER_GETLINEINFOF_SOURCE);
  if( err != MMSYSERR_NOERROR )
	 return(NULL);
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
  return(NULL);

// get Control detail, i.e. current status (switched On or Off)
 pMixerControlDetails->cbStruct = sizeof(MIXERCONTROLDETAILS);
 pMixerControlDetails->dwControlID = pMixerControl->dwControlID;
 /* This is always 1 for a MIXERCONTROL_CONTROLF_UNIFORM control */
 pMixerControlDetails->cChannels = 1;

 /* This is always 0 except for a MIXERCONTROL_CONTROLF_MULTIPLE control */
 pMixerControlDetails->cMultipleItems = 0;

 return(hMixer);
}


BOOL getVoiceInGain(INT16 *pVoiceInGain, INT16 *pVoiceInGainMin, INT16 *pVoiceInGainMax, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS_SIGNED    btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;


 if( !btOn )
 {
	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, INPUT_DEST_LINE, INPUT_VOICE_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_VOLUME);
	 if( m_hMixer == NULL )
	  return(FALSE);

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &value;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);

	  *pVoiceInGain =    (INT16)value.dwValue;
	  *pVoiceInGainMin = (INT16)mixerControl.Bounds.dwMinimum;
	  *pVoiceInGainMax = (INT16)mixerControl.Bounds.dwMaximum;
 }
 else
 {
	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, BT_VOICE_DEST_LINE, BTDIN_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_DECIBELS);
	 if( m_hMixer == NULL )
	  return(FALSE);

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);

	  *pVoiceInGain =    (INT16)btvalue.lValue;
	  *pVoiceInGainMin = (INT16)mixerControl.Bounds.dwMinimum;
	  *pVoiceInGainMax = (INT16)mixerControl.Bounds.dwMaximum;
 }

 return(TRUE);
}


BOOL getBTVoiceInMute(BOOL *pMute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_BOOLEAN   btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;

	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, BT_VOICE_DEST_LINE, BTDIN_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_MUTE);
	 if( m_hMixer == NULL )
	  return(FALSE);

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);

	 *pMute = btvalue.fValue;

	 return(TRUE);
}


BOOL putVoiceInGain(INT16 voiceOutGain, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS_SIGNED    btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MMRESULT      err;
 //unsigned long i, n, numSrc;

 if( !btOn )
 {
	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, INPUT_DEST_LINE, INPUT_VOICE_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_VOLUME);
	 if( m_hMixer == NULL )
	  return(FALSE);

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &value;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);

	 if( ( voiceOutGain <= mixerControl.Bounds.lMaximum) && ( voiceOutGain >= mixerControl.Bounds.lMinimum) )
	 {
	   value.dwValue = voiceOutGain;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		return(FALSE);
	 }
	 else
	  return(FALSE);
 }
 else
 {
	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, BT_VOICE_DEST_LINE, BTDIN_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_DECIBELS);
	 if( m_hMixer == NULL )
	  return(FALSE);

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);

	 if( (voiceOutGain <= mixerControl.Bounds.lMaximum) && ( voiceOutGain >= mixerControl.Bounds.lMinimum) )
	 {
	   btvalue.lValue = voiceOutGain;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		return(FALSE);
	 }
	 else
	  return(FALSE);

 }
// *pVoiceOutGain = value.dwValue;

 return(TRUE);
}


BOOL setBTVoiceInMute(BOOL mute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_BOOLEAN   btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;

	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, BT_VOICE_DEST_LINE, BTDIN_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_MUTE);
	 if( m_hMixer == NULL )
	  return(FALSE);

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);


	 if( (mute <= (BOOL)mixerControl.Bounds.dwMaximum) && ( mute >= (BOOL)mixerControl.Bounds.dwMinimum) )
	 {
	   btvalue.fValue = mute;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		return(FALSE);
	 }
	 else
	  return(FALSE);

	 return(TRUE);
}





BOOL getVoiceOutGain(INT16 *pVoiceOutGain, INT16 *pVoiceOutGainMin, INT16 *pVoiceOutGainMax, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS_SIGNED    btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

 if( !btOn )
 {
	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, VOICE_OUT_DEST_LINE, MIXERR_INVALLINE, MIXERCONTROL_CONTROLTYPE_VOLUME);
	 if( m_hMixer == NULL )
	  return(FALSE);


	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &value;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);

	  *pVoiceOutGain    = (INT16)value.dwValue;
	  *pVoiceOutGainMin = (INT16)mixerControl.Bounds.dwMinimum;
	  *pVoiceOutGainMax = (INT16)mixerControl.Bounds.dwMaximum;
 }
 else
 {
	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, BT_VOICE_DEST_LINE, BTDOUT_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_DECIBELS);
	 if( m_hMixer == NULL )
	  return(FALSE);


	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);

	  *pVoiceOutGain    = (INT16)btvalue.lValue;
	  *pVoiceOutGainMin = (INT16)mixerControl.Bounds.dwMinimum;
	  *pVoiceOutGainMax = (INT16)mixerControl.Bounds.dwMaximum;

 }

  return(TRUE);
}



BOOL getBTPlaybackOutGain(INT16 *pBTPlaybackOutGain, INT16 *pBTPlaybackOutGainMin, INT16 *pBTPlaybackOutGainMax)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, BT_PLAYBACK_DEST_LINE, BTPLBOUT_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_DECIBELS);
	 if( m_hMixer == NULL )
	  return(FALSE);


	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);

	  *pBTPlaybackOutGain    = (INT16)btvalue.lValue;
	  *pBTPlaybackOutGainMin = (INT16)mixerControl.Bounds.dwMinimum;
	  *pBTPlaybackOutGainMax = (INT16)mixerControl.Bounds.dwMaximum;

  return(TRUE);
}


BOOL getBTVoiceOutMute(BOOL *pMute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_BOOLEAN   btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;

	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, BT_VOICE_DEST_LINE, BTDOUT_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_MUTE);
	 if( m_hMixer == NULL )
	  return(FALSE);

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);

	 *pMute = btvalue.fValue;

	 return(TRUE);
}



BOOL getBTPlaybackOutMute(BOOL *pMute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_BOOLEAN   btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;

	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, BT_PLAYBACK_DEST_LINE, BTPLBOUT_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_MUTE);
	 if( m_hMixer == NULL )
	  return(FALSE);

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);

	 *pMute = btvalue.fValue;

	 return(TRUE);
}


BOOL putVoiceOutGain(INT16 voiceOutGain, BOOL btOn)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS_SIGNED    btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MMRESULT      err;


 if( !btOn )
 {
	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, VOICE_OUT_DEST_LINE, MIXERR_INVALLINE, MIXERCONTROL_CONTROLTYPE_VOLUME);
	 if( m_hMixer == NULL )
	  return(FALSE);

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &value;

	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);

	if( (voiceOutGain <= mixerControl.Bounds.lMaximum) && ( voiceOutGain >= mixerControl.Bounds.lMinimum) )
	 {
	   value.dwValue = voiceOutGain;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		return(FALSE);
	 }
	 else
	  return(FALSE);
 }
 else
 {
	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, BT_VOICE_DEST_LINE, BTDOUT_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_DECIBELS);
	 if( m_hMixer == NULL )
	  return(FALSE);

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);

	if( (voiceOutGain <= mixerControl.Bounds.lMaximum) && ( voiceOutGain >= mixerControl.Bounds.lMinimum) )
	 {
	   btvalue.lValue = voiceOutGain;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		return(FALSE);
	 }
	 else
	  return(FALSE);

 }

 return(TRUE);
}



BOOL putBTPlaybackOutGain(INT16 btPlaybackOutGain)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MMRESULT      err;


 	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, BT_PLAYBACK_DEST_LINE, BTPLBOUT_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_DECIBELS);
	 if( m_hMixer == NULL )
	  return(FALSE);

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);

	if( (btPlaybackOutGain <= mixerControl.Bounds.lMaximum) && ( btPlaybackOutGain >= mixerControl.Bounds.lMinimum) )
	 {
	   btvalue.lValue = btPlaybackOutGain;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		return(FALSE);
	 }
	 else
	  return(FALSE);

  return(TRUE);
}

BOOL setBTVoiceOutMute(BOOL mute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_BOOLEAN   btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;

	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, BT_VOICE_DEST_LINE, BTDOUT_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_MUTE);
	 if( m_hMixer == NULL )
	  return(FALSE);

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);


	 if( (mute <= (BOOL)mixerControl.Bounds.dwMaximum) && ( mute >= (BOOL)mixerControl.Bounds.dwMinimum) )
	 {
	   btvalue.fValue = mute;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		return(FALSE);
	 }
	 else
	  return(FALSE);

	 return(TRUE);
}


BOOL setBTPlaybackOutMute(BOOL mute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROLDETAILS_BOOLEAN   btvalue;
 MIXERCONTROLDETAILS           mixerControlDetails;
 MIXERCONTROL                  mixerControl;
 MMRESULT                      err;

	 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, BT_PLAYBACK_DEST_LINE, BTPLBOUT_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_MUTE);
	 if( m_hMixer == NULL )
	  return(FALSE);

	 /* Give mixerGetControlDetails() the address of the
	   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
	 mixerControlDetails.paDetails = &btvalue;

	 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
	 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

	 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	 if( err != MMSYSERR_NOERROR )
	  return(FALSE);


	 if( (mute <= (BOOL)mixerControl.Bounds.dwMaximum) && ( mute >= (BOOL)mixerControl.Bounds.dwMinimum) )
	 {
	   btvalue.fValue = mute;
	   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
	   if( err != MMSYSERR_NOERROR )
		return(FALSE);
	 }
	 else
	  return(FALSE);

	 return(TRUE);
}


BOOL getMicMute(BOOL *pMicMute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, INPUT_DEST_LINE, MIC_MUTE_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_MUTE);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

  *pMicMute = value.fValue;

 return(TRUE);

}


BOOL getExtMicMute(BOOL *pMicMute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, EXT_AUDIO_DEST_LINE, EXT_MIC_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_MUTE);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

  *pMicMute = value.fValue;

 return(TRUE);

}


BOOL setMicMute(BOOL micMute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, INPUT_DEST_LINE, MIC_MUTE_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_MUTE);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 if( (micMute <= (BOOL)mixerControl.Bounds.dwMaximum) && ( micMute >= (BOOL)mixerControl.Bounds.dwMinimum) )
 {
   value.fValue = micMute;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);
 }
 else
  return(FALSE);


 return(TRUE);

}


BOOL setExtMicMute(BOOL micMute)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_BOOLEAN   value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, EXT_AUDIO_DEST_LINE , EXT_MIC_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_MUTE);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 if( (micMute <= (BOOL)mixerControl.Bounds.dwMaximum) && ( micMute >= (BOOL)mixerControl.Bounds.dwMinimum) )
 {
   value.fValue = micMute;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);
 }
 else
  return(FALSE);


 return(TRUE);

}


#if 0
BOOL oldgetMicAnalogGain(UINT16 *pMicAnalogGain, UINT16 *pMicAnalogGainMin, UINT16 *pMicAnalogGainMax)
{
 HMIXER                        m_hMixer;
 MIXERCAPS                     mixercaps;
 MIXERLINE                     mixerline;
 MIXERCONTROL                  mixerControl;
 MIXERLINECONTROLS             mixerLineControls;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
// unsigned long i, n, numSrc;

  err = mixerOpen(&m_hMixer,0,0,NULL,MIXER_OBJECTF_MIXER | CALLBACK_WINDOW);
 if( err != MMSYSERR_NOERROR )
	 return(FALSE);
 
 err = mixerGetDevCaps(0, &mixercaps, sizeof(MIXERCAPS));
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 // 3 destination lines in Audio Codec 
 if( mixercaps.cDestinations != CODEC_DESTINATION_LINES )
  return(FALSE);

 mixerline.cbStruct = sizeof(MIXERLINE);
 mixerline.dwDestination = INPUT_DEST_LINE;
 err = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mixerline, MIXER_GETLINEINFOF_DESTINATION);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 // Get Source line 'INPUT_VOICE_SOURCE_LINE'
 if( mixerline.cConnections != INPUT_SOURCE_LINES )
   return(FALSE);

 // get Source Line
 mixerline.dwSource = MIC_MUTE_SOURCE_LINE;
 err = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mixerline, MIXER_GETLINEINFOF_SOURCE);


 mixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
 mixerLineControls.pamxctrl = &mixerControl;
 mixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);
 mixerLineControls.dwLineID = mixerline.dwLineID;
 /* We want to fetch info on only 1 control */
 mixerLineControls.cControls = 1;
 // Get ON_OFF control only
 mixerLineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_DECIBELS;

 err = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mixerLineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 // get Control detail, i.e. current status (switched On or Off)
 mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
 mixerControlDetails.dwControlID = mixerControl.dwControlID;
 /* This is always 1 for a MIXERCONTROL_CONTROLF_UNIFORM control */
 mixerControlDetails.cChannels = 1;

 /* This is always 0 except for a MIXERCONTROL_CONTROLF_MULTIPLE control */
 mixerControlDetails.cMultipleItems = 0;

 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

  *pMicAnalogGain = (UINT16)value.lValue;
  *pMicAnalogGainMin = (UINT16)mixerControl.Bounds.dwMinimum;
  *pMicAnalogGainMax = (UINT16)mixerControl.Bounds.dwMaximum;

 return(TRUE);

}
#endif

BOOL getMicAnalogGain(UINT16 *pMicAnalogGain, UINT16 *pMicAnalogGainMin, UINT16 *pMicAnalogGainMax)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, INPUT_DEST_LINE, MIC_MUTE_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_DECIBELS);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

  *pMicAnalogGain = (UINT16)value.lValue;
  *pMicAnalogGainMin = (UINT16)mixerControl.Bounds.dwMinimum;
  *pMicAnalogGainMax = (UINT16)mixerControl.Bounds.dwMaximum;

 return(TRUE);

}


BOOL getExtMicDigitalGain(INT16 *pMicDigitalGain, INT16 *pMicDigitalGainMin, INT16 *pMicDigitalGainMax)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, EXT_AUDIO_DEST_LINE, EXT_MIC_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_DECIBELS);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

  *pMicDigitalGain = (INT16)value.lValue;
  *pMicDigitalGainMin = (INT16)mixerControl.Bounds.dwMinimum;
  *pMicDigitalGainMax = (INT16)mixerControl.Bounds.dwMaximum;

 return(TRUE);

}


#if 0
BOOL oldputMicAnalogGain(UINT16 micAnalogGain)
{
 HMIXER                        m_hMixer;
 MIXERCAPS                     mixercaps;
 MIXERLINE                     mixerline;
 MIXERCONTROL                  mixerControl;
 MIXERLINECONTROLS             mixerLineControls;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
// unsigned long i, n, numSrc;

  err = mixerOpen(&m_hMixer,0,0,NULL,MIXER_OBJECTF_MIXER | CALLBACK_WINDOW);
 if( err != MMSYSERR_NOERROR )
	 return(FALSE);
 
 err = mixerGetDevCaps(0, &mixercaps, sizeof(MIXERCAPS));
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 // 3 destination lines in Audio Codec 
 if( mixercaps.cDestinations != CODEC_DESTINATION_LINES )
  return(FALSE);

 mixerline.cbStruct = sizeof(MIXERLINE);
 mixerline.dwDestination = INPUT_DEST_LINE;
 err = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mixerline, MIXER_GETLINEINFOF_DESTINATION);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 // Get Source line 'INPUT_VOICE_SOURCE_LINE'
 if( mixerline.cConnections != INPUT_SOURCE_LINES )
   return(FALSE);

 // get Source Line
 mixerline.dwSource = MIC_MUTE_SOURCE_LINE;
 err = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mixerline, MIXER_GETLINEINFOF_SOURCE);


 mixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
 mixerLineControls.pamxctrl = &mixerControl;
 mixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);
 mixerLineControls.dwLineID = mixerline.dwLineID;
 /* We want to fetch info on only 1 control */
 mixerLineControls.cControls = 1;
 // Get ON_OFF control only
 mixerLineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_DECIBELS;

 err = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mixerLineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 // get Control detail, i.e. current status (switched On or Off)
 mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
 mixerControlDetails.dwControlID = mixerControl.dwControlID;
 /* This is always 1 for a MIXERCONTROL_CONTROLF_UNIFORM control */
 mixerControlDetails.cChannels = 1;

 /* This is always 0 except for a MIXERCONTROL_CONTROLF_MULTIPLE control */
 mixerControlDetails.cMultipleItems = 0;

 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

if( (micAnalogGain <= mixerControl.Bounds.dwMaximum) && ( micAnalogGain >= mixerControl.Bounds.dwMinimum) )
 {
   value.lValue = micAnalogGain;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);
 }
 else
  return(FALSE);

 return(TRUE);

}
#endif


BOOL putMicAnalogGain(UINT16 micAnalogGain)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, INPUT_DEST_LINE, MIC_MUTE_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_DECIBELS);
 if( m_hMixer == NULL )
  return(FALSE);

 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

if( (micAnalogGain <= mixerControl.Bounds.dwMaximum) && ( micAnalogGain >= mixerControl.Bounds.dwMinimum) )
 {
   value.lValue = micAnalogGain;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);
 }
 else
  return(FALSE);

 return(TRUE);

}


BOOL putExtMicDigitalGain(INT16 micDigitalGain)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_SIGNED    value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, EXT_AUDIO_DEST_LINE, EXT_MIC_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_DECIBELS);
 if( m_hMixer == NULL )
  return(FALSE);

 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

if( (micDigitalGain <= mixerControl.Bounds.lMaximum) && ( micDigitalGain >= mixerControl.Bounds.lMinimum) )
 {
   value.lValue = micDigitalGain;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);
 }
 else
  return(FALSE);

 return(TRUE);

}


#if 0
BOOL oldgetMicDigitalGain(UINT16 *pMicDigitalGain, UINT16 *pMicDigitalGainMin, UINT16 *pMicDigitalGainMax)
{
 HMIXER                        m_hMixer;
 MIXERCAPS                     mixercaps;
 MIXERLINE                     mixerline;
 MIXERCONTROL                  mixerControl;
 MIXERLINECONTROLS             mixerLineControls;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
// unsigned long i, n, numSrc;

  err = mixerOpen(&m_hMixer,0,0,NULL,MIXER_OBJECTF_MIXER | CALLBACK_WINDOW);
 if( err != MMSYSERR_NOERROR )
	 return(FALSE);
 
 err = mixerGetDevCaps(0, &mixercaps, sizeof(MIXERCAPS));
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 // 3 destination lines in Audio Codec 
 if( mixercaps.cDestinations != CODEC_DESTINATION_LINES )
  return(FALSE);

 mixerline.cbStruct = sizeof(MIXERLINE);
 mixerline.dwDestination = INPUT_DEST_LINE;
 err = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mixerline, MIXER_GETLINEINFOF_DESTINATION);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 // Get Source line 'INPUT_VOICE_SOURCE_LINE'
 if( mixerline.cConnections != INPUT_SOURCE_LINES )
   return(FALSE);

 // get Source Line
 mixerline.dwSource = MIC_MUTE_SOURCE_LINE;
 err = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mixerline, MIXER_GETLINEINFOF_SOURCE);


 mixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
 mixerLineControls.pamxctrl = &mixerControl;
 mixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);
 mixerLineControls.dwLineID = mixerline.dwLineID;
 /* We want to fetch info on only 1 control */
 mixerLineControls.cControls = 1;
 // Get ON_OFF control only
 mixerLineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;

 err = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mixerLineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 // get Control detail, i.e. current status (switched On or Off)
 mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
 mixerControlDetails.dwControlID = mixerControl.dwControlID;
 /* This is always 1 for a MIXERCONTROL_CONTROLF_UNIFORM control */
 mixerControlDetails.cChannels = 1;

 /* This is always 0 except for a MIXERCONTROL_CONTROLF_MULTIPLE control */
 mixerControlDetails.cMultipleItems = 0;

 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

  *pMicDigitalGain    = (UINT16)value.dwValue;
  *pMicDigitalGainMin = (UINT16)mixerControl.Bounds.dwMinimum;
  *pMicDigitalGainMax = (UINT16)mixerControl.Bounds.dwMaximum;

 return(TRUE);

}
#endif


BOOL getMicDigitalGain(UINT16 *pMicDigitalGain, UINT16 *pMicDigitalGainMin, UINT16 *pMicDigitalGainMax)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, INPUT_DEST_LINE, MIC_MUTE_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_VOLUME);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

  *pMicDigitalGain    = (UINT16)value.dwValue;
  *pMicDigitalGainMin = (UINT16)mixerControl.Bounds.dwMinimum;
  *pMicDigitalGainMax = (UINT16)mixerControl.Bounds.dwMaximum;

 return(TRUE);

}


BOOL getExtMicAnalogGain(UINT16 *pMicAnalogGain, UINT16 *pMicAnalogGainMin, UINT16 *pMicAnalogGainMax)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, EXT_AUDIO_DEST_LINE, EXT_MIC_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_VOLUME);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

  *pMicAnalogGain    = (UINT16)value.dwValue;
  *pMicAnalogGainMin = (UINT16)mixerControl.Bounds.dwMinimum;
  *pMicAnalogGainMax = (UINT16)mixerControl.Bounds.dwMaximum;

 return(TRUE);

}


#if 0
BOOL oldputMicDigitalGain(UINT16 micDigitalGain)
{
 HMIXER                        m_hMixer;
 MIXERCAPS                     mixercaps;
 MIXERLINE                     mixerline;
 MIXERCONTROL                  mixerControl;
 MIXERLINECONTROLS             mixerLineControls;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
// unsigned long i, n, numSrc;

  err = mixerOpen(&m_hMixer,0,0,NULL,MIXER_OBJECTF_MIXER | CALLBACK_WINDOW);
 if( err != MMSYSERR_NOERROR )
	 return(FALSE);
 
 err = mixerGetDevCaps(0, &mixercaps, sizeof(MIXERCAPS));
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 // 3 destination lines in Audio Codec 
 if( mixercaps.cDestinations != CODEC_DESTINATION_LINES )
  return(FALSE);

 mixerline.cbStruct = sizeof(MIXERLINE);
 mixerline.dwDestination = INPUT_DEST_LINE;
 err = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mixerline, MIXER_GETLINEINFOF_DESTINATION);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 // Get Source line 'INPUT_VOICE_SOURCE_LINE'
 if( mixerline.cConnections != INPUT_SOURCE_LINES )
   return(FALSE);

 // get Source Line
 mixerline.dwSource = MIC_MUTE_SOURCE_LINE;
 err = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mixerline, MIXER_GETLINEINFOF_SOURCE);


 mixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
 mixerLineControls.pamxctrl = &mixerControl;
 mixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);
 mixerLineControls.dwLineID = mixerline.dwLineID;
 /* We want to fetch info on only 1 control */
 mixerLineControls.cControls = 1;
 // Get ON_OFF control only
 mixerLineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;

 err = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mixerLineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

 // get Control detail, i.e. current status (switched On or Off)
 mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
 mixerControlDetails.dwControlID = mixerControl.dwControlID;
 /* This is always 1 for a MIXERCONTROL_CONTROLF_UNIFORM control */
 mixerControlDetails.cChannels = 1;

 /* This is always 0 except for a MIXERCONTROL_CONTROLF_MULTIPLE control */
 mixerControlDetails.cMultipleItems = 0;

 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

if( (micDigitalGain <= mixerControl.Bounds.dwMaximum) && ( micDigitalGain >= mixerControl.Bounds.dwMinimum) )
 {
   value.dwValue = micDigitalGain;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);
 }
 else
  return(FALSE);

 return(TRUE);

}
#endif


BOOL putMicDigitalGain(UINT16 micDigitalGain)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, INPUT_DEST_LINE, MIC_MUTE_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_VOLUME);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

if( (micDigitalGain <= mixerControl.Bounds.dwMaximum) && ( micDigitalGain >= mixerControl.Bounds.dwMinimum) )
 {
   value.dwValue = micDigitalGain;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);
 }
 else
  return(FALSE);

 return(TRUE);

}



BOOL putExtMicAnalogGain(UINT16 micAnalogGain)
{
 HMIXER                        m_hMixer;
 MIXERCONTROL                  mixerControl;
 MIXERCONTROLDETAILS_UNSIGNED  value;
 MIXERCONTROLDETAILS           mixerControlDetails;

 MMRESULT      err;
 //unsigned long i, n, numSrc;

 m_hMixer = getMixerSourceLineControl(&mixerControl, &mixerControlDetails, EXT_AUDIO_DEST_LINE, EXT_MIC_SOURCE_LINE, MIXERCONTROL_CONTROLTYPE_VOLUME);
 if( m_hMixer == NULL )
  return(FALSE);


 /* Give mixerGetControlDetails() the address of the
   MIXERCONTROLDETAILS_UNSIGNED struct into which to return the value */
 mixerControlDetails.paDetails = &value;

 /* Tell mixerGetControlDetails() how big the MIXERCONTROLDETAILS_BOOLEAN is */
 mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

 err = mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
 if( err != MMSYSERR_NOERROR )
  return(FALSE);

if( (micAnalogGain <= mixerControl.Bounds.dwMaximum) && ( micAnalogGain >= mixerControl.Bounds.dwMinimum) )
 {
   value.dwValue = micAnalogGain;
   err = mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   if( err != MMSYSERR_NOERROR )
    return(FALSE);
 }
 else
  return(FALSE);

 return(TRUE);

}
