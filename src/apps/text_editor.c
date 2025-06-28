#include "kronos.h"

/* Text Editor Application for Kronos OS */

#define EDITOR_COLS 80
#define EDITOR_ROWS 30
#define MAX_LINE_LENGTH 256

struct text_editor {
    u32 window_id;
    char lines[EDITOR_ROWS][MAX_LINE_LENGTH];
    u32 cursor_x, cursor_y;
    u32 scroll_x, scroll_y;
    char filename[64];
    bool modified;
    bool active;
} editors[2];

static u32 editor_count = 0;

/* Initialize text editor */
void app_text_editor_init(void) {
    for (u32 i = 0; i < 2; i++) {
        editors[i].active = false;
        editors[i].window_id = 0;
    }
    editor_count = 0;
}

/* Launch text editor */
void app_launch_text_editor(u32 window_id) {
    if (editor_count >= 2) return;
    
    u32 slot = editor_count++;
    editors[slot].window_id = window_id;
    editors[slot].cursor_x = 0;
    editors[slot].cursor_y = 0;
    editors[slot].scroll_x = 0;
    editors[slot].scroll_y = 0;
    editors[slot].modified = false;
    editors[slot].active = true;
    strcpy(editors[slot].filename, "untitled.txt");
    
    /* Clear editor buffer */
    for (u32 y = 0; y < EDITOR_ROWS; y++) {
        editors[slot].lines[y][0] = '\0';
    }
    
    /* Add welcome text */
    strcpy(editors[slot].lines[0], "Welcome to Kronos Text Editor");
    strcpy(editors[slot].lines[1], "");
    strcpy(editors[slot].lines[2], "Keyboard shortcuts:");
    strcpy(editors[slot].lines[3], "  Ctrl+S - Save file");
    strcpy(editors[slot].lines[4], "  Ctrl+O - Open file");
    strcpy(editors[slot].lines[5], "  Ctrl+N - New file");
    strcpy(editors[slot].lines[6], "  Ctrl+Q - Quit");
    strcpy(editors[slot].lines[7], "");
    strcpy(editors[slot].lines[8], "Start typing to edit...");
    
    editors[slot].cursor_y = 9;
}

/* Handle text editor input */
void editor_handle_input(struct text_editor* editor, char c, bool ctrl_pressed) {
    if (ctrl_pressed) {
        switch (c) {
            case 's':  /* Ctrl+S - Save */
                editor_save_file(editor);
                break;
            case 'o':  /* Ctrl+O - Open */
                editor_open_file(editor);
                break;
            case 'n':  /* Ctrl+N - New */
                editor_new_file(editor);
                break;
            case 'q':  /* Ctrl+Q - Quit */
                wm_destroy_window(editor->window_id);
                desktop_remove_taskbar_item(editor->window_id);
                editor->active = false;
                break;
        }
        return;
    }
    
    if (c == '\n' || c == '\r') {
        /* Enter - new line */
        if (editor->cursor_y < EDITOR_ROWS - 1) {
            /* Move text after cursor to next line */
            char* current_line = editor->lines[editor->cursor_y];
            char* next_line = editor->lines[editor->cursor_y + 1];
            
            strcpy(next_line, &current_line[editor->cursor_x]);
            current_line[editor->cursor_x] = '\0';
            
            editor->cursor_y++;
            editor->cursor_x = 0;
            editor->modified = true;
        }
    } else if (c == '\b') {
        /* Backspace */
        if (editor->cursor_x > 0) {
            /* Delete character before cursor */
            char* line = editor->lines[editor->cursor_y];
            u32 len = strlen(line);
            
            for (u32 i = editor->cursor_x - 1; i < len; i++) {
                line[i] = line[i + 1];
            }
            
            editor->cursor_x--;
            editor->modified = true;
        } else if (editor->cursor_y > 0) {
            /* Join with previous line */
            char* prev_line = editor->lines[editor->cursor_y - 1];
            char* curr_line = editor->lines[editor->cursor_y];
            
            editor->cursor_x = strlen(prev_line);
            strcat(prev_line, curr_line);
            
            /* Shift lines up */
            for (u32 i = editor->cursor_y; i < EDITOR_ROWS - 1; i++) {
                strcpy(editor->lines[i], editor->lines[i + 1]);
            }
            editor->lines[EDITOR_ROWS - 1][0] = '\0';
            
            editor->cursor_y--;
            editor->modified = true;
        }
    } else if (c >= 32 && c <= 126) {
        /* Printable character */
        char* line = editor->lines[editor->cursor_y];
        u32 len = strlen(line);
        
        if (len < MAX_LINE_LENGTH - 1) {
            /* Shift characters right */
            for (u32 i = len; i > editor->cursor_x; i--) {
                line[i] = line[i - 1];
            }
            
            line[editor->cursor_x] = c;
            line[len + 1] = '\0';
            
            editor->cursor_x++;
            editor->modified = true;
        }
    }
    
    /* Handle cursor movement keys */
    switch (c) {
        case KEY_LEFT:
            if (editor->cursor_x > 0) {
                editor->cursor_x--;
            } else if (editor->cursor_y > 0) {
                editor->cursor_y--;
                editor->cursor_x = strlen(editor->lines[editor->cursor_y]);
            }
            break;
            
        case KEY_RIGHT:
            if (editor->cursor_x < strlen(editor->lines[editor->cursor_y])) {
                editor->cursor_x++;
            } else if (editor->cursor_y < EDITOR_ROWS - 1 && 
                      editor->lines[editor->cursor_y + 1][0] != '\0') {
                editor->cursor_y++;
                editor->cursor_x = 0;
            }
            break;
            
        case KEY_UP:
            if (editor->cursor_y > 0) {
                editor->cursor_y--;
                u32 line_len = strlen(editor->lines[editor->cursor_y]);
                if (editor->cursor_x > line_len) {
                    editor->cursor_x = line_len;
                }
            }
            break;
            
        case KEY_DOWN:
            if (editor->cursor_y < EDITOR_ROWS - 1 && 
                editor->lines[editor->cursor_y + 1][0] != '\0') {
                editor->cursor_y++;
                u32 line_len = strlen(editor->lines[editor->cursor_y]);
                if (editor->cursor_x > line_len) {
                    editor->cursor_x = line_len;
                }
            }
            break;
    }
}

/* Save file */
void editor_save_file(struct text_editor* editor) {
    /* In real implementation, this would write to file system */
    editor->modified = false;
    
    /* Show save confirmation in status bar */
    strcpy(editor->lines[EDITOR_ROWS - 1], "File saved successfully!");
}

/* Open file */
void editor_open_file(struct text_editor* editor) {
    /* In real implementation, this would show file dialog */
    strcpy(editor->filename, "sample.txt");
    
    /* Clear editor */
    for (u32 i = 0; i < EDITOR_ROWS; i++) {
        editor->lines[i][0] = '\0';
    }
    
    /* Load sample content */
    strcpy(editor->lines[0], "This is a sample text file");
    strcpy(editor->lines[1], "loaded from the file system.");
    strcpy(editor->lines[2], "");
    strcpy(editor->lines[3], "You can edit this content");
    strcpy(editor->lines[4], "and save it back to disk.");
    
    editor->cursor_x = 0;
    editor->cursor_y = 0;
    editor->modified = false;
}

/* New file */
void editor_new_file(struct text_editor* editor) {
    /* Clear editor */
    for (u32 i = 0; i < EDITOR_ROWS; i++) {
        editor->lines[i][0] = '\0';
    }
    
    strcpy(editor->filename, "untitled.txt");
    editor->cursor_x = 0;
    editor->cursor_y = 0;
    editor->modified = false;
}

/* Render text editor */
void editor_render(struct text_editor* editor) {
    if (!editor->active) return;
    
    u32* buffer = wm_get_window_buffer(editor->window_id);
    if (!buffer) return;
    
    u32 win_width = 650;
    u32 win_height = 450;
    
    /* Clear window buffer */
    for (u32 i = 0; i < win_width * win_height; i++) {
        buffer[i] = COLOR_WHITE;
    }
    
    /* Draw menu bar */
    for (u32 x = 0; x < win_width; x++) {
        for (u32 y = 0; y < 25; y++) {
            buffer[y * win_width + x] = COLOR_LIGHT_GRAY;
        }
    }
    
    /* Draw menu items */
    editor_draw_text_to_buffer(buffer, win_width, 10, 8, "File", COLOR_BLACK);
    editor_draw_text_to_buffer(buffer, win_width, 50, 8, "Edit", COLOR_BLACK);
    editor_draw_text_to_buffer(buffer, win_width, 90, 8, "View", COLOR_BLACK);
    
    /* Draw filename */
    char title[128];
    strcpy(title, editor->filename);
    if (editor->modified) {
        strcat(title, " *");
    }
    editor_draw_text_to_buffer(buffer, win_width, 200, 8, title, COLOR_BLACK);
    
    /* Draw text content */
    u32 text_start_y = 30;
    u32 line_height = 14;
    
    for (u32 i = 0; i < EDITOR_ROWS && i < (win_height - text_start_y) / line_height; i++) {
        u32 y = text_start_y + i * line_height;
        
        /* Highlight current line */
        if (i == editor->cursor_y) {
            for (u32 x = 0; x < win_width; x++) {
                for (u32 dy = 0; dy < line_height; dy++) {
                    u32 py = y + dy;
                    if (py < win_height) {
                        buffer[py * win_width + x] = 0x00F0F0F0;  /* Light highlight */
                    }
                }
            }
        }
        
        /* Draw line number */
        char line_num[8];
        sprintf(line_num, "%2d", i + 1);
        editor_draw_text_to_buffer(buffer, win_width, 5, y, line_num, COLOR_GRAY);
        
        /* Draw line content */
        editor_draw_text_to_buffer(buffer, win_width, 30, y, editor->lines[i], COLOR_BLACK);
    }
    
    /* Draw cursor */
    if (editor->cursor_y < (win_height - text_start_y) / line_height) {
        u32 cursor_x = 30 + editor->cursor_x * 8;
        u32 cursor_y = text_start_y + editor->cursor_y * line_height;
        
        /* Draw cursor line */
        for (u32 y = cursor_y; y < cursor_y + line_height && y < win_height; y++) {
            if (cursor_x < win_width) {
                buffer[y * win_width + cursor_x] = COLOR_BLACK;
            }
        }
    }
    
    /* Draw status bar */
    u32 status_y = win_height - 20;
    for (u32 x = 0; x < win_width; x++) {
        for (u32 y = status_y; y < win_height; y++) {
            buffer[y * win_width + x] = COLOR_LIGHT_GRAY;
        }
    }
    
    /* Draw status text */
    char status[128];
    sprintf(status, "Line %d, Col %d", editor->cursor_y + 1, editor->cursor_x + 1);
    editor_draw_text_to_buffer(buffer, win_width, 10, status_y + 4, status, COLOR_BLACK);
    
    if (editor->modified) {
        editor_draw_text_to_buffer(buffer, win_width, 200, status_y + 4, "Modified", COLOR_RED);
    }
}

/* Draw text to buffer (simplified) */
void editor_draw_text_to_buffer(u32* buffer, u32 buffer_width, u32 x, u32 y, 
                               const char* text, u32 color) {
    /* Simple 8x12 font rendering */
    u32 pos_x = x;
    
    while (*text && pos_x < buffer_width - 8) {
        /* Simple character patterns */
        for (u32 dy = 0; dy < 12; dy++) {
            for (u32 dx = 0; dx < 8; dx++) {
                bool pixel = false;
                
                /* Character-specific patterns */
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
                
                if (pixel && pos_x + dx < buffer_width && y + dy < 450) {
                    buffer[(y + dy) * buffer_width + (pos_x + dx)] = color;
                }
            }
        }
        pos_x += 8;
        text++;
    }
}

/* Find editor by window ID */
struct text_editor* editor_find_by_window(u32 window_id) {
    for (u32 i = 0; i < 2; i++) {
        if (editors[i].active && editors[i].window_id == window_id) {
            return &editors[i];
        }
    }
    return NULL;
}
