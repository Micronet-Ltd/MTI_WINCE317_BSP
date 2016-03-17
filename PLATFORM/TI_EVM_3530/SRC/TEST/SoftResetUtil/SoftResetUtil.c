#include <windows.h>
#include <pkfuncs.h>

void main()
{
    KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL);
}