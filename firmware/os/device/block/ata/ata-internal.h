#ifndef DEVICE_BLOCK_ATA_ATAINTERNAL_H_INC
#define DEVICE_BLOCK_ATA_ATAINTERNAL_H_INC
/*
	AT Attachment (ATA) interface driver function and macro declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include "include/byteorder.h"
#include "include/types.h"
#include "device/devctl.h"


#define ATA_BASE			((u8 *) 0x00e20000)	/* Base address of the ATA interface	*/

#define ATA_OFFSET			(0x1000)			/* Offset of bus 1 from bus 0 registers */

#define ATA_NUM_BUSES		(1)			/* # ATA buses (i.e. channels) in hardware		*/
#define ATA_DEVICES_PER_BUS	(2)			/* # devices which may be connected to each bus	*/

#define ATA_MAX_DEVICES		(ATA_NUM_BUSES * ATA_DEVICES_PER_BUS)

#define ATA_SECTOR_SIZE		(512)		/* Sector size - constant for all ATA devices	*/
#define ATA_LOG_SECTOR_SIZE	(9)			/* = log2(ATA_SECTOR_SIZE)						*/

#define ATA_TIMEOUT_VAL		(1000000)	/* Timeout ctr initial val.  TODO: improve this	*/


/*
	Macros which wait for the ATA device's BSY flag to be asserted (ATA_WAIT_BSY()) or negated
	(ATA_WAIT_NBSY).
*/
#define ATA_WAIT_BSY(bus)																	\
					(__extension__ ({														\
						u32 __t = ATA_TIMEOUT_VAL;											\
						for(; !(ATA_REG(bus, ATA_R_STATUS) & ATA_STATUS_BSY) && --__t;) ;	\
						__t;																\
					}))

#define ATA_WAIT_NBSY(bus)																	\
					(__extension__ ({														\
						u32 __t = ATA_TIMEOUT_VAL;											\
						for(; (ATA_REG(bus, ATA_R_STATUS) & ATA_STATUS_BSY) && --__t;) ;	\
						__t;																\
					}))


/*
	ATA registers
	-------------

	Note: the CPU address bus is mapped to the ATA control and address bus thus:

		ATA:	nCS1	nCS0	DA2		DA1		DA0
		CPU:	A5		A4		A3		A2		A1

	Byte-wide registers must be read/written on odd addresses.  Half-word registers (i.e. the ATA
	data port) must be read/written on even addresses.  For example, address ATA_BASE + 0x2f
	decodes to:

		ATA:	nCS1	nCS0	DA2		DA1		DA0
				1		0		1		1		1		[1]
				-----------   -------------------------------
				     2                       f

	...i.e. the status register (for reads) / the command register (for writes)

	MAIN REGISTERS (nCS1 negated, nCS0 asserted)

	reg #				read									write
	--------------------------------------------------------------------------------------------
	0					Data									Data							*/
#define ATA_R_DATA				(0x20)

/*	1					Error									Features						*/
#define ATA_R_ERROR				(0x23)
#define ATA_R_FEATURES			(0x23)

/*	2					Sector count							Sector count					*/
#define ATA_R_SECTOR_COUNT		(0x25)

/*	3					Sector number							Sector number					*/
#define ATA_R_SECTOR_NUMBER		(0x27)

/*	4					Cylinder low							Cylinder low					*/
#define ATA_R_CYLINDER_LOW		(0x29)

/*	5					Cylinder high							Cylinder high					*/
#define ATA_R_CYLINDER_HIGH		(0x2b)

/*	6					Device / head							Device / head					*/
#define ATA_R_DEVICE_HEAD		(0x2d)

/*	7					Status									Commmand						*/
#define ATA_R_STATUS			(0x2f)
#define ATA_R_COMMAND			(0x2f)

/*
	ALTERNATE REGISTERS (nCS1 negated, nCS0 asserted)

	reg #				read									write
	--------------------------------------------------------------------------------------------
	6					Alternate status						Device control					*/
#define ATA_R_ALT_STATUS		(0x1d)
#define ATA_R_DEVICE_CONTROL	(0x1d)


#define ATA_REG_DATA(bus) 	*((volatile u16 *) ((u16 *) (ATA_BASE + (bus * ATA_OFFSET) + ATA_R_DATA)))
#define ATA_REG(bus, off)	*((volatile u8 *) (ATA_BASE + (bus * ATA_OFFSET) + off))

/*
	ATA command codes
*/

enum __ata_command
{
	ATA_CMD_CFA_ERASE_SECTORS					= 0xc0,
	ATA_CMD_CFA_REQUEST_EXTENDED_ERROR			= 0x03,
	ATA_CMD_CFA_TRANSLATE_SECTOR				= 0x87,
	ATA_CMD_CFA_WRITE_MULTIPLE_WITHOUT_ERASE	= 0xcd,
	ATA_CMD_CFA_WRITE_SECTORS_WITHOUT_ERASE		= 0x38,
	ATA_CMD_CHECK_POWER_MODE					= 0xe5,
	ATA_CMD_DEVICE_RESET						= 0x08,
	ATA_CMD_DOWNLOAD_MICROCODE					= 0x92,
	ATA_CMD_EXECUTE_DEVICE_DIAGNOSTIC			= 0x90,
	ATA_CMD_FLUSH_CACHE							= 0xe7,
	ATA_CMD_GET_MEDIA_STATUS					= 0xda,
	ATA_CMD_IDENTIFY_DEVICE						= 0xec,
	ATA_CMD_IDENTIFY_PACKET_DEVICE				= 0xa1,
	ATA_CMD_IDLE								= 0xe3,
	ATA_CMD_IDLE_IMMEDIATE						= 0xe1,
	ATA_CMD_INITIALIZE_DEVICE_PARAMETERS		= 0x91,
	ATA_CMD_MEDIA_EJECT							= 0xed,
	ATA_CMD_MEDIA_LOCK							= 0xde,
	ATA_CMD_MEDIA_UNLOCK						= 0xdf,
	ATA_CMD_NOP									= 0x00,
	ATA_CMD_PACKET								= 0xa0,
	ATA_CMD_READ_BUFFER							= 0xe4,
	ATA_CMD_READ_DMA							= 0xc8,
	ATA_CMD_READ_DMA_QUEUED						= 0xc7,
	ATA_CMD_READ_MULTIPLE						= 0xc4,
	ATA_CMD_READ_NATIVE_MAX_ADDRESS				= 0xf8,
	ATA_CMD_READ_SECTORS						= 0x20,
	ATA_CMD_READ_VERIFY_SECTORS					= 0x40,
	ATA_CMD_SECURITY_DISABLE_PASSWORD			= 0xf6,
	ATA_CMD_SECURITY_ERASE_PREPARE				= 0xf3,
	ATA_CMD_SECURITY_ERASE_UNIT					= 0xf4,
	ATA_CMD_SECURITY_FREEZE						= 0xf5,
	ATA_CMD_SECURITY_SET_PASSWORD				= 0xf1,
	ATA_CMD_SECURITY_UNLOCK						= 0xf2,
	ATA_CMD_SEEK								= 0x70,
	ATA_CMD_SERVICE								= 0xa2,
	ATA_CMD_SET_FEATURES						= 0xef,
	ATA_CMD_SET_MAX_ADDRESS						= 0xf9,
	ATA_CMD_SET_MULTIPLE_MODE					= 0xc6,
	ATA_CMD_SLEEP								= 0xe6,
	ATA_CMD_SMART_DISABLE_OPERATIONS			= 0xb0,
	ATA_CMD_SMART_ENABLE_AUTOSAVE				= 0xb0,
	ATA_CMD_SMART_ENABLE_OPERATIONS				= 0xb0,
	ATA_CMD_SMART_EXECUTE_OFF_LINE				= 0xb0,
	ATA_CMD_SMART_READ_DATA						= 0xb0,
	ATA_CMD_SMART_READ_LOG_SECTOR				= 0xb0,
	ATA_CMD_SMART_RETURN_STATUS					= 0xb0,
	ATA_CMD_SMART_WRITE_LOG_SECTOR				= 0xb0,
	ATA_CMD_STANDBY								= 0xe2,
	ATA_CMD_STANDBY_IMMEDIATE					= 0xe0,
	ATA_CMD_WRITE_BUFFER						= 0xe8,
	ATA_CMD_WRITE_DMA							= 0xca,
	ATA_CMD_WRITE_DMA_QUEUED					= 0xcc,
	ATA_CMD_WRITE_MULTIPLE						= 0xc5,
	ATA_CMD_WRITE_SECTORS						= 0x30
};


typedef enum __ata_command ata_command_t;

typedef u32 ata_ret_t;


/*
	ATA register bits
*/

/* Status register																				*/
#define ATA_STATUS_BSY					(0x80)
#define ATA_STATUS_DRDY					(0x40)
#define ATA_STATUS_DRQ					(0x08)
#define ATA_STATUS_ERR					(0x01)

/* Device control register																		*/
#define ATA_DEVICE_CONTROL_SRST			(0x04)
#define ATA_DEVICE_CONTROL_NIEN			(0x02)

/* Device/head register																			*/
#define ATA_DH_DEV						(0x10)
#define ATA_DH_LBA						(0x40)

/* Error register																				*/
#define ATA_ERROR_MED					(0x01)	/* Media error									*/
#define ATA_ERROR_NM					(0x02)	/* No media										*/
#define ATA_ERROR_ABRT					(0x04)	/* Abort										*/
#define ATA_ERROR_MCR					(0x08)	/* Media change request							*/
#define ATA_ERROR_IDNF					(0x10)	/* ID not found (no device / bad sector num)	*/
#define ATA_ERROR_MC					(0x20)	/* Media changed								*/
#define ATA_ERROR_WP					(0x40)	/* Write protect enabled						*/
#define ATA_ERROR_UNC					(0x40)	/* Uncorrectable data error						*/
#define ATA_ERROR_ICRC					(0x80)	/* Interface CRC error							*/


/*
	struct __ata_identify_device: 512-byte struct which may be mapped over the response to an
	IDENTIFY DEVICE command.
*/
struct ata_identify_device_ret
{
	u16 general_config;				/*   0: General config bit-significant information			*/
	u16 log_cyls;					/*   1: Number of logical cylinders							*/
	u16 specific_config;			/*   2: Specific configuration								*/
	u16 log_heads;					/*   3: Number of logical heads								*/
	u16 _word_4;					/*   4: [retired]											*/
	u16 _word_5;					/*   5: [retired]											*/
	u16 log_sects_per_log_track;	/*   6: Number of logical sectors per logical track			*/
	u16 _word_7;					/*	 7: [reserved]											*/
	u16 _word_8;					/*   8: [reserved]											*/
	u16 _word_9;					/*   9: [retired]											*/
	char serial_number[20];			/*  10: Serial number										*/
	u16 _word_20;					/*  20: [retired]											*/
	u16 _word_21;					/*  21: [retired]											*/
	u16 _word_22;					/*  22: [obsolete]											*/
	char firmware_revision[8];		/*  23: Firmware revision									*/
	char model_number[40];			/*  27: Model number										*/
	u16 sects_per_int_multi;		/*  47: Max # sectors xferred per interrupt in R/W MULTI	*/
	u16 _word_48;					/*  48: [reserved]											*/
	u16 capabilities_1;				/*  49: Capabilities 1										*/
	u16 capabilities_2;				/*  50: Capabilities 2										*/
	u16 _word_51;					/*  51: [retired]											*/
	u16 _word_52;					/*  52: [obsolete]											*/
	u16 capabilities_3;				/*  53: Capabilities 3										*/
	u16 current_log_cyls;			/*  54: Number of current logical cylinders					*/
	u16 current_log_heads;			/*  55: Number of current logical heads						*/
	u16 current_log_sects_per_track;/*  56: Number of current logical sectors per track			*/
	u16 current_capacity_sects_1;	/*  57: Current capacity in sectors (1/2)					*/
	u16 current_capacity_sects_2;	/*  58: Current capacity in sectors (2/2)					*/
	u16 current_sects_per_int_multi;/*  59: Current # sectors/int xferred in R/W MULTI			*/
	u16 user_addressable_sects_1;	/*  60: Total # user-addressable sectors in LBA mode (1/2)	*/
	u16 user_addressable_sects_2;	/*  61: Total # user-addressable sectors in LBA mode (2/2)	*/
	u16 _word_62;					/*  62: [obsolete]											*/
	u16 multiword_dma_level;		/*  63: Multiword DMA supported/selected level				*/
	u16 advanced_pio;				/*  64: Advanced PIO mode support							*/
	u16 min_multiword_dma_tcycle;	/*  65: Min multiword DMA t(cycle)/ns						*/
	u16 rec_multiword_dma_tcycle;	/*  66: Recommended multiword DMA t(cycle)/ns				*/
	u16 min_pio_tcycle_no_flow_ctl;	/*  67: Min PIO xfer t(cycle)/ns without flow control		*/
	u16 min_pio_tcycle_iordy;		/*  68: Min PIO xfer t(cycle)/ns with IORDY flow control	*/
	u16 _word_69;					/*  69: [reserved]											*/
	u16 _word_70;					/*  70: [reserved]											*/
	u16 _word_71;					/*  71: [reserved]											*/
	u16 _word_72;					/*  72: [reserved]											*/
	u16 _word_73;					/*  73: [reserved]											*/
	u16 _word_74;					/*  74: [reserved]											*/
	u16 queue_depth;				/*  75: Maximum queue depth - 1								*/
	u16 _word_76;					/*  76: [reserved]											*/
	u16 _word_77;					/*  77: [reserved]											*/
	u16 _word_78;					/*  78: [reserved]											*/
	u16 _word_79;					/*  79: [reserved]											*/
	u16 major_version_number;		/*  80: Major version number								*/
	u16 minor_version_number;		/*  81: Minor version number								*/
	u16 cmd_set_supported_1;		/*  82: Command set supported (1/3)							*/
	u16 cmd_set_supported_2;		/*  83: Command set supported (2/3)							*/
	u16 cmd_set_supported_3;		/*  84: Command set supported (3/3)							*/
	u16 cmd_feature_enabled_1;		/*  85: Command set / feature enabled (1/2)					*/
	u16 cmd_feature_enabled_2;		/*  86: Command set / feature enabled (2/2)					*/
	u16 cmd_feature_default;		/*  87: Command set / feature default						*/
	u16 udma_level;					/*  88: UDMA supported/selected level						*/
	u16 security_erase_time;		/*  89: Time required for SECURITY ERASE UNIT				*/
	u16 enh_security_erase_time;	/*  90: Time required for ENHANCED SECURITY ERASE UNIT		*/
	u16 current_apm;				/*  91: Current advanced power management value				*/
	u16 master_password_rev;		/*  92: Master password revision code						*/
	u16 hw_reset_result;			/*  93: Hardware reset result								*/
	u16 _word_94;					/*  94: [reserved]											*/
	u16 _word_95;					/*  95: [reserved]											*/
	u16 _word_96;					/*  96: [reserved]											*/
	u16 _word_97;					/*  97: [reserved]											*/
	u16 _word_98;					/*  98: [reserved]											*/
	u16 _word_99;					/*  99: [reserved]											*/
	u16 _word_100;					/* 100: [reserved]											*/
	u16 _word_101;					/* 101: [reserved]											*/
	u16 _word_102;					/* 102: [reserved]											*/
	u16 _word_103;					/* 103: [reserved]											*/
	u16 _word_104;					/* 104: [reserved]											*/
	u16 _word_105;					/* 105: [reserved]											*/
	u16 _word_106;					/* 106: [reserved]											*/
	u16 _word_107;					/* 107: [reserved]											*/
	u16 _word_108;					/* 108: [reserved]											*/
	u16 _word_109;					/* 109: [reserved]											*/
	u16 _word_110;					/* 110: [reserved]											*/
	u16 _word_111;					/* 111: [reserved]											*/
	u16 _word_112;					/* 112: [reserved]											*/
	u16 _word_113;					/* 113: [reserved]											*/
	u16 _word_114;					/* 114: [reserved]											*/
	u16 _word_115;					/* 115: [reserved]											*/
	u16 _word_116;					/* 116: [reserved]											*/
	u16 _word_117;					/* 117: [reserved]											*/
	u16 _word_118;					/* 118: [reserved]											*/
	u16 _word_119;					/* 119: [reserved]											*/
	u16 _word_120;					/* 120: [reserved]											*/
	u16 _word_121;					/* 121: [reserved]											*/
	u16 _word_122;					/* 122: [reserved]											*/
	u16 _word_123;					/* 123: [reserved]											*/
	u16 _word_124;					/* 124: [reserved]											*/
	u16 _word_125;					/* 125: [reserved]											*/
	u16 _word_126;					/* 126: [reserved]											*/
	u16 rm_notification_supported;	/* 127: Remov. media status notif'n feature set supported	*/
	u16 security_status;			/* 128: Security status										*/
	u16 _word_129;					/* 129: [vendor-specific]									*/
	u16 _word_130;					/* 130: [vendor-specific]									*/
	u16 _word_131;					/* 131: [vendor-specific]									*/
	u16 _word_132;					/* 132: [vendor-specific]									*/
	u16 _word_133;					/* 133: [vendor-specific]									*/
	u16 _word_134;					/* 134: [vendor-specific]									*/
	u16 _word_135;					/* 135: [vendor-specific]									*/
	u16 _word_136;					/* 136: [vendor-specific]									*/
	u16 _word_137;					/* 137: [vendor-specific]									*/
	u16 _word_138;					/* 138: [vendor-specific]									*/
	u16 _word_139;					/* 139: [vendor-specific]									*/
	u16 _word_140;					/* 140: [vendor-specific]									*/
	u16 _word_141;					/* 141: [vendor-specific]									*/
	u16 _word_142;					/* 142: [vendor-specific]									*/
	u16 _word_143;					/* 143: [vendor-specific]									*/
	u16 _word_144;					/* 144: [vendor-specific]									*/
	u16 _word_145;					/* 145: [vendor-specific]									*/
	u16 _word_146;					/* 146: [vendor-specific]									*/
	u16 _word_147;					/* 147: [vendor-specific]									*/
	u16 _word_148;					/* 148: [vendor-specific]									*/
	u16 _word_149;					/* 149: [vendor-specific]									*/
	u16 _word_150;					/* 150: [vendor-specific]									*/
	u16 _word_151;					/* 151: [vendor-specific]									*/
	u16 _word_152;					/* 152: [vendor-specific]									*/
	u16 _word_153;					/* 153: [vendor-specific]									*/
	u16 _word_154;					/* 154: [vendor-specific]									*/
	u16 _word_155;					/* 155: [vendor-specific]									*/
	u16 _word_156;					/* 156: [vendor-specific]									*/
	u16 _word_157;					/* 157: [vendor-specific]									*/
	u16 _word_158;					/* 158: [vendor-specific]									*/
	u16 _word_159;					/* 159: [vendor-specific]									*/
	u16 cfa_power_mode_1;			/* 160: CFA power mode 1									*/
	u16 _word_161;					/* 161: [reserved]											*/
	u16 _word_162;					/* 162: [reserved]											*/
	u16 _word_163;					/* 163: [reserved]											*/
	u16 _word_164;					/* 164: [reserved]											*/
	u16 _word_165;					/* 165: [reserved]											*/
	u16 _word_166;					/* 166: [reserved]											*/
	u16 _word_167;					/* 167: [reserved]											*/
	u16 _word_168;					/* 168: [reserved]											*/
	u16 _word_169;					/* 169: [reserved]											*/
	u16 _word_170;					/* 170: [reserved]											*/
	u16 _word_171;					/* 171: [reserved]											*/
	u16 _word_172;					/* 172: [reserved]											*/
	u16 _word_173;					/* 173: [reserved]											*/
	u16 _word_174;					/* 174: [reserved]											*/
	u16 _word_175;					/* 175: [reserved]											*/
	u16 _word_176;					/* 176: [reserved]											*/
	u16 _word_177;					/* 177: [reserved]											*/
	u16 _word_178;					/* 178: [reserved]											*/
	u16 _word_179;					/* 179: [reserved]											*/
	u16 _word_180;					/* 180: [reserved]											*/
	u16 _word_181;					/* 181: [reserved]											*/
	u16 _word_182;					/* 182: [reserved]											*/
	u16 _word_183;					/* 183: [reserved]											*/
	u16 _word_184;					/* 184: [reserved]											*/
	u16 _word_185;					/* 185: [reserved]											*/
	u16 _word_186;					/* 186: [reserved]											*/
	u16 _word_187;					/* 187: [reserved]											*/
	u16 _word_188;					/* 188: [reserved]											*/
	u16 _word_189;					/* 189: [reserved]											*/
	u16 _word_190;					/* 190: [reserved]											*/
	u16 _word_191;					/* 191: [reserved]											*/
	u16 _word_192;					/* 192: [reserved]											*/
	u16 _word_193;					/* 193: [reserved]											*/
	u16 _word_194;					/* 194: [reserved]											*/
	u16 _word_195;					/* 195: [reserved]											*/
	u16 _word_196;					/* 196: [reserved]											*/
	u16 _word_197;					/* 197: [reserved]											*/
	u16 _word_198;					/* 198: [reserved]											*/
	u16 _word_199;					/* 199: [reserved]											*/
	u16 _word_200;					/* 200: [reserved]											*/
	u16 _word_201;					/* 201: [reserved]											*/
	u16 _word_202;					/* 202: [reserved]											*/
	u16 _word_203;					/* 203: [reserved]											*/
	u16 _word_204;					/* 204: [reserved]											*/
	u16 _word_205;					/* 205: [reserved]											*/
	u16 _word_206;					/* 206: [reserved]											*/
	u16 _word_207;					/* 207: [reserved]											*/
	u16 _word_208;					/* 208: [reserved]											*/
	u16 _word_209;					/* 209: [reserved]											*/
	u16 _word_210;					/* 210: [reserved]											*/
	u16 _word_211;					/* 211: [reserved]											*/
	u16 _word_212;					/* 212: [reserved]											*/
	u16 _word_213;					/* 213: [reserved]											*/
	u16 _word_214;					/* 214: [reserved]											*/
	u16 _word_215;					/* 215: [reserved]											*/
	u16 _word_216;					/* 216: [reserved]											*/
	u16 _word_217;					/* 217: [reserved]											*/
	u16 _word_218;					/* 218: [reserved]											*/
	u16 _word_219;					/* 219: [reserved]											*/
	u16 _word_220;					/* 220: [reserved]											*/
	u16 _word_221;					/* 221: [reserved]											*/
	u16 _word_222;					/* 222: [reserved]											*/
	u16 _word_223;					/* 223: [reserved]											*/
	u16 _word_224;					/* 224: [reserved]											*/
	u16 _word_225;					/* 225: [reserved]											*/
	u16 _word_226;					/* 226: [reserved]											*/
	u16 _word_227;					/* 227: [reserved]											*/
	u16 _word_228;					/* 228: [reserved]											*/
	u16 _word_229;					/* 229: [reserved]											*/
	u16 _word_230;					/* 230: [reserved]											*/
	u16 _word_231;					/* 231: [reserved]											*/
	u16 _word_232;					/* 232: [reserved]											*/
	u16 _word_233;					/* 233: [reserved]											*/
	u16 _word_234;					/* 234: [reserved]											*/
	u16 _word_235;					/* 235: [reserved]											*/
	u16 _word_236;					/* 236: [reserved]											*/
	u16 _word_237;					/* 237: [reserved]											*/
	u16 _word_238;					/* 238: [reserved]											*/
	u16 _word_239;					/* 239: [reserved]											*/
	u16 _word_240;					/* 240: [reserved]											*/
	u16 _word_241;					/* 241: [reserved]											*/
	u16 _word_242;					/* 242: [reserved]											*/
	u16 _word_243;					/* 243: [reserved]											*/
	u16 _word_244;					/* 244: [reserved]											*/
	u16 _word_245;					/* 245: [reserved]											*/
	u16 _word_246;					/* 246: [reserved]											*/
	u16 _word_247;					/* 247: [reserved]											*/
	u16 _word_248;					/* 248: [reserved]											*/
	u16 _word_249;					/* 249: [reserved]											*/
	u16 _word_250;					/* 250: [reserved]											*/
	u16 _word_251;					/* 251: [reserved]											*/
	u16 _word_252;					/* 252: [reserved]											*/
	u16 _word_253;					/* 253: [reserved]											*/
	u16 _word_254;					/* 254: [reserved]											*/
	u16 integrity_word;				/* 255: Integrity word										*/
} __attribute__((packed));

typedef struct ata_identify_device_ret ata_identify_device_ret_t;


struct ata_address
{
	u32 bus;
	u32 device;
};

enum ata_device_status
{
	offline = 0, timeout, unsupported, failed, online
};

/*

*/
struct ata_device
{
	struct ata_address addr;

	char model[48];
	char serial[24];
	char firmware[12];

	u32 num_sectors;

	enum ata_device_status status;
};


typedef struct ata_device ata_device_t;


/*
	Internal functions (called by "driver" functions)
*/
ata_ret_t ata_send_command(const u32 devid, const ata_command_t cmd);

void ata_read_data(ku32 bus, void *buf);
void ata_write_data(ku32 bus, const void *buf);

#endif

