/*
    Keyboard-related functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2016.
*/

#ifdef WITH_KEYBOARD

#include <kernel/include/keyboard.h>
#include <klibc/include/ctype.h>


/*
    Keymap mapping en_GB key codes to ISO-8859-1 characters
*/
intl_keymap km_en_GB_ISO_8859_1 =
{
    .countrycode = "en_GB",
    .map =
    {
        .normal =
        {
             0,     'a',    'b',    'c',    'd',    'e',    'f',    'g',
            'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
            'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
            'x',    'y',    'z',     0,      0,      0,      0,      0,
            '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
            '8',    '9',     0,      0,      0,      0,      0,      0,
             0,      0,      0,      0,      0,      0,      0,      0,
             0,      0,      0,      0,      0,      0,      0,      0,
            '/',    '*',    '-',    '+',    '\n',   '.',    '0',    '1',
            '2',    '3',    '4',    '5',    '6',    '7',    '8',    '9',
             0,     '`',    '-',    '=',    '\b',   '[',    ']',    ';',
             '\'',  '#',    '\\',   ' ',    ',',    '.',    '/',    '\t',
            '\n',    0,      0,      0,      0,      0,      0,      0,
             0,      0,      0,      0,      0,      0,      0,      0,
             0,      0,      0,      0,      0,      0,      0,      0,
             0,      0,      0,      0,      0,      0,      0,      0
        },
        .shifted =
        {
             0,     'A',    'B',    'C',    'D',    'E',    'F',    'G',
            'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
            'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
            'X',    'Y',    'Z',     0,      0,      0,      0,      0,
            ')',    '!',    '"',    '\xa3', '$',    '%',    '^',    '&',
            '*',    '(',     0,      0,      0,      0,      0,      0,
             0,      0,      0,      0,      0,      0,      0,      0,
             0,      0,      0,      0,      0,      0,      0,      0,
            '/',    '*',    '-',    '+',    '\n',    0,      0,      0,
             0,      0,      0,      0,      0,      0,      0,      0,
             0,     '\xac', '_',    '+',    '\b',   '{',    '}',    ':',
            '@',    '~',    '|',    ' ',    '<',    '>',    '?',     0,
            '\n',    0,      0,      0,      0,      0,      0,      0,
             0,      0,      0,      0,      0,      0,      0,      0,
             0,      0,      0,      0,      0,      0,      0,      0,
             0,      0,      0,      0,      0,      0,      0,      0
        }
    }
};

intl_keymap *current_keymap = KEYMAP_DEFAULT;


/*
    keymap_get() - given a key code and a set of modifier flags, look up the corresponding character
    in the current keyboard map.
*/
char keymap_get(const KeyCode code, const key_modifier mod)
{
    char c;

    if(mod & KMF_SHIFT)
        c = current_keymap->map.shifted[code];
    else
        c = current_keymap->map.normal[code];

    if(mod & KMF_CAPS)
        c = (mod & KMF_SHIFT) ? tolower(c) : toupper(c);

    return c;
}

#endif /* WITH_KEYBOARD */
