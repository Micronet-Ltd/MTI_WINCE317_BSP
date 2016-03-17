
#include <windows.h>
#include <bsp.h>

// translate registry-like formatted AT command (such as "AT<cr>") to char array ("AT\r") ready for sending to modem
void ExpandATMacro(TCHAR *source, char *dest)
{
	int i;

	#define LMSCH   '<'
	#define RMSCH   '>'

	#define CR_MACRO            "<cr>"
	#define CR_MACRO_LENGTH     4
	#define LF_MACRO            "<lf>"
	#define LF_MACRO_LENGTH     4

	#define CR                  '\r'        // 0x0D
	#define LF                  '\n'        // 0x0A

	int len = wcslen(source);
	char *tmpbuf = new char[len + 1];
	if (!tmpbuf)
	{
		dest[0] = 0;
		return;
	}

	wcstombs(tmpbuf, source, len);
	char *s = dest;

	for (i = 0; i < len; )
	{
		if (tmpbuf[i] == LMSCH)
		{
			// <cr>
            if (!strncmp(&tmpbuf[i], CR_MACRO, CR_MACRO_LENGTH)) 
            {
                *s++ = CR;
                i += CR_MACRO_LENGTH;
                continue;
            }
            // <lf>
            if (!strncmp(&tmpbuf[i], LF_MACRO, LF_MACRO_LENGTH)) 
            {
                *s++ = LF;
                i += LF_MACRO_LENGTH;
                continue;
            }
		}

		*s++ = tmpbuf[i++];
	}
	*s = 0;
	delete [] tmpbuf;
}
