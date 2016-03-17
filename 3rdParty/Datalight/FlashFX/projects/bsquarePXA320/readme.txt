
			FlashFX Sample Project for the
			    BSQUARE PXA3xx DevKit

	This project is designed to be used with the BSQUARE PXA3xx DevKit
	with Microsoft Windows CE 6.0 and the onboard NAND flash.

	The FIM has been defined in ffxconf.h as FFXFIM_nand, and the NTM
	list contains FFXNTM_pxa320, which is correct for the NAND flash
        used in this platform.

	Additionally you will likely want to change the "Format" value to
	cause FlashFX to format the flash.

	---------[ Performance Information ]---------

	Hardware Configuration:
		Platform:		BSQUARE PXA3xx DevKit
		Processor:		PXA320
		Processor speed:	806MHz
		Bus speed:		?
		Flash type:		Samsung K9F1G08Q0M (128MB)

	Software Configuration:
		Operating System:	Windows CE 6.0
		FlashFX Pro version:	v3.16
		Flash disk size:	80MB
		VBF block size:		2KB
		Device sector size:	2KB
        ECC Computations: Hardware
		Test Date:

	FlashFX Unit Test Results:
	---------------------------------------------------------------------
	FMSLTEST   Write w/o ECC:	  n/a
                   Read w/o ECC:	  n/a
	           Write w/ ECC:	 3886 KB per second
                   Read w/ ECC:		 3074 KB per second
                   Erase:		 69164KB per second

	VBFTEST	   Write:		 1348 KB per second
                   Read:		 3017 KB per second
                   Discard:		 310  KB per second
                   Disk mount:		 2653 ms per mount
                   Region mount:	 631  ms per mount


	Notes:

	1. There is a known issue with the PXA320 controller that may cause
	   The FlashFX FMSL ECC Test to fail.  This issue does not affect
	   normal FlashFX operation.
