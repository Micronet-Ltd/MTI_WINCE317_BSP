/*  Registers
*/
#define EMIFS_BASE_PHYS     (0xFFFECC00)
#define EMIFS_BASE_EXTENT   (0x100)

#define EMIFS_CONFIG        (gEMIFS_BASE + 3)
#define EMIFS_CCS0          (gEMIFS_BASE + 4)
#define EMIFS_CCS1          (gEMIFS_BASE + 5)
#define EMIFS_CCS2          (gEMIFS_BASE + 6)
#define EMIFS_CCS3          (gEMIFS_BASE + 7)
#define EMIFS_ACS0          (gEMIFS_BASE + 20)
#define EMIFS_ACS1          (gEMIFS_BASE + 21)
#define EMIFS_ACS2          (gEMIFS_BASE + 22)
#define EMIFS_ACS3          (gEMIFS_BASE + 23)


#define NAND_CTRL_BASE_PHYS     (0x0A000000)
#define NAND_CTRL_BASE_EXTENT   (0x1000)

#define NAND_CTLR_DATA      (gNAND_CTLR_BASE)
#define NAND_CTLR_COMMAND   (gNAND_CTLR_BASE + 2)
#define NAND_CTLR_ADDR      (gNAND_CTLR_BASE + 4)

#define WRITE_REGISTER8(addr, val) (*((volatile unsigned char *) (addr)) = (val))
#define READ_REGISTER8(addr) (*((volatile unsigned char *) (addr)))
#define WRITE_REGISTER16(addr, val) (*((volatile unsigned short *) (addr)) = (val))
#define READ_REGISTER16(addr) (*((volatile unsigned short *) (addr)))
#define WRITE_REGISTER32(addr, val) (*((volatile unsigned long *) (addr)) = (val))
#define READ_REGISTER32(addr) (*((volatile unsigned long *) (addr)))

/*  Register values and flags
*/
#define EMIFS_CONFIG_FLAGS  (0x00000001)

//#define EMIFS_CCS2_VALUE    (0xFF80FFFB) /* safe */
//#define EMIFS_CCS2_VALUE    (0xFF85FFFB) /* puts clk sig external (mode 5) */
//#define EMIFS_CCS2_VALUE    (0xFF80FFF8) /* Good */
#define EMIFS_CCS2_VALUE    (0xF8804FF8) /* Fast */


#define NAND_RDY            (0x00000010)


