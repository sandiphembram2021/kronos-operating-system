#include "kronos.h"

/* Desktop Environment for Kronos OS */

#define TASKBAR_HEIGHT 40
#define ICON_SIZE 32
#define MAX_DESKTOP_ICONS 16
#define MAX_TASKBAR_ITEMS 8

/* Desktop icon structure */
struct desktop_icon {
    u32 x, y;
    char name[32];
    char command[64];
    u32 icon_color;
    bool visible;
} desktop_icons[MAX_DESKTOP_ICONS];

/* Taskbar item structure */
struct taskbar_item {
    u32 window_id;
    char title[32];
    bool active;
} taskbar_items[MAX_TASKBAR_ITEMS];

/* Desktop state */
static bool start_menu_open = false;
static u32 desktop_icon_count = 0;
static u32 taskbar_item_count = 0;
static char current_time[16] = "12:00";
static char current_date[16] = "2025-06-28";

/* Wallpaper colors for gradient */
#define WALLPAPER_TOP    0x004A90E2
#define WALLPAPER_BOTTOM 0x00357ABD

/* Initialize desktop environment */
void desktop_init(void) {
    /* Clear desktop icons */
    for (u32 i = 0; i < MAX_DESKTOP_ICONS; i++) {
        desktop_icons[i].visible = false;
    }
    
    /* Clear taskbar items */
    for (u32 i = 0; i < MAX_TASKBAR_ITEMS; i++) {
        taskbar_items[i].window_id = 0;
        taskbar_items[i].active = false;
    }
    
    /* Add default desktop icons */
    desktop_add_icon(50, 50, "Terminal", "terminal", UBUNTU_ORANGE);
    desktop_add_icon(50, 100, "File Manager", "files", COLOR_BLUE);
    desktop_add_icon(50, 150, "Text Editor", "editor", COLOR_GREEN);
    desktop_add_icon(50, 200, "Calculator", "calc", COLOR_PURPLE);
    desktop_add_icon(50, 250, "Settings", "settings", COLOR_GRAY);
    desktop_add_icon(50, 300, "Web Browser", "browser", COLOR_ORANGE);
    
    start_menu_open = false;
    desktop_icon_count = 6;
    taskbar_item_count = 0;
}

/* Add desktop icon */
void desktop_add_icon(u32 x, u32 y, const char* name, const char* command, u32 color) {
    if (desktop_icon_count >= MAX_DESKTOP_ICONS) return;
    
    u32 slot = desktop_icon_count;
    desktop_icons[slot].x = x;
    desktop_icons[slot].y = y;
    strcpy(desktop_icons[slot].name, name);
    strcpy(desktop_icons[slot].command, command);
    desktop_icons[slot].icon_color = color;
    desktop_icons[slot].visible = true;
    
    desktop_icon_count++;
}

/* Draw gradient wallpaper */
static void draw_wallpaper(void) {
    struct framebuffer* fb = fb_get_info();
    
    for (u32 y = 0; y < fb->height - TASKBAR_HEIGHT; y++) {
        /* Calculate gradient color */
        u32 ratio = (y * 255) / (fb->height - TASKBAR_HEIGHT);
        u32 r_top = (WALLPAPER_TOP >> 16) & 0xFF;
        u32 g_top = (WALLPAPER_TOP >> 8) & 0xFF;
        u32 b_top = WALLPAPER_TOP & 0xFF;
        u32 r_bot = (WALLPAPER_BOTTOM >> 16) & 0xFF;
        u32 g_bot = (WALLPAPER_BOTTOM >> 8) & 0xFF;
        u32 b_bot = WALLPAPER_BOTTOM & 0xFF;
        
        u32 r = r_top + ((r_bot - r_top) * ratio) / 255;
        u32 g = g_top + ((g_bot - g_top) * ratio) / 255;
        u32 b = b_top + ((b_bot - b_top) * ratio) / 255;
        
        u32 color = (r << 16) | (g << 8) | b;
        
        for (u32 x = 0; x < fb->width; x++) {
            fb_set_pixel(x, y, color);
        }
    }
}

/* Draw desktop icon */
static void draw_desktop_icon(struct desktop_icon* icon) {
    if (!icon->visible) return;
    
    /* Draw icon background */
    fb_draw_rect_border(icon->x, icon->y, ICON_SIZE, ICON_SIZE, 
                       icon->icon_color, COLOR_WHITE);
    
    /* Draw icon symbol (simplified) */
    u32 center_x = icon->x + ICON_SIZE / 2;
    u32 center_y = icon->y + ICON_SIZE / 2;
    
    if (strcmp(icon->command, "terminal") == 0) {
        /* Terminal icon - draw rectangle with cursor */
        fb_draw_rect(icon->x + 4, icon->y + 4, ICON_SIZE - 8, ICON_SIZE - 8, COLOR_BLACK);
        fb_draw_string(icon->x + 8, icon->y + 8, ">_", COLOR_GREEN, COLOR_BLACK);
    } else if (strcmp(icon->command, "files") == 0) {
        /* File manager icon - draw folder */
        fb_draw_rect(icon->x + 6, icon->y + 8, ICON_SIZE - 12, ICON_SIZE - 16, COLOR_YELLOW);
        fb_draw_rect(icon->x + 6, icon->y + 6, 8, 4, COLOR_YELLOW);
    } else if (strcmp(icon->command, "editor") == 0) {
        /* Text editor icon - draw document */
        fb_draw_rect(icon->x + 8, icon->y + 4, ICON_SIZE - 16, ICON_SIZE - 8, COLOR_WHITE);
        fb_draw_line(icon->x + 10, icon->y + 8, icon->x + ICON_SIZE - 10, icon->y + 8, COLOR_BLACK);
        fb_draw_line(icon->x + 10, icon->y + 12, icon->x + ICON_SIZE - 10, icon->y + 12, COLOR_BLACK);
    } else {
        /* Generic icon - draw circle */
        fb_draw_circle_filled(center_x, center_y, ICON_SIZE / 3, icon->icon_color);
    }
    
    /* Draw icon label */
    fb_draw_string(icon->x, icon->y + ICON_SIZE + 4, icon->name, COLOR_WHITE, COLOR_TRANSPARENT);
}

/* Draw taskbar */
static void draw_taskbar(void) {
    struct framebuffer* fb = fb_get_info();
    u32 taskbar_y = fb->height - TASKBAR_HEIGHT;
    
    /* Draw taskbar background */
    fb_draw_rect(0, taskbar_y, fb->width, TASKBAR_HEIGHT, UBUNTU_PURPLE);
    
    /* Draw start button */
    fb_draw_rect_border(4, taskbar_y + 4, 80, TASKBAR_HEIGHT - 8, 
                       start_menu_open ? COLOR_LIGHT_GRAY : UBUNTU_ORANGE, COLOR_WHITE);
    fb_draw_string(12, taskbar_y + 16, "Kronos", COLOR_WHITE, 
                  start_menu_open ? COLOR_LIGHT_GRAY : UBUNTU_ORANGE);
    
    /* Draw taskbar items */
    u32 item_x = 100;
    for (u32 i = 0; i < taskbar_item_count; i++) {
        if (taskbar_items[i].window_id != 0) {
            u32 item_color = taskbar_items[i].active ? COLOR_LIGHT_GRAY : COLOR_GRAY;
            fb_draw_rect_border(item_x, taskbar_y + 4, 120, TASKBAR_HEIGHT - 8,
                               item_color, COLOR_WHITE);
            fb_draw_string(item_x + 8, taskbar_y + 16, taskbar_items[i].title, 
                          COLOR_BLACK, item_color);
            item_x += 124;
        }
    }
    
    /* Draw system tray */
    u32 tray_x = fb->width - 150;
    
    /* Draw clock */
    fb_draw_string(tray_x, taskbar_y + 8, current_time, COLOR_WHITE, UBUNTU_PURPLE);
    fb_draw_string(tray_x, taskbar_y + 20, current_date, COLOR_WHITE, UBUNTU_PURPLE);
    
    /* Draw system indicators */
    fb_draw_string(tray_x - 40, taskbar_y + 16, "♪ ⚡ 📶", COLOR_WHITE, UBUNTU_PURPLE);
}

/* Draw start menu */
static void draw_start_menu(void) {
    if (!start_menu_open) return;
    
    struct framebuffer* fb = fb_get_info();
    u32 menu_width = 250;
    u32 menu_height = 300;
    u32 menu_x = 4;
    u32 menu_y = fb->height - TASKBAR_HEIGHT - menu_height;
    
    /* Draw menu background */
    fb_draw_rect_border(menu_x, menu_y, menu_width, menu_height, 
                       COLOR_WHITE, UBUNTU_PURPLE);
    
    /* Draw menu header */
    fb_draw_rect(menu_x + 2, menu_y + 2, menu_width - 4, 40, UBUNTU_ORANGE);
    fb_draw_string(menu_x + 10, menu_y + 16, "Applications", COLOR_WHITE, UBUNTU_ORANGE);
    
    /* Draw menu items */
    const char* menu_items[] = {
        "Terminal",
        "File Manager", 
        "Text Editor",
        "Calculator",
        "Web Browser",
        "Settings",
        "System Monitor",
        "About Kronos"
    };
    
    for (u32 i = 0; i < 8; i++) {
        u32 item_y = menu_y + 50 + i * 30;
        fb_draw_string(menu_x + 20, item_y, menu_items[i], COLOR_BLACK, COLOR_WHITE);
    }
    
    /* Draw separator */
    fb_draw_line(menu_x + 10, menu_y + menu_height - 50, 
                menu_x + menu_width - 10, menu_y + menu_height - 50, COLOR_GRAY);
    
    /* Draw power options */
    fb_draw_string(menu_x + 20, menu_y + menu_height - 30, "Shutdown", COLOR_BLACK, COLOR_WHITE);
    fb_draw_string(menu_x + 120, menu_y + menu_height - 30, "Restart", COLOR_BLACK, COLOR_WHITE);
}

/* Add window to taskbar */
void desktop_add_taskbar_item(u32 window_id, const char* title) {
    if (taskbar_item_count >= MAX_TASKBAR_ITEMS) return;
    
    u32 slot = taskbar_item_count;
    taskbar_items[slot].window_id = window_id;
    strcpy(taskbar_items[slot].title, title);
    taskbar_items[slot].active = true;
    
    /* Deactivate other items */
    for (u32 i = 0; i < taskbar_item_count; i++) {
        taskbar_items[i].active = false;
    }
    taskbar_items[slot].active = true;
    
    taskbar_item_count++;
}

/* Remove window from taskbar */
void desktop_remove_taskbar_item(u32 window_id) {
    for (u32 i = 0; i < taskbar_item_count; i++) {
        if (taskbar_items[i].window_id == window_id) {
            /* Shift remaining items */
            for (u32 j = i; j < taskbar_item_count - 1; j++) {
                taskbar_items[j] = taskbar_items[j + 1];
            }
            taskbar_item_count--;
            break;
        }
    }
}

/* Handle desktop click */
void desktop_handle_click(u32 x, u32 y) {
    struct framebuffer* fb = fb_get_info();
    u32 taskbar_y = fb->height - TASKBAR_HEIGHT;
    
    /* Check if click is on taskbar */
    if (y >= taskbar_y) {
        /* Check start button */
        if (x >= 4 && x <= 84) {
            start_menu_open = !start_menu_open;
            return;
        }
        
        /* Check taskbar items */
        u32 item_x = 100;
        for (u32 i = 0; i < taskbar_item_count; i++) {
            if (x >= item_x && x <= item_x + 120) {
                /* Focus this window */
                wm_set_window_focus(taskbar_items[i].window_id, true);
                
                /* Update active state */
                for (u32 j = 0; j < taskbar_item_count; j++) {
                    taskbar_items[j].active = (j == i);
                }
                return;
            }
            item_x += 124;
        }
    }
    
    /* Check desktop icons */
    for (u32 i = 0; i < desktop_icon_count; i++) {
        if (desktop_icons[i].visible &&
            x >= desktop_icons[i].x && x <= desktop_icons[i].x + ICON_SIZE &&
            y >= desktop_icons[i].y && y <= desktop_icons[i].y + ICON_SIZE) {
            
            /* Launch application */
            desktop_launch_app(desktop_icons[i].command);
            return;
        }
    }
    
    /* Close start menu if clicking elsewhere */
    start_menu_open = false;
}

/* Launch application */
void desktop_launch_app(const char* command) {
    if (strcmp(command, "terminal") == 0) {
        u32 win_id = wm_create_window(200, 150, 600, 400, "Terminal", 
                                     WINDOW_RESIZABLE | WINDOW_CLOSABLE | WINDOW_MINIMIZABLE);
        desktop_add_taskbar_item(win_id, "Terminal");
        app_launch_terminal(win_id);
    } else if (strcmp(command, "files") == 0) {
        u32 win_id = wm_create_window(300, 200, 700, 500, "File Manager",
                                     WINDOW_RESIZABLE | WINDOW_CLOSABLE | WINDOW_MINIMIZABLE);
        desktop_add_taskbar_item(win_id, "Files");
        app_launch_file_manager(win_id);
    } else if (strcmp(command, "editor") == 0) {
        u32 win_id = wm_create_window(250, 100, 650, 450, "Text Editor",
                                     WINDOW_RESIZABLE | WINDOW_CLOSABLE | WINDOW_MINIMIZABLE);
        desktop_add_taskbar_item(win_id, "Editor");
        app_launch_text_editor(win_id);
    }
    /* Add more applications as needed */
}

/* Render desktop */
void desktop_render(void) {
    /* Draw wallpaper */
    draw_wallpaper();
    
    /* Draw desktop icons */
    for (u32 i = 0; i < desktop_icon_count; i++) {
        draw_desktop_icon(&desktop_icons[i]);
    }
    
    /* Draw taskbar */
    draw_taskbar();
    
    /* Draw start menu if open */
    draw_start_menu();
}

/* Update desktop time */
void desktop_update_time(const char* time, const char* date) {
    strcpy(current_time, time);
    strcpy(current_date, date);
}
