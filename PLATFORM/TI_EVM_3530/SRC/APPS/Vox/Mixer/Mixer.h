//#pragma once
//#include "resource.h"

#ifndef _MIXER_H
#define _MIXER_H


///////////////// Mixer Structure of CE-500 ////////////////////////////////////
//   Mixer name: "Audio Mixer" (szPname of 'MIXERCAPS')
//
//
//
//DESTINATION line # 0 - "Recorder Volume" ("Recorder Volume")
// Line Status is: 'MIXERLINE_LINEF_ACTIVE'
// MIXERLINE_COMPONENTTYPE_DST_WAVEIN
//
//	   Num of Controls: 1
//
//		"Record Volume" ("Record Volume") ff000000, 50030001 
//		"VOLUME" "[DISABLED]"
//		0  65535  1
//
//
//   Num of SOURCE LINES: 1
//
//     "Microphone" ("Microphone"):  MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE
//    Channels: 1
//    Connections: 0
//    Controls: 3
//    LineID: 0x0
//    Target: 0
//
//	   Num of Controls: 3
//
//		"Mic Mute" ("Mute") 0, 20010002 
//		"MUTE" ""
//		0  1  0
//
//		"Mic Analog Gain" ("MicAnGain") 1, 30040000 
//		"DECIBELS" ""
//		0  30  0
//
//		"Mic Digital Gain" ("MicDigGain") 2, 50030001 
//		"VOLUME" ""
//		0  31  0
//
//
//
//
//DESTINATION line # 1 - "Master Volume" ("Master Volume")
// Line Status is: 'MIXERLINE_LINEF_ACTIVE'
// MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
//
//	   Num of Controls: 2
//
//		"Master Volume" ("Master Volume") ff010000, 50030001 
//		"VOLUME" "[DISABLED]"
//		0  65535  1
//
//		"Master Mute" ("Mute") ff010001, 20010002 
//		"MUTE" ""
//		0  1  0
//
//
//
//
//DESTINATION line # 2 - "Voice Modem" ("VoiceM")
// Line Status is: 'MIXERLINE_LINEF_ACTIVE'
// MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
//
//	   Num of Controls: 1
//
//		"Voice Out OnOff" ("VOnOff") ff020000, 20010001 
//		"ONOFF" ""
//		0  1  0
//
//
//   Num of SOURCE LINES: 2
//
//     "Voice Input" ("VIn"):  MIXERLINE_COMPONENTTYPE_SRC_DIGITAL
//    Channels: 1
//    Connections: 0
//    Controls: 2
//    LineID: 0x2
//    Target: 0
//
//	   Num of Controls: 2
//
//		"Voice Input Digital Gain" ("VInDigGain") 20000, 30040000 
//		"DECIBELS" ""
//		0  31  1
//
//		"Voice Input Mute" ("VInMute") 20001, 20010002 
//		"MUTE" ""
//		0  1  0
//
//
//     "Voice Output" ("VOut"):  MIXERLINE_COMPONENTTYPE_SRC_DIGITAL
//    Channels: 1
//    Connections: 0
//    Controls: 2
//    LineID: 0x102
//    Target: 1
//
//	   Num of Controls: 2
//
//		"Voice Out Volume" ("V Out Vol") 1020000, 30040000 
//		"DECIBELS" "[DISABLED]"
//		-60  12  1
//
//		"Voice Out Mute" ("VMute") 1020001, 20010002 
//		"MUTE" ""
//		0  1  0
//
//
//
//
//DESTINATION line # 3 - "Bluetooth Voice Modem" ("BTVoiceM")
// Line Status is: 'MIXERLINE_LINEF_ACTIVE'
// MIXERLINE_COMPONENTTYPE_DST_UNDEFINED
//
//	   Num of Controls: 1
//
//		"Bluetooth Voice OnOff" ("BTVOnOff") ff030000, 20010001 
//		"ONOFF" ""
//		0  1  0
//
//
//   Num of SOURCE LINES: 2
//
//     "Input from BT" ("BTDIN"):  MIXERLINE_COMPONENTTYPE_SRC_DIGITAL
//    Channels: 1
//    Connections: 0
//    Controls: 2
//    LineID: 0x3
//    Target: 0
//
//	   Num of Controls: 2
//
//		"Bluetooth Voice In Mute" ("BTDIN Mute") 30000, 20010002 
//		"MUTE" ""
//		0  1  0
//
//		"Bluetooth In Volume" ("BTDIN Vol") 30001, 30040000 
//		"DECIBELS" "[DISABLED]"
//		-15  30  3
//
//
//     "Output from Codec" ("BTDOUT"):  MIXERLINE_COMPONENTTYPE_SRC_DIGITAL
//    Channels: 1
//    Connections: 0
//    Controls: 2
//    LineID: 0x103
//    Target: 1
//
//	   Num of Controls: 2
//
//		"Bluetooth Voice Out Mute" ("BTDOUT Mute") 1030000, 20010002 
//		"MUTE" ""
//		0  1  0
//
//		"Bluetooth Out Volume" ("BTDOUT Vol") 1030001, 30040000 
//		"DECIBELS" "[DISABLED]"
//		-30  15  3
//
//
//
//
//DESTINATION line # 4 - "Bluetooth Playback" ("BTPlb")
// Line Status is: 'MIXERLINE_LINEF_ACTIVE'
// MIXERLINE_COMPONENTTYPE_DST_UNDEFINED
//
//	   Num of Controls: 1
//
//		"Bluetooth Playback OnOff" ("BTPlbOnOff") ff040000, 20010001 
//		"ONOFF" ""
//		0  1  0
//
//
//   Num of SOURCE LINES: 1
//
//     "BT playback from Codec" ("BTPLBOUT"):  MIXERLINE_COMPONENTTYPE_SRC_DIGITAL
//    Channels: 1
//    Connections: 0
//    Controls: 2
//    LineID: 0x4
//    Target: 0
//
//	   Num of Controls: 2
//
//		"Bluetooth Playback Mute" ("BTPlbOUT Mute") 40000, 20010002 
//		"MUTE" ""
//		0  1  0
//
//		"Bluetooth Playback Volume" ("BTPlbOUT Vol") 40001, 30040000 
//		"DECIBELS" "[DISABLED]"
//		-30  15  3
//
//
//
//
//DESTINATION line # 5 - "External Audio" ("ExtAudio")
// Line Status is: 'MIXERLINE_LINEF_ACTIVE'
// MIXERLINE_COMPONENTTYPE_DST_UNDEFINED
//
//	   Num of Controls: 1
//
//		"External Audio OnOff" ("ExtAudio OnOff") ff050000, 20010001 
//		"ONOFF" ""
//		0  1  0
//
//
//   Num of SOURCE LINES: 1
//
//     "External Microphone" ("ExtMic"):  MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE
//    Channels: 1
//    Connections: 0
//    Controls: 3
//    LineID: 0x5
//    Target: 0
//
//	   Num of Controls: 3
//
//		"External Microphone Mute" ("ExtMicMute") 50000, 20010002 
//		"MUTE" ""
//		0  1  0
//
//		"External Microphone Analog Gain" ("ExtMicAnGain") 50001, 50030001 
//		"VOLUME" ""
//		20  30  10
//
//		"External Microphone Digital Gain" ("ExtMicDigGain") 50002, 30040000 
//		"DECIBELS" ""
//		-90  3  1
//

/************** STRUCTURES, ENUMS, AND TYPEDEFS *******************************/
typedef enum
{
    MIXER_OK = 0,
	MIXER_DEVICE_ERR,
	MIXER_OPEN_ERR,
    MIXER_INVALID_PARAM,
    MIXER_FILE_ERR,

} MIXER_ERROR_CODES;


struct MIXER_CONTROL_s
{
  TCHAR  *pszControlName;
  TCHAR  *pszShortControlName;

  MIXER_CONTROL_s  *pNextMixControl;

}; 


struct MIXER_SRC_LINE_s
{
  TCHAR  *pszLineName;
  TCHAR  *pszShortLineName;

  MIXER_CONTROL_s   *pMixControl;
  MIXER_SRC_LINE_s  *pNextMixSrcLine;

};


struct MIXER_DST_LINE_s
{
  DWORD   lineStatus;
  TCHAR  *pszLineName;
  TCHAR  *pszShortLineName;

  MIXER_CONTROL_s   *pMixControl;
  MIXER_SRC_LINE_s  *pMixSrcLine;

  MIXER_DST_LINE_s  *pNextMixDstLine;

};



#ifdef __cplusplus
	extern "C" {
#endif


// ================================= General functions====================================

//------------------------------------------------------------------------------
// Function name	: turnVoiceOnOff
// Description	    : This function turns on/off the the Voice of GSM modem.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : OnOf – FALSE if we want to turn off the VOice and TRUE if we want 
//                  : to turn on the Voice.
//                  : btOn - True, if Bluetooth is on  
//------------------------------------------------------------------------------
BOOL turnVoiceOnOff(BOOL OnOff, BOOL btOn);


// Turn Audio Codec Bluetooth Playback On/Off
//------------------------------------------------------------------------------
// Function name	: turnBTPlbOnOff
// Description	    : This function turns on/off the Bluetooth Playback
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : OnOf – FALSE if we want to turn off the Bluetooth Playback and 
//                           TRUE if we want to turn on the Bluetooth Playback.
//------------------------------------------------------------------------------
BOOL turnBTPlbOnOff(BOOL OnOff);



//------------------------------------------------------------------------------
// Function name	: turnExtAudioOnOff
// Description	    : This function routes Audio to External Audio
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : OnOf – FALSE if we want to turn off the External Audio and 
//                           TRUE if we want to turn on the External Audio.
//------------------------------------------------------------------------------
BOOL turnExtAudioOnOff(BOOL OnOff);


//------------------------------------------------------------------------------
// Function name	: queryExtAudio
// Description	    : This function checks route of External Audio
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pOnOf – return FALSE if External Audio is Off and 
//                           TRUE if On.
//------------------------------------------------------------------------------
BOOL queryExtAudio(BOOL *pOnOff);

//------------------------------------------------------------------------------
// Function name	: getVoiceInGain
// Description	    : This function returns parameters of the Voice input digital gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pVoiceInGain - returned current gain
//                  : pVoiceInGainMin - returned minimum gain 
//                  : pVoiceInGainMax - returned maximum gain 
//                  : btOn - True, if Bluetooth is on  
//------------------------------------------------------------------------------
BOOL getVoiceInGain(INT16 *pVoiceInGain, INT16 *pVoiceInGainMin, INT16 *pVoiceInGainMax, BOOL btOn);


//------------------------------------------------------------------------------
// Function name	: getVoiceInMute
// Description	    : This function returns parameters of the Voice input mute status.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pMute - returned current mute status
//                  : btOn - TRUE if Bluetooth, FALSE if only Voice Modem
//------------------------------------------------------------------------------
BOOL getVoiceInMute(BOOL *pMute, BOOL btOn);

//------------------------------------------------------------------------------
// Function name	: putVoiceInGain
// Description	    : This function sets input digital gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : voiceOutGainn - input gain to set
//                  : btOn - True, if Bluetooth is on  
//------------------------------------------------------------------------------
BOOL putVoiceInGain(INT16 voiceOutGain, BOOL btOn);


//------------------------------------------------------------------------------
// Function name	: setVoiceInMute
// Description	    : This function sets the Voice input mute.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : mute - Voice input mute, TRUE if mute.
//                  : btOn - True, if Bluetooth is on  
//------------------------------------------------------------------------------
BOOL setVoiceInMute(BOOL mute, BOOL btOn);

//------------------------------------------------------------------------------
// Function name	: getVoiceOutGain
// Description	    : This function returns parameters of the Voice output digital gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pVoiceOutGain - returned current gain
//                  : pVoiceOutnGainMin - returned minimum gain 
//                  : pVoiceOutGainMax - returned maximum gain 
//                  : btOn - True, if Bluetooth is on  
//------------------------------------------------------------------------------
BOOL getVoiceOutGain(INT16 *pVoiceOutGain, INT16 *pVoiceOutGainMin, INT16 *pVoiceOutGainMax, BOOL btOn);


//------------------------------------------------------------------------------
// Function name	: getBTPlaybackOutGain
// Description	    : This function returns parameters of the Voice output digital gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pBTPlaybackOutGain - returned current gain
//                  : pBTPlaybackOutnGainMin - returned minimum gain 
//                  : pBTPlaybackOutGainMax - returned maximum gain 
//------------------------------------------------------------------------------
BOOL getBTPlaybackOutGain(INT16 *pBTPlaybackOutGain, INT16 *pBTPlaybackOutGainMin, INT16 *pBTPlaybackOutGainMax);



//------------------------------------------------------------------------------
// Function name	: getVoiceOutMute
// Description	    : This function returns parameters of the Voice Output mute status.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pMute - returned current mute status
//                  : btOn - True, if Bluetooth is on  
//------------------------------------------------------------------------------
BOOL getVoiceOutMute(BOOL *pMute, BOOL btOn);


//------------------------------------------------------------------------------
// Function name	: getBTPlaybackOutMute
// Description	    : This function returns parameters of the 
//                    Bluetooth Playback Output mute status.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pMute - returned current mute status
//------------------------------------------------------------------------------
BOOL getBTPlaybackOutMute(BOOL *pMute);

//------------------------------------------------------------------------------
// Function name	: putVoiceOutGain
// Description	    : This function sets output digital gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : voiceOutGainn - output gain to set
//                  : btOn - True, if Bluetooth is on  
//------------------------------------------------------------------------------
BOOL putVoiceOutGain(INT16 voiceOutGain, BOOL btOn);


//------------------------------------------------------------------------------
// Function name	: putBTPlaybackOutGain
// Description	    : This function sets Bluetooth playback output digital gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : btPlaybackOutGain - output gain to set
//------------------------------------------------------------------------------
BOOL putBTPlaybackOutGain(INT16 btPlaybackOutGain);


//------------------------------------------------------------------------------
// Function name	: setVoiceOutMute
// Description	    : This function sets the Voice Output mute.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : mute - Voice Output mute, TRUE if mute.
//                  : btOn - True, if Bluetooth is on  
//------------------------------------------------------------------------------
BOOL setVoiceOutMute(BOOL mute, BOOL btOn);


//------------------------------------------------------------------------------
// Function name	: setBTPlaybackOutMute
// Description	    : This function sets the Bluetooth Playback Output mute.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : mute - Playback Output mute, TRUE if mute.
//------------------------------------------------------------------------------
BOOL setBTPlaybackOutMute(BOOL mute);

//------------------------------------------------------------------------------
// Function name	: setMicMute
// Description	    : This function mute/unmute microphone
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : micMute - TRUE to mute, FALSE - to unmute
//------------------------------------------------------------------------------
BOOL setMicMute(BOOL micMute);


//------------------------------------------------------------------------------
// Function name	: setExtMicMute
// Description	    : This function mute/unmute external microphone
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : micMute - TRUE to mute, FALSE - to unmute
//------------------------------------------------------------------------------
BOOL setExtMicMute(BOOL micMute);


//------------------------------------------------------------------------------
// Function name	: setSpkMute
// Description	    : This function mute/unmute Speaker
// Return type		: TRUE if succeeded, FALSE if failed.
//                  
// Argument         : mute - TRUE to mute, FALSE - to unmute
//------------------------------------------------------------------------------
BOOL setSpkMute(BOOL mute);


//------------------------------------------------------------------------------
// Function name	: getSpkMute
// Description	    : This function returns mute/unmute Speaker status
// Return type		: TRUE if succeeded, FALSE if failed.
//                  
// Argument         : pMute - mute status
//------------------------------------------------------------------------------
BOOL getSpkMute(BOOL *pMute);

#if 0
//------------------------------------------------------------------------------
// Function name	: setSpkVol
// Description	    : This function set AudioCodec Analog gain
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : vol - from 0 to 0xFFFF
//------------------------------------------------------------------------------
BOOL setSpkVol(UINT16 vol);

//------------------------------------------------------------------------------
// Function name	: getSpkVol
// Description	    : This function returns parameters of the AudioCodec Analog gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pVol - returned current AudioCodec Analog gain
//                  : pVolMin - returned minimum gain 
//                  : pVolMax - returned maximum gain 
//------------------------------------------------------------------------------
BOOL getSpkVol(UINT16 *pVol, UINT16 *pVolMin, UINT16 *pVolMax);
#endif

//------------------------------------------------------------------------------
// Function name	: getMicMute
// Description	    : This function returns microphone mute status
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pMicMute - mute status
//------------------------------------------------------------------------------
BOOL getMicMute(BOOL *pMicMute);



//------------------------------------------------------------------------------
// Function name	: getExtMicMute
// Description	    : This function returns external microphone mute status
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pMicMute - mute status
//------------------------------------------------------------------------------
BOOL getExtMicMute(BOOL *pMicMute);

#if 0
//------------------------------------------------------------------------------
// Function name	: setRecVol
// Description	    : This function sets the Record Volume.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : recVol - Record Volume
//------------------------------------------------------------------------------
BOOL setRecVol(UINT16 recVol);

//------------------------------------------------------------------------------
// Function name	: getRecVol
// Description	    : This function returns parameters of the the Record Volume.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pRecVol - returned current Record Volume
//                  : pRecVolMin - returned minimum Record Volumen 
//                  : pRecVolMax - returned maximum Record Volume 
//------------------------------------------------------------------------------
BOOL getRecVol(UINT16 *pRecVol, UINT16 *pRecVolMin, UINT16 *pRecVolMax);
#endif

//------------------------------------------------------------------------------
// Function name	: getMicAnalogGain
// Description	    : This function returns parameters of the microphone analog gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pMicAnalogGain - returned current microphone analog gain
//                  : pMicAnalogGainMin - returned minimum gain 
//                  : pMicAnalogGainMax - returned maximum gain 
//------------------------------------------------------------------------------
BOOL getMicAnalogGain(UINT16 *pMicAnalogGain, UINT16 *pMicAnalogGainMin, UINT16 *pMicAnalogGainMax);


//------------------------------------------------------------------------------
// Function name	: getExtMicAnalogGain
// Description	    : This function returns parameters of the external microphone analog gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pMicAnalogGain - returned current microphone analog gain
//                  : pMicAnalogGainMin - returned minimum gain 
//                  : pMicAnalogGainMax - returned maximum gain 
//------------------------------------------------------------------------------
BOOL getExtMicAnalogGain(UINT16 *pMicAnalogGain, UINT16 *pMicAnalogGainMin, UINT16 *pMicAnalogGainMax);

//------------------------------------------------------------------------------
// Function name	: putMicAnalogGain
// Description	    : This function sets microphone analog gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : micAnalogGain - microphone analog gain to set
//------------------------------------------------------------------------------
BOOL putMicAnalogGain(UINT16 micAnalogGain);



//------------------------------------------------------------------------------
// Function name	: putExtMicAnalogGain
// Description	    : This function sets external microphone analog gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : micAnalogGain - microphone analog gain to set
//------------------------------------------------------------------------------
BOOL putExtMicAnalogGain(UINT16 micAnalogGain);

//------------------------------------------------------------------------------
// Function name	: getMicDigitalGain
// Description	    : This function returns parameters of the microphone Digital gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pMicDigitalGain - returned current microphone Digital gain
//                  : pMicDigitalGainMin - returned minimum gain 
//                  : pMicDigitalGainMax - returned maximum gain 
//------------------------------------------------------------------------------
BOOL getMicDigitalGain(UINT16 *pMicDigitalGain, UINT16 *pMicDigitalGainMin, UINT16 *pMicDigitalGainMax);



//------------------------------------------------------------------------------
// Function name	: getExtMicDigitalGain
// Description	    : This function returns parameters of the external microphone Digital gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : pMicDigitalGain - returned current microphone Digital gain
//                  : pMicDigitalGainMin - returned minimum gain 
//                  : pMicDigitalGainMax - returned maximum gain 
//------------------------------------------------------------------------------
BOOL getExtMicDigitalGain(INT16 *pMicDigitalGain, INT16 *pMicDigitalGainMin, INT16 *pMicDigitalGainMax);

//------------------------------------------------------------------------------
// Function name	: putMicDigitalGain
// Description	    : This function sets microphone Digital gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : micAnalogGain - microphone Digital gain to set
//------------------------------------------------------------------------------
BOOL putMicDigitalGain(UINT16 micDigitalGain);


//------------------------------------------------------------------------------
// Function name	: putExtMicDigitalGain
// Description	    : This function sets external microphone Digital gain.
// Return type		: TRUE if succeeded, FALSE f failed.
//                  
// Argument         : micAnalogGain - microphone Digital gain to set
//------------------------------------------------------------------------------
BOOL putExtMicDigitalGain(INT16 micDigitalGain);


//------------------------------------------------------------------------------
// Function name	: dumpMixer
// Description	    : This function print current mixer configuration to a File.
// Return type		: MIXER_ERROR_CODES enum type
//                  
// Argument         : dump file name
MIXER_ERROR_CODES   dumpMixer(LPCTSTR pszDumpfile);

MIXER_ERROR_CODES enumMixer(void);
MIXER_ERROR_CODES deEnumMixer(void);

//////////////////////////////////////////////////////////
// For provide callbacks
HMIXER openMixer(HWND hwnd);
void closeMixer(void);


#ifdef __cplusplus
	}
#endif	


#endif /* _MIXER_H */