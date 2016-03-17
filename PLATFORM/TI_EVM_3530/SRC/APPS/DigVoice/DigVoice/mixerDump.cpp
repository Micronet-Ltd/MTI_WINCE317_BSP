// mixerDump.cpp - console app
//
#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
/*
#include <conio.h>
#include <malloc.h>
*/

TCHAR   g_dumpfile[256] = TEXT("mixerDumpFile.txt");
FILE*	g_filePtr = NULL;

void dump(UINT uMixerID = 0);
void dumpa();
static TCHAR *GetTypeText(UINT dwControlType);
void SetMuxInput(HMIXER mix, MIXERCONTROL * control, DWORD channels, DWORD target);


/*
int main(int argc, char* argv[])
{
	dumpa();
	dump();
	return 0;
}
*/


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

/*
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
*/
/* */
static void ListControls(HMIXER, MIXERLINE *);
static void DumpMuxDetails(HMIXER, MIXERCONTROL *, DWORD);



void dtprint(const TCHAR *pszFormat, ...)
{
#if 0
    TCHAR buf[1024];
	buf[0] = 0;
	va_list arglist;
	va_start(arglist, pszFormat);
    //vsprintf(&buf[strlen(buf)], pszFormat, arglist);
	_vstprintf_s
	va_end(arglist);
    strcat(buf, "\n");
	printf(buf);
	//_tprintf(TEXT("%s"), buf);
    OutputDebugString(buf);
#endif
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
void dumpa()
{
    UINT k;//, m = mixerGetNumDevs();
	UINT m = 1;

    /* */
    for (k = 0; k < m; k++)
    {
        MMRESULT error;
        HMIXER mixer;

        error = mixerOpen(&mixer, k, 0, (DWORD)GetModuleHandle(NULL), MIXER_OBJECTF_MIXER);
        if (MMSYSERR_NOERROR == error)
        {
            MIXERCAPS caps;

            error = mixerGetDevCaps((UINT)mixer, &caps, sizeof(MIXERCAPS));
            if (MMSYSERR_NOERROR == error)
            {
                MIXERLINE linea;
                UINT j;

				dtprint(TEXT("%s\r\n"), caps.szPname);
                linea.cbStruct = sizeof(MIXERLINE);

                for (j = 0; j < caps.cDestinations; j++)
                {
                    linea.dwDestination = j;
                    error = mixerGetLineInfo((HMIXEROBJ)mixer, &linea, MIXER_GETLINEINFOF_DESTINATION);
                    if (MMSYSERR_NOERROR == error)
                    {
                        if (MIXERLINE_COMPONENTTYPE_DST_WAVEIN == linea.dwComponentType)
                        {
							dtprint(TEXT("\t%s (%s)\r\n"), linea.szName, linea.szShortName);
                            ListControls(mixer, &linea);
                        }
                    }
                    else
                    {
						dtprint(TEXT("Error %ld en mixerGetLineInfo()\r\n"), error);
                    }
                }
            }
            else
            {
				dtprint(TEXT("Error %ld en mixerGetDevCaps()\r\n"), error);
            }
            mixerClose(mixer);
        }
        else
        {
			dtprint(TEXT("Error %ld en mixerOpen()\r\n"), error);
        }
    }
    return;
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
		dtprint(TEXT("Error %ld on LocalAlloc()\r\n"), GetLastError());
        return;
    }
    controls.pamxctrl = control;

	/* 
	// to get mux only 
	controls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUX;
    error = mixerGetLineControls((HMIXEROBJ)mix, &controls, MIXER_GETLINECONTROLSF_ONEBYTYPE);
    if (MMSYSERR_NOERROR == error)
    {
        for (int c=0; c < ml->cControls; c++) // c should equal 0 (1 control) but you never know
        {
            // MUX - MULTIPLE SELECT
            if (MIXERCONTROL_CONTROLTYPE_MUX == (MIXERCONTROL_CONTROLTYPE_MUX & control[c].dwControlType))
            {
				SetInput(mix, control+c, ml->cChannels, target);
            }
        }
	}*/

	controls.dwControlType = 0;
    error = mixerGetLineControls((HMIXEROBJ)mix, &controls, MIXER_GETLINECONTROLSF_ALL);

    if (error == MMSYSERR_NOERROR)
    {
        DWORD c;

		dtprint(TEXT("\r\n\t   Controls: %d\r\n"), ml->cControls);

        for (c = 0; c < ml->cControls; c++)
        {
#if 0
            /* MUX - MULTIPLE SELECT */
            if (MIXERCONTROL_CONTROLTYPE_MUX == (MIXERCONTROL_CONTROLTYPE_MUX & control[c].dwControlType))
            {
				dtprint(TEXT("\t\t%s (%s)\r\n"), control[c].szName, control[c].szShortName );
                //dprint("\t\t%s\	n\n", control[c].szName);
				//SetMuxInput(mix, control+c, ml->cChannels, 2);
                DumpMuxDetails(mix, (control + c), ml->cChannels);
            }
			else
			{
				// other control
				dtprint(TEXT("\t\t%s (%s)\r\n"), control[c].szName, control[c].szShortName );
			}
#endif
                dtprint( TEXT("\t%s (%s) %x, %x \r\n\t %s %s\r\n\t%d  %d  %d\r\n"),
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
		dtprint(TEXT("Error %ld in mixerGetLineControls()\r\n"), error);
    }

    LocalFree(control);
}



static void DumpMuxDetails(HMIXER mix, MIXERCONTROL * control, DWORD channels)
{
    MMRESULT error;
    MIXERCONTROLDETAILS cd;

    cd.cbStruct = sizeof(MIXERCONTROLDETAILS);
    cd.dwControlID = control->dwControlID;
    cd.cChannels = channels;
    cd.cMultipleItems = control->cMultipleItems;
    cd.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);

    MIXERCONTROLDETAILS_LISTTEXT * lt = (MIXERCONTROLDETAILS_LISTTEXT *)LocalAlloc (LPTR, cd.cChannels * cd.cMultipleItems * cd.cbDetails);
    if (NULL == lt) return;

    cd.paDetails = lt;
    error = mixerGetControlDetails((HMIXEROBJ)mix, &cd, MIXER_GETCONTROLDETAILSF_LISTTEXT);

    if (MMSYSERR_NOERROR == error)
    {
        cd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

        MIXERCONTROLDETAILS_BOOLEAN * lv = (MIXERCONTROLDETAILS_BOOLEAN *)LocalAlloc (LPTR, cd.cChannels * cd.cMultipleItems * cd.cbDetails);
        if (NULL != lv)
        {
            cd.paDetails = lv;
            error = mixerGetControlDetails((HMIXEROBJ)mix, &cd, MIXER_GETCONTROLDETAILSF_VALUE);

            if (MMSYSERR_NOERROR == error)
            {
                DWORD i;

                for (i = 0; i < cd.cMultipleItems; i++)
                {
					dtprint(TEXT("%32s [%c]\r\n"), lt[i].szName, lv[i].fValue ? 'X' : ' ');
                   // dprint("%32s [%c]\n", lt[i].szName, lv[i].fValue ? 'X' : ' ');
                }
            }
            else
            {
				dtprint(TEXT("Error %ld in mixerGetControlDetails()\r\n"), error);
            }
            LocalFree(lv);
        }
    }
    else
    {
		dtprint(TEXT("Error %ld in mixerGetControlDetails()\r\n"), error);
    }

    LocalFree(lt);
}




void SetMuxInput(HMIXER mix, MIXERCONTROL * control, DWORD channels, DWORD target)
{
    MMRESULT error;
    MIXERCONTROLDETAILS cd;

    cd.cbStruct = sizeof(MIXERCONTROLDETAILS);
    cd.dwControlID = control->dwControlID;
    cd.cChannels = channels;
    cd.cMultipleItems = control->cMultipleItems;

    cd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);

    MIXERCONTROLDETAILS_BOOLEAN * lv = (MIXERCONTROLDETAILS_BOOLEAN *)LocalAlloc (LPTR, cd.cChannels * cd.cMultipleItems * cd.cbDetails);
    if (NULL != lv)
    {
        cd.paDetails = lv;
        error = mixerGetControlDetails((HMIXEROBJ)mix, &cd, MIXER_GETCONTROLDETAILSF_VALUE);
        // make sure they are all turned off except the one we are turning on
        for (DWORD i = 0; i < cd.cMultipleItems; i++)
            lv[i].fValue = 0;
         // set the mux to our source
        lv[target].fValue = 1;
        error = mixerSetControlDetails((HMIXEROBJ)mix, &cd, MIXER_SETCONTROLDETAILSF_VALUE);

        LocalFree(lv);
    }
}


void dump(UINT uMixerID)
{
	HMIXER hMixer;
	HRESULT hr;
	errno_t err;

	hr = mixerOpen(&hMixer, uMixerID, 0, 0, 0);
	if (FAILED(hr)) return;

	DeleteFile(g_dumpfile);
	err = _tfopen_s( &g_filePtr, g_dumpfile, TEXT("w+b") );
	if( err != 0)
     _tprintf(TEXT("\r\nDump File '%s' was not opened\r\n"), g_dumpfile);
	else
	 _tprintf(TEXT("\r\nDump File '%s' was opened succesfully\r\n"), g_dumpfile);


	MIXERLINE mxl;
	//MIXERLINECONTROLS mxlc;
	DWORD count, item=-1;

	UINT id = 0;
	MIXERCAPS caps;

	mixerGetID((HMIXEROBJ)hMixer, &id, MIXER_OBJECTF_HMIXER);
	mixerGetDevCaps(id, &caps, sizeof(MIXERCAPS));

	dtprint(TEXT("\r\nMixer: %s\r\n"), caps.szPname);
	//dprint("\r\nMixer: %s\r\n", caps.szPname);

	int nDestinations = caps.cDestinations;


	for (int d=0; d<nDestinations; d++)
	{
		mxl.cbStruct = sizeof(mxl);
//			mxl.dwComponentType = ComponentType;

		mxl.dwDestination = d;

		// get the title of the destination
		hr = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl, MIXER_GETLINEINFOF_DESTINATION);

		dtprint(TEXT("\r\n\r\nDESTINATION %i - %s (%s)\r\n"), d, mxl.szName, mxl.szShortName);

		if (mxl.dwComponentType >= MIXERLINE_COMPONENTTYPE_DST_FIRST &&	mxl.dwComponentType <= MIXERLINE_COMPONENTTYPE_DST_LAST)
		 dtprint(TEXT("%s\r\n"), ComponentTypesDst[mxl.dwComponentType-MIXERLINE_COMPONENTTYPE_DST_FIRST]);

		if (MIXERLINE_COMPONENTTYPE_DST_WAVEIN == mxl.dwComponentType)
		{
			int x = 0;
		}

        //// Michael !!!!  Destination Line can has its own controls too !!!
		ListControls(hMixer, &mxl);

		count = mxl.cConnections;
		if( count > 0 )
		 dtprint(TEXT("\r\n   SOURCE LINES: %d\r\n"), count);

		for(UINT i = 0; i < count; i++)
		{
			mxl.dwSource = i;
			hr = mixerGetLineInfo((HMIXEROBJ)hMixer, &mxl, MIXER_GETLINEINFOF_SOURCE);
			if (hr)
				break;


			if (mxl.dwComponentType >= MIXERLINE_COMPONENTTYPE_SRC_FIRST &&
				mxl.dwComponentType <= MIXERLINE_COMPONENTTYPE_SRC_LAST)
			{
				dtprint(TEXT("\r\n    %s [%s]:  %s\r\n"), 	mxl.szName,	mxl.szShortName, ComponentTypesSrc[mxl.dwComponentType-MIXERLINE_COMPONENTTYPE_SRC_FIRST]);
			}
			else
				dtprint(TEXT("\r\n    %s [%s]:  %s\r\n"), 	mxl.szName,	mxl.szShortName, "INVALID COMPONENT TYPE");

			dtprint(TEXT("    Channels: %i\r\n    Connections: %i\r\n     Controls: %i\r\n     LineID: 0x%x\r\n     Target: %i\r\n"),
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
#if 0
			if (mxl.cControls) // has controls (volume slider etc)
			{
				int iMCBufferSize = sizeof(MIXERCONTROL) * mxl.cControls;
				MIXERCONTROL *pmc = (MIXERCONTROL *)malloc(iMCBufferSize);

 		        dtprint(TEXT("\r\nControls:\r\n"));

				// get details on the controls
				//mc.cbStruct = sizeof(mc);
				mxlc.cbStruct = sizeof(mxlc);
				mxlc.dwLineID = mxl.dwLineID;	// set the line ID of our component
				mxlc.dwControlType = 0;
				mxlc.cControls = mxl.cControls;	// the number of controls we are supposed to have for this component
				mxlc.cbmxctrl = sizeof(MIXERCONTROL);
				mxlc.pamxctrl = pmc;
				hr = mixerGetLineControls((HMIXEROBJ)hMixer, &mxlc, MIXER_GETLINECONTROLSF_ALL);

				for (UINT c=0; c<mxlc.cControls; c++)
				{

					dtprint(TEXT("\t\t%i '%s' - %x %s %s\r\n"), pmc[c].dwControlID, 
						pmc[c].szName, 
						pmc[c].dwControlType, 
						GetTypeText(pmc[c].dwControlType), 
						pmc[c].fdwControl & MIXERCONTROL_CONTROLF_DISABLED?TEXT("[DISABLED]"):TEXT(""));
				}

				free (pmc);
			}
#endif
		}

/*		if (item >= 0)
		{
			mc.cbStruct = sizeof(mc);
			mxlc.cbStruct = sizeof(mxlc);
			mxlc.dwLineID = mxl.dwLineID;
			mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
			mxlc.cControls = 1;
			mxlc.cbmxctrl = sizeof(MIXERCONTROL);
			mxlc.pamxctrl = &mc;
			hr = mixerGetLineControls((HMIXEROBJ)hMixer, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);
			m_dwControlID = mc.dwControlID;
		}*/
	}

	mixerClose(hMixer);

//	int ch;
//	ch = _getch();
	if( g_filePtr != NULL )
      fclose(g_filePtr);

	_gettchar();

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