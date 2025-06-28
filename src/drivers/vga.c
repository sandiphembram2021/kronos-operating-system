#include "kronos.h"

/* VGA Text Mode Driver */

static u16* vga_buffer = (u16*)VGA_MEMORY;
static u8 vga_color = 0x0F; /* White on black */
static u8 cursor_x = 0;
static u8 cursor_y = 0;

/* Create VGA entry */
static inline u16 vga_entry(unsigned char uc, u8 color) {
    return (u16) uc | (u16) color << 8;
}

/* Create VGA color */
static inline u8 vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

/* Initialize VGA */
void vga_init(void) {
    vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    cursor_x = 0;
    cursor_y = 0;
}

/* Set VGA color */
void vga_set_color(u8 fg, u8 bg) {
    vga_color = vga_entry_color(fg, bg);
}

/* Clear screen */
void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', vga_color);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    vga_update_cursor();
}

/* Scroll screen up */
static void vga_scroll(void) {
    /* Move all lines up */
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    /* Clear bottom line */
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);
    }
}

/* Update hardware cursor */
void vga_update_cursor(void) {
    u16 pos = cursor_y * VGA_WIDTH + cursor_x;
    
    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

/* Put character at position */
void vga_putentryat(char c, u8 color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    vga_buffer[index] = vga_entry(c, color);
}

/* Put character */
void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~(8 - 1);
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_putentryat(' ', vga_color, cursor_x, cursor_y);
        }
    } else {
        vga_putentryat(c, vga_color, cursor_x, cursor_y);
        cursor_x++;
    }
    
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if (cursor_y >= VGA_HEIGHT) {
        vga_scroll();
        cursor_y = VGA_HEIGHT - 1;
    }
    
    vga_update_cursor();
}

/* Put string */
void vga_puts(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

/* Simple printf implementation */
void vga_printf(const char* format, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, format);
    
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int num = __builtin_va_arg(args, int);
                    char buffer[32];
                    itoa(num, buffer, 10);
                    vga_puts(buffer);
                    break;
                }
                case 'x': {
                    unsigned int num = __builtin_va_arg(args, unsigned int);
                    char buffer[32];
                    itoa(num, buffer, 16);
                    vga_puts("0x");
                    vga_puts(buffer);
                    break;
                }
                case 's': {
                    char* str = __builtin_va_arg(args, char*);
                    vga_puts(str);
                    break;
                }
                case 'c': {
                    char c = (char)__builtin_va_arg(args, int);
                    vga_putchar(c);
                    break;
                }
                case '%':
                    vga_putchar('%');
                    break;
                default:
                    vga_putchar('%');
                    vga_putchar(*format);
                    break;
            }
        } else {
            vga_putchar(*format);
        }
        format++;
    }
    
    __builtin_va_end(args);
}

/* Integer to string conversion */
static void itoa(int value, char* buffer, int base) {
    char* ptr = buffer;
    char* ptr1 = buffer;
    char tmp_char;
    int tmp_value;
    
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }
    
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
        ptr1++;
    }
    
    while (value) {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    }
    
    *ptr-- = '\0';
    
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}
