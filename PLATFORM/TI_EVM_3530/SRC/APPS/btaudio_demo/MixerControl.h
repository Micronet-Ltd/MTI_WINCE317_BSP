#include <windows.h>

BOOL getMixerControl(WCHAR *szControlShortName, MIXERCONTROL *dwControl);

class MixerControl
{
private:
	MIXERCONTROL m_MixerControl;
	MIXERCONTROLDETAILS m_MixerControlDetails;

public:
	MixerControl() { }

	BOOL CreateByShortName(WCHAR *szShortName)
	{
		return getMixerControl(szShortName, &m_MixerControl);
	}

	WCHAR *GetName() { return m_MixerControl.szName; }
	DWORD GetMinimum() { return m_MixerControl.Bounds.dwMinimum; }
	DWORD GetMaximum() { return m_MixerControl.Bounds.dwMaximum; }
	DWORD GetSteps() { return m_MixerControl.Metrics.cSteps; }
	DWORD GetValue() 
	{ 
		DWORD value = 0;
		m_MixerControlDetails.cbStruct = sizeof(m_MixerControlDetails);
		m_MixerControlDetails.dwControlID = m_MixerControl.dwControlID;
		m_MixerControlDetails.cChannels = 1;
		m_MixerControlDetails.cMultipleItems = m_MixerControl.cMultipleItems;
		m_MixerControlDetails.cbDetails = sizeof(value); // MIXERCONTROLDETAILS_SIGNED?
		m_MixerControlDetails.paDetails = &value;

		MMRESULT err = mixerGetControlDetails((HMIXEROBJ)0, &m_MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
		if( err != MMSYSERR_NOERROR )
			DebugBreak();

		return value; 
	}

	void  SetValue(DWORD value) 
	{ 
		m_MixerControlDetails.cbStruct = sizeof(m_MixerControlDetails);
		m_MixerControlDetails.dwControlID = m_MixerControl.dwControlID;
		m_MixerControlDetails.cChannels = 1;
		m_MixerControlDetails.cMultipleItems = m_MixerControl.cMultipleItems;
		m_MixerControlDetails.cbDetails = sizeof(value); // MIXERCONTROLDETAILS_SIGNED?
		m_MixerControlDetails.paDetails = &value;

		MMRESULT err = mixerSetControlDetails((HMIXEROBJ)0, &m_MixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE);
		if( err != MMSYSERR_NOERROR )
			DebugBreak();
	}
};

BOOL getControlByName(MIXERLINE *line, WCHAR *name, MIXERCONTROL *dwValue)
{
	BOOL ret = FALSE;
	MMRESULT error;
    MIXERLINECONTROLS controls;
    MIXERCONTROL * control;

    controls.cbStruct = sizeof(MIXERLINECONTROLS);
    controls.dwLineID = line->dwLineID;  // from MIXERLINE
    controls.cControls = line->cControls;// from MIXERLINE
    controls.cbmxctrl = sizeof(MIXERCONTROL);

    control = (MIXERCONTROL *)LocalAlloc(LPTR, sizeof(MIXERCONTROL) * line->cControls);

    if (NULL == control)
        return ret;

	controls.pamxctrl = control;
	controls.dwControlType = 0;
    error = mixerGetLineControls((HMIXEROBJ)0, &controls, MIXER_GETLINECONTROLSF_ALL);
    if (error == MMSYSERR_NOERROR)
    {
        DWORD c;
        for (c = 0; c < line->cControls; c++)
        {
			if (wcscmp(control[c].szShortName, name) == 0)
			{
				memcpy(dwValue, &control[c], sizeof(MIXERCONTROL));
				ret = TRUE;
				break;
			}
        }
    }

    LocalFree(control);
	return ret;
}

BOOL getMixerControl(WCHAR *szControlShortName, MIXERCONTROL *dwControl)
{
	MIXERCAPS caps = {0};
    MMRESULT res = mixerGetDevCaps(0, &caps, sizeof(caps));
    if (MMSYSERR_NOERROR == res)
    {
		MIXERLINE line = {0};
        line.cbStruct = sizeof(MIXERLINE);

        for (line.dwDestination = 0; line.dwDestination < caps.cDestinations; line.dwDestination++)
        {
            res = mixerGetLineInfo((HMIXEROBJ)0, &line, MIXER_GETLINEINFOF_DESTINATION);
			if (MMSYSERR_NOERROR == res)
			{
				if (getControlByName(&line, szControlShortName, dwControl))
					return TRUE;

				int count = line.cConnections;
				for (int i = 0; i < count; i++)
				{
					line.dwSource = i;
					res = mixerGetLineInfo((HMIXEROBJ)0, &line, MIXER_GETLINEINFOF_SOURCE);
					if (MMSYSERR_NOERROR != res)
						break;

					if (getControlByName(&line, szControlShortName, dwControl))
						return TRUE;
				}
			}
        }
    }

	return FALSE;
}