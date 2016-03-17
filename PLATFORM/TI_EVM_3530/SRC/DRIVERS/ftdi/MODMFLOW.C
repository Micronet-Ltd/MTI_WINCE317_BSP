#include "ftdi_ioctl.h"


ULONG
SerialHandleModemUpdate(
    PSERIAL_DEVICE_EXTENSION Extension,
    BOOLEAN DoingTX
    )
{
	//
	// Alot simpler than the Windows version. Just signal the event.
	//
	return 0;
}

UCHAR
SerialProcessLSR(
    PSERIAL_DEVICE_EXTENSION Extension
    )
{
	return 0;
}

VOID
SerialPutChar(
    PSERIAL_DEVICE_EXTENSION Extension,
    UCHAR CharToPut
    )
{

	Extension->ReadBuffer.Buffer[Extension->ReadBuffer.rbPut] = CharToPut;

	Extension->ReadBuffer.rbPut++;

	if(Extension->ReadBuffer.rbPut >= Extension->ReadBuffer.rbSize) {
		Extension->ReadBuffer.rbPut = 0;
	}

    Extension->ReadBuffer.rbBytesFree --;

	return;

}