// Copyright (c) 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky

#define BSP_LCD_POWER_GPIO	GPIO_164
//#define BSP_LCD_V3P3V_GPIO		GPIO_29
#define BSP_LCD_BUFF_EN		GPIO_104

// Triton GPIO controlling DVI enable: 0 = disable, 1 = enable
//#define BSP_LCD_DVIENABLE_GPIO  (192 /* T2 GPIO base */ + 7)
// DVI bit location in T2 regs
//#define BSP_LCD_DVIENABLE_T2READ    TWL_GPIODATAIN1
//#define BSP_LCD_DVIENABLE_T2WRITE   TWL_GPIODATAOUT1
//#define BSP_LCD_DVIENABLE_T2DIR     TWL_GPIODATADIR1
//#define BSP_LCD_DVIENABLE_T2MASK    0x80

int  lcd_gpio_en(int en);
void lcd_power_en(int en);
void lcd_buff_en(int en);
//void lcd_bkl_en(int en);
//void LcdDviEnableControl(BOOL bEnable);
void LcdStall(DWORD dwMicroseconds);
void LcdSleep(DWORD dwMilliseconds);






