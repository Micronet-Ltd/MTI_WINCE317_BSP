#ifndef _SWUPDATE_H
#define _SWUPDATE_H


/////////// SwUpdateFlags //////////////////////////////
#define	SW_START_UPDATE		0x00000001	// to start sw update
#define	SW_END_UPDATE		0x00000002	// update ended: check errors
#define	SW_USER_CANCEL		0x00000004	// update canceled
#define	SW_CANNOT_OPEN		0x00010000	// boot cannot open file
#define	SW_CANNOT_READ		0x00020000	// boot cannot read file
#define	SW_DATA_ERROR		0x00040000	// file data error
#define	SW_FLASH_ERROR		0x00080000

typedef enum
{
	sw_type_xldr	= 0x01,
	sw_type_eboot	= 0x02,
	sw_type_mnfr	= 0x04,
	sw_type_logo	= 0x08,
	sw_type_nk		= 0x10
} sw_type_data;

typedef union
{
	struct
	{
		unsigned long conversation	: 4;
		unsigned long data_type		: 6;
		unsigned long params		: 6;
		unsigned long err_type		: 4;
		unsigned long err_data_type : 6;
		unsigned long err_params	: 6;
	};
	UINT32 bits;
} sw_update_state;

#define		SECUR_PASS			8491
#define		MIN_SECUR_SIZE		sizeof(UINT32) * 2
#endif
