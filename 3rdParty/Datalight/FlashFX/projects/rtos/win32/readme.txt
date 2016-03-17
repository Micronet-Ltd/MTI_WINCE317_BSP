
        FlashFX Pro Sample Project for Win32

    This sample project demonstrates how the OS Services can be
    customized for the target OS environment.  The resulting FlashFX Pro
    library must be linked with the application that will utilize the
    FlashFX Pro software.  It is designed to be used with the RTOS
    porting kit and the MSVS toolset (Microsoft Visual Studio) to work
    in a Win32 environment.

    FlashFX Pro is configured to create simulated flash devices using
    the NOR file FIM.  Additionally, support for both Reliance and FAT
    file systems are disabled.  The FlashFX Pro driver interface, defined
    in [ffxp]/os/rtos/driver/fxdrv.c, can be used to read and write
    sectors on the flash.  For more details on the project configuration,
    or to change the configuration, see ffxconf.h.

    The FlashFX Pro RTOS porting kit driver interface is compatible
    with the Reliance RTOS porting kit driver.  To use FlashFX Pro with
    Reliance, change the following configuration in ffxconf.h to TRUE.

    #define FFXCONF_RELIANCESUPPORT  TRUE

    To use FlashFX Pro with a FAT file system, change the following
    configuration in ffxconf.h to TRUE.

    #define FFXCONF_RELIANCESUPPORT  TRUE
