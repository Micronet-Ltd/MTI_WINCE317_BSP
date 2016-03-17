
		FlashFX Sample Project for the Freescale MX31
                             with Samsung OneNAND
                       EBOOT Library (WINCEBL) Project

	This project is designed to be used with the Freescale MX31
	platform in combination with Microsoft Windows CE Platform Builder
	and Samsung OneNAND.  It is designed to generate a library that
	can be integrated with EBOOT to launch NK.bin saved on a Reliance
	formatted FlashFX Pro disk.

	The FIM has been defined in ffxconf.h as FFXFIM_nand, and the NTM
	list contains FFXNTM_onenand, which is correct for the Samsung
	OneNAND used in this platform.

	Generally, this project is integrated into EBOOT after 
	FlashFX Pro and Reliance are integrated into the operating system.

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
		Operating System:	EBOOT from Windows CE 5.0 Build 1400
		FlashFX Pro version: 	v3.01 Build 1307
		Flash disk size:	4MB
		VBF block size:		2048B
		Device sector size:	2048B
        ECC Computations: Hardware
		Test Date:		04AUG2006

	FlashFX Unit Test Results:
	---------------------------------------------------------------------				
	FMSLTEST   Write:       4005 KB per second
                   Read:        5204 KB per second
                   Erase:      58589 KB per second

	VBFTEST	   Write:             3111 KB per second
                   Read:              5253 KB per second
                   Discard:           2921 KB per second
                   Partition mount:    578 ms per mount
                   Region mount:        42 ms per mount


	Results are in KB/second.  

 
