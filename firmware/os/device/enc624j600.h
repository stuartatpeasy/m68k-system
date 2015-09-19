#ifndef DEVICE_ENC624J600_H_INC
#define DEVICE_ENC624J600_H_INC
/*
    Definitions relating to the Microchip ENC624J600 Ethernet controller

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/


#define ENCX624_SFR_BASE        (0x7e00)	/* Base address of special-function registers		*/
#define ENCX624_SFR_SHIFT		(1)			/* Regs are 16 bit, so regnum must be <<'ed by 1	*/

/* This macro provides the offset of register x from the start of the controller's memory map */
#define ENCX624_SFR_REG(x)		(ENCX624_SFR_BASE + (((u32) (x)) << ENCX624_SFR_SHIFT))

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
    ETXST       = 0x00,
    ETXLEN      = 0x01,
    ERXST       = 0x02,
    ERXTAIL     = 0x03,
    ERXHEAD     = 0x04,
    EDMAST      = 0x05,
    EDMALEN     = 0x06,
    EDMADST     = 0x07,
    EDMACS      = 0x08,
    ETXSTAT     = 0x09,
    ETXWIRE     = 0x0a,
    EUDAST      = 0x0b,
    EUDAND      = 0x0c,
    ESTAT       = 0x0d,
    EIR         = 0x0e,
    ECON1       = 0x0f,
    EHT1        = 0x10,
    EHT2        = 0x11,
    EHT3        = 0x12,
    EHT4        = 0x13,
    EPMM1       = 0x14,
    EPMM2       = 0x15,
    EPMM3       = 0x16,
    EPMM4       = 0x17,
    EPMCS       = 0x18,
    EPMO        = 0x19,
    ERXFCON     = 0x1a,

    MACON1      = 0x20,
    MACON2      = 0x21,
    MABBIPG     = 0x22,
    MAIPG       = 0x23,
    MACLCON     = 0x24,
    MAMXFL      = 0x25,

    MICMD       = 0x29,   /* MII management command   */
    MIREGADR    = 0x2a,   /* MII management address   */

    MAADR3      = 0x30,
    MAADR2      = 0x31,
    MAADR1      = 0x32,
    MIWR        = 0x33,
    MIRD        = 0x34,
    MISTAT      = 0x35,   /* MII management status    */
    EPAUS       = 0x36,
    ECON2       = 0x37,
    ERXWM       = 0x38,
    EIE         = 0x39,
    EIDLED      = 0x3a,

    EGPDATA     = 0x40,
    ERXDATA     = 0x41,
    EUDADATA    = 0x42,
    EGPRDPT     = 0x43,
    EGPWRPT     = 0x44,
    ERXRDPT     = 0x45,
    ERXWRPT     = 0x46,
    EUDARDPT    = 0x47,
    EUDAWRPT    = 0x48,

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
    PHCON1      = 0x00,
    PHSTAT1     = 0x01,

    PHANA       = 0x04,
    PHANLPA     = 0x05,
    PHANE       = 0x06,

    PHCON2      = 0x11,

    PHSTAT2     = 0x1b,
    PHSTAT3     = 0x1f
};

#endif
