
		FlashFX Sample Project for the PXA320 processor on 
		         the Sophia Systems Sandgate 3

	This project is designed to be used with the Sophia Systems Sandgate 3
	platform in combination with Microsoft Windows CE Platform Builder
        and Samsung NAND.

	The FIM has been defined in ffxconf.h as FFXFIM_nand, and the NTM
	list contains FFXNTM_pxa320, which is correct for the Samsung
        NAND used in this platform.

	Additionally you will likely want to change the "Format" value to 
	cause FlashFX to format the flash.

	In Platform Builder, simply add FlashFX to your CE project,
	then access the FlashFX Settings window and set the FX_PROJNAME
	value to "sandgate3_ce".  

	---------[ Performance Information ]---------

	Hardware Configuration:
		Platform:		Sophia Systems Sandgate 3
		Processor:		PXA320
		Processor speed:	?
		Bus speed:		?
		Flash type:		Samsung NAND K9K1208R0B 512Mb

	Software Configuration:
		Operating System:	Windows CE 5.0 Build 1400
		FlashFX Pro version:	v3.10 Build 1444
		Flash disk size:	60MB
		VBF block size:		512B
		Device sector size:	512B
        ECC Computations: Hardware
		Test Date:		

	FlashFX Unit Test Results:
	---------------------------------------------------------------------				
	FMSLTEST   Write w/o ECC:	1953 KB per second
                   Read w/o ECC:	6480 KB per second
	           Write w/ ECC:	1941 KB per second
                   Read w/ ECC:		6362 KB per second
                   Erase:		9196 KB per second

	VBFTEST	   Write:		1465 KB per second
                   Read:		6195 KB per second
                   Discard:		1116 KB per second
                   Disk mount:		 469 ms per mount
                   Region mount:	 144 ms per mount


	File System Test         Microsoft     Microsoft     Datalight
	Results:		    FAT	          TFAT	   Reliance v1.2
	---------------------------------------------------------------------
	FSIOTEST /SEQ  Read  	       ? 	     ?		   ?
		       Write 	       ? 	     ?		   ?
		 /RAND Read  	       ? 	     ?		   ?
		       Write 	       ? 	     ?		   ?

	Results are in KB/second.  

 
