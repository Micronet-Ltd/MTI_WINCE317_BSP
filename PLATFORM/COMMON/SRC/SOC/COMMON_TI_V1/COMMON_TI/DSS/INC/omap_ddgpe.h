// All rights reserved ADENEO EMBEDDED 2010
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/

#ifndef __OMAPDDGPE_H__
#define __OMAPDDGPE_H__


//------------------------------------------------------------------------------
//  Defines

#define MIN_SURFACE_SIZE            16384

#define SURFACEMGR_FLAT             0
#define SURFACEMGR_VRFB             1

#define SURFACE_NOROTATION          (0xFFFFFFFF)

#define DEBUG_NEON_MEMORY_LEAK      FALSE

//------------------------------------------------------------------------------
//
//  Class:  OMAPDDGPEGlobals
//
//  Global control attributes read from registry
//
class OMAPDDGPEGlobals
{
public:
    DWORD   m_dwSurfaceMgr;
    DWORD   m_dwRotationAngle;
    DWORD   m_dwOffscreenMemory;
    _TCHAR  m_szContext[MAX_PATH];
    DWORD   m_dwTVOutEnable;
    DWORD   m_dwTvOut_FilterLevel;      // Flicker filter level for TV (0-3)
    DWORD   m_dwTvOut_AspectRatio_W;    // Aspect Ratio for TV (width)
    DWORD   m_dwTvOut_AspectRatio_H;    // Aspect Ratio for TV (height)
    DWORD   m_dwTvOut_Resize_W;         // Percent
    DWORD   m_dwTvOut_Resize_H;         // Percent
    LONG    m_lTvOut_Offset_W;          // Pixels
    LONG    m_lTvOut_Offset_H;          // Pixels
    //DWORD   m_dwDVIEnable;
    DWORD   m_dwCursorEnable;
    DWORD   m_dwDSSIRQ;
    DWORD   m_dwISTPriority;
    DWORD   m_dwEnableGammaCorr;
    DWORD   m_dwEnableWaitForVerticalBlank;
    DWORD   m_dwEnableNeonBlts;
    DWORD   m_dwDssIspRszEnabled;
};

//------------------------------------------------------------------------------
//
//  Class:  OMAPDDGPESurface
//
//  Generic NULL display driver Surface class
//
class OMAPDDGPESurface : public DDGPESurf
{
private:
    OMAPSurface*        m_pSurface;  
    OMAPSurface*        m_pAssocSurface;
    OMAPDDGPESurface*   m_pParentSurface;
    
    
public:
    //------------------------------------------------------------------------------
    //
    //  Method: constructor
    //
    OMAPDDGPESurface();

    //------------------------------------------------------------------------------
    //
    //  Method: constructor for surfaces in video memory
    //
    OMAPDDGPESurface(
        OMAPSurfaceManager* pSurfaceMgr,
        OMAPSurface*        pSurface
        );

    //------------------------------------------------------------------------------
    //
    //  Method: destructor
    //
    virtual
    ~OMAPDDGPESurface();


    //------------------------------------------------------------------------------
    //
    //  Properties: OMAPSurface
    //
    OMAPSurface*        OmapSurface() { return m_pSurface; }

    OMAPDDGPESurface*   Parent() { return m_pParentSurface; }
    VOID                SetParent( OMAPDDGPESurface* pParent ) { m_pParentSurface = pParent; }

    OMAPSurface*        OmapAssocSurface() { return m_pAssocSurface; }
    VOID                SetAssocSurface( OMAPSurface* pAssocSurface ) { m_pAssocSurface = pAssocSurface; }
    

    //------------------------------------------------------------------------------
    //
    //  Method: SetOrientation
    //
    //  Sets the orientation of the surface relative to its original creation
    //  width and height. 
    //
    virtual
    BOOL    SetOrientation(
                OMAP_SURF_ORIENTATION       eOrientation
                );
};



//------------------------------------------------------------------------------
//
//  Class:  OMAPDDGPE
//
//  Generic NULL display driver that supports both DDraw and GDI interfaces.
//
class OMAPDDGPE : public DDGPE
{
protected:

    GPEModeEx m_gpeDefaultMode;

    DWORD     m_iGraphicsRotate;                            // VRFB rotate angle; keep m_iRotate always = DMDO_0
    
    CRITICAL_SECTION            m_csOperationLock;          // Lock for operations that need to be serialized
    
    OMAPDisplayController*      m_pDisplayContr;            // OMAP Display Controller
    OMAPSurfaceManager*         m_pSurfaceMgr;              // Video memory surface manager
    
    BOOL                        m_bFlipInProgress;          // Flag indicating flip in progress
    
    OMAPDDGPESurface*           m_pPrimarySurf;             // Primary surface layer   (gfx)
    OMAPDDGPESurface*           m_pOverlay1Surf;            // Overlay surface layer 1 (video 1)
    OMAPDDGPESurface*           m_pOverlay2Surf;            // Overlay surface layer 2 (video 2)

    RECT                        m_rcOverlay1Src;            // Cached src and dest rects for overlay layers
    RECT                        m_rcOverlay1Dest;
    RECT                        m_rcOverlay2Src;
    RECT                        m_rcOverlay2Dest;


    OMAPDDGPESurface*           m_pTVSurf;                  // Surface being displayed on TV
    OMAP_DSS_PIPELINE           m_eTVPipeline;              // Pipeline designated for TV out (video1 or video2)

    BOOL                        m_bTVOutEnable;             // TV out enabled/disabled
    DWORD                       m_dwTvOut_FilterLevel;      // Flicker filter level for TV (0-3)
    DWORD                       m_dwTvOut_AspectRatio_W;    // Aspect Ratio for TV (width)
    DWORD                       m_dwTvOut_AspectRatio_H;    // Aspect Ratio for TV (height)
    DWORD                       m_dwTvOut_Resize_W;         // Percent
    DWORD                       m_dwTvOut_Resize_H;         // Percent
    LONG                        m_lTvOut_Offset_W;          // Pixels
    LONG                        m_lTvOut_Offset_H;          // Pixels

    BOOL                        m_bDVIEnable;               // DVI enabled/disabled
    BOOL                        m_bEnableGammaCorr;
    

    BOOL                        m_bGameEnable;              // GAPI enabled
    BOOL                        m_bGameScale;               // Scale GAPI surfaces to display full screen
    OMAPSurface*                m_pGameSurf;                // GAPI game surface

    BOOL                        m_bHdmiEnable;              // HDMI enabled

    BOOL                        m_bDssIspRszEnabled;        // resize using ISP enabled
    
private:
    // software cursor emulation
    BOOL   m_cursorDisabled;
    BOOL   m_cursorVisible;
    BOOL   m_cursorForcedOff;
    RECTL  m_cursorRect;
    POINTL m_cursorSize;
    POINTL m_cursorHotspot;
    UCHAR* m_cursorStore;
    UCHAR* m_cursorXor;
    UCHAR* m_cursorAnd;

    SCODE WrappedEmulatedLine(GPELineParms*);

#if DEBUG_NEON_MEMORY_LEAK
    BOOL m_bNeonBlt;
    DWORD m_dwNeonBltCount;
    DWORD m_dwNeonBltCountLastMessage;
	MEMORYSTATUS m_MemoryStatusBefore;
	MEMORYSTATUS m_MemoryStatusAfter;
	DWORD m_dwAvailPhysDelta;
	DWORD m_dwAvailPageFileDelta;
#endif
    
public:
    //------------------------------------------------------------------------------
    //
    //  Method: constructor
    //
    OMAPDDGPE();

    //------------------------------------------------------------------------------
    //
    //  Method: destructor
    //
    virtual
    ~OMAPDDGPE();

    //------------------------------------------------------------------------------
    //
    //  Method: NumModes
    //
    virtual
    int
    NumModes();

    //------------------------------------------------------------------------------
    //
    //  Method: GetModeInfo, GetModeInfoEx
    //
    virtual
    SCODE
    GetModeInfo(
        GPEMode * pMode,
        int       modeNumber
        );

    virtual
    SCODE
    GetModeInfoEx(
        GPEModeEx * pModeEx,
        int         modeNumber
        );

    virtual
    OMAP_DSS_PIXELFORMAT
    GetPrimaryPixelFormat();
    
    //------------------------------------------------------------------------------
    //
    //  Method: SetMode
    //
    virtual
    SCODE
    SetMode(
        int        modeId,
        HPALETTE * palette
        );

    //------------------------------------------------------------------------------
    //
    //  Method: SetPalette
    //
    virtual
    SCODE
    SetPalette(
        const PALETTEENTRY * source,
        USHORT               firstEntry,
        USHORT               numEntries
        );

    //------------------------------------------------------------------------------
    //
    //  Method: GetVirtualVideoMemory
    //
    void
    GetVirtualVideoMemory(
        unsigned long * virtualMemoryBase,
        unsigned long * videoMemorySize,
        unsigned long * videoMemoryFree
        );

	// Helper functions
    VOID CursorOn();
    VOID CursorOff();

#if (_WINCEOSVER==700)
    void
    GetVirtualVideoMemoryList(
        LPDDHAL_DDVIDMEMLIST    pVidMemList
        );
#endif

    //------------------------------------------------------------------------------
    //
    //  Method: GAPI support functions
    //
#if (_WINCEOSVER<600)
    int
    GetGameXInfo(
        ULONG ulSize,
        VOID *pGameInfo
        );

    int
    GetGameFrameBuffer(
        ULONG ulSize,
        VOID *pGameFrameBuffer
        );

    int
    GameEnable(
        BOOL bEnable
        );

    int
    GameVRAMtoDRAM();

    int
    GameDRAMtoVRAM();

#endif

    //------------------------------------------------------------------------------
    //
    //  Method: GetMasks
    //
    virtual
    ULONG *
    GetMasks();

    //------------------------------------------------------------------------------
    //
    //  Method: SetPointerShape
    //
    virtual
    SCODE
    SetPointerShape(
        GPESurf * mask,
        GPESurf * colorSurface,
        int xHotspot, 
        int yHotspot, 
        int xSize, 
        int ySize 
        );

    //------------------------------------------------------------------------------
    //
    //  Method: MovePointer
    //
    virtual
    SCODE
    MovePointer(
        int xPosition,
        int yPosition
        );

    //------------------------------------------------------------------------------
    //
    //  Method: IsBusy, WaitForNotBusy, InVBlank
    //
    virtual
    int
    IsBusy();

    virtual
    void
    WaitForNotBusy();

    virtual
    int
    InVBlank();

    virtual
    void
    WaitForVBlank();

    DWORD 
    GetScanLine();

    virtual
    int
    FlipInProgress();

    BOOL
    SurfaceFlipping(
        OMAPDDGPESurface*   pSurf,
        BOOL                matchExactSurface
        );

    //------------------------------------------------------------------------------
    //
    //  Method: Line
    //
    virtual
    SCODE
    Line(
        GPELineParms * lineParameters,
        EGPEPhase      phase
        );

    //------------------------------------------------------------------------------
    //
    //  Method: BltPrepare
    //
    virtual
    SCODE
    BltPrepare(
        GPEBltParms * blitParameters
        );

    //------------------------------------------------------------------------------
    //
    //  Method: BltComplete
    //
    virtual
    SCODE
    BltComplete(
        GPEBltParms * blitParameters
        );

    //------------------------------------------------------------------------------
    //
    //  Method: DMA Blt routines
    //
    
    SCODE DMAFill(GPEBltParms* pParms);
    SCODE DMASrcCopy(GPEBltParms* pParms);


    //------------------------------------------------------------------------------
    //
    //  Method: OMAP optimized Blt routines
    //

    SCODE DesignateBlt(GPEBltParms* pParms);

    void DesignateBltROP1(GPEBltParms*, ROP4);
    void DesignateBltROP2(GPEBltParms*, ROP4);
    void DesignateBltROP3(GPEBltParms*, ROP4);
    void DesignateBltROP4(GPEBltParms*, ROP4);

    // ROP1s

    // ROP2s
    void DesignateBltSRCCOPY(GPEBltParms*);
      void DesignateBltSRCCOPY_toLUT8(GPEBltParms*);
        void DesignateBltSRCCOPY_LUT8toLUT8(GPEBltParms*);
      void DesignateBltSRCCOPY_toRGB16(GPEBltParms*);
        void DesignateBltSRCCOPY_LUT8toRGB16(GPEBltParms*);
        void DesignateBltSRCCOPY_RGB16toRGB16(GPEBltParms*);
        void DesignateBltSRCCOPY_BGR24toRGB16(GPEBltParms*);
        void DesignateBltSRCCOPY_BGRA32toRGB16(GPEBltParms*);
      void DesignateBltSRCCOPY_toBGR24(GPEBltParms*);
        void DesignateBltSRCCOPY_LUT8toBGR24(GPEBltParms*);
        void DesignateBltSRCCOPY_RGB16toBGR24(GPEBltParms*);
        void DesignateBltSRCCOPY_BGR24toBGR24(GPEBltParms*);
        void DesignateBltSRCCOPY_BGRA32toBGR24(GPEBltParms*);
      void DesignateBltSRCCOPY_toBGRA32(GPEBltParms*);
        void DesignateBltSRCCOPY_LUT8toBGRA32(GPEBltParms*);
        void DesignateBltSRCCOPY_RGB16toBGRA32(GPEBltParms*);
        void DesignateBltSRCCOPY_BGR24toBGRA32(GPEBltParms*);
        void DesignateBltSRCCOPY_BGRA32toBGRA32(GPEBltParms*);

    // ROP3s
    void DesignateBltPATCOPY(GPEBltParms*);
      void DesignateBltPATCOPY_toLUT8(GPEBltParms*);
        void DesignateBltPATCOPY_solidtoLUT8(GPEBltParms*);
      void DesignateBltPATCOPY_toRGB16(GPEBltParms*);
        void DesignateBltPATCOPY_solidtoRGB16(GPEBltParms*);
      void DesignateBltPATCOPY_toBGR24(GPEBltParms*);
        void DesignateBltPATCOPY_solidtoBGR24(GPEBltParms*);
      void DesignateBltPATCOPY_toBGRA32(GPEBltParms*);
        void DesignateBltPATCOPY_solidtoBGRA32(GPEBltParms*);

    // ROP4s
    void DesignateBltAACC(GPEBltParms*);
      void DesignateBltAACC_toRGB16(GPEBltParms*);
        void DesignateBltAACC_RGB16toRGB16(GPEBltParms*);
      void DesignateBltAACC_toBGRA32(GPEBltParms*);
        void DesignateBltAACC_BGRA32toBGRA32(GPEBltParms*);

    SCODE EmulatedBlockFill8(GPEBltParms* pParms);
    SCODE EmulatedBlockCopy8to8(GPEBltParms* pParms);
    
    SCODE EmulatedBlockFill16(GPEBltParms* pParms);
    SCODE EmulatedBlockCopyLUT8to16(GPEBltParms* pParms);
    SCODE EmulatedBlockCopy16to16(GPEBltParms* pParms);
    SCODE EmulatedBlockCopyBGR24toRGB16(GPEBltParms* pParms);
    SCODE EmulatedBlockCopyBGRx32toRGB16(GPEBltParms* pParms);
    
    SCODE EmulatedBlockFill24(GPEBltParms* pParms);
    SCODE EmulatedBlockCopyLUT8to24(GPEBltParms* pParms);
    SCODE EmulatedBlockCopyRGB16toBGR24(GPEBltParms* pParms);
    SCODE EmulatedBlockCopy24to24(GPEBltParms* pParms);
    SCODE EmulatedBlockCopyXYZx32toXYZ24(GPEBltParms* pParms);
    
    SCODE EmulatedBlockFill32(GPEBltParms* pParms);
    SCODE EmulatedBlockCopyLUT8to32(GPEBltParms* pParms);
    SCODE EmulatedBlockCopyRGB16toBGRx32(GPEBltParms* pParms);
    SCODE EmulatedBlockCopyXYZ24toXYZx32(GPEBltParms* pParms);
    SCODE EmulatedBlockCopy32to32(GPEBltParms* pParms);

    SCODE EmulatedMaskCopy16to16withA1(GPEBltParms* pParms);
    SCODE EmulatedMaskCopy32to32withA1(GPEBltParms* pParms);

    SCODE EmulatedPerPixelAlphaBlendBGRA32toRGB16(GPEBltParms* pParms);


    //------------------------------------------------------------------------------
    //
    //  Method: DrvEscape
    //
    virtual
    ULONG
    DrvEscape(
        SURFOBJ * pso,
        ULONG     iEsc,
        ULONG     cjIn,
        void    * pvIn,
        ULONG     cjOut,
        void    * pvOut
        );

    //------------------------------------------------------------------------------
    //
    //  Method: Lock, Unlock
    //
    virtual
    void
    Lock() { EnterCriticalSection( &m_csOperationLock ); }

    virtual
    void
    Unlock() { LeaveCriticalSection( &m_csOperationLock ); }

    //------------------------------------------------------------------------------
    //
    //  Method: AllocSurface
    //
    virtual
    SCODE
    AllocSurface(
        GPESurf    ** surface,
        int           width,
        int           height,
        EGPEFormat    format,
        int           surfaceFlags
        );

    virtual
    SCODE
    AllocSurface(
        DDGPESurf         ** ppSurf,
        int                  width,
        int                  height,
        EGPEFormat           format,
        EDDGPEPixelFormat    pixelFormat,
        int                  surfaceFlags
        );

    virtual
    SCODE
    AllocSurface(
        OMAPDDGPESurface  ** ppSurf,
        OMAP_DSS_PIXELFORMAT pixelFormat,
        int                  width,
        int                  height
        );

    
    SCODE
    AllocSurface(
        OMAPDDGPESurface  *  pSurf,
        OMAP_DSS_PIXELFORMAT pixelFormat,
        int                  width,
        int                  height        
        );

    //------------------------------------------------------------------------------
    //
    //  Method: NumVisibleOverlays
    //          ShowOverlay
    //          MoveOverlay
    //          HideOverlay
    //
    DWORD
    NumVisibleOverlays();    

    DWORD
    ShowOverlay(
        OMAPDDGPESurface*   pOverlaySurf,
        RECT*               pSrcRect,
        RECT*               pDestRect,
        BOOL                bMirror = FALSE
        );    

    DWORD
    MoveOverlay(
        OMAPDDGPESurface*   pOverlaySurf,
        LONG                lXPos,
        LONG                lYPos
        );    

    DWORD
    HideOverlay(
        OMAPDDGPESurface*   pOverlaySurf
        );

    //------------------------------------------------------------------------------
    //
    //  Method: SetSrcColorKey
    //          SetDestColorKey
    //          SetAlphaConst
    //          DisableColorKey
    //
    DWORD
    SetSrcColorKey(
        DWORD               dwColorKey
        );    

    DWORD
    SetDestColorKey(
        DWORD               dwColorKey
        );    

    DWORD
    SetAlphaConst(
        OMAP_DSS_COLORKEY   eColorKey,
        DWORD               dwColorKey
        );    

    DWORD
    DisableColorKey(
        );    

    DWORD
    DisableAlphaConst(
        );    

    //------------------------------------------------------------------------------
    //
    //  Method: FlipSurface
    //
    DWORD
    FlipSurface(
        OMAPDDGPESurface*   pSurf
        );    


    //------------------------------------------------------------------------------
    //
    //  Method: DetermineTvOutSurface
    //
    VOID
    DetermineTvOutSurface(
        BOOL bForceUpdate = FALSE
        );    


    BOOL                
    GetTvOutScaling(
        OMAPSurface *pSurface,
        DWORD       *pScaledWidth,
        DWORD       *pScaledHeight,
        DWORD       *pScaledXPos = NULL,
        DWORD       *pScaledYPos = NULL
        );    
    
    BOOL 
    CopySurfaceParams(
        OMAPDDGPESurface*   pSurfSrc,
        OMAPDDGPESurface*   pSurfDest);

    //------------------------------------------------------------------------------
    //
    //  Method: PixelFormatToBpp
    //          PixelFormatToGPEFormat
    //          PixelFormatToDDGPEFormat
    //          PixelFormatToBitMask
    //          
    //  Various global utility functions
    //
    static
    DWORD               PixelFormatToBpp(
                            OMAP_DSS_PIXELFORMAT    ePixelFormat
                            );    

    static
    EGPEFormat          PixelFormatToGPEFormat(
                            OMAP_DSS_PIXELFORMAT    ePixelFormat
                            );    

    static
    EDDGPEPixelFormat   PixelFormatToDDGPEFormat(
                            OMAP_DSS_PIXELFORMAT    ePixelFormat
                            );    

    static
    BOOL                PixelFormatToBitMask(
                            OMAP_DSS_PIXELFORMAT    ePixelFormat,
                            DWORD                   *pAlphaBitMask,
                            DWORD                   *pRBitMask,
                            DWORD                   *pGBitMask,
                            DWORD                   *pBBitMask
                            );    



    //------------------------------------------------------------------------------
    //
    //  Method: buildDDHALInfo
    //
    friend
    void
    buildDDHALInfo(
        LPDDHALINFO lpddhi,
        DWORD       modeidx
        );

    virtual
    void
    DDHALInfo(
        LPDDHALINFO lpddhi,
        DWORD modeidx 
        );
};



#endif __OMAPDDGPE_H__

