#ifndef __MCP2515_H
#define __MCP2515_H

#include "CANB.h"


// Defines ========================================================================================

// Purpose		: Print array of data to debug output
// Limitation	: All elements of array should be of the same type
// _debug_zone	- appropriate debug zone
// _msg			- message that will be printed before data
// _arr			- array of data elements
// _data_type	- type of data element in array

#ifdef DEBUG
#define DEBUGMSG_ARRAY( _debug_zone, _msg, _arr, _elements_count ) \
	{ \
		DEBUGMSG ( _debug_zone, ( _T( "\r\n" ) ) ); \
		DEBUGMSG ( _debug_zone, _msg ); \
		for ( int iIndex = 0; iIndex < _elements_count; iIndex++ ) \
		{ \
			DEBUGMSG ( _debug_zone, ( _T( "0x%04X   " ), _arr[ iIndex ] ) ); \
		} \
		DEBUGMSG ( _debug_zone, ( _T( "\r\n" ) ) ); \
	}
#else
#define DEBUGMSG_ARRAY( _debug_zone, _msg, _arr, _elements_count ) {}
#endif // _DEBUG


#define DEFAULT_INTR_ENABLE	((unsigned __int8)0xFF)	// Default Interrupts Enable

// Maximal time MCP2515 changes its operation mode, ie the time to complete all pending message transmissions
#define MCP2515_MODE_CHANGE_MSEC_MAX	16  // msec
// MCP2515 mode-change-completion polling-time
#define MCP2515_MODE_CHANGE_MSEC_POLL	 4	// msec

#define MCP2515_MAX_CLOCK_FREQ		1500000			// Allowed speed in Hz
#define SPI_BIT_COUNT				32 /*8 */				// Length of the transfer

//#define CAN_MCP2515_RESET_OFFSET	0x00000288
//#define CS_ALLOC_WINDOW				0x400

// CE-500
#define nCAN_RESET                  PCA_GPIO_56
#define CAN_ON                      PCA_GPIO_57
//#define CAN_TR_ON                   PCA_GPIO_58

#define nCAN_IRQ                    GPIO_59


#define MCP2515_RESET_ON			0
#define MCP2515_RESET_OFF			1



// MCP2515 SPI Commands ===========================================================================

#define MCP2515SPICMD_RESET			( ( unsigned __int8 ) 0xC0 )
#define MCP2515SPICMD_READ			( ( unsigned __int8 ) 0x03 )
#define MCP2515SPICMD_WRITE			( ( unsigned __int8 ) 0x02 )
#define MCP2515SPICMD_RTS			( ( unsigned __int8 ) 0x80 )
#define MCP2515SPICMD_BITMODIFY		( ( unsigned __int8 ) 0x05 )

// MCP2515 Operation Modes ========================================================================

enum MCPOPMODE { OPMODE_NORMAL			= 0, 
				 OPMODE_SLEEP			= 1, 
				 OPMODE_LOOPBACK		= 2, 
				 OPMODE_LISTEN			= 3, 
				 OPMODE_CONFIGURATION	= 4 };


// Interrupt Flag Codes ===========================================================================

enum INTRCODE
{
	INTRFC_NOINTR	= 0, 
	INTRFC_ERR		= 1, 
	INTRFC_WAKUP	= 2, 
	INTRFC_TXB0		= 3, 
	INTRFC_TXB1		= 4, 
	INTRFC_TXB2		= 5, 
	INTRFC_RXB0		= 6, 
	INTRFC_RXB1		= 7
};


// Registers and Bits =============================================================================

#define		ON					( ( unsigned __int8 ) 0x01 ) // Bit ON
#define		OFF					( ( unsigned __int8 ) 0x00 ) // Bit OFF

// ------------------------------------------------------------------------------------------------

#define		CANSTAT_ADDR		( ( unsigned __int8 ) 0x0E ) // Status
#define		CANSTAT_LEN									 1 // 1 byte

typedef struct 
{
	unsigned __int8		    	:	1; // NOT USED
	unsigned __int8		ICOD	:	3; // Interrupt Flag Code
	unsigned __int8		    	:	1; // NOT USED
	unsigned __int8		OPMOD	:	3; // MCP2515 Operation Mode

	unsigned __int8		pad2;
	unsigned __int8		pad3;
	unsigned __int8		pad4;

}CANSTAT_DT, *PCANSTAT_DT;

// ------------------------------------------------------------------------------------------------

#define		CANCTRL_ADDR		( ( unsigned __int8 ) 0x0F ) // CAN Control
#define		CANCTR_LEN									 1	// 1 byte

typedef struct 
{
	unsigned __int8		CLKPRE	:	2; // CLKOUT Pin Prescaler
	unsigned __int8		CLKEN	:	1; // CLKOUT Pin Enable
	unsigned __int8		OSM		:	1; // One Shot Mode
	unsigned __int8		ABAT	:	1; // Abort all Pending Transmissions
	unsigned __int8		REQOP	:	3; // Request Operation Mode for MCP2515

	unsigned __int8		pad2;
	unsigned __int8		pad3;
	unsigned __int8		pad4;

}CANCTRL_DT, *PCANCTRL_DT;


#define		CLKPRE_m			( ( unsigned __int8 ) 0x03 )
#define		CLKPRE_s			( ( unsigned __int8 ) 0x00 )

#define		CLKEN_m				( ( unsigned __int8 ) 0x04 )
#define		CLKEN_s				( ( unsigned __int8 ) 0x02 )

#define		OSM_m				( ( unsigned __int8 ) 0x08 )
#define		OSM_s				( ( unsigned __int8 ) 0x03 )

#define		ABAT_m				( ( unsigned __int8 ) 0x10 )
#define		ABAT_s				( ( unsigned __int8 ) 0x04 )

#define		REQOP_m				( ( unsigned __int8 ) 0xE0 )
#define		REQOP_s				( ( unsigned __int8 ) 0x05 )

// ------------------------------------------------------------------------------------------------

#define		CNF_ADDR			( ( unsigned __int8 ) 0x28 ) // Configuration Registers
#define		CNF_LEN										 3   // 3 byte

typedef struct
{
	unsigned __int8		PHSEG2	:	3; // Phase Segment #2 Length
	unsigned __int8		    	:	3; // NOT USED
	unsigned __int8		WAKFIL	:	1; // Wake-Up Filter Enable
	unsigned __int8		SOF		:	1; // Start Of Frame Signal

	unsigned __int8		PRSEG	:	3; // Propagation Segment Length
	unsigned __int8		PHSEG1	:	3; // Phase Segment #1 Length
	unsigned __int8		SAM		:	1; // Sample Point Configuration
	unsigned __int8		BTLMODE	:	1; // Phase Segment #2 Bit Time Length

	unsigned __int8		BRP		:	6; // Baud Rate Prescaler
	unsigned __int8		SJW		:	2; // Sinchronization Jump Width Length

	unsigned __int8		pad4;

}CNF_DT, *PCNF_DT;

#define		PHSEG2_m			( ( unsigned __int8 ) 0x07 )
#define		PHSEG2_s			( ( unsigned __int8 ) 0x00 )

#define		WAKFIL_m			( ( unsigned __int8 ) 0x40 )
#define		WAKFIL_s			( ( unsigned __int8 ) 0x06 )

#define		SOF_m				( ( unsigned __int8 ) 0x80 )
#define		SOF_s				( ( unsigned __int8 ) 0x07 )

#define		PRSEG_m				( ( unsigned __int8 ) 0x07 )
#define		PRSEG_s				( ( unsigned __int8 ) 0x00 )

#define		PHSEG1_m			( ( unsigned __int8 ) 0x38 )
#define		PHSEG1_s			( ( unsigned __int8 ) 0x03 )

#define		SAM_m				( ( unsigned __int8 ) 0x40 )
#define		SAM_s				( ( unsigned __int8 ) 0x06 )

#define		BTLMODE_m			( ( unsigned __int8 ) 0x80 )
#define		BTLMODE_s			( ( unsigned __int8 ) 0x07 )

#define		BRP_m				( ( unsigned __int8 ) 0x3F )
#define		BRP_s				( ( unsigned __int8 ) 0x00 )

#define		SJW_m				( ( unsigned __int8 ) 0xC0 )
#define		SJW_s				( ( unsigned __int8 ) 0x06 )

// ------------------------------------------------------------------------------------------------

#define		CANINTE_ADDR		( ( unsigned __int8 ) 0x2B ) // CAN Interrupt Enable/Disable
#define		CANINTE_LEN									 1   // 1 byte

typedef struct
{
	unsigned __int8		RX0IE	:	1; // Rx Buffer #0 Full Interrupt Enable/Disable
	unsigned __int8		RX1IE	:	1; // Rx Buffer #1 Full Interrupt Enable/Disable
	unsigned __int8		TX0IE	:	1; // Tx Buffer #0 Empty Interrupt Enable/Disable
	unsigned __int8		TX1IE	:	1; // Tx Buffer #1 Empty Interrupt Enable/Disable
	unsigned __int8		TX2IE	:	1; // Tx Buffer #2 Empty Interrupt Enable/Disable
	unsigned __int8		ERRIE	:	1; // Error, sources in EFLG Interrupt Enable/Disable
	unsigned __int8		WAKIE	:	1; // Wake-Up Interrupt Enable/Disable
	unsigned __int8		MERRE	:	1; // Message Error during Rx / Tx Interrupt Enable/Disable

	unsigned __int8		pad2;
	unsigned __int8		pad3;
	unsigned __int8		pad4;

}CANINTE_DT, *PCANINTE_DT;


#define		CANINTF_ADDR		( ( unsigned __int8 ) 0x2C ) // CAN Interrupt Flag
#define		CANINTF_LEN									 1   // 1 byte

typedef struct
{
	unsigned __int8		RX0IF	:	1; // Rx Buffer #0 Full Interrupt Flag
	unsigned __int8		RX1IF	:	1; // Rx Buffer #1 Full Interrupt Flag
	unsigned __int8		TX0IF	:	1; // Tx Buffer #0 Empty Interrupt Flag
	unsigned __int8		TX1IF	:	1; // Tx Buffer #1 Empty Interrupt Flag
	unsigned __int8		TX2IF	:	1; // Tx Buffer #2 Empty Interrupt Flag
	unsigned __int8		ERRIF	:	1; // Error, sources in EFLG Interrupt Flag
	unsigned __int8		WAKIF	:	1; // Wake-Up Interrupt Flag
	unsigned __int8		MERRF	:	1; // Message Error during Rx / Tx Interrupt Flag

	unsigned __int8		pad2;
	unsigned __int8		pad3;
	unsigned __int8		pad4;

}CANINTF_DT, *PCANINTF_DT;


#define RX0I_bit	0
#define	RX1I_bit	1
#define	TX0I_bit	2
#define	TX1I_bit	3
#define	TX2I_bit	4
#define	ERRI_bit	5
#define	WAKI_bit	6
#define	MERR_bit	7

// ------------------------------------------------------------------------------------------------

#define		EFLG_ADDR			( ( unsigned __int8 ) 0x2D ) // Error Flag
#define		EFLG_LEN									 1   // 1 byte

typedef struct
{
	unsigned __int8		EWARN	:	1; // Warning, TEC | REC >= 96
	unsigned __int8		RXWAR	:	1; // Rx Warning, REC >= 96
	unsigned __int8		TXWAR	:	1; // Tx Warning, REC >= 96
	unsigned __int8		RXEP	:	1; // Rx Error Passive Flag, REC >= 128
	unsigned __int8		TXEP	:	1; // Tx Error Passive Flag, TEC >= 128
	unsigned __int8		TXBO	:	1; // Bus-OFF Error, TEC == 255
	unsigned __int8		RX0OVR	:	1; // Rx Buffer #0 Overflow
	unsigned __int8		RX1OVR	:	1; // Rx Buffer #1 Overflow

	unsigned __int8		pad2;
	unsigned __int8		pad3;
	unsigned __int8		pad4;

}EFLG_DT, *PEFLG_DT;

#define		EWARN_bit			0
#define		RXWAR_bit			1
#define		TXWAR_bit			2
#define		RXEP_bit			3
#define		TXEP_bit			4
#define		TXBO_bit			5
#define		RX0OVR_bit			6
#define		RX1OVR_bit			7

// ------------------------------------------------------------------------------------------------

#define		TXRTSCTRL_ADDR		( ( unsigned __int8 ) 0x0D ) // Request To Send Pin Control and Status
#define		TXRTSCTRL_LEN							     1   // 1 byte

typedef struct
{
	unsigned __int8		B0RTSM	:	1; // ^TX0RTS Pin Mode
	unsigned __int8		B1RTSM	:	1; // ^TX1RTS Pin Mode
	unsigned __int8		B2RTSM	:	1; // ^TX2RTS Pin Mode
	unsigned __int8		B0RTS	:	1; // ^TX0RTS Pin State
	unsigned __int8		B1RTS	:	1; // ^TX1RTS Pin State
	unsigned __int8		B2RTS	:	1; // ^TX2RTS Pin State
	unsigned __int8		    	:	2; // NOT USED

	unsigned __int8		pad2;
	unsigned __int8		pad3;
	unsigned __int8		pad4;

}TXRTSCTRL_DT, *PTXRTSCTRL_DT;

#define		B0RTSM_m			( ( unsigned __int8 ) 0x01 )
#define		B0RTSM_s			( ( unsigned __int8 ) 0x00 )

#define		B1RTSM_m			( ( unsigned __int8 ) 0x02 )
#define		B1RTSM_s			( ( unsigned __int8 ) 0x01 )

#define		B2RTSM_m			( ( unsigned __int8 ) 0x04 )
#define		B2RTSM_s			( ( unsigned __int8 ) 0x02 )

#define		B0RTS_m				( ( unsigned __int8 ) 0x08 )
#define		B0RTS_s				( ( unsigned __int8 ) 0x03 )

#define		B1RTS_m				( ( unsigned __int8 ) 0x10 )
#define		B1RTS_s				( ( unsigned __int8 ) 0x04 )

#define		B2RTS_m				( ( unsigned __int8 ) 0x20 )
#define		B2RTS_s				( ( unsigned __int8 ) 0x05 )


#define		TXB0CTRL_ADDR		( ( unsigned __int8 ) 0x30 ) // Transmit Buffer #0 Control
#define		TXB1CTRL_ADDR		( ( unsigned __int8 ) 0x40 ) // Transmit Buffer #1 Control
#define		TXB2CTRL_ADDR		( ( unsigned __int8 ) 0x50 ) // Transmit Buffer #2 Control

#define		TXBnCTRL_LEN								 1   // 1 byte

typedef struct
{
	unsigned __int8		TXP		:	2; // Tx Buffer Priority, Internal to MCP2515
	unsigned __int8		    	:	1; // NOT USED
	unsigned __int8		TXREQ	:	1; // Message Transmit Request
	unsigned __int8		TXERR	:	1; // Transmission Error Detected on Bus
	unsigned __int8		MLOA	:	1; // Message Lost Arbitration Flag
	unsigned __int8		ABTF	:	1; // Message Aborted Flag
	unsigned __int8		    	:	1; // NOT USED

	unsigned __int8		pad2;
	unsigned __int8		pad3;
	unsigned __int8		pad4;

}TXBnCTRL_DT, *PTXBnCTRL_DT;

#define		TXP_bit				0 // two bits
#define		TXREQ_bit			3
#define		TXERR_bit			4
#define		MLOA_bit			5
#define		ABTF_bit			6

#define		TEC_ADDR			( ( unsigned __int8 ) 0x1C ) // Transmit Error Counter

typedef struct
{
	unsigned __int8		TEC		:	8; // Transmit Error Counter

	unsigned __int8		pad2;
	unsigned __int8		pad3;
	unsigned __int8		pad4;

}TEC_DT, *PTEC_DT;


#define		TXB0SIDH_ADDR		( ( unsigned __int8 ) 0x31 ) // Standart ID High, Starting of Tx Buffer #0
#define		TXB1SIDH_ADDR		( ( unsigned __int8 ) 0x41 ) // Standart ID High, Starting of Tx Buffer #1
#define		TXB2SIDH_ADDR		( ( unsigned __int8 ) 0x51 ) // Standart ID High, Starting of Tx Buffer #2

#define		TXBnPACKET_LEN								13	 // 13 bytes

typedef struct
{
	unsigned __int8		SIDH	:	8;	// Standard Identifier High bits <10:3>
	unsigned __int8		EIDH	:	2;  // Extended Identifier High bits <17:16>
	unsigned __int8				:	1;	// NOT USED
	unsigned __int8		EXIDE	:	1;	// Extended Identifier Enable
	unsigned __int8				:	1;	// NOT USED
	unsigned __int8		SIDL	:	3;	// Standard Identifier Low bits <2:0>
	unsigned __int8		EID8	:	8;	// Extended Identifier bits <15:8>
	unsigned __int8		EID0	:	8;	// Extended Identifier bits <7:0>
	unsigned __int8		DLC		:	4;	// Data Length Code
	unsigned __int8				:	2;	// NOT USED
	unsigned __int8		RTR		:	1;	// Remote Transmission Request
	unsigned __int8				:	1;	// NOT USED	

	unsigned __int8		DATA[ 8 ]	 ;	// 8 Data Bytes

	unsigned __int8		pad2;
	unsigned __int8		pad3;
	unsigned __int8		pad4;

}TXBnPACKET_DT, *PTXBnPACKET_DT;

// ------------------------------------------------------------------------------------------------

#define		BFPCTRL_ADDR		( ( unsigned __int8 ) 0x0C ) // Buffer is full Pin Control and Status
#define		BFPCTRL_LEN									 1   // 1 byte

typedef struct 
{
	unsigned __int8		B0BFM	:	1; // ^RX0BFM Pin Operation Mode
	unsigned __int8		B1BFM	:	1; // ^RX1BFM Pin Operation Mode
	unsigned __int8		B0BFE	:	1; // ^RX0BFE Pin Function Enable
	unsigned __int8		B1BFE	:	1; // ^RX1BFE Pin Function Enable
	unsigned __int8		B0BFS	:	1; // ^RX0BFS Pin Pin State
	unsigned __int8		B1BFS	:	1; // ^RX1BFS Pin Pin State
	unsigned __int8		    	:	2; // NOT USED

	unsigned __int8		pad2;
	unsigned __int8		pad3;
	unsigned __int8		pad4;

}BFPCTRL_DT, *PBFPCTRL_DT;

#define		B0BFM_m				( ( unsigned __int8 ) 0x01 )
#define		B0BFM_s				( ( unsigned __int8 ) 0x00 )

#define		B1BFM_m				( ( unsigned __int8 ) 0x02 )
#define		B1BFM_s				( ( unsigned __int8 ) 0x01 )

#define		B0BFE_m				( ( unsigned __int8 ) 0x04 )
#define		B0BFE_s				( ( unsigned __int8 ) 0x02 )

#define		B1BFE_m				( ( unsigned __int8 ) 0x08 )
#define		B1BFE_s				( ( unsigned __int8 ) 0x03 )

#define		B0BFS_m				( ( unsigned __int8 ) 0x10 )
#define		B0BFS_s				( ( unsigned __int8 ) 0x04 )

#define		B1BFS_m				( ( unsigned __int8 ) 0x20 )
#define		B1BFS_s				( ( unsigned __int8 ) 0x05 )


#define		RXB0CTRL_ADDR		( ( unsigned __int8 ) 0x60 ) // Receive Buffer #0 Control
#define		RXB1CTRL_ADDR		( ( unsigned __int8 ) 0x70 ) // Receive Buffer #1 Control

#define		RXBnCTRL_LEN								1	 // 1 byte

typedef struct
{
	unsigned __int8		FILHIT	:	3; // Filter Hit, different in RXB0 and RXB1 ( Rollover issue )
	unsigned __int8		RXRTR	:	1; // Received Remote Transfer Request
	unsigned __int8		    	:	1; // NOT USED
	unsigned __int8		RXM		:	2; // Receive Buffer Operating Mode
	unsigned __int8		    	:	1; // NOT USED

	unsigned __int8		pad2;
	unsigned __int8		pad3;
	unsigned __int8		pad4;

}RXBnCTRL_DT, *PRXBnCTRL_DT;

#define		FILHIT_m			( ( unsigned __int8 ) 0x07 )
#define		FILHIT_s			( ( unsigned __int8 ) 0x00 )

#define		RXRTR_m				( ( unsigned __int8 ) 0x08 )
#define		RXRTR_s				( ( unsigned __int8 ) 0x03 )

#define		RXM_m				( ( unsigned __int8 ) 0x60 )
#define		RXM_s				( ( unsigned __int8 ) 0x05 )


#define		REC_ADDR			( ( unsigned __int8 ) 0x1D ) // Receive Error Counter
#define		REC_LEN										1    // 1 byte

typedef struct
{
	unsigned __int8		REC		:	8; // Receive Error Counter

	unsigned __int8		pad2;
	unsigned __int8		pad3;
	unsigned __int8		pad4;

}REC_DT, *PREC_DT;


#define		RXM0SIDH_ADDR		( ( unsigned __int8 ) 0x20 ) // Mask Starting Address for Buffer #0 
#define		RXM1SIDH_ADDR		( ( unsigned __int8 ) 0x24 ) // Mask Starting Address for Buffer #1

#define		RXMnPACKET_LEN								4    // 4 bytes

typedef struct
{
	unsigned __int8		SIDH	:	8;	// Standard Identifier High bits <10:3>
	unsigned __int8		EIDH	:	2;	// Extended Identifier High bits <17:16>
	unsigned __int8				:	3;	// NOT USED
	unsigned __int8		SIDL	:	3;	// Standard Identifier Low bits <2:0>
	unsigned __int8		EID8	:	8;	// Extended Identifier bits <15:8>
	unsigned __int8		EID0	:	8;  // Extended Identifier bits <7:8>

}RXMnPACKET, *PRXMnPACKET;


#define		RXF0SIDH_ADDR		( ( unsigned __int8 ) 0x00 ) // Mask Starting Address for Buffer #0 
#define		RXF1SIDH_ADDR		( ( unsigned __int8 ) 0x04 ) // Mask Starting Address for Buffer #1
#define		RXF2SIDH_ADDR		( ( unsigned __int8 ) 0x08 ) // Mask Starting Address for Buffer #2
#define		RXF3SIDH_ADDR		( ( unsigned __int8 ) 0x10 ) // Mask Starting Address for Buffer #3
#define		RXF4SIDH_ADDR		( ( unsigned __int8 ) 0x14 ) // Mask Starting Address for Buffer #4
#define		RXF5SIDH_ADDR		( ( unsigned __int8 ) 0x18 ) // Mask Starting Address for Buffer #5

#define		RXFnPACKET_LEN								4    // 4 bytes

typedef struct
{
	unsigned __int8		SIDH	:	8;	// Standard Identifier High bits <10:3>
	unsigned __int8		EIDH	:	2;	// Extended Identifier High bits <17:16>
	unsigned __int8				:	1;	// NOT USED
	unsigned __int8		EXIDE	:	1;	// Extended Identifier Enable
	unsigned __int8				:	1;	// NOT USED
	unsigned __int8		SIDL	:	3;	// Standard Identifier Low bits <2:0>
	unsigned __int8		EID8	:	8;	// Extended Identifier bits <15:8>
	unsigned __int8		EID0	:	8;  // Extended Identifier bits <7:8>


}RXFnPACKET, *PRXFnPACKET;


#define		RXB0SIDH_ADDR		( ( unsigned __int8 ) 0x61 ) // Standard ID High, Starting of Rx Buffer #0
#define		RXB1SIDH_ADDR		( ( unsigned __int8 ) 0x71 ) // Standard ID High, Starting of Rx Buffer #1

#define		RXBnPACKET_LEN								13    // 13 bytes

typedef struct
{
	unsigned __int8		SIDH	:	8;	// Standard Identifier High bits <10:3>
	unsigned __int8		EIDH	:	2;  // Extended Identifier High bits <17:16>
	unsigned __int8	    		:	1;	// NOT USED
	unsigned __int8		IDE		:	1;	// Extended Identifier Message Flag
	unsigned __int8		SRR		:	1;	// Standard Frame Remote Transmit Request
	unsigned __int8		SIDL	:	3;	// Standard Identifier Low bits <2:0>
	unsigned __int8		EID8	:	8;  // Extended Identifier bits <15:8>
	unsigned __int8		EID0	:	8;  // Extended Identifier bits <7:8>
	unsigned __int8		DLC		:	4;	// Data Length Code
	unsigned __int8		RB0		:	1;	// Reserved Bit #0
	unsigned __int8		RB1		:	1;	// Reserved Bit #1
	unsigned __int8		RTR		:	1;	// Remote Transmission Request
	unsigned __int8	    		:	1;	// NOT USED

	unsigned __int8		DATA[ 8 ]	 ;	// 8 Data Bytes

	unsigned __int8		pad2;
	unsigned __int8		pad3;
	unsigned __int8		pad4;

}RXBnPACKET_DT, *PRXBnPACKET_DT;



// Functions ======================================================================================

// Function name	: MCP2515Init
// Description	    : Initialize MCP2515 to default settings and 
//					  Bit Timing according configuration from Registry
// Return type		: BOOL MCP2515Init 
// Argument         : PCANREGSETTINGS

BOOL MCP2515Init ( CAN_INIT_CONTEXT    *pCxt, PCANREGSETTINGS );

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515DeInit
// Description	    : DeInitializes System Resources for MCP2515
// Return type		: BOOL MCP2515DeInit 

BOOL MCP2515DeInit ( CAN_INIT_CONTEXT    *pCxt );


// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515HwReset
// Description	    : Performes Hardware Reset for MCP2515
// Return type		: void MCP2515HwReset 

void MCP2515HwReset( CAN_INIT_CONTEXT    *pCxt );


// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515ReadSPI
// Description	    : Read N <= 16 Bytes from specific address
// Return type		: BOOL MCP2515ReadSPI 
// Argument         : unsigned __int8*
// Argument         : unsigned __int8
// Argument         : unsigned __int8

BOOL MCP2515ReadSPI ( void *, 
					  unsigned __int8, 
					  unsigned __int8 );

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515WriteSPI
// Description	    : Write N Bytes to specific address
// Return type		: BOOL MCP2515WriteSPI 
// Argument         : unsigned __int8*
// Argument         : unsigned __int8
// Argument         : unsigned __int8

BOOL MCP2515WriteSPI ( void *, 
					   unsigned __int8, 
					   unsigned __int8 );

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515RequestToSendSPI
// Description	    : Performes Rquest To Send for Specific Tx Buffer
// Return type		: BOOL MCP2515RequestToSendSPI 
// Argument         : TXBID

BOOL MCP2515RequestToSendSPI ( TXBID );

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515BitModifySPI
// Description	    : Modifies specific Bit or Group of Bits
// Return type		: BOOL MCP2515BitModifySPI 
// Argument         : unsigned __int8
// Argument         : unsigned __int8
// Argument         : unsigned __int8

void MCP2515BitModifySPI(unsigned __int8, unsigned __int8, unsigned __int8);

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515ResetSPI
// Description	    : Reset internal Registers to Default State, 
//					  Sets Configuration Mode
// Return type		: void MCP2515ResetSPI 
void MCP2515ResetSPI();

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515SetModeSPI
// Description	    : Set MCP2515 to specific Operation Mode
// Return type		: BOOL MCP2515SetModeSPI 
// Argument         : MCPOPMODE

BOOL MCP2515SetModeSPI ( MCPOPMODE );

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515GetModeSPI
// Description	    : Gets MCP2515 Operation Mode
// Return type		: MCPOPMODE MCP2515GetModeSPI 

MCPOPMODE MCP2515GetModeSPI(void);

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515SetMask
// Description	    : Set Mask to Specific Rx Buffer
// Return type		: BOOL MCP2515BSetMask 
// Argument         : unsigned __int8
// Argument         : unsigned __int32
// Argument         : unsigned __int32

BOOL MCP2515SetMask ( unsigned __int8, 
					  unsigned __int32, 
					  unsigned __int32 );

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515GetMask
// Description	    : Get Mask of specific Rx Buffer
// Return type		: BOOL MCP2515GetMask 
// Argument         : unsigned __int8
// Argument         : unsigned __int32 *
// Argument         : unsigned __int32 *

BOOL MCP2515GetMask ( unsigned __int8, 
					  unsigned __int32 *, 
					  unsigned __int32 * );

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515SetFilter
// Description	    : Set Mask to Specific Rx Buffer
// Return type		: BOOL MCP2515SetFilter 
// Argument         : unsigned __int8
// Argument         : unsigned __int32
// Argument         : unsigned __int32
// Argument         : BOOL

BOOL MCP2515SetFilter ( unsigned __int8, 
					    unsigned __int32, 
						unsigned __int32, 
						BOOL );

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515GetFilter
// Description	    : Get Filter of Specific Rx Buffer
// Return type		: BOOL MCP2515GetFilter 
// Argument         : unsigned __int8
// Argument         : unsigned __int32 *
// Argument         : unsigned __int32 *
// Argument         : BOOL *

BOOL MCP2515GetFilter ( unsigned __int8, 
						unsigned __int32 *, 
						unsigned __int32 *, 
						BOOL *);

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515SetRXM
// Description	    : Set Receive Buffer Opearint Mode
// Return type		: BOOL MCP2515SetRXM 
// Argument         : unsigned __int8
// Argument         : unsigned __int32

BOOL MCP2515SetRXM ( unsigned __int8, 
					 unsigned __int32 );

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515SetRXM
// Description	    : Set Receive Buffer Opearint Mode
// Return type		: BOOL MCP2515SetRXM 
// Argument         : unsigned __int8
// Argument         : RXBOPMODE *

BOOL MCP2515GetRXM ( unsigned __int8 , 
					 RXBOPMODE *);

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515SetCNF
// Description	    : Set CAN Configuration
// Return type		: BOOL MCP2515SetCNF 
// Argument         : PCANREGSETTINGS

BOOL MCP2515SetCNF ( PCANREGSETTINGS );

// ------------------------------------------------------------------------------------------------

// Function name	: MCP2515GetCNF
// Description	    : Get CAN Configuration
// Return type		: BOOL MCP2515GetCNF 
// Argument         : PCNF_DT

BOOL MCP2515GetCNF ( PCNF_DT );

// ------------------------------------------------------------------------------------------------

void MCP2515PowerOff(CAN_INIT_CONTEXT  *pCxt);
void MCP2515PowerOn(CAN_INIT_CONTEXT   *pCxt);
BOOL MCP2515PowerStatus(CAN_INIT_CONTEXT    *pCxt);

// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------


#endif //__MCP2515_H