#include "kronos.h"

/* Main GUI System for Kronos OS */

static bool gui_mode = false;
static bool mouse_enabled = false;

/* Mouse state */
static u32 mouse_x = 512;
static u32 mouse_y = 384;
static bool mouse_left = false;
static bool mouse_right = false;

/* Initialize GUI system */
void gui_init(void) {
    vga_puts("Initializing GUI system...\n");
    
    /* Initialize framebuffer */
    vga_puts("Setting up framebuffer... ");
    fb_init();
    vga_puts("OK\n");
    
    /* Initialize window manager */
    vga_puts("Starting window manager... ");
    wm_init();
    vga_puts("OK\n");
    
    /* Initialize desktop environment */
    vga_puts("Loading desktop environment... ");
    desktop_init();
    vga_puts("OK\n");
    
    /* Initialize applications */
    vga_puts("Initializing applications... ");
    app_terminal_init();
    app_file_manager_init();
    app_text_editor_init();
    vga_puts("OK\n");
    
    gui_mode = true;
    mouse_enabled = true;
    
    vga_puts("GUI system ready!\n");
    vga_puts("Switching to graphical mode...\n\n");
    
    /* Small delay to show messages */
    for (volatile u32 i = 0; i < 10000000; i++);
    
    /* Start GUI main loop */
    gui_main_loop();
}

/* Main GUI event loop */
void gui_main_loop(void) {
    while (gui_mode) {
        /* Handle input events */
        gui_handle_input();
        
        /* Update desktop time */
        static u32 time_counter = 0;
        if (++time_counter > 1000000) {  /* Update every ~1 second */
            time_counter = 0;
            gui_update_time();
        }
        
        /* Render everything */
        gui_render();
        
        /* Small delay to prevent 100% CPU usage */
        for (volatile u32 i = 0; i < 1000; i++);
    }
}

/* Handle input events */
void gui_handle_input(void) {
    /* Handle keyboard input */
    if (keyboard_has_input()) {
        char c = keyboard_getchar();
        gui_handle_keyboard(c);
    }
    
    /* Handle mouse input (simulated for now) */
    gui_handle_mouse();
}

/* Handle keyboard input */
void gui_handle_keyboard(char c) {
    /* Special key combinations */
    static bool ctrl_pressed = false;
    static bool alt_pressed = false;
    
    if (c == 1) {  /* Ctrl+A */
        ctrl_pressed = true;
        return;
    } else if (c == 27) {  /* Alt */
        alt_pressed = true;
        return;
    }
    
    /* System shortcuts */
    if (ctrl_pressed && alt_pressed) {
        if (c == 't') {  /* Ctrl+Alt+T - Open terminal */
            desktop_launch_app("terminal");
        } else if (c == 'f') {  /* Ctrl+Alt+F - Open file manager */
            desktop_launch_app("files");
        } else if (c == 'e') {  /* Ctrl+Alt+E - Open text editor */
            desktop_launch_app("editor");
        } else if (c == 'q') {  /* Ctrl+Alt+Q - Quit GUI */
            gui_mode = false;
            return;
        }
    }
    
    /* Send input to focused window */
    /* This would be implemented with proper window focus tracking */
    
    ctrl_pressed = false;
    alt_pressed = false;
}

/* Handle mouse input (simulated) */
void gui_handle_mouse(void) {
    /* Simulate mouse movement and clicks for demo */
    static u32 mouse_counter = 0;
    static bool click_state = false;
    
    mouse_counter++;
    
    /* Simulate occasional mouse clicks for demo */
    if (mouse_counter % 5000000 == 0) {
        click_state = !click_state;
        
        if (click_state) {
            /* Simulate click on desktop */
            desktop_handle_click(mouse_x, mouse_y);
            wm_handle_mouse_click(mouse_x, mouse_y, true);
        }
    }
    
    /* Simulate mouse movement */
    if (mouse_counter % 100000 == 0) {
        mouse_x += (mouse_counter % 3) - 1;  /* Random movement */
        mouse_y += (mouse_counter % 3) - 1;
        
        /* Keep mouse in bounds */
        if (mouse_x >= 1024) mouse_x = 1023;
        if (mouse_y >= 768) mouse_y = 767;
    }
}

/* Update system time */
void gui_update_time(void) {
    /* Simple time simulation */
    static u32 hours = 12;
    static u32 minutes = 0;
    static u32 seconds = 0;
    
    seconds++;
    if (seconds >= 60) {
        seconds = 0;
        minutes++;
        if (minutes >= 60) {
            minutes = 0;
            hours++;
            if (hours >= 24) {
                hours = 0;
            }
        }
    }
    
    char time_str[16];
    char date_str[16];
    
    sprintf(time_str, "%02d:%02d", hours, minutes);
    strcpy(date_str, "2025-06-28");
    
    desktop_update_time(time_str, date_str);
}

/* Render GUI */
void gui_render(void) {
    /* Render desktop */
    desktop_render();
    
    /* Render windows */
    wm_render();
    
    /* Render mouse cursor */
    gui_render_cursor();
}

/* Render mouse cursor */
void gui_render_cursor(void) {
    if (!mouse_enabled) return;
    
    /* Draw simple arrow cursor */
    u32 cursor_color = COLOR_WHITE;
    u32 outline_color = COLOR_BLACK;
    
    /* Cursor outline */
    fb_set_pixel(mouse_x, mouse_y, outline_color);
    fb_set_pixel(mouse_x + 1, mouse_y + 1, outline_color);
    fb_set_pixel(mouse_x + 2, mouse_y + 2, outline_color);
    fb_set_pixel(mouse_x + 3, mouse_y + 3, outline_color);
    fb_set_pixel(mouse_x + 4, mouse_y + 4, outline_color);
    fb_set_pixel(mouse_x, mouse_y + 1, outline_color);
    fb_set_pixel(mouse_x, mouse_y + 2, outline_color);
    fb_set_pixel(mouse_x + 1, mouse_y + 3, outline_color);
    fb_set_pixel(mouse_x + 2, mouse_y + 4, outline_color);
    
    /* Cursor fill */
    fb_set_pixel(mouse_x + 1, mouse_y + 1, cursor_color);
    fb_set_pixel(mouse_x + 2, mouse_y + 2, cursor_color);
    fb_set_pixel(mouse_x + 3, mouse_y + 3, cursor_color);
    fb_set_pixel(mouse_x + 1, mouse_y + 2, cursor_color);
    fb_set_pixel(mouse_x + 2, mouse_y + 3, cursor_color);
}

/* Switch between text and GUI mode */
void gui_toggle_mode(void) {
    if (gui_mode) {
        /* Switch to text mode */
        gui_mode = false;
        vga_clear();
        vga_puts("Switched to text mode.\n");
        vga_puts("Type 'gui' to return to graphical mode.\n");
        shell_run();  /* Return to shell */
    } else {
        /* Switch to GUI mode */
        gui_init();
    }
}

/* Check if GUI is active */
bool gui_is_active(void) {
    return gui_mode;
}

/* Get mouse position */
void gui_get_mouse_pos(u32* x, u32* y) {
    *x = mouse_x;
    *y = mouse_y;
}

/* Set mouse position */
void gui_set_mouse_pos(u32 x, u32 y) {
    if (x < 1024 && y < 768) {
        mouse_x = x;
        mouse_y = y;
    }
}

/* Handle window events */
void gui_handle_window_event(u32 window_id, u32 event_type, u32 param1, u32 param2) {
    switch (event_type) {
        case GUI_EVENT_CLOSE:
            wm_destroy_window(window_id);
            desktop_remove_taskbar_item(window_id);
            break;
            
        case GUI_EVENT_MINIMIZE:
            /* Minimize window */
            break;
            
        case GUI_EVENT_MAXIMIZE:
            /* Maximize window */
            break;
            
        case GUI_EVENT_FOCUS:
            wm_set_window_focus(window_id, true);
            break;
            
        case GUI_EVENT_MOVE:
            wm_move_window(window_id, param1, param2);
            break;
            
        case GUI_EVENT_RESIZE:
            wm_resize_window(window_id, param1, param2);
            break;
    }
}

/* GUI event types */
#define GUI_EVENT_CLOSE     1
#define GUI_EVENT_MINIMIZE  2
#define GUI_EVENT_MAXIMIZE  3
#define GUI_EVENT_FOCUS     4
#define GUI_EVENT_MOVE      5
#define GUI_EVENT_RESIZE    6

/* Show GUI demo */
void gui_show_demo(void) {
    if (!gui_mode) {
        vga_puts("Starting GUI demo...\n");
        gui_init();
        
        /* Launch demo applications */
        desktop_launch_app("terminal");
        
        /* Small delay */
        for (volatile u32 i = 0; i < 50000000; i++);
        
        desktop_launch_app("files");
        
        /* Small delay */
        for (volatile u32 i = 0; i < 50000000; i++);
        
        desktop_launch_app("editor");
    }
}
