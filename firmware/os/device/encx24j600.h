#ifndef DEVICE_ENC624J600_H_INC
#define DEVICE_ENC624J600_H_INC
/*
    Definitions relating to the Microchip ENCx24J600 Ethernet controller

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.

    This device has driver ID 0x81.
*/

#include <cpu/cpu.h>
#include <device/device.h>
#include <include/byteorder.h>
#include <include/defs.h>
#include <include/error.h>
#include <include/types.h>


#define ENCX24_SFR_BASE         (0x7e00)	/* Base address of special-function registers		*/
#define ENCX24_MEM_TOP          (0x6000)    /* End of RAM area + 1                              */
#define ENCX24_SFR_SHIFT		(1)			/* Regs are 16 bit, so regnum must be <<'ed by 1	*/

/* This macro provides the offset of register x from the start of the controller's memory map */
#define ENCX24_SFR_OFFSET(x)	    (ENCX24_SFR_BASE + (((u32) (x)) << ENCX24_SFR_SHIFT))

#define ENCX24_SFR_ADDR(base, x)    (((u32) base) + ENCX24_SFR_OFFSET(x))

/* This macro is an accessor for register x, given a controller at address "base" */
#define ENCX24_REG(base, x)         *((vu16 *) ENCX24_SFR_ADDR((base), (x)))

/*
    This macro is an accessor for the memory buffer in a controller.  Note: reads and writes must be
    aligned on two-byte boundaries, i.e. "x" must be even.
*/
#define ENCX24_MEM_ADDR(base, x)     ((vu16 *) ((u8 *) (base) + (x)))


/* Ethernet controller state structure */
typedef struct encx24j600_state
{
    u16     rx_buf_start;
    u16     rx_read_ptr;
} encx24j600_state_t;

/* Header added to incoming packets by the ENCx24J600 */
typedef struct encx24j600_rxhdr
{
    u16     next_packet_ptr;

    /* "RSV" - receive status vector */
    struct
    {
        u8      zero;               /* All bits set to zero                 */
        u8      rsv4;               /* } Packet type / filter match flags   */
        u8      rsv3;               /* }                                    */
        u8      status;             /* Packet status information            */
        u16     byte_count;         /* Number of bytes received             */
    } rsv;
} encx24j600_rxhdr_t;


s32 encx24j600_reset(dev_t *dev);
s32 encx24j600_init(dev_t *dev);
s32 encx24j600_shut_down(dev_t *dev);
s32 encx24j600_read(dev_t *dev);
s32 encx24j600_write(dev_t *dev);
s32 encx24j600_control(dev_t *dev);
void encx24j600_irq(ku32 irql, void *data);
void encx24j600_rx_buf_read(dev_t *dev, u16 len, void *out);


/*
    Cryptographic data memory map
*/

/* === Modular exponentiation === */
#define ENCX24_CRYPT_MOD_E_START        (0x7800)
#define ENCX24_CRYPT_MOD_E_LEN          (0x80)

#define ENCX24_CRYPT_MOD_DATA_START     (0x7880)
#define ENCX24_CRYPT_MOD_DATA_LEN       (0x80)

#define ENCX24_CRYPT_MOD_M_START        (0x7900)
#define ENCX24_CRYPT_MOD_M_LEN          (0x80)

/* === MD5/SHA-1 hashing === */
#define ENCX24_CRYPT_SHA_DATA_START     (0x7a00)
#define ENCX24_CRYPT_SHA_DATA_LEN       (0x40)

#define ENCX24_CRYPT_SHA_INITVEC_START  (0x7a40)
#define ENCX24_CRYPT_SHA_INITVEC_LEN    (0x14)

#define ENCX24_CRYPT_SHA_LSTATE_START   (0x7a54)
#define ENCX24_CRYPT_SHA_LSTATE_LEN     (0x8)

/* === AES === */
#define ENCX24_CRYPT_AES_KEY_START      (0x7c00)
#define ENCX24_CRYPT_AES_KEY_LEN        (0x20)

#define ENCX24_CRYPT_AES_TXTA_START     (0x7c20)
#define ENCX24_CRYPT_AES_TXTA_LEN       (0x10)

#define ENCX24_CRYPT_AES_TXTB_START     (0x7c30)
#define ENCX24_CRYPT_AES_TXTB_LEN       (0x10)

/*
    Special-function register (SFR) addresses (16-bit PSP interface)
*/
enum ENC624J600_SFR
{
/* === Special-function register (SFR) addresses === */
    ETXST       = 0x00,     /* TX buffer start address                  */
    ETXLEN      = 0x01,     /* TX buffer length                         */
    ERXST       = 0x02,     /* RX buffer start address                  */
    ERXTAIL     = 0x03,     /* RX buffer tail pointer                   */
    ERXHEAD     = 0x04,     /* RX buffer head pointer                   */
    EDMAST      = 0x05,     /* DMA start address                        */
    EDMALEN     = 0x06,     /* DMA length                               */
    EDMADST     = 0x07,     /* DMA destination address                  */
    EDMACS      = 0x08,     /* DMA checksum                             */
    ETXSTAT     = 0x09,     /* Ethernet transmit status                 */
    ETXWIRE     = 0x0a,     /* Ethernet transmit byte count on wire     */
    EUDAST      = 0x0b,     /* User-defined area start pointer          */
    EUDAND      = 0x0c,     /* User-defined area end pointer            */
    ESTAT       = 0x0d,     /* Ethernet status                          */
    EIR         = 0x0e,     /* Ethernet interrupt flags                 */
    ECON1       = 0x0f,     /* Ethernet control register 1              */
    EHT1        = 0x10,     /* Hash table filter, first word            */
    EHT2        = 0x11,     /* Hash table filter, second word           */
    EHT3        = 0x12,     /* Hash table filter, third word            */
    EHT4        = 0x13,     /* Hash table filter, fourth word           */
    EPMM1       = 0x14,     /* Pattern match filter mask, first word    */
    EPMM2       = 0x15,     /* Pattern match filter mask, second word   */
    EPMM3       = 0x16,     /* Pattern match filter mask, third word    */
    EPMM4       = 0x17,     /* Pattern match filter mask, fourth word   */
    EPMCS       = 0x18,     /* Pattern match filter checksum            */
    EPMO        = 0x19,     /* Pattern match filter offset              */
    ERXFCON     = 0x1a,     /* RX filter control                        */

    MACON1      = 0x20,     /* MAC control register 1                   */
    MACON2      = 0x21,     /* MAC control register 2                   */
    MABBIPG     = 0x22,     /* MAC back-to-back inter-packet gap        */
    MAIPG       = 0x23,     /* MAC inter-packet gap                     */
    MACLCON     = 0x24,     /* MAC collision control                    */
    MAMXFL      = 0x25,     /* MAC maximum frame length                 */

    MICMD       = 0x29,     /* MII management command                   */
    MIREGADR    = 0x2a,     /* MII management address                   */

    MAADR3      = 0x30,     /* MAC address, third word                  */
    MAADR2      = 0x31,     /* MAC address, second word                 */
    MAADR1      = 0x32,     /* MAC address, first word                  */
    MIWR        = 0x33,
    MIRD        = 0x34,
    MISTAT      = 0x35,     /* MII management status                    */
    EPAUS       = 0x36,     /* Pause timer value                        */
    ECON2       = 0x37,     /* Ethernet control register 2              */
    ERXWM       = 0x38,
    EIE         = 0x39,     /* Ethernet interrupt enable                */
    EIDLED      = 0x3a,     /* Ethernet ID status / LED control         */

    EGPDATA     = 0x40,     /* General-purpose data window register     */
    ERXDATA     = 0x41,     /* RX data window register                  */
    EUDADATA    = 0x42,     /* User data area window register           */
    EGPRDPT     = 0x43,     /* General-purpose window read pointer      */
    EGPWRPT     = 0x44,     /* General-purpose window write pointer     */
    ERXRDPT     = 0x45,     /* RX window read pointer                   */
    ERXWRPT     = 0x46,     /* RX window write pointer                  */
    EUDARDPT    = 0x47,     /* User data area read pointer              */
    EUDAWRPT    = 0x48,     /* USer data area write pointer             */

/* === SFR "bit set" register addresses === */
    ETXSTSET    = 0x80,
    ETXLENSET   = 0x81,
    ERXSTSET    = 0x82,
    ERXTAILSET  = 0x83,

    EDMASTSET   = 0x85,
    EDMALENSET  = 0x86,
    EDMADSTSET  = 0x87,
    EDMACSSET   = 0x88,

    EUDASTSET   = 0x8b,
    EUDANDSET   = 0x8c,

    EIRSET      = 0x8e,
    ECON1SET    = 0x8f,
    EHT1SET     = 0x90,
    EHT2SET     = 0x91,
    EHT3SET     = 0x92,
    EHT4SET     = 0x93,
    EPMM1SET    = 0x94,
    EPMM2SET    = 0x95,
    EPMM3SET    = 0x96,
    EPMM4SET    = 0x97,
    EPMCSSET    = 0x98,
    EPMOSET     = 0x99,
    ERXFCONSET  = 0x9a,

    EPAUSSET    = 0xb6,
    ECON2SET    = 0xb7,
    ERXWMSET    = 0xb8,
    EIESET      = 0xb9,
    EIDLEDSET   = 0xba,

/* === SFR "bit clear" register addresses === */
    ETXSTCLR    = 0xc0,
    ETXLENCLR   = 0xc1,
    ERXSTCLR    = 0xc2,
    ERXTAILCLR  = 0xc3,

    EDMASTCLR   = 0xc5,
    EDMALENCLR  = 0xc6,
    EDMADSTCLR  = 0xc7,
    EDMACSCLR   = 0xc8,

    EUDASTCLR   = 0xcb,
    EUDANDCLR   = 0xcc,

    EIRCLR      = 0xce,
    ECON1CLR    = 0xcf,
    EHT1CLR     = 0xd0,
    EHT2CLR     = 0xd1,
    EHT3CLR     = 0xd2,
    EHT4CLR     = 0xd3,
    EPMM1CLR    = 0xd4,
    EPMM2CLR    = 0xd5,
    EPMM3CLR    = 0xd6,
    EPMM4CLR    = 0xd7,
    EPMCSCLR    = 0xd8,
    EPMOCLR     = 0xd9,
    ERXFCONCLR  = 0xda,

    EPAUSCLR    = 0xf6,
    ECON2CLR    = 0xf7,
    ERXWMCLR    = 0xf8,
    EIECLR      = 0xf9,
    EIDLEDCLR   = 0xfa,
};


/*
    PHY Special-function register (SFR) addresses
*/
enum ENC624J600_PHYReg
{
    PHCON1      = 0x00,     /* PHY control register 1                       */
    PHSTAT1     = 0x01,     /* PHY status register 1                        */

    PHANA       = 0x04,     /* PHY auto-negotiation advertisement register  */
    PHANLPA     = 0x05,     /* PHY auto-negotiation link partner ability    */
    PHANE       = 0x06,     /* PHY auto-negotiation expansion register      */

    PHCON2      = 0x11,     /* PHY control register 2                       */

    PHSTAT2     = 0x1b,     /* PHY status register 2                        */
    PHSTAT3     = 0x1f      /* PHY status register 3                        */
};

/*
    Individual register bits

    NOTE: these bit-offsets assume that the halves of the data bus between the ENCx24J600 and
    the host CPU are swapped, i.e.:
        CPU_D[15..8]  ->  ENC_D[7..0]
        CPU_D[7..0]   ->  ENC_D[15..8]
    They are therefore not the same as the bit-offsets listed in the ENCx24J600 documentation.
*/

/* ECON1 register */
#define ECON1_MODEXST       (7)         /* Modular exponentiation start             (RW-0) */
#define ECON1_HASHEN        (6)         /* MD5/SHA-1 hash enable                    (RW-0) */
#define ECON1_HASHOP        (5)         /* MD5/SHA-1 hash operation control         (RW-0) */
#define ECON1_HASHLST       (4)         /* MD5/SHA-1 hash last block control        (RW-0) */
#define ECON1_AESST         (3)         /* AES encrypt/decrypt start                (RW-0) */
#define ECON1_AESOP1        (2)         /* AES operation control bit 1              (RW-0) */
#define ECON1_AESOP0        (1)         /* AES operation control bit 0              (RW-0) */
#define ECON1_PKTDEC        (0)         /* RX packet counter decrement control      (RW-0) */
#define ECON1_FCOP1         (15)        /* Flow control operation ctrl/status bit 1 (RW-0) */
#define ECON1_FCOP0         (14)        /* Flow control operation ctrl/status bit 0 (RW-0) */
#define ECON1_DMAST         (13)        /* DMA start                                (RW-0) */
#define ECON1_DMACPY        (12)        /* DMA copy control                         (RW-0) */
#define ECON1_DMACSSD       (11)        /* DMA checksum seed control                (RW-0) */
#define ECON1_DMANOCS       (10)        /* DMA no checksum control                  (RW-0) */
#define ECON1_TXRTS         (9)         /* Transmit RTS ctrl/status bit             (RW-0) */
#define ECON1_RXEN          (8)         /* RX enable                                (RW-0) */

/* ECON2 register */
#define ECON2_ETHEN         (7)        /* Ethernet enable                          (RW-1) */
#define ECON2_STRCH         (6)        /* LED stretching enable                    (RW-1) */
#define ECON2_TXMAC         (5)        /* Automatically transmit MAC address       (RW-0) */
#define ECON2_SHA1MD5       (4)        /* SHA-1/MD5 hash control                   (RW-0) */
#define ECON2_AUTOFC        (15)         /* Automatic flow control enable            (RW-0) */
#define ECON2_TXRST         (14)         /* Transmit logic reset                     (RW-0) */
#define ECON2_RXRST         (13)         /* Receive logic reset                      (RW-0) */
#define ECON2_ETHRST        (12)         /* Master Ethernet reset                    (RW-0) */

#define ECON2_COCON_MASK    (0x000f)    /* CLKOUT frequency control bits mask    (RW-1011) */
#define ECON2_COCON_SHIFT   (0)

#define ECON2_MODLEN_MASK   (0x0c00)    /* Modular exponentiation length mask      (RW-00) */
#define ECON2_MODLEN_SHIFT  (10)

#define ECON2_AESLEN_MASK   (0x0300)    /* AES key length mask                     (RW-00) */
#define ECON2_AESLEN_SHIFT  (8)

/* EIDLED register */
#define EIDLED_LACFG_MASK   (0x00f0)    /* LED A configuration mask              (RW-0010) */
#define EIDLED_LACFG_SHIFT  (4)
#define EIDLED_LBCFG_MASK   (0x000f)    /* LED B configuration mask              (RW-0110) */
#define EIDLED_LBCFG_SHIFT  (0)
#define EIDLED_DEVID_MASK   (0xe000)    /* Device ID mask                         (RW-001) */
#define EIDLED_DEVID_SHIFT  (13)
#define EIDLED_REVID_MASK   (0x1f00)    /* Silicon revision ID mask                        */
#define EIDLED_REVID_SHIFT  (8)

/* These constants define the events to be displayed by the LEDs attached to the controller */
#define EIDLED_LSTR         (15)        /* Link + speed + TX + RX                          */
#define EIDLED_LDTR         (14)        /* Link + duplex + TX + RX                         */
/*                          (13)           (reserved)                                      */
#define EIDLED_LC           (12)        /* Link + collisions                               */
#define EIDLED_LTR          (11)        /* Link + TX + RX                                  */
#define EIDLED_LR           (10)        /* Link + RX                                       */
#define EIDLED_LT           (9)         /* Link + TX                                       */
#define EIDLED_LS           (8)         /* Link + speed                                    */
#define EIDLED_LD           (7)         /* Link + duplex                                   */
#define EIDLED_TR           (6)         /* TX + RX                                         */
#define EIDLED_R            (5)         /* RX                                              */
#define EIDLED_T            (4)         /* TX                                              */
#define EIDLED_C            (3)         /* Collisions                                      */
#define EIDLED_L            (2)         /* Link                                            */
#define EIDLED_ON           (1)         /* LED permanently on                              */
#define EIDLED_OFF          (0)         /* LED permanently off                             */

/* EIE: Ethernet interrupt enable register */
#define EIE_INTIE           (7)         /* Global interrupt enable                  (RW-1) */
#define EIE_MODEXIE         (6)         /* Modular exponentiation interrupt enable  (RW-0) */
#define EIE_HASHIE          (5)         /* MD5/SHA-1 hash interrupt enable          (RW-0) */
#define EIE_AESIE           (4)         /* AES encrypt/decrypt interrupt enable     (RW-0) */
#define EIE_LINKIE          (3)         /* PHY link status change interrupt enable  (RW-0) */
/*                          (2)            (reserved - write as 0)                         */
/*                          (1)            (reserved - write as 0)                         */
/*                          (0)            (reserved - write as 0)                         */
/*                          (15)           (reserved - write as 0)                         */
#define EIE_PKTIE           (14)        /* RX packet pending interrupt enable       (RW-0) */
#define EIE_DMAIE           (13)        /* DMA interrupt enable                     (RW-0) */
/*                          (12)           (reserved - don't care on write, read 0)        */
#define EIE_TXIE            (11)        /* Transmit done interrupt enable           (RW-0) */
#define EIE_TXABTIE         (10)        /* Transmit abort interrupt enable          (RW-0) */
#define EIE_RXABTIE         (9)         /* Receive abort interrupt enable           (RW-0) */
#define EIE_PCFULIE         (8)         /* Packet counter full interrupt enable     (RW-0) */

/* EIR: Ethernet interrupt flag register */
#define EIR_CRYPTEN         (7)         /* Modular exp'n & AES crypto module enable (RW-0) */
#define EIR_MODEXIF         (6)         /* Modular exponentiation interrupt flag    (RW-0) */
#define EIR_HASHIF          (5)         /* MD5/SHA-1 hash interrupt flag            (RW-0) */
#define EIR_AESIF           (4)         /* AES encrypt/decrypt interrupt flag       (RW-0) */
#define EIR_LINKIF          (3)         /* PHY link status change interrupt flag    (RW-1) */
/*                          (2)            (reserved; read: ignore, write: don't care)     */
/*                          (1)            (reserved; read: ignore, write: don't care)     */
/*                          (0)            (reserved; read: ignore, write: don't care)     */
/*                          (15)           (reserved; read: ignore, write: don't care)     */
#define EIR_PKTIF           (14)        /* RX packet pending interrupt flag         (R-0)  */
#define EIR_DMAIF           (13)        /* DMA copy/cksum complete interrupt flag   (RW-0) */
/*                          (12)           (reserved; read: ignore, write: don't care)     */
#define EIR_TXIF            (11)        /* Transmit done interrupt flag             (RW-0) */
#define EIR_TXABTIF         (10)        /* Transmit abort interrupt flag            (RW-0) */
#define EIR_RXABTIF         (9)         /* Receive abort interrupt flag             (RW-0) */
#define EIR_PCFULIF         (8)         /* Packet counter full interrupt flag       (RW-0) */

/* ESTAT: Ethernet status register */
#define ESTAT_INT           (7)
#define ESTAT_FCIDLE        (6)
#define ESTAT_RXBUSY        (5)
#define ESTAT_CLKRDY        (4)
#define ESTAT_PHYDPX        (3)
#define ESTAT_PHYLINK       (2)
#define ESTAT_PKTCNT_MASK   (0xff00)

/* ERXFCON register */
#define ERXFCON_HTEN        (7)         /* Hash table collection filter enable      (RW-0) */
#define ERXFCON_MPEN        (6)         /* Magic packet collection filter enable    (RW-0) */
/*                          (5)            (unimplemented - read as 0)                     */
#define ERXFCON_NOTPM       (4)         /* Pattern match inversion control          (RW-0) */

#define ERXFCON_PMEN_MASK   (0x000f)    /* Pattern match filter enable bits      (RW-0000) */
#define ERXFCON_PMEN_SHIFT  (0)

#define ERXFCON_CRCEEN      (15)        /* CRC error collection filter enable       (RW-0) */
#define ERXFCON_CRCEN       (14)        /* CRC error rejection filter enable        (RW-1) */
#define ERXFCON_RUNTEEN     (13)        /* Runt error collection filter enable      (RW-0) */
#define ERXFCON_RUNTEN      (12)        /* Runt error rejection filter enable       (RW-1) */
#define ERXFCON_UCEN        (11)        /* Unicast dest. collection filter enable   (RW-1) */
#define ERXFCON_NOTMEEN     (10)        /* Not-Me unicast dest. coll. filter enable (RW-0) */
#define ERXFCON_MCEN        (9)         /* Multicast dest. coll. filter enable      (RW-0) */
#define ERXFCON_BCEN        (8)         /* Broadcast dest. coll. filter enable      (RW-1) */

/* MACON2 register */
/*                          (7)            (unimplemented - read as 0)                     */
#define MACON2_DEFER        (6)         /* Defer transmission enable                (RW-1) */
#define MACON2_BPEN         (5)         /* No back-off during back pressure         (RW-0) */
#define MACON2_NOBKOFF      (4)         /* No back-off enable                       (RW-0) */
/*                          (3)            (unimplemented - read as 0)                     */
/*                          (2)            (unimplemented - read as 0)                     */
/*                          (1)            (reserved - write as 0)                         */
/*                          (0)            (reserved - write as 0)                         */

#define MACON2_PADCFG_MASK  (0xe000)    /* Automatic pad and CRC configuration    (RW-101) */
#define MACON2_PADCFG_SHIFT (13)

#define MACON2_TXCRCEN      (12)        /* Transmit CRC enable                      (RW-1) */
#define MACON2_PHDREN       (11)        /* Proprietary header enable                (RW-0) */
#define MACON2_HFRMEN       (10)        /* Huge frame enable                        (RW-0) */
/*                          (9)            (reserved - write as 0)                         */
#define MACON2_FULDPX       (8)         /* Full duplex mode enable                  (RW-0) */


/* RSV (receive status vector) "status" byte field definitions */
#define ENCX24_RSV_STAT_OK          (7)     /* Packet received successfully                */
#define ENCX24_RSV_STAT_TOO_LONG    (6)     /* Packet length field indicates >1500 bytes   */
#define ENCX24_RSV_STAT_LEN_ERR     (5)     /* Length field value != actual frame length   */
#define ENCX24_RSV_STAT_CRC_ERR     (4)     /* CRC error                                   */
/*                                  (3)        (reserved)                                  */
#define ENCX24_RSV_STAT_CEPS        (2)     /* Carrier event previously seen               */
/*                                  (1)        (reserved)                                  */
#define ENCX24_RSV_STAT_PPI         (0)     /* Packet previously ignored                   */

#endif
