@rem ###############################################################################
@rem Name         : cebasecesysgen.bat
@rem Title        : Setup Environment variables
@rem Author       : Imagination Technologies
@rem Created      : 11th May 2004
@rem
@rem  Copyright   : 2003-2004 by Imagination Technologies. All rights reserved.
@rem              : No part of this software, either material or conceptual 
@rem              : may be copied or distributed, transmitted, transcribed,
@rem              : stored in a retrieval system or translated into any 
@rem              : human or computer language in any form by any means,
@rem              : electronic, mechanical, manual or other-wise, or 
@rem              : disclosed to third parties without the express written
@rem              : permission of Imagination Technologies Limited, Unit 8,
@rem              : HomePark Industrial Estate, King's Langley, Hertfordshire,
@rem              : WD4 8LZ, U.K.
@rem
@rem Description  : batch file
@rem
@rem Platform     : Windows XP
@rem
@rem Modifications:-
@rem $Log: cebasecesysgen-bin.bat $

@rem echo Setting PVR variables %1

if /i not "%1"=="preproc" goto :Not_Preproc
    set POWERVR_MODULES=
    goto :EOF
:Not_Preproc
if /i not "%1"=="pass1" goto :Not_Pass1

    if not "%SYSGEN_POWERVR%"=="1" goto :no_powervr

	:wince6_dependancies
    	set SYSGEN_CORESTRA=1
	set SYSGEN_STDIOA=1
	set SYSGEN_PM=1
	
        set POWERVR_MODULES=%POWERVR_MODULES% services 3dapi
    :no_powervr

    goto :EOF
:Not_Pass1
if /i not "%1"=="pass2" goto :Not_Pass2
    goto :EOF
:Not_Pass2
if /i not "%1"=="report" goto :Not_Report
    if not "%POWERVR_MODULES%"==""     echo POWERVR_MODULES=%POWERVR_MODULES%
    goto :EOF
:Not_Report
echo %0 Invalid parameter %1
