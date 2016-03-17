// All rights reserved ADENEO EMBEDDED 2010
/*=============================================================================
 *            Texas Instruments OMAP(TM) Platform Software
 * (c) Copyright 2007 Texas Instruments Incorporated. All Rights Reserved.
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied.
 *
=========================================================================== */

/** CHWCodec.h
 *  CHWCodec.h defines the interface for the Transform filter application to
 *  communicate to the Decode/Encode Codecs through the UCFL components
*/

#ifndef _HWCODECAPI_H_
#define _HWCODECAPI_H_

#include <windows.h> /**< Windows CE Standared header */


#ifdef __cplusplus
extern "C"
{
#endif

/*-------HwMediaManagerAPI.h reflection . BSP requested a unified header file--*/
/* The section below must match the contents of HwmediamangerAPI.h             */
#define MAX_STREAM_COUNT 10 /**< Maximum supported number of streams */
#define MDN_MAX_GAIN_SLOPES 40       /**< maximun number of gain slopes (left & right)*/
/**
 * IOCTL_MM_COMMAND_e Enum defines the HMM device commands
 */

typedef enum _IOCTL_MM_COMMAND
{

    PROCESS_AUDIO_CMD     /**< Audio commands */

} IOCTL_MM_COMMAND_e;



/**
 * Media Manager commands enumerations
 */
typedef enum _MM_COMMANDTYPE
{
    /**< Set MIXER Sampling frequency
     *    param1 : 8000,11025,12000,16000,22050,24000,32000,44100,48000 (Hz)
     *    param2 : 0
     */
    MM_CommandWarnSampleFreqChange = 1,

    /**< Notify MIXER Sampling frequency - Must be sent after MM_CommandWarnSampleFreqChange
     *    param1 : 0
     *    param2 : 0
     */
    MM_CommandNotifySampleFreqChange,

    /**< Mixer GAIN
     *     param1 : Q15 gain value (0x0000 up to 0x8000 with 0x8000 representing +1)
     *     param2 : 0
     */
    MM_CommandMixerSWGain,

    /**< Stream GAIN
     *     param1 : Q15 gain value (0x0000 up to 0x8000 with 0x8000 representing +1)
     *     param2 : 0
     *     streamID must be sent
     */
    MM_CommandSWGain,

    /**< Stream MUTE
     *     param1 : 0
     *     param2 : 0
     */
    MM_CommandStreamMute,

    /**< Stream UNMUTE
     * param1 : 0x00008000
     * param2 : 0
     */
    MM_CommandStreamUnMute,

    /**< Input Stream ID
     * param1 : 0
     * param2 : 0
     */
    MM_CommandIsInputStreamAvailable,

    /**< Output Stream ID
     * param1 : 0
     * param2 : 0
     */
    MM_CommandIsOutputStreamAvailable,

    /**< Absolute time from MIXER - Not Supported
    */
    MM_CommandGetAbsoluteTime,

    /**< MIXER Stereo Record - Not Supported
     */
    MM_CommandMixerStereoRecordSupport,

    /**< TDN Play
      *    All user parameters are 0.Handled inside for all TDN commands
      */
    MM_CommandTDNPlayMode,

    /**< TDN Loopback
     *    All user parameters are 0 . Handled inside for all TDN commands
     */
    MM_CommandTDNLoopBackMode,

    /**< TDN Loopback and play
     * All user parameters are 0 . Handled inside for all TDN commands
     */
    MM_CommandTDNPlayLoopBackMode,

    /**< TDN Downlink
     * All user parameters are 0 . Handled inside for all TDN commands
     */
    MM_CommandTDNDownlinkMode,

    /**< ACDN0 Noise suppressor
    * param1 : ACDNCOMMMODE_e enums
    * param2 : 0
    */
    MM_CommandACDN0AECNSMode,

    /**< ACDN0 Equalizer on DL Left channel
    * param1 : ACDNCOMMMODE_e enums
    * param2 : 0
    */
    MM_CommandACDN0EQDLLMode,

    /**< ACDN0 Equalizer on DL Right channel
    * param1 : ACDNCOMMMODE_e enums
    * param2 : 0
    */
    MM_CommandACDN0EQDLRMode,

    /**< ACDN1 Compressor
    * param1: ACDNCOMMMODE_e enums
    * param2 : 1 to 10 in steps of 1
    */
    MM_CommandACDN1CompressorMode,

    /**< ACDN1 Equalizer on UL
    * param1: ACDNCOMMMODE_e enums
    * param2 : 1 to 10 in steps of 1
    */
    MM_CommandACDN1EQULMode,


    /**< ACDN2 Noise suppressor
    * param1: ACDNCOMMMODE_e enums
    * param2 :0
    */
    MM_CommandACDN2NSMode,

    /**< ACDN2 Equalizer
    * param1: ACDNCOMMMODE_e enums
    * param2 :0
    */
    MM_CommandACDN2EQMode,

    /**< ACDN2 Compressor
    * param1: ACDNCOMMMODE_e enums
    * param2 :0
    */
    MM_CommandACDN2CompressorMode,

    /**< SIDETONE used along with ACDN
    * param1: 0
    * param2 :0
    */
    MM_CommandSideToneConfig,

    /**< SIDETONE enable
    * param1: 0
    * param2 :0
    */
    MM_CommandSideToneEnable,


    /**< SIDETONE disable
    * param1: 0
    * param2 :0
    */
    MM_CommandSideToneDisable,


    /**< SIDETONE gain
    * param1 : Q15 gain value (0x0000 up to 0x8000 with 0x8000 representing +1)
    * param2 : 0
    * Currently, sidetone configuration fits in a 32-bit argument. The sidetone gain in Q15 format
    * is provided in bits 15-0 and the sidetone enable/disable flag is passed in bit 16
    */
    MM_CommandSideToneGain,

    /**< Playback VOLUME
    * param1 : Q15 gain value (0x0000 up to 0x8000 with 0x8000 representing +1)
    * param2 is 0
    * streamID must be sent
    */
    MM_CommandPlaybackVolume,

    /**< Record VOLUME
    * param1 : Q15 gain value (0x0000 up to 0x8000 with 0x8000 representing +1)
    * param2 is 0
    * streamID must be sent
    */
    MM_CommandRecordVolume,

    /**< RTMIXER connect
    * param1 : 0
    * param2 : 0
    */
    MM_CommandRTMixerConnect,

    /**< RTMIXER Gain ramp
    * param1 : 0
    * param2 : 0
    * pRTMGainRamp must be used
    */
    MM_CommandRTGainRamp,

    /**< GAIN gain ramp
    * param1 : 0
    * param2 : 0
    * pMDNGainRamp must be used
    */
    MM_CommandGainRamp,


    /**< REMOVESTREAMID
    * param1 : 0
    * param2 : 0
    */
    MM_Exit
} MM_COMMANDTYPE_e;



/* Reference
ACDN 0 contains 3 algorithms
Alg. no O : AEC-NS  on the DL and UL paths
Alg. no 1 : EQ Left channel on the DL path
Alg. no 2 : EQ Right channel on the DL path
The ACDN1 contains 2 algorithms:
Alg. no O : Compressor on the DL path
Alg. no 1 : Equalizer on the UL path
*/

/**
 * The ACDNCOMMMODE enumeration is used to define acoustic communication mode
 */
typedef enum _ACDNCOMMMODE
{
    ENORMALMODEUP = 0, /**< Normal Mode UP link */
    ESPEAKERPHONEUP,    /**< Speaker phone UP link */
    EWIREDHEADSET,        /**< Wired headset  */
    ENORMALMODEDOWN,  /**< Normal Mode Down link */
    ESPEAKERPHONEDOWN, /**< Speaker phone Down link */
    EAECNSENABLE,  /**< Acoustic Echo Cancellation + Noise Suppression enabled  */
    EEQLChannel,       /**< Equalizer on Left channel */
    EEQRChannel,      /**< Equalizer on Right channel */
    ECOMPRESSORENABLE, /**< Compressor enable */
    EEQ_DLENABLE, /**< Equalizer enabled on Down link */
    EEQ_ULENABLE, /**< Equalizer enabled on Up link */
    ENOMODE,         /**< No mode */
    EGETACOUSTICPROPERTY /**< Get acoustic property */
} ACDNCOMMMODE_e;



/* ADN refers to Audio Device Node.DCTN DASF Control Task Node */

/**
 *  ADN_DCTN_PARAM_t defines DCTN parameters.
 */
typedef struct ADN_DCTN_PARAM
{
    DWORD    ulInPortMask;                /**< Input port mask value */
    DWORD    ulOutPortMask;              /**< Output port mask value */
    DWORD    data[1022];                   /**< Parameters are passed in this array to indicate active ports*/
} ADN_DCTN_PARAM_t;



/**
 * structure used in smooth gain control commands
 */
typedef struct MDN_GAIN_SLOPE {

    DWORD       ulTargetGain;        /**< target gain (Q15 format)*/
    DWORD       ulDurationMS;        /**< duration in msec to reach target gain */

} MDN_GAIN_SLOPE_t;


/**
 * structure used to support gain envelopes ( fade in fade out )
 */
typedef struct MDN_GAIN_ENVELOPE {

    DWORD          ulSize;                                        /**< struct size for cache flush */
    WORD           bDspReleased;                             /**< ARM init to FALSE and DSP set to TRUE when done*/
    DWORD          ulNumGainSlopes_Left;                /**< number of gain slopes (left chan)*/
    DWORD          ulNumGainSlopes_Right;              /**< number of gain slopes (right chan)*/
    MDN_GAIN_SLOPE tGainSlopes[MDN_MAX_GAIN_SLOPES]; /**< Gain values */
} MDN_GAIN_ENVELOPE_t;


/**
 *    The MM_COMMANDDATATYPE structure defines command data structure
 */
typedef struct _MM_COMMANDDATATYPE
{
  HANDLE            hComponent;           /**< Handle to a component ( optional/NULL ) */
  MM_COMMANDTYPE_e    AM_Cmd;     /**< HMM commands */
  DWORD         param1;                     /**< Paramter1 */
  DWORD         param2;                     /**< Parameter2 */
  DWORD         streamID;          /**< Unique Stream ID */
  MDN_GAIN_ENVELOPE_t *pMDNGainRamp; /**< Pointer to MDN gain ramp (optional set to NULL )*/
  ADN_DCTN_PARAM_t *pRTMGainRamp;  /**< Parameters for RTM gain ramp (optional set to NULL ) */

} MM_COMMANDDATATYPE_t;

/*----------------------------------------------------------------------*/

/*---------------Hwcodec adapter related header information ------------*/
#ifdef _HWADAPTER_EXPORTS_
#define HWCODEC_API __declspec(dllexport)
#else
#define HWCODEC_API __declspec(dllimport)
#endif

#define MONO_CHANNEL 1 /**< Mono Channel */
#define STEREO_CHANNEL 2 /**< Stereo Channel */



#define MAX_NUM_OF_INPUT_BUFFERS  4  /**< Maximum number of HWCDEC input buffers(client to hwcodec adapter)*/
#define MAX_NUM_OF_OUTPUT_BUFFERS  4    /**< Maximum number of HWCDEC input buffers(hwcodecadapter to client) */

#define MAX_INPUT_BUFFER_SIZE 4096    /**< Maximum size of the HWCODEC input buffers(client to hwcodec adapter)*/
#define MAX_OUTPUT_BUFFER_SIZE 4096    /**< Maximum size of the HWCODEC output buffers(hwcodecadapter to client)*/

#ifdef DEBUG
#define ZONE_DEBUG_API 1
#define ZONE_DEBUG_MSG 1

#define LOAD_COMMAND_TIMEOUT INFINITE    /**< Timeout for the load command */
#define STATE_CHANGE_TIMEOUT INFINITE    /**< Timeout for state changes */
#define BUFFER_HANDLING_TIMEOUT INFINITE /**< Timeout for buffer transfer */
#else
#define LOAD_COMMAND_TIMEOUT 10000    /**< Timeout for the load command */
#define STATE_CHANGE_TIMEOUT 10000    /**< Timeout for state changes */
#define BUFFER_HANDLING_TIMEOUT 10000 /**< Timeout for buffer transfer */
#endif

/**
 * STREAM_TYPE_e Enum defines the direction of the stream
 */
typedef enum _STREAM_TYPE_e
{
    PCMENC,    /**< PCMENC Pcm encoder */
    PCMDEC     /**< PCMDEC Pcm Decoder */
} STREAM_TYPE_e;


/**
 * DIRECTION_e Enum defines the direction of the stream
 */
typedef enum _DIRECTION_
{
    DIR_IN,       /**< DIR_IN Input */
    DIR_OUT     /**< DIR_OUT Output */
} DIRECTION_e;






/**
 * HWCODEC_PARAM_t Structure used to pass the codec information to the adapter
 */
typedef struct HWCODEC_PARAM
{
    DWORD Channels;         /**< Channels Number of channels */
    DWORD SamplesPerSec;    /**< SamplesPerSec Sampling Rate */
    DWORD BitsPerSample;    /**< BitsPerSample Bits Per Sample  */
} HWCODEC_PARAM_t;


/**
 * HWCODEC_AppRegisterCb_t Structure used to register callbacks from client
 */
typedef struct HWCODEC_AppRegisterCb
{

   /*-------------------------------------------------------------------------------------------*/
   /**
    *  pfnOutputCallback Callback function implemeted to acknowledge the output buffers in decode path
    *  @param [out] pAppBuf - Pointer to application buffer which has been rendered
    *  @param [out] pActualSize - Pointer to the actual size rendered
    *  @param [out] hContext - Handle to the application context registered
    */
    /*-------------------------------------------------------------------------------------------*/
    VOID
    (*pfnOutputCallback)(
     PBYTE pAppBuf,
     DWORD *pActualSize,
     HANDLE hContext
    );




   /*-------------------------------------------------------------------------------------------*/
   /**
    *  pfnInputCallback Callback function implemeted to acknowledge the input buffers in encode path
    *  @param [out] pAppBuf - Pointer to the application buffer which needs to be filled
    *  @param [out] pActualSize - Pointer to the actual size received from HW codec and filled
    *  @param [out] hContext - Handle to the application context registered
    */
    /*-------------------------------------------------------------------------------------------*/
    VOID
    (*pfnInputCallback)(
     PBYTE pAppBuf,
     DWORD *pActualSize,
     HANDLE hContext
    );


   /*-------------------------------------------------------------------------------------------*/
   /**
    *  pfnErrorNotifyCallback Callback function implemeted to notify any runtime errors in the adapter. Will be called only on errors
    *  @param [out] Status - FALSE indicates a failure. The failure has to be handled by the client appropriately.
    *  @param [out] hContext - Handle to the application context registered
    */
    /*-------------------------------------------------------------------------------------------*/
    VOID
    (*pfnErrorNotifyCallback)(
     BOOL Status,
     HANDLE hContext
    );

} HWCODEC_AppRegisterCb_t;

/**
 * HWCODEC_AppInfo_t Structure used to pass the application information to HW Codec
 */
typedef struct HWCODEC_AppInfo
{
    HANDLE hAppInstance;     /**< Application instance or the calling object instance */
    DWORD InAllocSize;         /**< InAllocSize Client specified input size(optional) */
    DWORD OutAllocSize;       /**< OutAllocSize Client specified output size(optional) */
    DWORD InNoOfHWBuffers;      /**< Number of Input buffers allocated by HW codec adapter and sent to Client when GetHWBuffer is set */
    DWORD InBufArray[MAX_NUM_OF_INPUT_BUFFERS]; /**< Array having the internally allocated HWBuffers which must be filled by the client */
    DWORD InBufAllocOffset[MAX_NUM_OF_INPUT_BUFFERS]; /**< Array having cache aligned offset for the buffers. No action from client required */
    DWORD OutNoOfHWBuffers;      /**< Number of Input buffers allocated by HW codec adapter and sent to client when GetHWBuffer is set */
    DWORD OutBufArray[MAX_NUM_OF_OUTPUT_BUFFERS]; /**< Array having the internally allocated HWBuffers which must be filled by the client */
    DWORD OutBufAllocOffset[MAX_NUM_OF_OUTPUT_BUFFERS]; /**< Array having cache aligned offset for the buffers. No action from client required */

} HWCODEC_AppInfo_t;




#ifdef __cplusplus
}
#endif

#endif /*_CHWCODEC_H_ */

