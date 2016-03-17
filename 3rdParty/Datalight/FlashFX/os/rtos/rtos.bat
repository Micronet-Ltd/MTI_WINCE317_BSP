REM
REM This builds "driver" last, even after "samples", "tests", and "tools",
REM since unlike some other OS abstractions, there is no external interface.
REM Those directories all build libraries, which during the course of building
REM "driver" are all combined into one flashfx.lib.
REM

call %B_TRAVERSE% os\%P_OS% common services hooks examples tests tools driver

