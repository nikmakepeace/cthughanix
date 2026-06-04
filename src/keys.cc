#include "cthugha.h"
#include "Configuration.h"
#include "keys.h"
#include "display.h"

#ifdef CTH_XWIN
#include "xcthugha.h"
#endif

int key_esc = 0; /* disable/enable ESC-key. When enable it
                    sometimes happens that when pressing
                    functions keys or cursor keys cthugha
                    only get the leading ESC and quits. */
int x11_key = CK_NONE;

void configureKeys(const InputConfig& config) {
    key_esc = config.escapeKeyEnabled;
}

// to handle keys, that give shifted and normal the same result
static int shiftMap[][2] = {
    { '0', CK_SHIFT(0) },
    { '1', CK_SHIFT(1) },
    { '2', CK_SHIFT(2) },
    { '3', CK_SHIFT(3) },
    { '4', CK_SHIFT(4) },
    { '5', CK_SHIFT(5) },
    { '6', CK_SHIFT(6) },
    { '7', CK_SHIFT(7) },
    { '8', CK_SHIFT(8) },
    { '9', CK_SHIFT(9) },
};
static int nShiftMap = sizeof(shiftMap) / sizeof(int[2]);

int shift(int key, int shift) {
    if (shift) {
        for (int i = 0; i < nShiftMap; i++)
            if (key == shiftMap[i][0]) {
                return shiftMap[i][1];
            }
    }

    return key;
}

KeyAssoc keyAssoc[] = {
    { "F10", CK_FKT(10) },
    { "F11", CK_FKT(11) },
    { "F12", CK_FKT(12) },
    { "F13", CK_FKT(13) },
    { "F14", CK_FKT(14) },
    { "F15", CK_FKT(15) },
    { "F16", CK_FKT(16) },
    { "F17", CK_FKT(17) },
    { "F18", CK_FKT(18) },
    { "F19", CK_FKT(19) },
    { "F20", CK_FKT(20) },
    { "F21", CK_FKT(21) },
    { "F22", CK_FKT(22) },
    { "F23", CK_FKT(23) },
    { "F24", CK_FKT(24) },
    { "F1", CK_FKT(1) },
    { "F2", CK_FKT(2) },
    { "F3", CK_FKT(3) },
    { "F4", CK_FKT(4) },
    { "F5", CK_FKT(5) },
    { "F6", CK_FKT(6) },
    { "F7", CK_FKT(7) },
    { "F8", CK_FKT(8) },
    { "F9", CK_FKT(9) },

    { "Return", CK_ENTER },
    { "Up", CK_UP },
    { "Down", CK_DOWN },
    { "Left", CK_LEFT },
    { "Right", CK_RIGHT },
    { "Prior", CK_PGUP },
    { "Next", CK_PGDN },
    { "End", CK_END },
    { "Home", CK_HOME },
    { "Print", CK_PRINT },
    { "BackSpace", CK_BACK },
    { "Delete", CK_DELETE },

    { "KP_0", '0' },
    { "KP_1", '1' },
    { "KP_2", '2' },
    { "KP_3", '3' },
    { "KP_4", '4' },
    { "KP_5", '5' },
    { "KP_6", '6' },
    { "KP_7", '7' },
    { "KP_8", '8' },
    { "KP_9", '9' },

    { "S-0", CK_SHIFT(0) }, // these are not X11 keys, but are only for the keymap
    { "S-1", CK_SHIFT(1) },
    { "S-2", CK_SHIFT(2) },
    { "S-3", CK_SHIFT(3) },
    { "S-4", CK_SHIFT(4) },
    { "S-5", CK_SHIFT(5) },
    { "S-6", CK_SHIFT(6) },
    { "S-7", CK_SHIFT(7) },
    { "S-8", CK_SHIFT(8) },
    { "S-9", CK_SHIFT(9) },

};
int nKeyAssoc = sizeof(keyAssoc) / sizeof(KeyAssoc);

#ifdef CTH_XWIN

/*
 * Handler for key-board
 */
void keys_x11(char* input, int state) {

    if (input[1] == '\0')
        switch (input[0]) {
        case 0:
        case -1:
            x11_key = CK_NONE;
            break;
        case 27:
            x11_key = (key_esc ? CK_ESC : CK_NONE);
            break;
        case 10:
        case 13:
            x11_key = CK_ENTER;
            break;
        case 8:
            x11_key = CK_BACK;
            break;
        default:
            x11_key = input[0];
        }
    else {
        int i;
        for (i = 0; i < nKeyAssoc; i++)
            if (strcasecmp(input, keyAssoc[i].name) == 0) {
                x11_key = keyAssoc[i].keyValue;
                return;
            }
        x11_key = CK_NONE;
    }

    x11_key = shift(x11_key, state & ShiftMask);
}

int getkey_x11() {
    int key;
    key = x11_key;
    x11_key = CK_NONE;
    return key;
}

#endif /* CTH_XWIN */

int getkey() {

#ifdef CTH_XWIN
    // first get the X key
    int k = getkey_x11();
    if (k != CK_NONE)
        return k;
#endif

    return CK_NONE;
}
