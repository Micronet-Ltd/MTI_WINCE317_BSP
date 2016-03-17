//  File:  bl.h
//
#ifndef __BL_H
#define __BL_H

//for AdvBacklight
#define BL_REG_KEY                  TEXT("Drivers\\BuiltIn\\Backlight")
#define BACKLIGHT_REGKEY			TEXT("ControlPanel\\Backlight")

#define BL_REG_BATT_LEVEL       TEXT("DutyCyclesBatt")
#define BL_REG_AC_LEVEL			TEXT("DutyCyclesAC")

#define BL_REG_MIN_LEVEL_DEF	TEXT("MaxDutyCyclesDef")
#define BL_REG_MIN_LEVEL_7I		TEXT("MaxDutyCycles7i")

#define BACKLIGHT_REG_LS			TEXT("DependOnLight")

#define BACKLIGHT_LOW		100
#define BACKLIGHT_HIGH		0
#define BACKLIGHT_LEVELS	100

#define		MAX_REGSTR		64
#define		STATES_QTY		7

#define BKL_DEFAULT_DUTY_CYCLE	0x60
#define BKL_7INCH_DUTY_CYCLE	0x2F

#endif //__BL_H