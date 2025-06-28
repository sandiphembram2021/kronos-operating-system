#include "kronos.h"

/* Window Manager for Kronos OS GUI */

#define MAX_WINDOWS 32
#define TITLE_BAR_HEIGHT 24
#define BORDER_WIDTH 2

/* Window structure */
struct window {
    u32 id;
    u32 x, y;
    u32 width, height;
    char title[64];
    u32 flags;
    bool visible;
    bool focused;
    bool minimized;
    u32* buffer;  /* Window's pixel buffer */
} windows[MAX_WINDOWS];

/* Window flags */
#define WINDOW_RESIZABLE    0x01
#define WINDOW_CLOSABLE     0x02
#define WINDOW_MINIMIZABLE  0x04
#define WINDOW_MAXIMIZABLE  0x08
#define WINDOW_MODAL        0x10

static u32 next_window_id = 1;
static u32 focused_window = 0;
static u32 window_count = 0;

/* Mouse state */
struct mouse_state {
    u32 x, y;
    bool left_button;
    bool right_button;
    bool middle_button;
} mouse;

/* Initialize window manager */
void wm_init(void) {
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        windows[i].id = 0;
        windows[i].visible = false;
        windows[i].buffer = NULL;
    }
    
    mouse.x = fb_get_info()->width / 2;
    mouse.y = fb_get_info()->height / 2;
    mouse.left_button = false;
    mouse.right_button = false;
    mouse.middle_button = false;
    
    window_count = 0;
    focused_window = 0;
}

/* Create new window */
u32 wm_create_window(u32 x, u32 y, u32 width, u32 height, const char* title, u32 flags) {
    if (window_count >= MAX_WINDOWS) return 0;
    
    /* Find free window slot */
    u32 slot = 0;
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id == 0) {
            slot = i;
            break;
        }
    }
    
    /* Initialize window */
    windows[slot].id = next_window_id++;
    windows[slot].x = x;
    windows[slot].y = y;
    windows[slot].width = width;
    windows[slot].height = height;
    strcpy(windows[slot].title, title);
    windows[slot].flags = flags;
    windows[slot].visible = true;
    windows[slot].focused = true;
    windows[slot].minimized = false;
    
    /* Allocate window buffer */
    windows[slot].buffer = (u32*)kmalloc(width * height * sizeof(u32));
    if (!windows[slot].buffer) {
        windows[slot].id = 0;
        return 0;
    }
    
    /* Clear window buffer */
    for (u32 i = 0; i < width * height; i++) {
        windows[slot].buffer[i] = COLOR_WHITE;
    }
    
    /* Set as focused window */
    if (focused_window != 0) {
        wm_set_window_focus(focused_window, false);
    }
    focused_window = windows[slot].id;
    
    window_count++;
    return windows[slot].id;
}

/* Destroy window */
void wm_destroy_window(u32 window_id) {
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id == window_id) {
            if (windows[i].buffer) {
                kfree(windows[i].buffer);
            }
            windows[i].id = 0;
            windows[i].visible = false;
            windows[i].buffer = NULL;
            window_count--;
            
            /* If this was the focused window, focus another */
            if (focused_window == window_id) {
                focused_window = 0;
                for (u32 j = 0; j < MAX_WINDOWS; j++) {
                    if (windows[j].id != 0 && windows[j].visible) {
                        focused_window = windows[j].id;
                        windows[j].focused = true;
                        break;
                    }
                }
            }
            break;
        }
    }
}

/* Set window focus */
void wm_set_window_focus(u32 window_id, bool focused) {
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id == window_id) {
            windows[i].focused = focused;
            if (focused) {
                /* Unfocus other windows */
                for (u32 j = 0; j < MAX_WINDOWS; j++) {
                    if (j != i && windows[j].id != 0) {
                        windows[j].focused = false;
                    }
                }
                focused_window = window_id;
            }
            break;
        }
    }
}

/* Move window */
void wm_move_window(u32 window_id, u32 x, u32 y) {
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id == window_id) {
            windows[i].x = x;
            windows[i].y = y;
            break;
        }
    }
}

/* Resize window */
void wm_resize_window(u32 window_id, u32 width, u32 height) {
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id == window_id) {
            if (!(windows[i].flags & WINDOW_RESIZABLE)) return;
            
            /* Reallocate buffer */
            if (windows[i].buffer) {
                kfree(windows[i].buffer);
            }
            
            windows[i].buffer = (u32*)kmalloc(width * height * sizeof(u32));
            if (windows[i].buffer) {
                windows[i].width = width;
                windows[i].height = height;
                
                /* Clear new buffer */
                for (u32 j = 0; j < width * height; j++) {
                    windows[i].buffer[j] = COLOR_WHITE;
                }
            }
            break;
        }
    }
}

/* Draw window title bar */
static void draw_title_bar(struct window* win) {
    u32 title_color = win->focused ? UBUNTU_ORANGE : COLOR_GRAY;
    u32 text_color = COLOR_WHITE;
    
    /* Draw title bar background */
    fb_draw_rect(win->x, win->y, win->width, TITLE_BAR_HEIGHT, title_color);
    
    /* Draw title text */
    fb_draw_string(win->x + 8, win->y + 8, win->title, text_color, title_color);
    
    /* Draw window controls */
    u32 control_x = win->x + win->width - 20;
    
    /* Close button */
    if (win->flags & WINDOW_CLOSABLE) {
        fb_draw_rect(control_x - 60, win->y + 4, 16, 16, COLOR_RED);
        fb_draw_string(control_x - 56, win->y + 8, "X", COLOR_WHITE, COLOR_RED);
    }
    
    /* Maximize button */
    if (win->flags & WINDOW_MAXIMIZABLE) {
        fb_draw_rect(control_x - 40, win->y + 4, 16, 16, COLOR_GREEN);
        fb_draw_string(control_x - 36, win->y + 8, "□", COLOR_WHITE, COLOR_GREEN);
    }
    
    /* Minimize button */
    if (win->flags & WINDOW_MINIMIZABLE) {
        fb_draw_rect(control_x - 20, win->y + 4, 16, 16, COLOR_YELLOW);
        fb_draw_string(control_x - 16, win->y + 8, "_", COLOR_BLACK, COLOR_YELLOW);
    }
}

/* Draw window border */
static void draw_window_border(struct window* win) {
    u32 border_color = win->focused ? UBUNTU_ORANGE : COLOR_GRAY;
    
    /* Draw border around entire window */
    fb_draw_rect_border(win->x - BORDER_WIDTH, win->y - BORDER_WIDTH,
                       win->width + 2 * BORDER_WIDTH, 
                       win->height + TITLE_BAR_HEIGHT + 2 * BORDER_WIDTH,
                       COLOR_TRANSPARENT, border_color);
}

/* Render single window */
static void render_window(struct window* win) {
    if (!win->visible || win->minimized) return;
    
    /* Draw window border */
    draw_window_border(win);
    
    /* Draw title bar */
    draw_title_bar(win);
    
    /* Draw window content */
    u32 content_y = win->y + TITLE_BAR_HEIGHT;
    for (u32 y = 0; y < win->height; y++) {
        for (u32 x = 0; x < win->width; x++) {
            u32 color = win->buffer[y * win->width + x];
            fb_set_pixel(win->x + x, content_y + y, color);
        }
    }
}

/* Render all windows */
void wm_render(void) {
    /* Render windows from back to front */
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id != 0 && !windows[i].focused) {
            render_window(&windows[i]);
        }
    }
    
    /* Render focused window last (on top) */
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id != 0 && windows[i].focused) {
            render_window(&windows[i]);
            break;
        }
    }
}

/* Handle mouse click */
void wm_handle_mouse_click(u32 x, u32 y, bool left_button) {
    mouse.x = x;
    mouse.y = y;
    mouse.left_button = left_button;
    
    if (!left_button) return;
    
    /* Check if click is on a window */
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id == 0 || !windows[i].visible) continue;
        
        u32 win_x = windows[i].x - BORDER_WIDTH;
        u32 win_y = windows[i].y - BORDER_WIDTH;
        u32 win_w = windows[i].width + 2 * BORDER_WIDTH;
        u32 win_h = windows[i].height + TITLE_BAR_HEIGHT + 2 * BORDER_WIDTH;
        
        if (x >= win_x && x < win_x + win_w && y >= win_y && y < win_y + win_h) {
            /* Focus this window */
            wm_set_window_focus(windows[i].id, true);
            
            /* Check if click is on title bar controls */
            if (y >= windows[i].y && y < windows[i].y + TITLE_BAR_HEIGHT) {
                u32 control_x = windows[i].x + windows[i].width - 20;
                
                /* Close button */
                if (x >= control_x - 60 && x < control_x - 44) {
                    wm_destroy_window(windows[i].id);
                }
                /* Minimize button */
                else if (x >= control_x - 20 && x < control_x - 4) {
                    windows[i].minimized = !windows[i].minimized;
                }
            }
            break;
        }
    }
}

/* Get window buffer for drawing */
u32* wm_get_window_buffer(u32 window_id) {
    for (u32 i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].id == window_id) {
            return windows[i].buffer;
        }
    }
    return NULL;
}
