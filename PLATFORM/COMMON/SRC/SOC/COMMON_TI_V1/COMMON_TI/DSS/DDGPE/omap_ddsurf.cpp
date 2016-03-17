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

#include "precomp.h"


BOOL OMAPDDGPE::CopySurfaceParams(OMAPDDGPESurface*   pSurfSrc,OMAPDDGPESurface*   pSurfDest)
{
    OMAPSurface * pOMAPSurfSrc = pSurfSrc->OmapSurface();
    OMAPSurface * pOMAPSurfDest = pSurfDest->OmapSurface();
    BOOL useReszier;

    useReszier=pOMAPSurfDest->UseResizer(pOMAPSurfSrc->isResizerEnabled());    
    pOMAPSurfDest->SetRSZParams(*(pOMAPSurfSrc->ResizeParams()));
    pOMAPSurfDest->SetRSZHandle(pOMAPSurfSrc->GetRSZHandle(FALSE),FALSE);   

    return TRUE;
}


//------------------------------------------------------------------------------
SCODE
OMAPDDGPE::AllocSurface(
    GPESurf    ** ppSurf,
    int           width,
    int           height,
    EGPEFormat    format,
    int           surfaceFlags
    )
{
    UNREFERENCED_PARAMETER(surfaceFlags);
    //  Allocate GDI from video memory if 2D ops can be accelerated 
    //  Otherwise, allocate a standard GPESurf
    if( m_pSurfaceMgr->SupportsOffscreenSurfaces() &&
        (width*height >= 8192) &&
        (format == m_pPrimarySurf->Format()) )
    {
        BOOL                    bResult;
        OMAPSurface*            pOmapSurface;
    

        //  Allocate OMAP surface
        bResult = m_pSurfaceMgr->AllocateGDI(
                                m_pPrimarySurf->OmapSurface()->PixelFormat(),
                                width,
                                height,
                                &pOmapSurface );
        if (bResult == FALSE)
        {
            DEBUGMSG(GPE_ZONE_WARNING, (L"OMAP GDI AllocSurface - Out of Video Memory\n"));
            goto trySystemMem;
        }

        //  Allocate a OMAPDDGPESurface object to wrap the OMAP surface
        *ppSurf = new OMAPDDGPESurface(m_pSurfaceMgr, pOmapSurface);
        if (*ppSurf == NULL)
        {
            //  Out of system memory
            DEBUGMSG(GPE_ZONE_WARNING, (L"OMAP GDI AllocSurface - Out of Memory\n"));
            delete pOmapSurface;
            goto trySystemMem;
        }

        //  Surface created in video memory
        return S_OK;
    }

trySystemMem:

    // Allocate GDI surfaces from system memory
    *ppSurf = new GPESurf(width, height, format);
    if (*ppSurf != NULL)
    {
        // check we allocated bits succesfully
        if (((*ppSurf)->Buffer()) == NULL)
        {
            delete *ppSurf;
        }
        else
        {
            return S_OK;
        }
    }

    DEBUGMSG(GPE_ZONE_WARNING, (L"GDI AllocSurface - Out of Memory\r\n"));
    return E_OUTOFMEMORY;
}


//------------------------------------------------------------------------------
SCODE 
OMAPDDGPE::AllocSurface(
    DDGPESurf         ** ppSurf,
    int                  width,
    int                  height,
    EGPEFormat           format,
    EDDGPEPixelFormat    pixelFormat,
    int                  surfaceFlags
    )
{
    DWORD bpp  = EGPEFormatToBpp[format];
    DWORD stride = ((bpp * width + 31) >> 5) << 2;

    UNREFERENCED_PARAMETER(surfaceFlags);

    // Allocate DDraw surface from system memory
    *ppSurf = new DDGPESurf(width, height, stride, format, pixelFormat);
    if (*ppSurf != NULL)
    {
        // check we allocated bits succesfully
        if (((*ppSurf)->Buffer()) == NULL)
        {
            delete *ppSurf;
        }
        else
        {
            return S_OK;
        }
    }

    DEBUGMSG(GPE_ZONE_WARNING, (L"DDraw AllocSurface - Out of Memory\r\n"));
    return E_OUTOFMEMORY;
}

//------------------------------------------------------------------------------
SCODE 
OMAPDDGPE::AllocSurface(
    OMAPDDGPESurface  ** ppSurf,
    OMAP_DSS_PIXELFORMAT pixelFormat,
    int                  width,
    int                  height
    )
{
    BOOL            bResult;
    OMAPSurface*    pOmapSurface;

    //  Allocate OMAP surface
    bResult = m_pSurfaceMgr->Allocate(
                            pixelFormat,
                            width,
                            height,
                            &pOmapSurface );
    if (bResult == FALSE)
    {
        DEBUGMSG(GPE_ZONE_WARNING, (L"OMAP DDraw AllocSurface - Out of Video Memory\n"));
        return DDERR_OUTOFVIDEOMEMORY;
    }

    //  Allocate a OMAPDDGPESurface object to wrap the OMAP surface
    *ppSurf = new OMAPDDGPESurface(m_pSurfaceMgr, pOmapSurface);
    if (*ppSurf == NULL)
    {
        //  Out of system memory
        DEBUGMSG(GPE_ZONE_WARNING, (L"OMAP DDraw AllocSurface - Out of Memory\n"));
        delete pOmapSurface;
        return E_OUTOFMEMORY;
    }

    //  Surface created in video memory
    return S_OK;
}

SCODE 
OMAPDDGPE::AllocSurface(
    OMAPDDGPESurface*    pSurf,
    OMAP_DSS_PIXELFORMAT pixelFormat,
    int                  width,
    int                  height    
    )
{
    BOOL            bResult;
    OMAPSurface*    pAssocOmapSurface;
    OMAPSurface*    pOmapSurface = pSurf->OmapSurface();

    if (!m_bDssIspRszEnabled)
        return DDERR_GENERIC;

    //  Allocate OMAP surface
    bResult = m_pSurfaceMgr->Allocate(
                            pixelFormat,
                            width,
                            height,
                            &pAssocOmapSurface,
                            pOmapSurface );
    if (bResult == FALSE)
    {
        DEBUGMSG(GPE_ZONE_WARNING, (L"OMAP DDraw AllocSurface - Out of Video Memory\n"));
        return DDERR_OUTOFVIDEOMEMORY;
    }
    
    // add rsz surface to pSurf
    pSurf->SetAssocSurface(pAssocOmapSurface);    
    
    //  Surface created in video memory
    return S_OK;
}


//------------------------------------------------------------------------------
DWORD
OMAPDDGPE::NumVisibleOverlays()
{
    //  Return number of visible overlays
    if( m_pOverlay1Surf && m_pOverlay2Surf )
        return 2;
        
    if( m_pOverlay1Surf || m_pOverlay2Surf )
        return 1;

    return 0;            
}  

//------------------------------------------------------------------------------
DWORD
OMAPDDGPE::ShowOverlay(
    OMAPDDGPESurface*   pOverlaySurf,
    RECT*               pSrcRect,
    RECT*               pDestRect,
    BOOL                bMirror
    )
{
    DWORD               dwResult = DD_OK;
    BOOL                bResult;
    OMAP_DSS_ROTATION   eRotation = OMAP_DSS_ROTATION_0;
    RECT                hdmiRect;
    
    
    //  HDMI support
    if( m_bHdmiEnable )
    {
        //  Just show the overlay in full screen
        DWORD srcWidth = pSrcRect->right - pSrcRect->left;
        DWORD srcHeight = pSrcRect->bottom - pSrcRect->top;
        DWORD dstWidth;
        DWORD dstHeight;
        float fSrcAspect = (float)srcWidth / (float) srcHeight;
        float fDstAspect = (float)1280.0 / (float)720.0;

        //  Scale overlay to fill screen
        if( fDstAspect > fSrcAspect )
        {
            dstWidth  = (DWORD)(fSrcAspect * 1280.0 / fDstAspect);
            dstHeight = 720;
        }
        else
        {
            dstWidth  = 1280;
            dstHeight = (DWORD)(fDstAspect * 720.0 / fSrcAspect);
        }

        //  Scaled to fit HDMI 720p screen
        hdmiRect.left   = (1280 - dstWidth)/2;
        hdmiRect.right  = hdmiRect.left + dstWidth;
        hdmiRect.top    = (720 - dstHeight)/2;
        hdmiRect.bottom = hdmiRect.top + dstHeight;

        pDestRect = &hdmiRect;        
        
        //  Disable GFX pipeline as we are in fullscreen to save memory bandwith. 
        m_pDisplayContr->DisablePipeline(OMAP_DSS_PIPELINE_GFX);              
        m_pDisplayContr->DisableColorKey( OMAP_DSS_COLORKEY_TRANS_SOURCE, OMAP_DSS_DESTINATION_LCD );
    }
    
    //  Check if the overlay surface is part of a flipping chain on layer 1 (video 1)
    //  and that video1 is not the TV out surface
    if( m_pOverlay1Surf && (m_pOverlay1Surf->Parent() == pOverlaySurf->Parent()) )
    {
        //  Ensure that the VID1 pipeline is setup for TV out (repositioning not allowed to TV)
        if( !m_bTVOutEnable || m_eTVPipeline != OMAP_DSS_PIPELINE_VIDEO1 )
        {
            // if back-to-back show overlay 
            CopySurfaceParams(pOverlaySurf->Parent(), pOverlaySurf);
            //  Mirror the pipeline output
            m_pDisplayContr->MirrorPipeline(
                                OMAP_DSS_PIPELINE_VIDEO1,
                                bMirror );                                               

            //  Update the scaling info for the pipeline
            m_pDisplayContr->UpdateScalingAttribs(
                                OMAP_DSS_PIPELINE_VIDEO1,
                                pSrcRect,
                                pDestRect );   
            CopySurfaceParams(pOverlaySurf, pOverlaySurf->Parent());
                                
            //  Cache the rects    
            m_rcOverlay1Src  = *pSrcRect;                                                             
            m_rcOverlay1Dest = *pDestRect;
            goto done;
        }
    }


    //  Check if the overlay surface is part of a flipping chain on layer 2 (video 2)
    //  and that video2 is not the TV out surface
    if( m_pOverlay2Surf && (m_pOverlay2Surf->Parent() == pOverlaySurf->Parent()) )
    {
        //  Ensure that the VID1 pipeline is setup for TV out (repositioning not allowed to TV)
        if( !m_bTVOutEnable || m_eTVPipeline != OMAP_DSS_PIPELINE_VIDEO2 )
        {
            // if back-to-back show overlay 
            CopySurfaceParams(pOverlaySurf->Parent(), pOverlaySurf);

            //  Mirror the pipeline output
            m_pDisplayContr->MirrorPipeline(
                                OMAP_DSS_PIPELINE_VIDEO2,
                                bMirror );                                               

            //  Update the scaling info for the pipeline
            m_pDisplayContr->UpdateScalingAttribs(
                                OMAP_DSS_PIPELINE_VIDEO2,
                                pSrcRect,
                                pDestRect );   

            CopySurfaceParams(pOverlaySurf, pOverlaySurf->Parent());
                                
            //  Cache the rects    
            m_rcOverlay2Src  = *pSrcRect;                                                             
            m_rcOverlay2Dest = *pDestRect;
            goto done;

        }
    }
    

    //  If both overlays are active and this is not a flipping chain update, can't show any more overlays
    if( m_pOverlay1Surf != NULL && m_pOverlay2Surf != NULL )
    {
        DEBUGMSG(GPE_ZONE_ERROR, (L"DDraw ShowOverlay - already showing 2 overlays\r\n"));
            
        dwResult = (DWORD) DDERR_OUTOFCAPS;
        goto done;
    }


    //  If there is no current overlay surface on VID1, setup VID1 to show this one
    if( m_pOverlay1Surf == NULL )
    {
        //  Set the rotation of the pipeline to match the GFX plane
        switch( m_iGraphicsRotate )
        {
            case DMDO_0:
                eRotation = OMAP_DSS_ROTATION_0;
                break;    

            case DMDO_90:
                eRotation = OMAP_DSS_ROTATION_90;
                break;    

            case DMDO_180:
                eRotation = OMAP_DSS_ROTATION_180;
                break;    

            case DMDO_270:
                eRotation = OMAP_DSS_ROTATION_270;
                break;    
        }


        //  Disable the pipeline
        bResult = m_pDisplayContr->DisablePipeline(
                                        OMAP_DSS_PIPELINE_VIDEO1 );                                               


        //  Configure the display controller to show this overlay on the LCD scaled to the dest rect size
        //  using video 1 layer
        bResult = m_pDisplayContr->SetPipelineAttribs(
                                        OMAP_DSS_PIPELINE_VIDEO1,
                                        OMAP_DSS_DESTINATION_LCD, 
                                        pOverlaySurf->OmapSurface() );                                 

        //  Rotate the pipeline output
        bResult = m_pDisplayContr->RotatePipeline(
                                        OMAP_DSS_PIPELINE_VIDEO1,
                                        eRotation );                                               

        //  Mirror the pipeline output
        bResult = m_pDisplayContr->MirrorPipeline(
                                        OMAP_DSS_PIPELINE_VIDEO1,
                                        bMirror );                                               

        //  Set the scaling info for the pipeline
        bResult = m_pDisplayContr->SetScalingAttribs(
                                        OMAP_DSS_PIPELINE_VIDEO1,
                                        pSrcRect,
                                        pDestRect );                                 

        CopySurfaceParams(pOverlaySurf, pOverlaySurf->Parent());
        
        //  Enable the pipeline
        bResult = m_pDisplayContr->EnablePipeline(
                                        OMAP_DSS_PIPELINE_VIDEO1 );                                               

        //  Cache current surface as the overlay surface
        m_pOverlay1Surf = pOverlaySurf;

        //  Cache the rects    
        m_rcOverlay1Src  = *pSrcRect;                                                             
        m_rcOverlay1Dest = *pDestRect;
    }


    //  If there is no current overlay surface on VID2, setup VID2 to show this one
    else if( m_pOverlay2Surf == NULL )
    {
        //  Set the rotation of the pipeline to match the GFX plane
        switch( m_iGraphicsRotate )
        {
            case DMDO_0:
                eRotation = OMAP_DSS_ROTATION_0;
                break;    

            case DMDO_90:
                eRotation = OMAP_DSS_ROTATION_90;
                break;    

            case DMDO_180:
                eRotation = OMAP_DSS_ROTATION_180;
                break;    

            case DMDO_270:
                eRotation = OMAP_DSS_ROTATION_270;
                break;    
        }


        //  Disable the pipeline
        bResult = m_pDisplayContr->DisablePipeline(
                                        OMAP_DSS_PIPELINE_VIDEO2 );                                               


        //  Configure the display controller to show this overlay on the LCD scaled to the dest rect size
        //  using video 1 layer
        bResult = m_pDisplayContr->SetPipelineAttribs(
                                        OMAP_DSS_PIPELINE_VIDEO2,
                                        OMAP_DSS_DESTINATION_LCD, 
                                        pOverlaySurf->OmapSurface() );                                 

        //  Rotate the pipeline output
        bResult = m_pDisplayContr->RotatePipeline(
                                        OMAP_DSS_PIPELINE_VIDEO2,
                                        eRotation );                                               

        //  Mirror the pipeline output
        bResult = m_pDisplayContr->MirrorPipeline(
                                        OMAP_DSS_PIPELINE_VIDEO2,
                                        bMirror );                                               

        //  Set the scaling info for the pipeline
        bResult = m_pDisplayContr->SetScalingAttribs(
                                        OMAP_DSS_PIPELINE_VIDEO2,
                                        pSrcRect,
                                        pDestRect );                                 

        CopySurfaceParams(pOverlaySurf, pOverlaySurf->Parent());
        
        //  Enable the pipeline
        bResult = m_pDisplayContr->EnablePipeline(
                                        OMAP_DSS_PIPELINE_VIDEO2 );                                               

        //  Cache current surface as the overlay surface
        m_pOverlay2Surf = pOverlaySurf;

        //  Cache the rects    
        m_rcOverlay2Src  = *pSrcRect;                                                             
        m_rcOverlay2Dest = *pDestRect;
    }


    //  Get the best surface to display on the TV
    DetermineTvOutSurface();


done:
    //  Return result
    return dwResult;
}  

//------------------------------------------------------------------------------
DWORD
OMAPDDGPE::MoveOverlay(
    OMAPDDGPESurface*   pOverlaySurf,
    LONG                lXPos,
    LONG                lYPos
    )
{
    //  Reposition the overlay (TV out is never repositioned b/c it is scaled)
    if( m_pOverlay1Surf && (pOverlaySurf->Parent() == m_pOverlay1Surf->Parent()) )
    {
        //  Ensure that the VID1 pipeline is setup for TV out (repositioning not allowed to TV)
        if( !m_bTVOutEnable || m_eTVPipeline != OMAP_DSS_PIPELINE_VIDEO1 )
        {
            m_pDisplayContr->MovePipeline( OMAP_DSS_PIPELINE_VIDEO1, lXPos, lYPos );
        }
    }
    
    if( m_pOverlay2Surf && (pOverlaySurf->Parent() == m_pOverlay2Surf->Parent()) )
    {
        //  Ensure that the VID1 pipeline is setup for TV out (repositioning not allowed to TV)
        if( !m_bTVOutEnable || m_eTVPipeline != OMAP_DSS_PIPELINE_VIDEO2 )
        {
            m_pDisplayContr->MovePipeline( OMAP_DSS_PIPELINE_VIDEO2, lXPos, lYPos );
        }
    }
    
    //  Result
    return DD_OK;
}  

//------------------------------------------------------------------------------
DWORD
OMAPDDGPE::HideOverlay(
    OMAPDDGPESurface*   pOverlaySurf
    )
{
    //  Reset the clipping region for the surface
    pOverlaySurf->OmapSurface()->SetClipping( NULL );
    if (pOverlaySurf->OmapAssocSurface())
        pOverlaySurf->OmapAssocSurface()->SetClipping(NULL);
    // Close RSZHandle
    pOverlaySurf->OmapSurface()->SetRSZHandle(NULL, TRUE);
    
    //  GFX pipeline may have been disabled if we went into fullscreen mode
    m_pDisplayContr->EnablePipeline(OMAP_DSS_PIPELINE_GFX);
    
    //  Disable the overlay pipeline
    if( m_pOverlay1Surf && (pOverlaySurf->Parent() == m_pOverlay1Surf->Parent()) )
    {
        m_pDisplayContr->DisablePipeline( OMAP_DSS_PIPELINE_VIDEO1 );
        m_pOverlay1Surf = NULL;
    }
    
    if( m_pOverlay2Surf && (pOverlaySurf->Parent() == m_pOverlay2Surf->Parent()) )
    {
        m_pDisplayContr->DisablePipeline( OMAP_DSS_PIPELINE_VIDEO2 );
        m_pOverlay2Surf = NULL;
    }

    //  Disable any global alpha value and color key if both overlays are off
    if( m_pOverlay1Surf == NULL && m_pOverlay2Surf == NULL )
    {
        m_pDisplayContr->DisableColorKey( OMAP_DSS_COLORKEY_TRANS_SOURCE, OMAP_DSS_DESTINATION_LCD );
        m_pDisplayContr->DisableColorKey( OMAP_DSS_COLORKEY_GLOBAL_ALPHA_GFX, OMAP_DSS_DESTINATION_LCD );
    }

    //  Get the best surface to display on the TV
    DetermineTvOutSurface();
    
    //  Result
    return DD_OK;
}  

//------------------------------------------------------------------------------
DWORD
OMAPDDGPE::SetSrcColorKey(
    DWORD dwColorKey
    )
{
    //  Enable the transparent source color key
    m_pDisplayContr->EnableColorKey( OMAP_DSS_COLORKEY_TRANS_SOURCE, OMAP_DSS_DESTINATION_LCD, dwColorKey );
    
    //  Result
    return DD_OK;
}  

//------------------------------------------------------------------------------
DWORD
OMAPDDGPE::SetDestColorKey(
    DWORD dwColorKey
    )
{
    //  Enable the transparent destination color key
    m_pDisplayContr->EnableColorKey( OMAP_DSS_COLORKEY_TRANS_DEST, OMAP_DSS_DESTINATION_LCD, dwColorKey );
    
    //  Result
    return DD_OK;
}  

//------------------------------------------------------------------------------
DWORD
OMAPDDGPE::SetAlphaConst(
    OMAP_DSS_COLORKEY   eColorKey,
    DWORD dwColorKey
    )
{
    //  Enable the alpha const for GFX or VID2
    m_pDisplayContr->EnableColorKey( eColorKey, OMAP_DSS_DESTINATION_LCD, dwColorKey );
    
    //  Result
    return DD_OK;
}  

//------------------------------------------------------------------------------
DWORD
OMAPDDGPE::DisableColorKey()
{
    //  Disable the transparent destination color key
    m_pDisplayContr->DisableColorKey( OMAP_DSS_COLORKEY_TRANS_SOURCE, OMAP_DSS_DESTINATION_LCD );
    
    //  Result
    return DD_OK;
}  

//------------------------------------------------------------------------------
DWORD
OMAPDDGPE::DisableAlphaConst()
{
    //  Disable the alpha const value
    m_pDisplayContr->DisableColorKey( OMAP_DSS_COLORKEY_GLOBAL_ALPHA_GFX, OMAP_DSS_DESTINATION_LCD );
    m_pDisplayContr->DisableColorKey( OMAP_DSS_COLORKEY_GLOBAL_ALPHA_VIDEO2, OMAP_DSS_DESTINATION_LCD );
    
    //  Result
    return DD_OK;
}  

//------------------------------------------------------------------------------
int
OMAPDDGPE::FlipInProgress()
{
    //  If there has been a FlipSurface called, check for VSYNC status bit   
    if( m_bFlipInProgress )
    {
        //  Check for VSYNC and clear it and the flip in progress flag
        if( InVBlank() )
            m_bFlipInProgress = FALSE;
    }

    //  Return status flip
    return (m_bFlipInProgress) ? 1 : 0;
}
//------------------------------------------------------------------------------
void
OMAPDDGPE::WaitForVBlank()
{  
    m_pDisplayContr->WaitForVsync(); 
}

int
OMAPDDGPE::InVBlank()
{
    return m_pDisplayContr->InVSync(TRUE);
}

DWORD
OMAPDDGPE::GetScanLine()
{
    return m_pDisplayContr->GetScanLine();
}

BOOL
OMAPDDGPE::SurfaceFlipping(
    OMAPDDGPESurface*   pSurf,
    BOOL                matchExactSurface
    )
{
    BOOL    bResult = FALSE;        
    if(m_bFlipInProgress == FALSE)
    {        
        return bResult;
    }    
    
    if( m_pPrimarySurf && (m_pPrimarySurf->Parent() == pSurf->Parent()) )
    {
        bResult |= m_pDisplayContr->IsPipelineFlipping(
                                    OMAP_DSS_PIPELINE_GFX,
                                    pSurf->OmapSurface(),
                                    matchExactSurface );                                               
    }    
    if( m_pOverlay1Surf && (m_pOverlay1Surf->Parent() == pSurf->Parent()) )
    {
        bResult |= m_pDisplayContr->IsPipelineFlipping(
                                    OMAP_DSS_PIPELINE_VIDEO1,
                                    pSurf->OmapSurface(),
                                    matchExactSurface );                                               
    }
    if( m_pOverlay2Surf && (m_pOverlay2Surf->Parent() == pSurf->Parent()) )
    {
        bResult |= m_pDisplayContr->IsPipelineFlipping(
                                    OMAP_DSS_PIPELINE_VIDEO2,
                                    pSurf->OmapSurface(),
                                    matchExactSurface );                                               
    }    
    
    if( m_pTVSurf && (m_pTVSurf->Parent() == pSurf->Parent()) )
    {
        bResult |= m_pDisplayContr->IsPipelineFlipping(
                                    m_eTVPipeline,
                                    pSurf->OmapSurface(),
                                    matchExactSurface );                                               
    }    
    return bResult;
    
   
}

//------------------------------------------------------------------------------
DWORD
OMAPDDGPE::FlipSurface(
    OMAPDDGPESurface*   pSurf
    )
{
    BOOL    bResult;

    //
    //  Determine which pipeline this surface is associated with and flip it
    //
    if( m_pPrimarySurf && (m_pPrimarySurf->Parent() == pSurf->Parent()) )
    {
        bResult = m_pDisplayContr->FlipPipeline(
                                    OMAP_DSS_PIPELINE_GFX,
                                    pSurf->OmapSurface() );                                               
    }    
    
    if( m_pOverlay1Surf && (m_pOverlay1Surf->Parent() == pSurf->Parent()) )
    {
        CopySurfaceParams(pSurf->Parent(), pSurf);
        bResult = m_pDisplayContr->FlipPipeline(
                                    OMAP_DSS_PIPELINE_VIDEO1,
                                    pSurf->OmapSurface() );                                               
    }    

    if( m_pOverlay2Surf && (m_pOverlay2Surf->Parent() == pSurf->Parent()) )
    {
        CopySurfaceParams(pSurf->Parent(), pSurf);
        bResult = m_pDisplayContr->FlipPipeline(
                                    OMAP_DSS_PIPELINE_VIDEO2,
                                    pSurf->OmapSurface() );                                               
    }    
    
    if( m_pTVSurf && (m_pTVSurf->Parent() == pSurf->Parent()) )
    {
        bResult = m_pDisplayContr->FlipPipeline(
                                    m_eTVPipeline,
                                    pSurf->OmapSurface() );                                               
    }    
    
    //  Set flag
    m_bFlipInProgress = TRUE;
    
    //  Result
    return DD_OK;
}  

//------------------------------------------------------------------------------
VOID
OMAPDDGPE::DetermineTvOutSurface(
    BOOL bForceUpdate
    )
{
    BOOL                bResult;
    OMAPDDGPESurface*   pOldTVSurf = m_pTVSurf;    
    
    
    //  When GAPI is on, it take precedence when displaying content on the LCD and TV
    if( m_bGameEnable && m_pGameSurf )
    {
        //  Show GAPI surface on TV if TV out enabled
        if( m_bTVOutEnable )
        {
            DWORD   dwScaledWidth,
                    dwScaledHeight;
            DWORD   dwPosX,
                    dwPosY;                    
                    
            //  Set flicker filter level
            m_pDisplayContr->SetTvOutFilterLevel( m_dwTvOut_FilterLevel );
        
            //  Get the scaling for TV display of game surface
            GetTvOutScaling( m_pGameSurf, &dwScaledWidth, &dwScaledHeight, &dwPosX, &dwPosY );
                                    
            //  Configure the display controller to show the game surface on VID2
            bResult = m_pDisplayContr->SetPipelineAttribs(
                                            OMAP_DSS_PIPELINE_VIDEO2,
                                            OMAP_DSS_DESTINATION_TVOUT, 
                                            m_pGameSurf );

            //  Set the scaling info for the pipeline to NTSC
            bResult = m_pDisplayContr->SetScalingAttribs(
                                            OMAP_DSS_PIPELINE_VIDEO2,
                                            dwScaledWidth,
                                            dwScaledHeight,
                                            dwPosX,
                                            dwPosY );                                 

            //  Enable the pipeline
            bResult = m_pDisplayContr->EnablePipeline(
                                            OMAP_DSS_PIPELINE_VIDEO2 );                                               
        }
        
        //  Return; rest of this logic doesn't apply when GAPI on
        return;
    }


    //  Determine the best surface for displaying on the TV
    if( m_bTVOutEnable )
    {
        //  Need to show a surface on TV out.  Pick one with emphasis on overlay
        if( m_pOverlay1Surf && m_pOverlay2Surf )
        {
            //  All 3 layers are active (primary always is).  Display the TV out pipeline
            if( m_eTVPipeline == OMAP_DSS_PIPELINE_VIDEO1 )
            {
                //  Show VID1 overlay
                m_pTVSurf = m_pOverlay1Surf;
            }

            if( m_eTVPipeline == OMAP_DSS_PIPELINE_VIDEO2 )
            {
                //  Show VID2 overlay
                m_pTVSurf = m_pOverlay2Surf;
            }
        }
        else if( m_pOverlay1Surf )
        {
            //  Only overlay surface is active. Show it
            //  Set the TV out pipeline to be the other VID
            m_pTVSurf = m_pOverlay1Surf;
            m_eTVPipeline = OMAP_DSS_PIPELINE_VIDEO2;
        }
        else if( m_pOverlay2Surf )
        {
            //  Only overlay surface is active. Show it
            //  Set the TV out pipeline to be the other VID
            m_pTVSurf = m_pOverlay2Surf;
            m_eTVPipeline = OMAP_DSS_PIPELINE_VIDEO1;
        }
        else
        {
            //  No overlays on.  Show graphics on VID2
            m_pTVSurf = m_pPrimarySurf;
            m_eTVPipeline = OMAP_DSS_PIPELINE_VIDEO2;
        }
    }
    else
    {
        //  TV out disabled.  See if an overlay on the TV needs to migrate
        //  back onto the LCD (only happens when both overlays are active)
        if( m_pTVSurf && m_pOverlay1Surf && m_pOverlay2Surf )
        {
            //  Disable the TV pipeline
            bResult = m_pDisplayContr->DisablePipeline(
                                            m_eTVPipeline );                                               

            //  Configure the display controller to show the pipeline on the LCD
            bResult = m_pDisplayContr->SetPipelineAttribs(
                                            m_eTVPipeline,
                                            OMAP_DSS_DESTINATION_LCD, 
                                            m_pTVSurf->OmapSurface() );

            //  Set the scaling info for the pipeline back to be on the LCD
            //  using the cached source and destination rects
            if( m_eTVPipeline == OMAP_DSS_PIPELINE_VIDEO1 )
            {
                bResult = m_pDisplayContr->SetScalingAttribs(
                                                m_eTVPipeline,
                                                &m_rcOverlay1Src,
                                                &m_rcOverlay1Dest );
            }

            if( m_eTVPipeline == OMAP_DSS_PIPELINE_VIDEO2 )
            {
                bResult = m_pDisplayContr->SetScalingAttribs(
                                                m_eTVPipeline,
                                                &m_rcOverlay2Src,
                                                &m_rcOverlay2Dest );
            }

            //  Enable the pipeline back on the LCD
            bResult = m_pDisplayContr->EnablePipeline(
                                            m_eTVPipeline );                                               
        }
        else if( m_pTVSurf )
        {
            //  Disable TV out
            bResult = m_pDisplayContr->DisablePipeline(
                                            m_eTVPipeline );                                               
        }
        
        //  No TV surface
        m_pTVSurf = NULL;
    }
    

    //  Enable the TV out display    
    if( m_pTVSurf && (bForceUpdate || (m_pTVSurf != pOldTVSurf)) )
    {
        DWORD   dwScaledWidth,
                dwScaledHeight;
        DWORD   dwPosX,
                dwPosY;     

        OMAPSurface *pSurface = m_pTVSurf->OmapSurface();
                
        //  Disable the TV pipeline
        bResult = m_pDisplayContr->DisablePipeline(
                                        m_eTVPipeline );                                               

        
        if ((pSurface->Width() > 800) || (pSurface->Height() > 800))
        {
            RETAILMSG(1,(L"width(%d) aor height(%d) exceeds TVOUT capabilities\r\n",pSurface->Width(),pSurface->Height()));
            return;
        }
        

        //  Set flicker filter level
        m_pDisplayContr->SetTvOutFilterLevel( m_dwTvOut_FilterLevel );
    
        //  Get the scaling for TV display
        GetTvOutScaling( m_pTVSurf->OmapSurface(), &dwScaledWidth, &dwScaledHeight, &dwPosX, &dwPosY );
                                
        //  Configure the display controller to show selected TV surface
        bResult = m_pDisplayContr->SetPipelineAttribs(
                                        m_eTVPipeline,
                                        OMAP_DSS_DESTINATION_TVOUT, 
                                        m_pTVSurf->OmapSurface() );

        //  Set the scaling info for the pipeline 
        bResult = m_pDisplayContr->SetScalingAttribs(
                                        m_eTVPipeline,
                                        dwScaledWidth,
                                        dwScaledHeight,
                                        dwPosX,
                                        dwPosY );    
        
        //  Enable the pipeline
        bResult = m_pDisplayContr->EnablePipeline(
                                        m_eTVPipeline );                                               
    }
}  

//------------------------------------------------------------------------------
BOOL
OMAPDDGPE::GetTvOutScaling(
    OMAPSurface *pSurface,
    DWORD       *pScaledWidth,
    DWORD       *pScaledHeight,
    DWORD       *pScaledXPos,
    DWORD       *pScaledYPos
)
{
    BOOL    bResult = FALSE;
    float   fTvAspect,
            fSurfaceAspect,
            fScalingFactorW,
            fScalingFactorH;

    
    //  Check pointers
    if( pSurface == NULL ||
        pScaledWidth == NULL ||
        pScaledHeight == NULL )
        return bResult;

    //  Compute aspect ratios of TV and surface
    fTvAspect       = (float)(m_dwTvOut_AspectRatio_W) / (float)(m_dwTvOut_AspectRatio_H);
    fSurfaceAspect  = (float)(pSurface->Width()) / (float)(pSurface->Height());
    fScalingFactorW = (float)(m_dwTvOut_Resize_W / 100.0);
    fScalingFactorH = (float)(m_dwTvOut_Resize_H / 100.0);
    
    
#if 0
    //  Ignore the scaling factors if there is no flicker filter enabled
    //  And can fit the surface on the screen with just interlacing mode
    if( m_dwTvOut_FilterLevel == OMAP_TV_FILTER_LEVEL_OFF && 
        (pSurface->Width()  <= m_pDisplayContr->GetTVWidth()/2) &&
        (pSurface->Height() <= m_pDisplayContr->GetTVHeight()/2) )
    {
        //  Can display surface on TV without interlacing (2x the scaled size to indicate that)
        *pScaledWidth  = 2 * pSurface->Width();
        *pScaledHeight = 2 * pSurface->Height();
    }
    else if( m_dwTvOut_FilterLevel == OMAP_TV_FILTER_LEVEL_OFF &&
        (pSurface->Width()  <= m_pDisplayContr->GetTVWidth()) &&
        (pSurface->Height() <= m_pDisplayContr->GetTVHeight()) )
    {
        //  Can display surface on TV with interlacing
        *pScaledWidth  = pSurface->Width();
        *pScaledHeight = pSurface->Height();
    }
    else
#endif
    {
        //  Can't use or don't want to use interlace mode; use scaling instead
        //  Check aspect ratios of surface and TV out to scale the surface to maximize the TV screen
        if( fTvAspect > fSurfaceAspect )
        {
            //  Compute output width and height to maximize height on TV display
            *pScaledWidth  = (DWORD)(fScalingFactorW * fSurfaceAspect * m_pDisplayContr->GetTVWidth() / fTvAspect);
            *pScaledHeight = (DWORD)(fScalingFactorH * m_pDisplayContr->GetTVHeight());
        }
        else
        {
            //  Compute ouptut width and height to maximize width on TV display
            *pScaledWidth  = (DWORD)(fScalingFactorW * m_pDisplayContr->GetTVWidth());
            *pScaledHeight = (DWORD)(fScalingFactorH * fTvAspect * m_pDisplayContr->GetTVHeight() / fSurfaceAspect);
        }
    }

    //  Compute offsets to center TV output
    if( pScaledXPos != NULL )
    {
        //  Compute offset to center display
        *pScaledXPos = (m_pDisplayContr->GetTVWidth() - *pScaledWidth)/2;
        
        //  Adjust by manual over-ride (bound result to stay within TV display
        if( ((LONG)*pScaledXPos + m_lTvOut_Offset_W) < 0 )
        {
            *pScaledXPos = 0;
        }
        else if( (*pScaledXPos + m_lTvOut_Offset_W + *pScaledWidth) > m_pDisplayContr->GetTVWidth() )
        {
            *pScaledXPos = m_pDisplayContr->GetTVWidth() - *pScaledWidth;
        }
        else
        {
            *pScaledXPos = *pScaledXPos + m_lTvOut_Offset_W;
        }
    }                 

    if( pScaledYPos != NULL )
    {
        //  Compute offset to center display
        *pScaledYPos = (m_pDisplayContr->GetTVHeight() - *pScaledHeight)/2;
        
        //  Adjust by manual over-ride (bound result to stay within TV display
        if( ((LONG)*pScaledYPos + m_lTvOut_Offset_H) < 0 )
        {
            *pScaledYPos = 0;
        }
        else if( (*pScaledYPos + m_lTvOut_Offset_H + *pScaledHeight) > m_pDisplayContr->GetTVHeight() )
        {
            *pScaledYPos = m_pDisplayContr->GetTVHeight() - *pScaledHeight;
        }
        else
        {
            *pScaledYPos = *pScaledYPos + m_lTvOut_Offset_H;
        }
    }                 


    //  Return result
    return bResult;
}


//------------------------------------------------------------------------------
OMAPDDGPESurface::OMAPDDGPESurface()
{
    m_pParentSurface = this;
    m_pAssocSurface = NULL;
}

//------------------------------------------------------------------------------
OMAPDDGPESurface::OMAPDDGPESurface(
    OMAPSurfaceManager* pSurfaceMgr,
    OMAPSurface*        pSurface
    )
{
    //  Set the surface pointer to the allocated OMAP surface
    m_pSurface = pSurface;
    m_pParentSurface = this;
    m_pAssocSurface = NULL;
    
    //  Initialize the DDGPESurf parent class
    DDGPESurf::Init( m_pSurface->Width(), 
                     m_pSurface->Height(), 
                     m_pSurface->VirtualAddr(), 
                     m_pSurface->Stride(), 
                     OMAPDDGPE::PixelFormatToGPEFormat(m_pSurface->PixelFormat()), 
                     OMAPDDGPE::PixelFormatToDDGPEFormat(m_pSurface->PixelFormat()) );
    
    //  Set surface attributes
    m_fInVideoMemory = TRUE;
    m_fInUserMemory = FALSE;
    m_dwSurfaceSize = m_pSurface->Height() * m_pSurface->Stride();
    m_nOffsetInVideoMemory = (DWORD)m_pSurface->VirtualAddr() - (DWORD)pSurfaceMgr->VirtualBaseAddr();

    
    DEBUGMSG(GPE_ZONE_VIDEOMEMORY, (TEXT("OMAPDDGPESurface (alloc): vaddr = 0x%08x paddr = 0x%08x\r\n"), m_pSurface->VirtualAddr(), m_pSurface->PhysicalAddr()));
    
}

//------------------------------------------------------------------------------
BOOL    
OMAPDDGPESurface::SetOrientation(
        OMAP_SURF_ORIENTATION       eOrientation
        )
{
    //  Check current orientation of the OMAP surface; do nothing if the same
    if( eOrientation == m_pSurface->Orientation() )
        return TRUE;

    //  Chage OMAP surface orientation
    m_pSurface->SetOrientation( eOrientation );
            
    //  Update DDGPE width, height and stride
    m_nWidth = m_pSurface->Width();
    m_nHeight = m_pSurface->Height();
    m_nStrideBytes = m_pSurface->Stride();

    return TRUE;
}

//------------------------------------------------------------------------------
OMAPDDGPESurface::~OMAPDDGPESurface()
{
    //  Free the video memory for this surface
    if( m_pSurface )
    {
        DEBUGMSG(GPE_ZONE_VIDEOMEMORY, (TEXT("OMAPDDGPESurface (free): vaddr = 0x%08x paddr = 0x%08x\r\n"), m_pSurface->VirtualAddr(), m_pSurface->PhysicalAddr()));        
        delete m_pSurface;        
        if (m_pAssocSurface) delete m_pAssocSurface;
    }
}



