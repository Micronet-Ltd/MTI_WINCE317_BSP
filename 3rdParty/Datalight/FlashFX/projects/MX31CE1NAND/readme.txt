
		FlashFX Sample Project for the Freescale MX31
                             with Samsung OneNAND

	This project is designed to be used with the Freescale MX31
	platform in combination with Microsoft Windows CE Platform Builder
        and Samsung OneNAND.

	The FIM has been defined in ffxconf.h as FFXFIM_nand, and the NTM
	list contains FFXNTM_onenand, which is correct for the Samsung
        OneNAND used in this platform.

	Additionally you will likely want to change the "Format" value to 
	cause FlashFX to format the flash.

	In Platform Builder, simply add FlashFX to your CE project,
	then access the FlashFX Settings window and set the FX_PROJNAME
	value to "MX31CE1NAND".  

	---------[ Performance Information ]---------

	Hardware Configuration:
		Platform:		Freescale i.MX31 ADS
		Processor:		i.MX31
		Processor speed:	?
		Bus speed:		?
		Flash type:		Samsung OneNAND KFG1G16U2M 1Gb

	Software Configuration:
		Operating System:	Windows CE 5.0 Build 1400
		FlashFX Pro version: 	v3.01 Build 1273
		Flash disk size:	4MB
		VBF block size:		2048B
		Device sector size:	2048B
        ECC Computations: Hardware
		Test Date:		26MAY2006

	FlashFX Unit Test Results:
	---------------------------------------------------------------------				
	FMSLTEST   Write:       5598 KB per second
                   Read:        7062 KB per second
                   Erase:      60909 KB per second

	VBFTEST	   Write:             3269 KB per second
                   Read:              7093 KB per second
                   Discard:           2951 KB per second
                   Partition mount:     63 ms per mount
                   Region mount:         0 ms per mount


	File System Test         Microsoft     Microsoft     Datalight
	Results:		    FAT	          TFAT	   Reliance v1.2
	---------------------------------------------------------------------
	FSIOTEST /SEQ  Read  	       ? 	     ?		   ?
		       Write 	       ? 	     ?		   ?
		 /RAND Read  	       ? 	     ?		   ?
		       Write 	       ? 	     ?		   ?

	Results are in KB/second.  

 
