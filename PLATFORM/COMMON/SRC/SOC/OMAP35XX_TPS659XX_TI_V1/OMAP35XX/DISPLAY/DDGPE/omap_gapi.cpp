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

#include "precomp.h"
#include "dispperf.h"
#include "dvfs.h"


#if (_WINCEOSVER<600)

//------------------------------------------------------------------------------
//  Defines

#define ESC_SUCCESS                 1
#define ESC_FAILED                  -1
#define ESC_NOT_SUPPORTED           0

#define GAPI_WIDTH                  240
#define GAPI_HEIGHT                 320

#define GAPI_SHARED_MEM_SIZE        (2 * 1024 * 1024)


//------------------------------------------------------------------------------
//
//  Class:  OMAPGameSurface
//
//  Class to manage a GAPI surface that that is on flat physical memory
//  Derived from OMAPSurface
//
class OMAPGameSurface : public OMAPSurface
{
public:
    //------------------------------------------------------------------------------
    //
    //  Method: constructor
    //
    OMAPGameSurface();

    //------------------------------------------------------------------------------
    //
    //  Method: destructor
    //
    virtual
    ~OMAPGameSurface();


    //------------------------------------------------------------------------------
    //
    //  Method: Allocate
    //
    //  Allocates a surface for with the given parameters
    //
    BOOL    Allocate(
                OMAP_DSS_PIXELFORMAT    ePixelFormat,
                DWORD                   dwWidth,
                DWORD                   dwHeight );

    //------------------------------------------------------------------------------
    //
    //  Properties:
    //

    virtual
    VOID*   VirtualAddr();
        
    //------------------------------------------------------------------------------
    //
    //  DMA Properties:
    //
    //  These properties of the surface are relative to the current orientation of
    //  the suface as well as the given rotation angle requested.  These properties
    //  are used for configuring DSS DMA output
    //

    virtual
    DWORD   Width(
                OMAP_DSS_ROTATION   eRotation = OMAP_DSS_ROTATION_0 );

    virtual
    DWORD   Height(
                OMAP_DSS_ROTATION   eRotation = OMAP_DSS_ROTATION_0 );

    virtual
    DWORD   Stride(
                OMAP_DSS_ROTATION   eRotation = OMAP_DSS_ROTATION_0 );

    virtual
    DWORD   PhysicalAddr(
                OMAP_DSS_ROTATION   eRotation = OMAP_DSS_ROTATION_0,
                BOOL                bMirror = FALSE );

    virtual
    DWORD   PixelIncr(
                OMAP_DSS_ROTATION   eRotation = OMAP_DSS_ROTATION_0,
                BOOL                bMirror = FALSE );
                                
    virtual
    DWORD   RowIncr(
                OMAP_DSS_ROTATION   eRotation = OMAP_DSS_ROTATION_0,
                BOOL                bMirror = FALSE );
                                

    //------------------------------------------------------------------------------
    //
    //  Method: SetOrientation
    //
    //  Sets the orientation of the surface relative to its original creation
    //  width and height.  This will cause a reprogramming of the VRFB tile (page)
    //  and size registers in order to swap surface width and height.
    //
    virtual
    BOOL    SetOrientation(
                OMAP_SURF_ORIENTATION       eOrientation
                );
  
  
 protected:
    //  Game surface properties  
    
    VOID*   m_pSurfaceVirtMem;
    VOID*   m_pSurfacePhysMem;
    DWORD   m_dwSurfacePhysMem;
};



//------------------------------------------------------------------------------
//  Structures


//------------------------------------------------------------------------------
//  Globals



//------------------------------------------------------------------------------
int
OMAPDDGPE::GetGameXInfo(
    ULONG ulSize,
    VOID *pGameInfo
    )
{
    GXDeviceInfo    *pInfo = (GXDeviceInfo*) pGameInfo;
    
    //  Fill out the GAPI device info structure
    if( pInfo->idVersion == kidVersion100 && pInfo != NULL && ulSize >= sizeof(GXDeviceInfo) )
    {
        //  Check for the game surface
        if( m_pGameSurf == NULL )
        {
            BOOL                bResult;
            OMAPGameSurface     *pGameSurf;
            
            //  Allocate a Game Surface object
            pGameSurf = new OMAPGameSurface;
            if( pGameSurf == NULL )
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return ESC_FAILED;
            }
            

            //  Allocate game surface memory to meet the GAPI requirements
            //  which is RGB16 QVGA for PocketPC
            bResult = pGameSurf->Allocate(
                                    OMAP_DSS_PIXELFORMAT_RGB16,
                                    GAPI_WIDTH,
                                    GAPI_HEIGHT );                                        
            if( bResult == FALSE )      
            {
                SetLastError(ERROR_OUTOFMEMORY);
                delete pGameSurf;
                return ESC_FAILED;
            }
            
            //  Save off game surface object
            m_pGameSurf = pGameSurf;     
        }
        
        
        //  Fill out the GAPI info struct with the game surface parameters
        pInfo->ffFormat         = kfDirect | kfDirect565;
        pInfo->cBPP             = PixelFormatToBpp(m_pGameSurf->PixelFormat());
        pInfo->cbStride         = m_pGameSurf->Stride();
        pInfo->cxWidth          = m_pGameSurf->Width();
        pInfo->cyHeight         = m_pGameSurf->Height();
        pInfo->pvFrameBuffer    = m_pGameSurf->VirtualAddr();
               
        
        //  On screen button locations are relative to the actual size of
        //  the LCD, not the game surface
        
        pInfo->vkButtonStartPortrait    = VK_TACTION;
        pInfo->vkButtonStartLandscape   = VK_TACTION;
        pInfo->ptButtonStart.x          = 120;
        pInfo->ptButtonStart.y          = 270;

        pInfo->vkButtonUpPortrait       = VK_UP;
        pInfo->vkButtonUpLandscape      = VK_LEFT;
        pInfo->ptButtonUp.x             = 120;
        pInfo->ptButtonUp.y             = 250;

        pInfo->vkButtonDownPortrait     = VK_DOWN;
        pInfo->vkButtonDownLandscape    = VK_RIGHT;
        pInfo->ptButtonDown.x           = 120;
        pInfo->ptButtonDown.y           = 290;

        pInfo->vkButtonLeftPortrait     = VK_LEFT;
        pInfo->vkButtonLeftLandscape    = VK_DOWN;
        pInfo->ptButtonLeft.x           = 100;
        pInfo->ptButtonLeft.y           = 270;

        pInfo->vkButtonRightPortrait    = VK_RIGHT;
        pInfo->vkButtonRightLandscape   = VK_UP;
        pInfo->ptButtonRight.x          = 140;
        pInfo->ptButtonRight.y          = 270;

        pInfo->vkButtonAPortrait        = VK_TSOFT1;
        pInfo->vkButtonALandscape       = VK_TSOFT1;
        pInfo->ptButtonA.x              = 40;
        pInfo->ptButtonA.y              = 240;

        pInfo->vkButtonBPortrait        = VK_TSOFT2;
        pInfo->vkButtonBLandscape       = VK_TSOFT2;
        pInfo->ptButtonB.x              = 180;
        pInfo->ptButtonB.y              = 240;

        pInfo->vkButtonCPortrait        = VK_TSTAR;
        pInfo->vkButtonCLandscape       = VK_TSTAR;
        pInfo->ptButtonC.x              = 40;
        pInfo->ptButtonC.y              = 320;

        pInfo->pvReserved1 = (void *) 0;
        pInfo->pvReserved2 = (void *) 0;

        return ESC_SUCCESS;
    }
    else
    {
        //  Not valid GAPI structure
        SetLastError(ERROR_INVALID_PARAMETER);
        return ESC_FAILED;
    }
}

//------------------------------------------------------------------------------
int
OMAPDDGPE::GetGameFrameBuffer(
    ULONG ulSize,
    VOID *pGameFrameBuffer
    )
{
    RawFrameBufferInfo *pRFBI = (RawFrameBufferInfo *) pGameFrameBuffer;

    //  Fill out the GAPI raw frame buffer info structure for the primary surface
    if( pRFBI != NULL && ulSize >= sizeof(RawFrameBufferInfo) )
    {
        OMAPSurface     *pPrimarySurface = m_pPrimarySurf->OmapSurface();
        
        pRFBI->wBPP             = (WORD) PixelFormatToBpp(pPrimarySurface->PixelFormat());
        pRFBI->wFormat          = (pPrimarySurface->PixelFormat() == OMAP_DSS_PIXELFORMAT_RGB16) ? FORMAT_565 : FORMAT_OTHER;
        pRFBI->cxStride         = pPrimarySurface->PixelSize();
        pRFBI->cyStride         = pPrimarySurface->Stride();
        pRFBI->cxPixels         = pPrimarySurface->Width();
        pRFBI->cyPixels         = pPrimarySurface->Height();
        pRFBI->pFramePointer    = pPrimarySurface->VirtualAddr();

        return ESC_SUCCESS;
    }
    else
    {
        //  Not valid GAPI structure
        SetLastError(ERROR_INVALID_PARAMETER);
        return ESC_FAILED;
    }
}

//------------------------------------------------------------------------------
int
OMAPDDGPE::GameEnable(
    BOOL bEnable
    )
{
    //  Show game surface on screen
    if( bEnable )
    {
        //  Show the game surface on an overlay
        if( m_pGameSurf )
        {
            //  Disable all overlay pipelines
            m_pDisplayContr->DisablePipeline( OMAP_DSS_PIPELINE_VIDEO1 );
            m_pDisplayContr->DisablePipeline( OMAP_DSS_PIPELINE_VIDEO2 );


            //  Set the attributes and scaling of the pipeline
            m_pDisplayContr->SetPipelineAttribs(  
                                OMAP_DSS_PIPELINE_VIDEO1,
                                OMAP_DSS_DESTINATION_LCD,
                                m_pGameSurf );  

            m_pDisplayContr->SetScalingAttribs(  
                                OMAP_DSS_PIPELINE_VIDEO1,
                                m_pDisplayContr->GetLCDWidth(),
                                m_pDisplayContr->GetLCDHeight() );  


            //  Enable the game surface on LCD                                
            m_pDisplayContr->EnablePipeline( OMAP_DSS_PIPELINE_VIDEO1 ); 
            
            //  Update TV to show game surface
            DetermineTvOutSurface(TRUE);
        }
    }
    else
    {
        //  Turn off game surface on both LCD and TV out
        m_pDisplayContr->DisablePipeline( OMAP_DSS_PIPELINE_VIDEO1 ); 
        m_pDisplayContr->DisablePipeline( OMAP_DSS_PIPELINE_VIDEO2 );           

        //  Update TV to show game surface
        DetermineTvOutSurface(TRUE);
    }
            
    return ESC_SUCCESS;    
}

//------------------------------------------------------------------------------
int
OMAPDDGPE::GameDRAMtoVRAM()
{
    return ESC_SUCCESS;    
}

//------------------------------------------------------------------------------
int
OMAPDDGPE::GameVRAMtoDRAM()
{
    return ESC_SUCCESS;    
}


//------------------------------------------------------------------------------
OMAPGameSurface::OMAPGameSurface()
{
    m_pSurfaceVirtMem = NULL;
    m_pSurfacePhysMem = NULL;
    m_dwSurfacePhysMem = 0;
}

//------------------------------------------------------------------------------
OMAPGameSurface::~OMAPGameSurface()
{
    //  Free shared memory region
    if( m_pSurfaceVirtMem )
    {
        VirtualFree( m_pSurfaceVirtMem, 0, MEM_RELEASE );
    }
    
    //  Free physical memory region
    if( m_pSurfacePhysMem )
    {
        FreePhysMem( m_pSurfacePhysMem );
    }
}

//------------------------------------------------------------------------------
BOOL    
OMAPGameSurface::Allocate(
    OMAP_DSS_PIXELFORMAT    ePixelFormat,
    DWORD                   dwWidth,
    DWORD                   dwHeight
    )
{
    DWORD   dwSize;
    
    //  If already allocated, free
    if( m_pSurfaceVirtMem )
    {
        VirtualFree( m_pSurfaceVirtMem, 0, MEM_RELEASE );
        m_pSurfaceVirtMem = NULL;
    }

    //  Free physical memory region
    if( m_pSurfacePhysMem )
    {
        FreePhysMem( m_pSurfacePhysMem );
        m_pSurfacePhysMem = NULL;
        m_dwSurfacePhysMem = 0;
    }


    //  Initialize the surface properties for this surface type
    m_ePixelFormat   = ePixelFormat;
    m_dwPixelSize    = OMAPDisplayController::PixelFormatToPixelSize(m_ePixelFormat);
    m_eOrientation   = OMAP_SURF_ORIENTATION_STANDARD;
    m_dwWidth        = dwWidth;
    m_dwHeight       = dwHeight;

    //  Set clipping region to be entire surface
    SetClipping( NULL );


    //  Compute size of game surface
    dwSize = m_dwWidth * m_dwHeight * m_dwPixelSize;
    
    //  Allocate memory for the game surface
    m_pSurfacePhysMem = AllocPhysMem( dwSize, PAGE_READWRITE, 0, 0, &m_dwSurfacePhysMem );
    if( m_pSurfacePhysMem == NULL )
    {
        DEBUGMSG (GPE_ZONE_ERROR, (L"ERROR: Unable to allocate game surface physical memory\n"));
        goto cleanUp;
    }
    
    //  Need to perform the following steps to map the physical memory block
    //  into the shared memory region:
    //
    //  1) Reserve 2MB via VirtualAlloc
    //  2) VirtualCopy the physical memory into the reserved virtual memory block
    
    m_pSurfaceVirtMem = VirtualAlloc(0, GAPI_SHARED_MEM_SIZE, MEM_RESERVE, PAGE_NOACCESS);
    if( m_pSurfaceVirtMem == NULL )
    {
        DEBUGMSG (GPE_ZONE_ERROR, (L"ERROR: Unable to reserve game surface shared memory\n"));
        FreePhysMem( m_pSurfacePhysMem );
        m_pSurfacePhysMem = NULL;
        goto cleanUp;
    }

    if( !VirtualCopy(m_pSurfaceVirtMem, (void *)(m_dwSurfacePhysMem >> 8), dwSize, PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL) )
    {
        DEBUGMSG (GPE_ZONE_ERROR, (L"ERROR: Unable to map display buffer physical memory\n"));
        FreePhysMem( m_pSurfacePhysMem );
        VirtualFree( m_pSurfaceVirtMem, 0, MEM_RELEASE );
        m_pSurfacePhysMem = NULL;
        m_pSurfaceVirtMem = NULL;
        goto cleanUp;
    }
        
    //  Change the attributes of the buffer for write combine
    CeSetMemoryAttributes( m_pSurfaceVirtMem, (void *)(m_dwSurfacePhysMem >> 8), dwSize, PAGE_WRITECOMBINE );
        
    //  Clear out the surface memory
    memset( m_pSurfaceVirtMem, 0, dwSize );
    
cleanUp:        
    //  Return
    return (m_pSurfaceVirtMem != NULL);
}

//------------------------------------------------------------------------------
VOID*   
OMAPGameSurface::VirtualAddr()
{
    //  Return VM address of game surface buffer in shared virtual memory
    return m_pSurfaceVirtMem;
}

//------------------------------------------------------------------------------
DWORD   
OMAPGameSurface::Width(
    OMAP_DSS_ROTATION   eRotation
    )
{
    //  No rotation on game surface
    return m_dwWidth;
}

//------------------------------------------------------------------------------
DWORD   
OMAPGameSurface::Height(
    OMAP_DSS_ROTATION   eRotation
    )
{
    //  No rotation on game surface
    return m_dwHeight;
}

//------------------------------------------------------------------------------
DWORD   
OMAPGameSurface::Stride(
    OMAP_DSS_ROTATION   eRotation
    )
{
    //  No rotation on game surface
    return (m_dwPixelSize * m_dwWidth);
}

//------------------------------------------------------------------------------
DWORD   
OMAPGameSurface::PhysicalAddr(
    OMAP_DSS_ROTATION   eRotation,
    BOOL                bMirror 
    )
{
    //  Return game surface physical address
    return m_dwSurfacePhysMem;
}

//------------------------------------------------------------------------------
DWORD   
OMAPGameSurface::PixelIncr(
    OMAP_DSS_ROTATION   eRotation,
    BOOL                bMirror
    )
{
    //  Return increment
    return 1;
}
                            
//------------------------------------------------------------------------------
DWORD   
OMAPGameSurface::RowIncr(
    OMAP_DSS_ROTATION   eRotation,
    BOOL                bMirror
    )
{
    //  Return increment
    return 1;
}


//------------------------------------------------------------------------------
BOOL
OMAPGameSurface::SetOrientation(
    OMAP_SURF_ORIENTATION       eOrientation
    )
{
    //  Ignored
    return TRUE;
}

#endif