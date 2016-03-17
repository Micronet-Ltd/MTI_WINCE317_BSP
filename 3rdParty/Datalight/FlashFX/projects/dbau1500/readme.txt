
                FlashFX Sample Project for the AMD Alchemy DBAu1500

        This project is designed to be used with the AMD Alchemy DBAu1500
        platform in combination with Microsoft Windows CE Platform Builder.

        The FIM has been defined in ffxconf.h as FFXFIM_amb2x16 which is
        correct for the flash used in this platform.

        Before building this project you must edit the FLASHFX.REG file
        and specify the correct "Start" and "Length" values to reference
        the range of flash memory to use.

        The following settings are typical for development on the DPAu1500:

                Start  = 1E000000
                Length = 01000000 (maximum)

        Additionally you will likely want to change the "Format" value to
        cause FlashFX to format the flash.

        In Platform Builder, simply add FlashFX to your CE project, then
        access the FlashFX Settings window and set the FX_PROJNAME value
        to "dbau1500".

        ---------[ Test Configuration ]---------

        Hardware Configuration:
                Platform:               DBAu1500
                Processor:              MIPS
                Processor speed:        ?
                Bus speed:              ?
                Flash type:             AMD 2x16 MirrorBit

        Software Configuration:
                Operating System:       Windows CE 4.2
                FlashFX Pro version:    3.1
                Flash disk size:        16MB
                VBF block size:         512B
                Device sector size:     512B
                Test Date:              12/13/2006

        FlashFX Performance Results:
        ---------------------------------------------------------------------
        FMSLTEST  Read  3808 KB/second    VBFTEST     Read 3822 KB/second
                 Write   331 KB/second               Write  277 KB/second
                 Erase    97 KB/second             Discard 7004 KB/second
                                                Compaction 1300 ms
                                           Partition Mount   12 ms
                                              Region Mount    2 ms

        File System                Microsoft  Microsoft    Datalight
        Performance Test Results:     FAT       TFAT     Reliance v3.0 (beta)
        ---------------------------------------------------------------------
        FSIOTEST     Seq Read        3531          ?           ?
                    Seq Write         149          ?           ?
                  Random Read         732          ?           ?
                 Random Write          55          ?           ?

        Results are in KB/second.  In all cases FSIOTEST was run using the
        standard /DLPERF option which dynamically configures and runs the
        sequential and random I/O tests.

        Reliance was configured to transact on close, flush, disk-full,
        dismount, and power-down conditions.  TFAT was configured to use
        its default options (not transacting on every write).

