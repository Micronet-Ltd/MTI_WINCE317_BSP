#include "ftdi_ioctl.h"

VOID
FT_CompleteReadIrp(
	PFTDI_DEVICE pDev
    )
{

#ifdef WINCE
/* +++ Call the Completion routine here */

	// For the moment just call SetEvent
	SetEvent(pDev->ReadRequest.hReadBytesSatisfied);

#else
    PIO_STACK_LOCATION irpStack;
    PIO_STACK_LOCATION nextStack;

    //
    // We are going to call the IRP's completion routine, so we have
    // to mimic the I/O Manager.
    // Get DeviceObject from the current stack location.
    // Get Context and CompletionRoutine from the next stack location.
    //

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    nextStack = IoGetNextIrpStackLocation(Irp);

    (nextStack->CompletionRoutine)(
        NULL,   //irpStack->DeviceObject,
        Irp,
        nextStack->Context
        );
#endif
}