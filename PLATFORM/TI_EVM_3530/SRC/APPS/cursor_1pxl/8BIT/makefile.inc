!if 0
Copyright (c) Microsoft Corporation.  All rights reserved.
!endif
!if 0
Use of this sample source code is subject to the terms of the Microsoft
license agreement under which you licensed this sample source code. If
you did not accept the terms of the license agreement, you are not
authorized to use this sample source code. For the terms of the license,
please see the license agreement between you and Microsoft or, if applicable,
see the LICENSE.RTF on your install media or the root of your tools installation.
THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
!endif
!if 0
!endif

$(REPLACE_ROOT)\oak\lib\$(_CPUINDPATH)\$(CURRENT_LANGID)\cursor8.res:$(_OBJDIR)\cursor8.res
    copy $? $@
$(REPLACE_ROOT)\oak\lib\$(_CPUINDPATH)\$(CURRENT_LANGID)\cursor8.lib:$(_OBJDIR)\cursor8.lib
    copy $? $@

$(_COMMONOAKROOT)\lib\$(_CPUINDPATH)\cursor8.res:$(_OBJDIR)\cursor8.res
    copy $? $@
