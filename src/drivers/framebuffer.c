#include "kronos.h"

/* Framebuffer Graphics Driver for Kronos OS */

#define FB_WIDTH 1024
#define FB_HEIGHT 768
#define FB_BPP 32  /* 32 bits per pixel (RGBA) */
#define FB_PITCH (FB_WIDTH * (FB_BPP / 8))

/* Framebuffer structure */
struct framebuffer {
    u32* buffer;
    u32 width;
    u32 height;
    u32 pitch;
    u32 bpp;
} fb;

/* Color definitions */
#define COLOR_BLACK     0x00000000
#define COLOR_WHITE     0x00FFFFFF
#define COLOR_RED       0x00FF0000
#define COLOR_GREEN     0x0000FF00
#define COLOR_BLUE      0x000000FF
#define COLOR_YELLOW    0x00FFFF00
#define COLOR_CYAN      0x0000FFFF
#define COLOR_MAGENTA   0x00FF00FF
#define COLOR_GRAY      0x00808080
#define COLOR_LIGHT_GRAY 0x00C0C0C0
#define COLOR_DARK_GRAY 0x00404040
#define COLOR_ORANGE    0x00FFA500
#define COLOR_PURPLE    0x00800080

/* Ubuntu-like color scheme */
#define UBUNTU_ORANGE   0x00E95420
#define UBUNTU_PURPLE   0x00772953
#define UBUNTU_LIGHT    0x00F7F7F7
#define UBUNTU_DARK     0x002C001E

/* Initialize framebuffer */
void fb_init(void) {
    /* For now, use a static buffer. In real implementation, 
       this would be mapped from VESA/GOP framebuffer */
    static u32 fb_memory[FB_WIDTH * FB_HEIGHT];
    
    fb.buffer = fb_memory;
    fb.width = FB_WIDTH;
    fb.height = FB_HEIGHT;
    fb.pitch = FB_PITCH;
    fb.bpp = FB_BPP;
    
    /* Clear screen to Ubuntu light background */
    fb_clear(UBUNTU_LIGHT);
}

/* Clear framebuffer with color */
void fb_clear(u32 color) {
    for (u32 i = 0; i < fb.width * fb.height; i++) {
        fb.buffer[i] = color;
    }
}

/* Set pixel at coordinates */
void fb_set_pixel(u32 x, u32 y, u32 color) {
    if (x >= fb.width || y >= fb.height) return;
    fb.buffer[y * fb.width + x] = color;
}

/* Get pixel at coordinates */
u32 fb_get_pixel(u32 x, u32 y) {
    if (x >= fb.width || y >= fb.height) return 0;
    return fb.buffer[y * fb.width + x];
}

/* Draw rectangle */
void fb_draw_rect(u32 x, u32 y, u32 width, u32 height, u32 color) {
    for (u32 dy = 0; dy < height; dy++) {
        for (u32 dx = 0; dx < width; dx++) {
            fb_set_pixel(x + dx, y + dy, color);
        }
    }
}

/* Draw filled rectangle with border */
void fb_draw_rect_border(u32 x, u32 y, u32 width, u32 height, u32 fill_color, u32 border_color) {
    /* Draw border */
    fb_draw_rect(x, y, width, 1, border_color);  /* Top */
    fb_draw_rect(x, y + height - 1, width, 1, border_color);  /* Bottom */
    fb_draw_rect(x, y, 1, height, border_color);  /* Left */
    fb_draw_rect(x + width - 1, y, 1, height, border_color);  /* Right */
    
    /* Fill interior */
    if (width > 2 && height > 2) {
        fb_draw_rect(x + 1, y + 1, width - 2, height - 2, fill_color);
    }
}

/* Draw line using Bresenham's algorithm */
void fb_draw_line(u32 x0, u32 y0, u32 x1, u32 y1, u32 color) {
    i32 dx = abs((i32)x1 - (i32)x0);
    i32 dy = abs((i32)y1 - (i32)y0);
    i32 sx = x0 < x1 ? 1 : -1;
    i32 sy = y0 < y1 ? 1 : -1;
    i32 err = dx - dy;
    
    while (1) {
        fb_set_pixel(x0, y0, color);
        
        if (x0 == x1 && y0 == y1) break;
        
        i32 e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

/* Draw circle */
void fb_draw_circle(u32 cx, u32 cy, u32 radius, u32 color) {
    i32 x = radius;
    i32 y = 0;
    i32 err = 0;
    
    while (x >= y) {
        fb_set_pixel(cx + x, cy + y, color);
        fb_set_pixel(cx + y, cy + x, color);
        fb_set_pixel(cx - y, cy + x, color);
        fb_set_pixel(cx - x, cy + y, color);
        fb_set_pixel(cx - x, cy - y, color);
        fb_set_pixel(cx - y, cy - x, color);
        fb_set_pixel(cx + y, cy - x, color);
        fb_set_pixel(cx + x, cy - y, color);
        
        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

/* Draw filled circle */
void fb_draw_circle_filled(u32 cx, u32 cy, u32 radius, u32 color) {
    for (u32 y = 0; y <= radius; y++) {
        for (u32 x = 0; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                fb_set_pixel(cx + x, cy + y, color);
                fb_set_pixel(cx - x, cy + y, color);
                fb_set_pixel(cx + x, cy - y, color);
                fb_set_pixel(cx - x, cy - y, color);
            }
        }
    }
}

/* Simple 8x8 bitmap font for text rendering */
static const u8 font_8x8[128][8] = {
    /* Space */
    [32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* A */
    [65] = {0x18, 0x3C, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    /* B */
    [66] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    /* Add more characters as needed... */
    /* For demo, we'll implement basic characters */
};

/* Draw character at position */
void fb_draw_char(u32 x, u32 y, char c, u32 color, u32 bg_color) {
    if (c < 0 || c >= 128) c = '?';
    
    for (u32 row = 0; row < 8; row++) {
        u8 line = font_8x8[(u8)c][row];
        for (u32 col = 0; col < 8; col++) {
            u32 pixel_color = (line & (0x80 >> col)) ? color : bg_color;
            fb_set_pixel(x + col, y + row, pixel_color);
        }
    }
}

/* Draw string at position */
void fb_draw_string(u32 x, u32 y, const char* str, u32 color, u32 bg_color) {
    u32 pos_x = x;
    while (*str) {
        if (*str == '\n') {
            pos_x = x;
            y += 8;
        } else {
            fb_draw_char(pos_x, y, *str, color, bg_color);
            pos_x += 8;
        }
        str++;
    }
}

/* Copy rectangular region */
void fb_copy_rect(u32 src_x, u32 src_y, u32 dst_x, u32 dst_y, u32 width, u32 height) {
    for (u32 y = 0; y < height; y++) {
        for (u32 x = 0; x < width; x++) {
            u32 color = fb_get_pixel(src_x + x, src_y + y);
            fb_set_pixel(dst_x + x, dst_y + y, color);
        }
    }
}

/* Get framebuffer info */
struct framebuffer* fb_get_info(void) {
    return &fb;
}
