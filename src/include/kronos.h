#ifndef KRONOS_H
#define KRONOS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Kronos OS Main Header File */

/* Basic type definitions */
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

/* Memory constants */
#define PAGE_SIZE 4096
#define KERNEL_VIRTUAL_BASE 0xFFFFFFFF80000000UL
#define KERNEL_PHYSICAL_BASE 0x100000UL

/* VGA Text Mode */
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

/* VGA Colors */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

/* Keyboard scan codes */
#define KEY_ESC     0x01
#define KEY_ENTER   0x1C
#define KEY_SPACE   0x39
#define KEY_BACKSPACE 0x0E

/* Function declarations */

/* VGA Driver */
void vga_init(void);
void vga_clear(void);
void vga_putchar(char c);
void vga_puts(const char* str);
void vga_printf(const char* format, ...);
void vga_set_color(u8 fg, u8 bg);
void vga_update_cursor(void);
void vga_putentryat(char c, u8 color, size_t x, size_t y);

/* Keyboard Driver */
void keyboard_init(void);
char keyboard_getchar(void);
bool keyboard_has_input(void);
void keyboard_interrupt_handler(void);

/* Memory Management */
void mm_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);
void get_memory_stats(size_t* total, size_t* used, size_t* free);

/* Interrupt Handling */
void idt_init(void);
void irq_install(void);

/* GDT */
void gdt_init(void);

/* Shell */
void shell_init(void);
void shell_run(void);

/* System */
void system_halt(void);
void system_reboot(void);
u64 get_uptime(void);

/* String functions */
size_t strlen(const char* str);
int strcmp(const char* str1, const char* str2);
char* strcpy(char* dest, const char* src);
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t size);
int strncmp(const char* str1, const char* str2, size_t n);
char* strchr(const char* str, int c);
char* strtok(char* str, const char* delim);
static void itoa(int value, char* buffer, int base);

/* I/O Port functions */
static inline void outb(u16 port, u8 val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline u8 inb(u16 port) {
    u8 ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* GUI and Graphics */
#define COLOR_TRANSPARENT 0xFF000000

/* Framebuffer functions */
void fb_init(void);
void fb_clear(u32 color);
void fb_set_pixel(u32 x, u32 y, u32 color);
u32 fb_get_pixel(u32 x, u32 y);
void fb_draw_rect(u32 x, u32 y, u32 width, u32 height, u32 color);
void fb_draw_rect_border(u32 x, u32 y, u32 width, u32 height, u32 fill_color, u32 border_color);
void fb_draw_line(u32 x0, u32 y0, u32 x1, u32 y1, u32 color);
void fb_draw_circle(u32 cx, u32 cy, u32 radius, u32 color);
void fb_draw_circle_filled(u32 cx, u32 cy, u32 radius, u32 color);
void fb_draw_char(u32 x, u32 y, char c, u32 color, u32 bg_color);
void fb_draw_string(u32 x, u32 y, const char* str, u32 color, u32 bg_color);
void fb_copy_rect(u32 src_x, u32 src_y, u32 dst_x, u32 dst_y, u32 width, u32 height);
struct framebuffer* fb_get_info(void);

/* Window Manager functions */
void wm_init(void);
u32 wm_create_window(u32 x, u32 y, u32 width, u32 height, const char* title, u32 flags);
void wm_destroy_window(u32 window_id);
void wm_set_window_focus(u32 window_id, bool focused);
void wm_move_window(u32 window_id, u32 x, u32 y);
void wm_resize_window(u32 window_id, u32 width, u32 height);
void wm_render(void);
void wm_handle_mouse_click(u32 x, u32 y, bool left_button);
u32* wm_get_window_buffer(u32 window_id);

/* Desktop Environment functions */
void desktop_init(void);
void desktop_add_icon(u32 x, u32 y, const char* name, const char* command, u32 color);
void desktop_add_taskbar_item(u32 window_id, const char* title);
void desktop_remove_taskbar_item(u32 window_id);
void desktop_handle_click(u32 x, u32 y);
void desktop_launch_app(const char* command);
void desktop_render(void);
void desktop_update_time(const char* time, const char* date);

/* Application functions */
void app_terminal_init(void);
void app_launch_terminal(u32 window_id);
void app_file_manager_init(void);
void app_launch_file_manager(u32 window_id);
void app_text_editor_init(void);
void app_launch_text_editor(u32 window_id);

#endif /* KRONOS_H */
