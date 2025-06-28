#include "kronos.h"

/* PS/2 Keyboard Driver */

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_BUFFER_SIZE 256

/* Keyboard scan code to ASCII mapping */
static const char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* Shifted characters */
static const char scancode_to_ascii_shift[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* Keyboard state */
static bool shift_pressed = false;
static bool ctrl_pressed = false;
static bool alt_pressed = false;
static bool caps_lock = false;

/* Keyboard buffer */
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static volatile size_t buffer_head = 0;
static volatile size_t buffer_tail = 0;

/* Initialize keyboard */
void keyboard_init(void) {
    buffer_head = 0;
    buffer_tail = 0;
    shift_pressed = false;
    ctrl_pressed = false;
    alt_pressed = false;
    caps_lock = false;
}

/* Check if keyboard has input */
bool keyboard_has_input(void) {
    return buffer_head != buffer_tail;
}

/* Get character from keyboard buffer */
char keyboard_getchar(void) {
    if (!keyboard_has_input()) {
        return 0;
    }
    
    char c = keyboard_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

/* Add character to keyboard buffer */
static void keyboard_buffer_add(char c) {
    size_t next_head = (buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
    if (next_head != buffer_tail) {
        keyboard_buffer[buffer_head] = c;
        buffer_head = next_head;
    }
}

/* Keyboard interrupt handler */
void keyboard_interrupt_handler(void) {
    u8 scancode = inb(KEYBOARD_DATA_PORT);
    
    /* Handle key release (bit 7 set) */
    if (scancode & 0x80) {
        scancode &= 0x7F; /* Remove release bit */
        
        switch (scancode) {
            case 0x2A: /* Left shift */
            case 0x36: /* Right shift */
                shift_pressed = false;
                break;
            case 0x1D: /* Ctrl */
                ctrl_pressed = false;
                break;
            case 0x38: /* Alt */
                alt_pressed = false;
                break;
        }
        return;
    }
    
    /* Handle key press */
    switch (scancode) {
        case 0x2A: /* Left shift */
        case 0x36: /* Right shift */
            shift_pressed = true;
            break;
        case 0x1D: /* Ctrl */
            ctrl_pressed = true;
            break;
        case 0x38: /* Alt */
            alt_pressed = true;
            break;
        case 0x3A: /* Caps lock */
            caps_lock = !caps_lock;
            break;
        default: {
            /* Convert scancode to ASCII */
            if (scancode < sizeof(scancode_to_ascii)) {
                char c;
                
                if (shift_pressed) {
                    c = scancode_to_ascii_shift[scancode];
                } else {
                    c = scancode_to_ascii[scancode];
                }
                
                /* Handle caps lock for letters */
                if (caps_lock && c >= 'a' && c <= 'z') {
                    c = c - 'a' + 'A';
                } else if (caps_lock && c >= 'A' && c <= 'Z') {
                    c = c - 'A' + 'a';
                }
                
                /* Handle Ctrl combinations */
                if (ctrl_pressed && c >= 'a' && c <= 'z') {
                    c = c - 'a' + 1; /* Ctrl+A = 1, Ctrl+B = 2, etc. */
                } else if (ctrl_pressed && c >= 'A' && c <= 'Z') {
                    c = c - 'A' + 1;
                }
                
                if (c != 0) {
                    keyboard_buffer_add(c);
                }
            }
            break;
        }
    }
}
