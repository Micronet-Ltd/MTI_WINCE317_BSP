
		FlashFX Sample Project for Intel PXA27x

	This project is designed to be used with the Intel PXA27x
	platform in combination with Microsoft Windows CE Platform
 	Builder.

	The FIM has been defined in ffxconf.h as FFXFIM_isf2x16 which is
	correct for the Intel 2x16 StrataFlash used on the board.

	Before building this project you must edit the FLASHFX.REG
	file and specify the correct "Start" and "Length" values to
	reference the range of flash memory to use.

	The following settings are typical for development on the
	DPBXA270: (refer to pxa27x developer's guide)

		Start  = 00100000
		Length = 01F00000 (maximum)

	Additionally you will likely want to change the "Format" value to
	cause FlashFX to format the flash.

	In Platform Builder, simply add FlashFX to your CE project,
	then access the FlashFX Settings window and set the FX_PROJNAME
	value to "pxa270".

	---------[ Performance Information ]---------

	Hardware Configuration:
		Platform:		Intel Mainstone II (PXA27x)
		Processor:		PXA270C0C520
		Processor speed:	312 MHz
		Bus speed:		104 MHz
		Flash type:		Intel 128L18B (2x16)

	Software Configuration:
		Operating System:	CE 5.0
		FlashFX Pro version:	1.11
		Flash disk size:	8MB
		VBF block size:		512B
		Device sector size:	512B
		Test Date:		12/16/2004

	Test Results:         Microsoft	     Microsoft
				FAT		TFAT
	---------------------------------------------------------------------
	FMSLTEST       Read    12136 		Same
		       Write  	 125 	 	 "
		       Erase   	  31 	         "
	VBFTEST	       Read    	9891 		 "
 		       Write   	  48 		 "
		       Discard 40027 	         "
	FSIOTEST /SEQ  Read  	7245 		6474
		       Write 	  30  		  42
		 /RAND Read  	1877 		1595
		       Write 	   3 		   2

	Results are in KB/second.  In all cases FSIOTEST was run with the
	/Max:3072 parameter to use a maximum of 3MB of the disk.
