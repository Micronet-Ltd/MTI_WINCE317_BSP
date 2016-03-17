/* This file contains code that is designed to be cut-and-pasted into the
   BSP module which contains the OEMInit() function.
*/


/* Place the following extern definitions at the top of the module.
*/
extern DWORD FXReadRegistryFromOEM(DWORD dwFlags, LPBYTE pbuf, DWORD len);
extern DWORD FXWriteRegistryToOEM( DWORD dwFlags, LPBYTE pBuf, DWORD len);

extern DWORD (*pReadRegistryFromOEM) (DWORD dwFlags, LPBYTE lpData, DWORD cbData);
extern DWORD (*pWriteRegistryToOEM)  (DWORD dwFlags, LPBYTE lpData, DWORD cbData);


/* Place the following code inside the OEMInit() function.
*/
pReadRegistryFromOEM = FXReadRegistryFromOEM;
pWriteRegistryToOEM = FXWriteRegistryToOEM;
