
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>

#include <oal.h>
#include <oalex.h>
#include <args.h>

#include <i2c.h>
#include <omap35xx_clocks.h>

#include <gpio.h>
#include <audioControl.h>

void *open_amplifiers_control(void)
{
	void *gpio;
 
	gpio = GPIOOpen();
	if(!gpio)
	{
		RETAILMSG(1, (L"%S: failure to open amplifiers control\r\n",__FUNCTION__));
		return (void *)-1;
	}

	// Configure all pins: select an internal microphone and speaker, shutdown external speaker
	//
	GPIOClrBit(gpio, MIC_INT_EXT_SW_GPIO);
	GPIOSetMode(gpio, MIC_INT_EXT_SW_GPIO, GPIO_DIR_OUTPUT);

	GPIOClrBit(gpio, SPKR_EXT_SD_GPIO);
	GPIOSetMode(gpio, SPKR_EXT_SD_GPIO, GPIO_DIR_OUTPUT);

	GPIOSetBit(gpio, SPKR_INT_SD_GPIO);
	GPIOSetMode(gpio, SPKR_INT_SD_GPIO, GPIO_DIR_OUTPUT);

	// START-UP time 3.2 ms
	Sleep(5);

	// TODO: Configure BT/Modem PCM
	//

	return gpio;
}

void close_amplifiers_control(void *gpio)
{
	speak_mut_all(gpio);
	GPIOClose(gpio);
}

void speak_mut_all(void *gpio)
{
	if(!gpio || (gpio == (void *)-1))
		return;

	GPIOClrBit(gpio, MIC_INT_EXT_SW_GPIO);
	GPIOClrBit(gpio, SPKR_EXT_SD_GPIO);

	return;
}

void spkr_route(void *gpio, SPKR_AMPLIFIERS_e r)
{
	if(!gpio || (gpio == (void *)-1))
		return;
	switch(r)
	{
		case SPKR_PREDRIVE_CLASSD_2_INT_SPKER:
		{
			GPIOClrBit(gpio, SPKR_EXT_SD_GPIO);
			GPIOSetBit(gpio, SPKR_INT_SD_GPIO);
			break;
		}
		case SPKR_PREDRIVE_CLASSD_2_EXT_SPKER:
		{
			GPIOClrBit(gpio, SPKR_INT_SD_GPIO);
			GPIOSetBit(gpio, SPKR_EXT_SD_GPIO);
			break;
		}
		default:
		{
			speak_mut_all(gpio);
			break;
		}
	}

	return;
}

SPKR_AMPLIFIERS_e spkr_current_route(void *gpio)
{
	if(GPIOGetBit(gpio, SPKR_EXT_SD_GPIO))
		return SPKR_PREDRIVE_CLASSD_2_EXT_SPKER;
	else if(GPIOGetBit(gpio, SPKR_INT_SD_GPIO))
		return SPKR_PREDRIVE_CLASSD_2_INT_SPKER;
	else
		return SPKR_NUM_OF_AMPLIFIERS;
}

void mic_route(void *gpio, MIC_SWITCH_e r)
{
	if(!gpio || (gpio == (void *)-1))
		return;
	switch(r)
	{
		case MIC_MAIN_2_EXT_MIC:
		{
			GPIOSetBit(gpio, MIC_INT_EXT_SW_GPIO);
			break;
		}
		case MIC_MAIN_2_INT_MIC:
		default:
		{
			GPIOClrBit(gpio, MIC_INT_EXT_SW_GPIO);
			break;
		}
	}

	return;
}

MIC_SWITCH_e mic_current_route(void *gpio)
{
	if(GPIOGetBit(gpio, MIC_INT_EXT_SW_GPIO))
		return MIC_MAIN_2_EXT_MIC;
	else
		return MIC_MAIN_2_INT_MIC;
}