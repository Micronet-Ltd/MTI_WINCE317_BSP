
        FlashFX Sample Project for the Freescale MX31
                with 8-bit wide 512B page NAND

    This project is designed to be used with the Freescale MX31
    platform in combination with Microsoft Windows CE Platform Builder
    and 8 bit wide, 512B NAND.  It was tested on the LogicPD
	MCIMX31LITEKIT.

    The FIM has been defined in ffxconf.h as FFXFIM_nand, and the NTM
    list contains FFXNTM_mx31.

    Additionally you will likely want to change the "Format" value to 
    cause FlashFX to format the flash.

    ---------[ Performance Information ]---------

    Hardware Configuration:
        Platform:       Freescale i.MX31 ADS
        Processor:      i.MX31
        Processor and bus speeds:
            Computed Fvco = 453142857 (assumes 26.0000000MHz Fref)
            Computed hclk = 151047619 
            Computed nfcclk = 11619047 (T=86ns)
        Flash type:     ST Micro NAND512W3A2BN6E

    Software Configuration:
        Operating System:   Windows CE 5.0 Build 1400
        FlashFX Pro version:    v3.10 Build 1450
        Flash disk size:    4MB
        VBF block size:     512B
        Device sector size: 512B
        ECC Computations: Hardware
        Test Date:      23MAR2007

    FlashFX Unit Test Results:
    ---------------------------------------------------------------------               
    FMSLTEST      Without ECC
                        Write:       2044 KB per second
                        Read:        5201 KB per second
                        Erase:       9009 KB per second
                      With ECC
                       Write:       2041 KB per second
                       Read:        5201 KB per second

    VBFTEST
                       Write:             1589 KB per second
                       Read:              5031 KB per second
                       Discard:           1144 KB per second
                       Compaction:          15 ms per compaction
                       Disk Mount:         115 ms per mount
                       Region Mount:        15 ms per mount



 
