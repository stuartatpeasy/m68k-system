#ifndef KERNEL_INCLUDE_KEYBOARD_H_INC
#define KERNEL_INCLUDE_KEYBOARD_H_INC
/*
    Keyboard-related declarations, including the KEY_* internal keystroke constants.

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2016.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>


/*
    Size of an array needed to hold a full mapping of internal key codes to characters, rounded up
    to the nearest power of two.
*/
#define KEYMAP_SIZE         (128)

/* The default keymap */
#define KEYMAP_DEFAULT      (&km_en_GB_ISO_8859_1)


/*
    keymap: struct mapping internal key codes to characters in a particular character set.  Two
    mappings are stored: "normal" (i.e. unshifted) mappings, and "shifted" (i.e. shift key pressed)
    mappings.
*/
typedef struct
{
    u8 normal[KEYMAP_SIZE];
    u8 shifted[KEYMAP_SIZE];
} keymap;


/* intl_keymap: associates a country code with a particular key map. */
typedef struct
{
    const char * countrycode;
    keymap map;
} intl_keymap;


/*
    Key modifier flags: when resolving a key code to a character, these indicate the combination of
    modifier keys pressed, or lock keys active, at the time the character key was pressed.
*/
#define KMF_NORMAL      (0)     /* No modifier keys pressed; no lock keys active    */
#define KMF_SHIFT       BIT(1)  /* One or more "shift" keys pressed                 */
#define KMF_CTRL        BIT(2)  /* One or more "ctrl" keys pressed                  */
#define KMF_ALT         BIT(3)  /* One or more "alt" keys pressed                   */
#define KMF_GUI         BIT(4)  /* One or more "gui" keys pressed                   */
#define KMF_CAPS        BIT(5)  /* "Caps lock" key state                            */
#define KMF_NUM         BIT(6)  /* "Num lock" key state                             */
#define KMF_SCROLL      BIT(7)  /* "Scroll lock" key state                          */

typedef u8 key_modifier;


/*
    Internal key codes: these represent every keyboard key, regardless of language, understood by
    the system.  Currently only the keys from the standard en_GB layout are included.
*/
typedef enum
{
    KEY_NONE                = 0x00,     /* Reserved code - indicates no keypress */

    KEY_A                   = 0x01,
    KEY_B                   = 0x02,
    KEY_C                   = 0x03,
    KEY_D                   = 0x04,
    KEY_E                   = 0x05,
    KEY_F                   = 0x06,
    KEY_G                   = 0x07,
    KEY_H                   = 0x08,
    KEY_I                   = 0x09,
    KEY_J                   = 0x0a,
    KEY_K                   = 0x0b,
    KEY_L                   = 0x0c,
    KEY_M                   = 0x0d,
    KEY_N                   = 0x0e,
    KEY_O                   = 0x0f,
    KEY_P                   = 0x10,
    KEY_Q                   = 0x11,
    KEY_R                   = 0x12,
    KEY_S                   = 0x13,
    KEY_T                   = 0x14,
    KEY_U                   = 0x15,
    KEY_V                   = 0x16,
    KEY_W                   = 0x17,
    KEY_X                   = 0x18,
    KEY_Y                   = 0x19,
    KEY_Z                   = 0x1a,

    KEY_0                   = 0x20,
    KEY_1                   = 0x21,
    KEY_2                   = 0x22,
    KEY_3                   = 0x23,
    KEY_4                   = 0x24,
    KEY_5                   = 0x25,
    KEY_6                   = 0x26,
    KEY_7                   = 0x27,
    KEY_8                   = 0x28,
    KEY_9                   = 0x29,

    KEY_F1                  = 0x30,
    KEY_F2                  = 0x31,
    KEY_F3                  = 0x32,
    KEY_F4                  = 0x33,
    KEY_F5                  = 0x34,
    KEY_F6                  = 0x35,
    KEY_F7                  = 0x36,
    KEY_F8                  = 0x37,
    KEY_F9                  = 0x38,
    KEY_F10                 = 0x39,
    KEY_F11                 = 0x3a,
    KEY_F12                 = 0x3b,

    KEY_NP_FWDSL            = 0x40,
    KEY_NP_STAR             = 0x41,
    KEY_NP_MINUS            = 0x42,
    KEY_NP_PLUS             = 0x43,
    KEY_NP_ENTER            = 0x44,
    KEY_NP_DOT              = 0x45,
    KEY_NP_0                = 0x46,
    KEY_NP_1                = 0x47,
    KEY_NP_2                = 0x48,
    KEY_NP_3                = 0x49,
    KEY_NP_4                = 0x4a,
    KEY_NP_5                = 0x4b,
    KEY_NP_6                = 0x4c,
    KEY_NP_7                = 0x4d,
    KEY_NP_8                = 0x4e,
    KEY_NP_9                = 0x4f,

    KEY_ESC                 = 0x50,
    KEY_BKTICK              = 0x51,
    KEY_MINUS               = 0x52,
    KEY_EQUALS              = 0x53,
    KEY_BKSPACE             = 0x54,
    KEY_BKT_L               = 0x55,
    KEY_BKT_R               = 0x56,
    KEY_SCOLON              = 0x57,
    KEY_APOS                = 0x58,
    KEY_HASH                = 0x59,
    KEY_BKSLASH             = 0x5a,
    KEY_SPACE               = 0x5b,
    KEY_COMMA               = 0x5c,
    KEY_DOT                 = 0x5d,
    KEY_FWDSL               = 0x5e,
    KEY_TAB                 = 0x5f,
    KEY_ENTER               = 0x60,
    KEY_CAPS                = 0x61,
    KEY_SCROLL              = 0x62,
    KEY_NUM                 = 0x63,
    KEY_SHIFT_L             = 0x64,
    KEY_SHIFT_R             = 0x65,
    KEY_CTRL_L              = 0x66,
    KEY_CTRL_R              = 0x67,
    KEY_ALT_L               = 0x68,
    KEY_ALT_R               = 0x69,
    KEY_GUI_L               = 0x6a,
    KEY_GUI_R               = 0x6b,
    KEY_APPS                = 0x6c,
    KEY_INSERT              = 0x6d,
    KEY_DELETE              = 0x6e,
    KEY_HOME                = 0x6f,
    KEY_END                 = 0x70,
    KEY_PG_UP               = 0x71,
    KEY_PG_DOWN             = 0x72,
    KEY_PRT_SC              = 0x73,
    KEY_PAUSE               = 0x74,
    KEY_ARROW_U             = 0x75,
    KEY_ARROW_D             = 0x76,
    KEY_ARROW_L             = 0x77,
    KEY_ARROW_R             = 0x78,
    KEY_POWER               = 0x79,
    KEY_SLEEP               = 0x7a,
    KEY_WAKE                = 0x7b
} KeyCode;


char keymap_get(const KeyCode code, const key_modifier mod);

#endif
