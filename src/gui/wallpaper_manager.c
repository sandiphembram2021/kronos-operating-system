#include "kronos.h"

/* Wallpaper Manager for Kronos OS */

#define MAX_WALLPAPERS 50
#define WALLPAPER_PREVIEW_WIDTH 150
#define WALLPAPER_PREVIEW_HEIGHT 100

/* Wallpaper types */
typedef enum {
    WALLPAPER_SOLID_COLOR,
    WALLPAPER_GRADIENT,
    WALLPAPER_IMAGE,
    WALLPAPER_SLIDESHOW
} wallpaper_type_t;

/* Wallpaper information */
struct wallpaper_info {
    char name[64];
    char path[256];
    wallpaper_type_t type;
    u32 primary_color;
    u32 secondary_color;
    u32 preview_data[WALLPAPER_PREVIEW_WIDTH * WALLPAPER_PREVIEW_HEIGHT];
    bool in_use;
};

/* Wallpaper manager state */
struct wallpaper_manager {
    struct wallpaper_info wallpapers[MAX_WALLPAPERS];
    u32 wallpaper_count;
    u32 current_wallpaper;
    u32 selected_wallpaper;
    bool slideshow_enabled;
    u32 slideshow_interval;  /* in seconds */
    u64 last_change_time;
} wallpaper_mgr;

/* Built-in wallpapers */
static struct wallpaper_info builtin_wallpapers[] = {
    {"Ubuntu Orange", "", WALLPAPER_GRADIENT, UBUNTU_ORANGE, UBUNTU_DARK, {0}, true},
    {"Ocean Blue", "", WALLPAPER_GRADIENT, 0x004A90E2, 0x00357ABD, {0}, true},
    {"Forest Green", "", WALLPAPER_GRADIENT, 0x0028A745, 0x00155724, {0}, true},
    {"Sunset Purple", "", WALLPAPER_GRADIENT, 0x006F42C1, 0x004C2A85, {0}, true},
    {"Deep Space", "", WALLPAPER_GRADIENT, 0x00000033, 0x00000066, {0}, true},
    {"Cherry Blossom", "", WALLPAPER_GRADIENT, 0x00FF69B4, 0x00FF1493, {0}, true},
    {"Arctic Blue", "", WALLPAPER_GRADIENT, 0x0087CEEB, 0x004682B4, {0}, true},
    {"Autumn Gold", "", WALLPAPER_GRADIENT, 0x00FFD700, 0x00FF8C00, {0}, true},
    {"Midnight Black", "", WALLPAPER_SOLID_COLOR, 0x00000000, 0x00000000, {0}, true},
    {"Pure White", "", WALLPAPER_SOLID_COLOR, 0x00FFFFFF, 0x00FFFFFF, {0}, true},
    {"Ubuntu Purple", "", WALLPAPER_SOLID_COLOR, 0x00772953, 0x00772953, {0}, true},
    {"Matrix Green", "", WALLPAPER_GRADIENT, 0x0000FF00, 0x00008000, {0}, true}
};

/* Initialize wallpaper manager */
void wallpaper_manager_init(void) {
    wallpaper_mgr.wallpaper_count = 0;
    wallpaper_mgr.current_wallpaper = 0;
    wallpaper_mgr.selected_wallpaper = 0;
    wallpaper_mgr.slideshow_enabled = false;
    wallpaper_mgr.slideshow_interval = 300;  /* 5 minutes */
    wallpaper_mgr.last_change_time = get_system_time();
    
    /* Load built-in wallpapers */
    u32 builtin_count = sizeof(builtin_wallpapers) / sizeof(builtin_wallpapers[0]);
    for (u32 i = 0; i < builtin_count && wallpaper_mgr.wallpaper_count < MAX_WALLPAPERS; i++) {
        wallpaper_mgr.wallpapers[wallpaper_mgr.wallpaper_count] = builtin_wallpapers[i];
        generate_wallpaper_preview(&wallpaper_mgr.wallpapers[wallpaper_mgr.wallpaper_count]);
        wallpaper_mgr.wallpaper_count++;
    }
    
    vga_puts("Wallpaper manager initialized\n");
}

/* Generate wallpaper preview */
static void generate_wallpaper_preview(struct wallpaper_info* wallpaper) {
    switch (wallpaper->type) {
        case WALLPAPER_SOLID_COLOR:
            for (u32 i = 0; i < WALLPAPER_PREVIEW_WIDTH * WALLPAPER_PREVIEW_HEIGHT; i++) {
                wallpaper->preview_data[i] = wallpaper->primary_color;
            }
            break;
            
        case WALLPAPER_GRADIENT:
            for (u32 y = 0; y < WALLPAPER_PREVIEW_HEIGHT; y++) {
                for (u32 x = 0; x < WALLPAPER_PREVIEW_WIDTH; x++) {
                    /* Create diagonal gradient */
                    float factor = (float)(x + y) / (WALLPAPER_PREVIEW_WIDTH + WALLPAPER_PREVIEW_HEIGHT);
                    u32 color = interpolate_color(wallpaper->primary_color, wallpaper->secondary_color, factor);
                    wallpaper->preview_data[y * WALLPAPER_PREVIEW_WIDTH + x] = color;
                }
            }
            break;
            
        case WALLPAPER_IMAGE:
            /* Load image preview - simplified for demo */
            generate_pattern_preview(wallpaper);
            break;
            
        case WALLPAPER_SLIDESHOW:
            /* Use first wallpaper as preview */
            for (u32 i = 0; i < WALLPAPER_PREVIEW_WIDTH * WALLPAPER_PREVIEW_HEIGHT; i++) {
                wallpaper->preview_data[i] = wallpaper->primary_color;
            }
            break;
    }
}

/* Generate pattern preview for image wallpapers */
static void generate_pattern_preview(struct wallpaper_info* wallpaper) {
    /* Create a simple pattern as placeholder */
    for (u32 y = 0; y < WALLPAPER_PREVIEW_HEIGHT; y++) {
        for (u32 x = 0; x < WALLPAPER_PREVIEW_WIDTH; x++) {
            u32 color;
            if ((x / 10 + y / 10) % 2 == 0) {
                color = wallpaper->primary_color;
            } else {
                color = wallpaper->secondary_color;
            }
            wallpaper->preview_data[y * WALLPAPER_PREVIEW_WIDTH + x] = color;
        }
    }
}

/* Interpolate between two colors */
static u32 interpolate_color(u32 color1, u32 color2, float factor) {
    if (factor < 0.0f) factor = 0.0f;
    if (factor > 1.0f) factor = 1.0f;
    
    u8 r1 = (color1 >> 16) & 0xFF;
    u8 g1 = (color1 >> 8) & 0xFF;
    u8 b1 = color1 & 0xFF;
    
    u8 r2 = (color2 >> 16) & 0xFF;
    u8 g2 = (color2 >> 8) & 0xFF;
    u8 b2 = color2 & 0xFF;
    
    u8 r = r1 + (u8)((r2 - r1) * factor);
    u8 g = g1 + (u8)((g2 - g1) * factor);
    u8 b = b1 + (u8)((b2 - b1) * factor);
    
    return (r << 16) | (g << 8) | b;
}

/* Set current wallpaper */
void wallpaper_set_current(u32 wallpaper_index) {
    if (wallpaper_index >= wallpaper_mgr.wallpaper_count) return;
    
    wallpaper_mgr.current_wallpaper = wallpaper_index;
    wallpaper_mgr.last_change_time = get_system_time();
    
    /* Apply wallpaper to desktop */
    desktop_set_wallpaper(&wallpaper_mgr.wallpapers[wallpaper_index]);
}

/* Get current wallpaper */
struct wallpaper_info* wallpaper_get_current(void) {
    if (wallpaper_mgr.current_wallpaper < wallpaper_mgr.wallpaper_count) {
        return &wallpaper_mgr.wallpapers[wallpaper_mgr.current_wallpaper];
    }
    return NULL;
}

/* Update slideshow */
void wallpaper_update_slideshow(void) {
    if (!wallpaper_mgr.slideshow_enabled) return;
    
    u64 current_time = get_system_time();
    u64 elapsed = current_time - wallpaper_mgr.last_change_time;
    
    if (elapsed >= wallpaper_mgr.slideshow_interval * 1000000) {  /* Convert to microseconds */
        /* Change to next wallpaper */
        u32 next_wallpaper = (wallpaper_mgr.current_wallpaper + 1) % wallpaper_mgr.wallpaper_count;
        wallpaper_set_current(next_wallpaper);
    }
}

/* Render desktop wallpaper */
void wallpaper_render_desktop(u32* buffer, u32 width, u32 height) {
    struct wallpaper_info* wallpaper = wallpaper_get_current();
    if (!wallpaper) return;
    
    switch (wallpaper->type) {
        case WALLPAPER_SOLID_COLOR:
            for (u32 i = 0; i < width * height; i++) {
                buffer[i] = wallpaper->primary_color;
            }
            break;
            
        case WALLPAPER_GRADIENT:
            render_gradient_wallpaper(buffer, width, height, wallpaper);
            break;
            
        case WALLPAPER_IMAGE:
            render_image_wallpaper(buffer, width, height, wallpaper);
            break;
            
        case WALLPAPER_SLIDESHOW:
            /* Render current slideshow wallpaper */
            render_gradient_wallpaper(buffer, width, height, wallpaper);
            break;
    }
}

/* Render gradient wallpaper */
static void render_gradient_wallpaper(u32* buffer, u32 width, u32 height, 
                                     struct wallpaper_info* wallpaper) {
    for (u32 y = 0; y < height; y++) {
        for (u32 x = 0; x < width; x++) {
            /* Create diagonal gradient */
            float factor = (float)(x + y) / (width + height);
            u32 color = interpolate_color(wallpaper->primary_color, wallpaper->secondary_color, factor);
            buffer[y * width + x] = color;
        }
    }
}

/* Render image wallpaper */
static void render_image_wallpaper(u32* buffer, u32 width, u32 height, 
                                  struct wallpaper_info* wallpaper) {
    /* Create a pattern as placeholder for image */
    for (u32 y = 0; y < height; y++) {
        for (u32 x = 0; x < width; x++) {
            u32 color;
            
            /* Create a more complex pattern */
            if ((x / 50 + y / 50) % 2 == 0) {
                if ((x / 10) % 2 == (y / 10) % 2) {
                    color = wallpaper->primary_color;
                } else {
                    color = interpolate_color(wallpaper->primary_color, wallpaper->secondary_color, 0.5f);
                }
            } else {
                color = wallpaper->secondary_color;
            }
            
            buffer[y * width + x] = color;
        }
    }
}

/* Launch wallpaper selector */
void app_launch_wallpaper_selector(u32 window_id) {
    /* This would open the wallpaper selection dialog */
    /* For now, cycle through available wallpapers */
    u32 next = (wallpaper_mgr.current_wallpaper + 1) % wallpaper_mgr.wallpaper_count;
    wallpaper_set_current(next);
}

/* Wallpaper selector window */
struct wallpaper_selector {
    u32 window_id;
    u32 selected_wallpaper;
    u32 scroll_position;
    bool active;
} wallpaper_selectors[2];

static u32 selector_count = 0;

/* Initialize wallpaper selector */
void app_wallpaper_selector_init(void) {
    for (u32 i = 0; i < 2; i++) {
        wallpaper_selectors[i].active = false;
        wallpaper_selectors[i].window_id = 0;
    }
    selector_count = 0;
}

/* Launch wallpaper selector window */
void app_launch_wallpaper_selector_window(u32 window_id) {
    if (selector_count >= 2) return;
    
    u32 slot = selector_count++;
    wallpaper_selectors[slot].window_id = window_id;
    wallpaper_selectors[slot].selected_wallpaper = wallpaper_mgr.current_wallpaper;
    wallpaper_selectors[slot].scroll_position = 0;
    wallpaper_selectors[slot].active = true;
}

/* Render wallpaper selector */
void wallpaper_selector_render(struct wallpaper_selector* selector) {
    if (!selector->active) return;
    
    u32* buffer = wm_get_window_buffer(selector->window_id);
    if (!buffer) return;
    
    u32 win_width = 600;
    u32 win_height = 500;
    
    /* Clear window buffer */
    for (u32 i = 0; i < win_width * win_height; i++) {
        buffer[i] = COLOR_WHITE;
    }
    
    /* Draw title */
    wallpaper_draw_text_to_buffer(buffer, win_width, 20, 20, "Choose Wallpaper", COLOR_BLACK);
    
    /* Draw wallpaper grid */
    u32 cols = 3;
    u32 preview_width = WALLPAPER_PREVIEW_WIDTH;
    u32 preview_height = WALLPAPER_PREVIEW_HEIGHT;
    u32 margin = 20;
    
    for (u32 i = 0; i < wallpaper_mgr.wallpaper_count; i++) {
        u32 col = i % cols;
        u32 row = i / cols;
        u32 x = margin + col * (preview_width + margin);
        u32 y = 60 + row * (preview_height + margin + 20);
        
        if (y + preview_height > win_height - 60) break;  /* Don't draw outside window */
        
        struct wallpaper_info* wallpaper = &wallpaper_mgr.wallpapers[i];
        
        /* Draw preview */
        for (u32 py = 0; py < preview_height; py++) {
            for (u32 px = 0; px < preview_width; px++) {
                if (x + px < win_width && y + py < win_height) {
                    buffer[(y + py) * win_width + (x + px)] = 
                        wallpaper->preview_data[py * preview_width + px];
                }
            }
        }
        
        /* Draw selection border */
        if (i == selector->selected_wallpaper) {
            u32 border_color = UBUNTU_ORANGE;
            /* Top and bottom borders */
            for (u32 px = 0; px < preview_width + 4; px++) {
                if (x + px - 2 < win_width) {
                    if (y - 2 < win_height) buffer[(y - 2) * win_width + (x + px - 2)] = border_color;
                    if (y + preview_height + 1 < win_height) {
                        buffer[(y + preview_height + 1) * win_width + (x + px - 2)] = border_color;
                    }
                }
            }
            /* Left and right borders */
            for (u32 py = 0; py < preview_height + 4; py++) {
                if (y + py - 2 < win_height) {
                    if (x - 2 < win_width) buffer[(y + py - 2) * win_width + (x - 2)] = border_color;
                    if (x + preview_width + 1 < win_width) {
                        buffer[(y + py - 2) * win_width + (x + preview_width + 1)] = border_color;
                    }
                }
            }
        }
        
        /* Draw wallpaper name */
        wallpaper_draw_text_to_buffer(buffer, win_width, x, y + preview_height + 5, 
                                     wallpaper->name, COLOR_BLACK);
    }
    
    /* Draw buttons */
    wallpaper_draw_button(buffer, win_width, win_width - 200, win_height - 50, 80, 30, "Apply", COLOR_GREEN);
    wallpaper_draw_button(buffer, win_width, win_width - 100, win_height - 50, 80, 30, "Cancel", COLOR_GRAY);
    
    /* Draw slideshow controls */
    wallpaper_draw_text_to_buffer(buffer, win_width, 20, win_height - 80, "Slideshow:", COLOR_BLACK);
    
    const char* slideshow_text = wallpaper_mgr.slideshow_enabled ? "Enabled" : "Disabled";
    u32 slideshow_color = wallpaper_mgr.slideshow_enabled ? COLOR_GREEN : COLOR_RED;
    wallpaper_draw_text_to_buffer(buffer, win_width, 100, win_height - 80, slideshow_text, slideshow_color);
    
    wallpaper_draw_button(buffer, win_width, 200, win_height - 85, 60, 20, "Toggle", COLOR_BLUE);
}

/* Handle wallpaper selector click */
void wallpaper_selector_handle_click(struct wallpaper_selector* selector, u32 x, u32 y) {
    u32 win_width = 600;
    u32 win_height = 500;
    
    /* Check wallpaper grid clicks */
    if (y >= 60 && y < win_height - 100) {
        u32 cols = 3;
        u32 preview_width = WALLPAPER_PREVIEW_WIDTH;
        u32 preview_height = WALLPAPER_PREVIEW_HEIGHT;
        u32 margin = 20;
        
        u32 col = (x - margin) / (preview_width + margin);
        u32 row = (y - 60) / (preview_height + margin + 20);
        u32 wallpaper_index = row * cols + col;
        
        if (wallpaper_index < wallpaper_mgr.wallpaper_count) {
            selector->selected_wallpaper = wallpaper_index;
        }
        return;
    }
    
    /* Check button clicks */
    if (y >= win_height - 50 && y < win_height - 20) {
        if (x >= win_width - 200 && x < win_width - 120) {
            /* Apply button */
            wallpaper_set_current(selector->selected_wallpaper);
            wm_destroy_window(selector->window_id);
            selector->active = false;
        } else if (x >= win_width - 100 && x < win_width - 20) {
            /* Cancel button */
            wm_destroy_window(selector->window_id);
            selector->active = false;
        }
        return;
    }
    
    /* Check slideshow toggle */
    if (y >= win_height - 85 && y < win_height - 65 && x >= 200 && x < 260) {
        wallpaper_mgr.slideshow_enabled = !wallpaper_mgr.slideshow_enabled;
    }
}

/* Draw button for wallpaper selector */
static void wallpaper_draw_button(u32* buffer, u32 buffer_width, u32 x, u32 y, 
                                 u32 width, u32 height, const char* text, u32 color) {
    /* Draw button background */
    for (u32 dy = 0; dy < height; dy++) {
        for (u32 dx = 0; dx < width; dx++) {
            if (x + dx < buffer_width && y + dy < 500) {
                buffer[(y + dy) * buffer_width + (x + dx)] = color;
            }
        }
    }
    
    /* Draw button text */
    u32 text_x = x + (width - strlen(text) * 8) / 2;
    u32 text_y = y + (height - 8) / 2;
    wallpaper_draw_text_to_buffer(buffer, buffer_width, text_x, text_y, text, COLOR_WHITE);
}

/* Draw text to buffer */
void wallpaper_draw_text_to_buffer(u32* buffer, u32 buffer_width, u32 x, u32 y, 
                                  const char* text, u32 color) {
    u32 pos_x = x;
    
    while (*text && pos_x < buffer_width - 8) {
        /* Simple character rendering */
        for (u32 dy = 0; dy < 12; dy++) {
            for (u32 dx = 0; dx < 8; dx++) {
                bool pixel = false;
                
                if (*text >= 'A' && *text <= 'Z') {
                    pixel = ((dx + dy + *text) % 3 == 0);
                } else if (*text >= 'a' && *text <= 'z') {
                    pixel = ((dx * 2 + dy + *text) % 4 == 0);
                } else if (*text >= '0' && *text <= '9') {
                    pixel = ((dx + dy * 2 + *text) % 3 == 0);
                } else if (*text == ' ') {
                    pixel = false;
                } else {
                    pixel = ((dx + dy) % 2 == 0);
                }
                
                if (pixel && pos_x + dx < buffer_width && y + dy < 500) {
                    buffer[(y + dy) * buffer_width + (pos_x + dx)] = color;
                }
            }
        }
        pos_x += 8;
        text++;
    }
}

/* Find wallpaper selector by window ID */
struct wallpaper_selector* wallpaper_selector_find_by_window(u32 window_id) {
    for (u32 i = 0; i < 2; i++) {
        if (wallpaper_selectors[i].active && wallpaper_selectors[i].window_id == window_id) {
            return &wallpaper_selectors[i];
        }
    }
    return NULL;
}
