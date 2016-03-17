// mixerDump.cpp - console app
//
//#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include "Mixer.h"

//TCHAR   g_dumpfile[256] = TEXT("mixerDumpFile.txt");
FILE*	g_filePtr = NULL;

static TCHAR *GetTypeText(UINT dwControlType);


TCHAR ComponentTypesDst[MIXERLINE_COMPONENTTYPE_DST_LAST-MIXERLINE_COMPONENTTYPE_DST_FIRST+1][80] = {
	TEXT("MIXERLINE_COMPONENTTYPE_DST_UNDEFINED"),
	TEXT("MIXERLINE_COMPONENTTYPE_DST_DIGITAL"),
	TEXT("MIXERLINE_COMPONENTTYPE_DST_LINE"),
	TEXT("MIXERLINE_COMPONENTTYPE_DST_MONITOR"),
	TEXT("MIXERLINE_COMPONENTTYPE_DST_SPEAKERS"),
	TEXT("MIXERLINE_COMPONENTTYPE_DST_HEADPHONES"),
	TEXT("MIXERLINE_COMPONENTTYPE_DST_TELEPHONE"),
	TEXT("MIXERLINE_COMPONENTTYPE_DST_WAVEIN"),
	TEXT("MIXERLINE_COMPONENTTYPE_DST_VOICEIN") };

TCHAR ComponentTypesSrc[MIXERLINE_COMPONENTTYPE_SRC_LAST-MIXERLINE_COMPONENTTYPE_SRC_FIRST+1][80] = {
	TEXT("MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED"),
	TEXT("MIXERLINE_COMPONENTTYPE_SRC_DIGITAL"),
	TEXT("MIXERLINE_COMPONENTTYPE_SRC_LINE"),
	TEXT("MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE"),
	TEXT("MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER"),
	TEXT("MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC"),
	TEXT("MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE"),
	TEXT("MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER"),
	TEXT("MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT"),
	TEXT("MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY"),
	TEXT("MIXERLINE_COMPONENTTYPE_SRC_ANALOG") };


static void ListControls(HMIXER, MIXERLINE *);



static void dtprint(const TCHAR *pszFormat, ...)
{
   ///////////////////////////////////
   va_list arglist;
   int len;
   TCHAR *buffer;

   va_start( arglist, pszFormat );
   //len = _vscprintf( pszFormat, arglist ) // _vscprintf doesn't count
   //                           + 1; // terminating '\0'

   //len = _vsctprintf( pszFormat, arglist ) // _vscprintf doesn't count
   //	                              + 1; // terminating '\0'

   len = 1000;

  // buffer = malloc( len * sizeof(char) );
   //control = (MIXERCONTROL *)LocalAlloc(LPTR, sizeof(MIXERCONTROL) * ml->cControls);
   buffer = (TCHAR *)LocalAlloc(LPTR, sizeof(TCHAR) * len);

   //vsprintf_s( buffer, len, format, arglist );
   _vstprintf_s( buffer, len, pszFormat, arglist );

   //puts( buffer );
   _putts( buffer );
   //free( buffer );
   if( g_filePtr != NULL ) 
   _fputts(buffer, g_filePtr);

   LocalFree(buffer);

}



/* */
static void ListControls(HMIXER mix, MIXERLINE * ml)
{
    MMRESULT error;
    MIXERLINECONTROLS controls;
    MIXERCONTROL * control;

    controls.cbStruct = sizeof(MIXERLINECONTROLS);
    controls.dwLineID = ml->dwLineID;  // from MIXERLINE
    controls.cControls = ml->cControls;// from MIXERLINE
    controls.cbmxctrl = sizeof(MIXERCONTROL);

    control = (MIXERCONTROL *)LocalAlloc(LPTR, sizeof(MIXERCONTROL) * ml->cControls);

    if (NULL == control)
    {
		dtprint(TEXT("// Error %ld on LocalAlloc()\r\n"), GetLastError());
        return;
    }
    controls.pamxctrl = control;

	controls.dwControlType = 0;
    error = mixerGetLineControls((HMIXEROBJ)mix, &controls, MIXER_GETLINECONTROLSF_ALL);

    if (error == MMSYSERR_NOERROR)
    {
        DWORD c;

		dtprint(TEXT("//\r\n//\t   Num of Controls: %d\r\n//\r\n"), ml->cControls);

        for (c = 0; c < ml->cControls; c++)
        {
		 dtprint( TEXT("//\t\t\"%s\" (\"%s\") %x, %x \r\n//\t\t\"%s\" \"%s\"\r\n//\t\t%d  %d  %d\r\n//\r\n"),
				  control[c].szName, control[c].szShortName,
				  control[c].dwControlID, control[c].dwControlType,
				  GetTypeText(control[c].dwControlType),
				  control[c].fdwControl & MIXERCONTROL_CONTROLF_DISABLED?TEXT("[DISABLED]"):TEXT(""),
				  control[c].Bounds.dwMinimum, control[c].Bounds.dwMaximum, control[c].Metrics.cSteps
				 );
        }
    }
    else
    {
		dtprint(TEXT("//  Error %ld in mixerGetLineControls()\r\n"), error);
    }

    LocalFree(control);
}





MIXER_ERROR_CODES dumpMixer(LPCTSTR pszDumpfile)
{
	HMIXER hMixer;
	HRESULT hr;
	errno_t err;

    if( pszDumpfile == NULL )
	{
     _tprintf(TEXT("Dump File name error\r\n"));
	 return(MIXER_INVALID_PARAM);
	}

	if( mixerGetNumDevs()== 0 )
	{
     _tprintf(TEXT("Mixer not found\r\n"));
	 return(MIXER_DEVICE_ERR);
	}

	hr = mixerOpen(&hMixer, 0, 0, 0, 0);
	if(FAILED(hr)) 
	 return(MIXER_OPEN_ERR);

	DeleteFile(pszDumpfile);
	err = _tfopen_s( &g_filePtr, pszDumpfile, TEXT("w") );
	if( err != 0)
	{
     _tprintf(TEXT("\r\nDump File '%s' was not opened\r\n"), pszDumpfile);
	 return(MIXER_FILE_ERR);
	}
	else
	 _tprintf(TEXT("\r\nDump File '%s' was opened succesfully\r\n"), pszDumpfile);


	MIXERLINE mxl;
	//MIXERLINECONTROLS mxlc;
	DWORD count, item=-1;

	UINT id = 0;
	MIXERCAPS caps;

	mixerGetID((HMIXEROBJ)hMixer, &id, MIXER_OBJECTF_HMIXER);
	mixerGetDevCaps(id, &caps, sizeof(MIXERCAPS));

	dtprint(TEXT("\r\n//   Mixer name: \"%s\" (szPname of 'MIXERCAPS')\r\n"), caps.szPname);


	int nDestinations = caps.cDestinations;


	for (int d=0; d<nDestinations; d++)
	{
		mxl.cbStruct = sizeof(mxl);
//			mxl.dwComponentType = ComponentType;

		mxl.dwDestination = d;

		// get the title of the destination
		hr = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl, MIXER_GETLINEINFOF_DESTINATION);

		dtprint(TEXT("//\r\n//\r\n//\r\n"));
		dtprint(TEXT("//DESTINATION line # %i - \"%s\" (\"%s\")\r\n"), d, mxl.szName, mxl.szShortName);
		// Print status of Destination Line
		switch(mxl.fdwLine)
		{
		 case  MIXERLINE_LINEF_ACTIVE:
		  {
			dtprint(TEXT("// Line Status is: 'MIXERLINE_LINEF_ACTIVE'\r\n")); 
		  } break;

		 case  MIXERLINE_LINEF_DISCONNECTED:
		  {
			dtprint(TEXT("// Line Status is: 'MIXERLINE_LINEF_DISCONNECTED'\r\n")); 
		  } break;

		 case  MIXERLINE_LINEF_SOURCE:
		  {
			dtprint(TEXT("// Line Status is: 'MIXERLINE_LINEF_SOURCE'\r\n")); 
		  } break;

		 default:
		  {
			dtprint(TEXT("// Line Status is unknown!\r\n")); 
		  } break;
		}

		if (mxl.dwComponentType >= MIXERLINE_COMPONENTTYPE_DST_FIRST &&	mxl.dwComponentType <= MIXERLINE_COMPONENTTYPE_DST_LAST)
		 dtprint(TEXT("// %s\r\n"), ComponentTypesDst[mxl.dwComponentType-MIXERLINE_COMPONENTTYPE_DST_FIRST]);

		if (MIXERLINE_COMPONENTTYPE_DST_WAVEIN == mxl.dwComponentType)
		{
			int x = 0;
		}

        //// Michael !!!!  Destination Line can has its own controls too !!!
		ListControls(hMixer, &mxl);

		count = mxl.cConnections;
		if( count > 0 )
		 dtprint(TEXT("//\r\n//   Num of SOURCE LINES: %d\r\n"), count);

		for(UINT i = 0; i < count; i++)
		{
			mxl.dwSource = i;
			hr = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl, MIXER_GETLINEINFOF_SOURCE);
			if (hr)
				break;


			if (mxl.dwComponentType >= MIXERLINE_COMPONENTTYPE_SRC_FIRST &&
				mxl.dwComponentType <= MIXERLINE_COMPONENTTYPE_SRC_LAST)
			{
				dtprint(TEXT("//\r\n//     \"%s\" (\"%s\"):  %s\r\n"), 	mxl.szName,	mxl.szShortName, ComponentTypesSrc[mxl.dwComponentType-MIXERLINE_COMPONENTTYPE_SRC_FIRST]);
			}
			else
				dtprint(TEXT("//\r\n//     \"%s\" (\"%s\"):  %s\r\n"), 	mxl.szName,	mxl.szShortName, "INVALID COMPONENT TYPE");

			dtprint(TEXT("//    Channels: %i\r\n//    Connections: %i\r\n//    Controls: %i\r\n//    LineID: 0x%x\r\n//    Target: %i\r\n"),
				mxl.cChannels,
				mxl.cConnections,
				mxl.cControls,
				mxl.dwLineID,
				mxl.dwSource,
				mxl.fdwLine,
				mxl.Target);

			// !!!! Call as a separate function.
			// It should be used for Source and Destination lines as well
 		    ListControls(hMixer, &mxl);
		}

	}

	mixerClose(hMixer);

//	int ch;
//	ch = _getch();
	if( g_filePtr != NULL )
      fclose(g_filePtr);

	//_gettchar();

	return(MIXER_OK);

}


static TCHAR *GetTypeText(UINT dwControlType)
{
	TCHAR *CTDesc = TEXT("");

	switch(dwControlType)
	{
	//MIXERCONTROL_CT_CLASS_CUSTOM
	case MIXERCONTROL_CONTROLTYPE_CUSTOM:
		CTDesc = TEXT("CUSTOM");
		break;
	//MIXERCONTROL_CT_CLASS_METER
	case MIXERCONTROL_CONTROLTYPE_BOOLEANMETER:
		CTDesc = TEXT("BOOLEANMETER");
		break;
	case MIXERCONTROL_CONTROLTYPE_SIGNEDMETER:
		CTDesc = TEXT("SIGNEDMETER");
		break;
	case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
		CTDesc = TEXT("PEAKMETER");
		break;
	case MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER:
		CTDesc = TEXT("UNSIGNEDMETER");
		break;
	//MIXERCONTROL_CT_CLASS_SWITCH
	case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
		CTDesc = TEXT("BOOLEAN");
		break;
	case MIXERCONTROL_CONTROLTYPE_ONOFF:
		CTDesc = TEXT("ONOFF");
		break;
	case MIXERCONTROL_CONTROLTYPE_MUTE:
		CTDesc = TEXT("MUTE");
		break;
	case MIXERCONTROL_CONTROLTYPE_MONO:
		CTDesc = TEXT("MONO");
		break;
	case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
		CTDesc =  TEXT("LOUDNESS");
		break;
	case MIXERCONTROL_CONTROLTYPE_STEREOENH:
		CTDesc = TEXT("STEREOENH");
		break;
#if 0
	case MIXERCONTROL_CONTROLTYPE_BASS_BOOST:
		CTDesc = TEXT("BASS_BOOST");
		break;
#endif
	case MIXERCONTROL_CONTROLTYPE_BUTTON:
		CTDesc = TEXT("BUTTON");
		break;
	//MIXERCONTROL_CT_CLASS_NUMBER
	case MIXERCONTROL_CONTROLTYPE_DECIBELS:
		CTDesc = TEXT("DECIBELS");
		break;
	case MIXERCONTROL_CONTROLTYPE_SIGNED:
		CTDesc = TEXT("SIGNED");
		break;
	case MIXERCONTROL_CONTROLTYPE_UNSIGNED:
		CTDesc = TEXT("UNSIGNED");
		break;
	case MIXERCONTROL_CONTROLTYPE_PERCENT:
		CTDesc = TEXT("PERCENT");
		break;
	//MIXERCONTROL_CT_CLASS_SLIDER
	case MIXERCONTROL_CONTROLTYPE_SLIDER:
		CTDesc = TEXT("SLIDER");
		break;
	case MIXERCONTROL_CONTROLTYPE_PAN:
		CTDesc = TEXT("PAN");
		break;
	case MIXERCONTROL_CONTROLTYPE_QSOUNDPAN:
		CTDesc = TEXT("QSOUNDPAN");
		break;
	//MIXERCONTROL_CT_CLASS_FADER
	case MIXERCONTROL_CONTROLTYPE_FADER:
		CTDesc = TEXT("FADER");
		break;
	case MIXERCONTROL_CONTROLTYPE_VOLUME:
		CTDesc = TEXT("VOLUME");
		break;
	case MIXERCONTROL_CONTROLTYPE_BASS:
		CTDesc = TEXT("BASS");
		break;
	case MIXERCONTROL_CONTROLTYPE_TREBLE:
		CTDesc = TEXT("TREBLE");
		break;
	case MIXERCONTROL_CONTROLTYPE_EQUALIZER:
		CTDesc = TEXT("EQUALIZER");
		break;
	//MIXERCONTROL_CT_CLASS_LIST 
	case MIXERCONTROL_CONTROLTYPE_SINGLESELECT:
		CTDesc = TEXT("SINGLESELECT");
		break;
	case MIXERCONTROL_CONTROLTYPE_MUX:
		CTDesc = TEXT("MUX");
		break;
	case MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT:
		CTDesc = TEXT("MULTIPLESELECT");
		break;
	case MIXERCONTROL_CONTROLTYPE_MIXER:
		CTDesc = TEXT("MIXER");
		break;
	//MIXERCONTROL_CT_CLASS_TIME
	case MIXERCONTROL_CONTROLTYPE_MICROTIME:
		CTDesc = TEXT("MICROTIME");
		break;
	case MIXERCONTROL_CONTROLTYPE_MILLITIME:
		CTDesc = TEXT("MILLITIME");
		break;
	}
	return CTDesc;
}