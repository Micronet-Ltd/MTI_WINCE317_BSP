// warm_boot.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "warm_boot.h"


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{

	return (int) SetSystemPowerState(0, POWER_STATE_RESET, POWER_FORCE);
}
