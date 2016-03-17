The code for the NEON BLTs is contained in this directory, but the code
is not assembled because the NEON instructions require an ARM assembler
with arm arch7 support. Binary libraries containing this code are located in
the src\drivers\display\ddgpe_neon debug and retail directories.

If work is done on these files (using an ARM assembler with ARCH7
support) then when you have finished be sure to update the binary
libraries by copying them from the ..\..\..\..\lib directory to the
retail/debug subdirectories below this directory.

To assemble this code, use the following environment variable:

set ASSEMBLER_ARM_ARCH7_SUPPORT=1
