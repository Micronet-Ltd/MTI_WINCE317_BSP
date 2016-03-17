
		FlashFX Sample Project for the Intel PXA250

	This project is designed to be used with the Intel PXA250/PXA255
	platform in combination with Microsoft Windows CE Platform Builder.

	The FIM has been defined in ffxconf.h as FFXFIM_isf2x16 which is
	correct for the Intel 2x16 StrataFlash used in this platform.

	Before building this project you must edit the FLASHFX.REG file
	and specify the correct "Start" and "Length" values to reference
	the range of flash memory to use.

	The following settings are typical for development on the PXA250:

		Start  = 00100000
		Length = 01F00000 (maximum)

	Additionally you will likely want to change the "Format" value to 
	cause FlashFX to format the flash.

	In Platform Builder, simply add FlashFX to your CE project,
	then access the FlashFX Settings window and set the FX_PROJNAME
	value to "pxa250".  

	---------[ Performance Information ]---------

	Hardware Configuration:
		Platform:		Intel Lubbock DBPXA255
		Processor:		PXA255A0C400
		Processor speed:	398 MHz
		Bus speed:		199 MHz (PXBus)
		Flash type:		Intel StrataFlash 28F128K3C (2x16)

	Software Configuration:
		Operating System:	Windows CE 4.2
		FlashFX Pro version:	1.2
		Flash disk size:	4MB
		VBF block size:		512B
		Device sector size:	512B
		Test Date:		05/20/2005

	FlashFX Unit Test Results:
	---------------------------------------------------------------------				
	FMSLTEST       Read         5916 KB/s
		       Write  	     331 
		       Erase   	     737 
	VBFTEST	       Read    	    6002 
 		       Write   	     176 
		       Discard      5700 

	File System Test         Microsoft     Microsoft     Datalight
	Results:		    FAT	          TFAT	   Reliance v1.2
	---------------------------------------------------------------------
	FSIOTEST /SEQ  Read  	    5215 	  5233		5233
		       Write 	     179 	    75		 470
		 /RAND Read  	    1815   	  1673		1935
		       Write 	      12 	    11		 107

	Results are in KB/second.  In all cases FSIOTEST was run with the
	/Max:3072 parameter to use a maximum of 3MB of the disk.

	Reliance was configured to transact on close, flush, disk-full,
	dismount, and power-down conditions.
 
