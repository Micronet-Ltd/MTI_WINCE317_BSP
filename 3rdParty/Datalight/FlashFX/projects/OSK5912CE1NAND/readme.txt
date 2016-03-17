
		FlashFX Sample Project for the OMAP5912 OSK
                             with Samsung OneNAND

	This project is designed to be used with the TI OMAP5912 OSK
	platform in combination with Microsoft Windows CE Platform Builder
        and Samsung OneNAND on CS2.

	The FIM has been defined in ffxconf.h as FFXFIM_nand, and the NTM
	list contains FFXNTM_onenand, which is correct for the Samsung
        OneNAND used in this platform.

	Additionally you will likely want to change the "Format" value to 
	cause FlashFX to format the flash.

	In Platform Builder, simply add FlashFX to your CE project,
	then access the FlashFX Settings window and set the FX_PROJNAME
	value to "OSK5912CE1NAND".  

	---------[ Performance Information ]---------

	Hardware Configuration:
		Platform:		OMAP5912 OSK
		Processor:		TI OMAP5912
		Processor speed:	?
		Bus speed:		?
		Flash type:		Samsung OneNAND KFG1G16U2M 1Gb

	Software Configuration:
		Operating System:	Windows CE 4.2
		FlashFX Pro version: 3.0 Build 1218
		Flash disk size:	4MB
		VBF block size:		Unknown
		Device sector size:	2048B
        OMAP 5912 CS2 EMIFS Settings:
            Asynchronous Mode
            ulEMIF_SLOW_CS2_CONFIG = 0x1131
            ulEMIF_SLOW_CS2_CONFIG_ADVANCED = 0x2
        ECC Computations: Hardware
		Test Date:		09MAR2006 1610

	FlashFX Unit Test Results:
	---------------------------------------------------------------------				
	FMSLTEST   Write:       4539 KB per second
               Read:        6191 KB per second
               Erase:      63584 KB per second
	VBFTEST	   Write:             2849 KB per second
               Read:              6328 KB per second
               Discard:           2738 KB per second
               Partition mount:     46 ms per mount
               Region mount:         0 ms per mount


	File System Test         Microsoft     Microsoft     Datalight
	Results:		    FAT	          TFAT	   Reliance v1.2
	---------------------------------------------------------------------
	FSIOTEST /SEQ  Read  	       ? 	     ?		   ?
		       Write 	       ? 	     ?		   ?
		 /RAND Read  	       ? 	     ?		   ?
		       Write 	       ? 	     ?		   ?

	Results are in KB/second.  

 
