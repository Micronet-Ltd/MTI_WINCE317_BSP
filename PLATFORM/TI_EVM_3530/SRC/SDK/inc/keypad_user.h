#ifndef __KEYPAD_USER_
	#define __KEYPAD_USER_

	typedef enum
	{
		KEYPAD_KEYS_MAP_DEFAULT = 0,
		KEYPAD_KEYS_MAP_EXT,
		KEYPAD_KEYS_MAP_EXT2,	// for future use
		KEYPAD_KEYS_MAP_EXT3,	// for future use
		KEYPAD_KEYS_MAP_EXT4,	// for future use
		KEYPAD_KEYS_MAP_EXT5,	// for future use
		KEYPAD_KEYS_MAP_EXT6,	// for future use
		KEYPAD_KEYS_MAP_EXT7,	// for future use
		KEYPAD_KEYS_MAP_LAST
	}KEYPAD_KEYS_MAP;

	typedef BOOL (* pfnSwitchKeysMap)(KEYPAD_KEYS_MAP);
	#ifdef __cplusplus
	extern "C" {
	#endif
		BOOL MIC_SwitchKeysMap(KEYPAD_KEYS_MAP table);
	#ifdef __cplusplus
	}
	#endif
#endif