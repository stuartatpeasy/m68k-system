#ifndef DEVICE_BLOCK_ATA_ATA_H_INC
#define DEVICE_BLOCK_ATA_ATA_H_INC
/*
	AT Attachment (ATA) interface driver function and macro declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include <device/block.h>
#include <device/device.h>
#include <include/types.h>


typedef enum ata_drive
{
    ata_drive_master    = 0,
    ata_drive_slave     = 1
} ata_drive_t;


typedef struct ata_dev_data
{
    ata_drive_t drive;

	char model[48];
	char serial[24];
	char firmware[12];

	u32 num_sectors;
} ata_dev_data_t;


/*
	"Driver" functions
*/

s32 ata_init(dev_t *dev);
s32 ata_bus_shut_down(dev_t *dev);

s32 ata_master_init(dev_t * dev);
s32 ata_slave_init(dev_t * dev);

s32 ata_read(dev_t *dev, ku32 offset, u32 len, void * buf);
s32 ata_write(dev_t *dev, ku32 offset, u32 len, const void * buf);

blockdev_stats_t g_ata_stats;


#define ATA_SECTOR_SIZE		(512)		/* Sector size - constant for all ATA devices	*/
#define ATA_LOG_SECTOR_SIZE	(9)			/* = log2(ATA_SECTOR_SIZE)						*/

#define ATA_TIMEOUT_VAL		(1000000)	/* Timeout ctr initial val.  TODO: improve this	*/


/*
	Macros which wait for the ATA device's BSY flag to be asserted (ATA_WAIT_BSY()) or negated
	(ATA_WAIT_NBSY).
*/
#define ATA_WAIT_BSY(base_addr)													\
    (__extension__ ({														    \
        u32 t_ = ATA_TIMEOUT_VAL;											    \
        for(; !(ATA_REG(base_addr, ATA_R_STATUS) & ATA_STATUS_BSY) && --t_;)	\
            ;	                                                                \
        t_;																        \
    }))

#define ATA_WAIT_NBSY(base_addr)												\
    (__extension__ ({														    \
        u32 t_ = ATA_TIMEOUT_VAL;											    \
        for(; (ATA_REG(base_addr, ATA_R_STATUS) & ATA_STATUS_BSY) && --t_;)		\
            ;	                                                                \
        t_;																        \
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


#define ATA_REG_DATA(base_addr)    \
 	*((volatile u16 *) ((u16 *) ((base_addr) + ATA_R_DATA)))

#define ATA_REG(base_addr, off)	\
    *((volatile u8 *) ((base_addr) + off))


/*
	ATA command codes
*/

typedef enum ata_command
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
} ata_command_t;


/*
	ATA register bits
*/

/* Status register																				*/
#define ATA_STATUS_BSY				BIT(7)
#define ATA_STATUS_DRDY				BIT(6)
#define ATA_STATUS_DRQ				BIT(3)
#define ATA_STATUS_ERR				BIT(0)

/* Device control register																		*/
#define ATA_DEVICE_CONTROL_SRST		BIT(2)
#define ATA_DEVICE_CONTROL_NIEN		BIT(1)

/* Device/head register																			*/
#define ATA_DH_DEV					BIT(3)
#define ATA_DH_LBA					BIT(6)

/* Error register																				*/
#define ATA_ERROR_MED				BIT(0)		/* Media error									*/
#define ATA_ERROR_NM				BIT(1)		/* No media										*/
#define ATA_ERROR_ABRT				BIT(2)		/* Abort										*/
#define ATA_ERROR_MCR				BIT(3)		/* Media change request							*/
#define ATA_ERROR_IDNF				BIT(4)		/* ID not found (no device / bad sector num)	*/
#define ATA_ERROR_MC				BIT(5)		/* Media changed								*/
#define ATA_ERROR_WP				BIT(6)		/* Write protect enabled						*/
#define ATA_ERROR_UNC				BIT(6)		/* Uncorrectable data error						*/
#define ATA_ERROR_ICRC				BIT(7)		/* Interface CRC error							*/


/*
	struct ata_identify_device: 512-byte struct which may be mapped over the response to an
	IDENTIFY DEVICE command.
*/
typedef struct ata_identify_device_ret
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
	u16 _reserved_1[6];             /*  69-74 [reserved]                                        */
	u16 queue_depth;				/*  75: Maximum queue depth - 1								*/
	u16 _reserved_2[4];             /*  76-79: [reserved]                                       */
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
	u16 _reserved_3[33];            /*  94-126: [reserved]                                      */
	u16 rm_notification_supported;	/* 127: Remov. media status notif'n feature set supported	*/
	u16 security_status;			/* 128: Security status										*/
	u16 _vendor_specific_1[31];     /* 129-159: [vendor-specific]                               */
	u16 cfa_power_mode_1;			/* 160: CFA power mode 1									*/
	u16 _reserved_4[94];            /* 161-254: [reserved]                                      */
	u16 integrity_word;				/* 255: Integrity word										*/
} __attribute__((packed)) ata_identify_device_ret_t;


#endif

