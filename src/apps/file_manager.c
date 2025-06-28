#include "kronos.h"

/* File Manager Application for Kronos OS */

#define MAX_FILES 50
#define FILE_NAME_LEN 64

/* File entry structure */
struct file_entry {
    char name[FILE_NAME_LEN];
    u32 size;
    bool is_directory;
    u32 icon_color;
} file_list[MAX_FILES];

/* File manager state */
struct file_manager {
    u32 window_id;
    char current_path[256];
    u32 file_count;
    u32 selected_file;
    u32 scroll_offset;
    bool active;
} file_managers[2];  /* Support 2 file manager instances */

static u32 fm_count = 0;

/* Initialize file manager */
void app_file_manager_init(void) {
    for (u32 i = 0; i < 2; i++) {
        file_managers[i].active = false;
        file_managers[i].window_id = 0;
    }
    fm_count = 0;
}

/* Launch file manager */
void app_launch_file_manager(u32 window_id) {
    if (fm_count >= 2) return;
    
    u32 slot = fm_count++;
    file_managers[slot].window_id = window_id;
    file_managers[slot].file_count = 0;
    file_managers[slot].selected_file = 0;
    file_managers[slot].scroll_offset = 0;
    file_managers[slot].active = true;
    strcpy(file_managers[slot].current_path, "/home/user");
    
    /* Load default file list */
    fm_load_directory(&file_managers[slot]);
}

/* Load directory contents */
void fm_load_directory(struct file_manager* fm) {
    /* Simulate file system - in real implementation, 
       this would read from actual file system */
    
    fm->file_count = 0;
    
    /* Add parent directory if not root */
    if (strcmp(fm->current_path, "/") != 0) {
        strcpy(file_list[fm->file_count].name, "..");
        file_list[fm->file_count].size = 0;
        file_list[fm->file_count].is_directory = true;
        file_list[fm->file_count].icon_color = COLOR_BLUE;
        fm->file_count++;
    }
    
    /* Add sample directories */
    const char* directories[] = {
        "Desktop", "Documents", "Downloads", "Pictures", "Music", "Videos"
    };
    
    for (u32 i = 0; i < 6; i++) {
        strcpy(file_list[fm->file_count].name, directories[i]);
        file_list[fm->file_count].size = 0;
        file_list[fm->file_count].is_directory = true;
        file_list[fm->file_count].icon_color = COLOR_BLUE;
        fm->file_count++;
    }
    
    /* Add sample files */
    const char* files[] = {
        "readme.txt", "kronos.conf", "system.log", "photo.jpg", "music.mp3"
    };
    u32 sizes[] = {1024, 512, 2048, 1048576, 3145728};
    
    for (u32 i = 0; i < 5; i++) {
        strcpy(file_list[fm->file_count].name, files[i]);
        file_list[fm->file_count].size = sizes[i];
        file_list[fm->file_count].is_directory = false;
        
        /* Set icon color based on file type */
        if (strstr(files[i], ".txt") || strstr(files[i], ".conf") || strstr(files[i], ".log")) {
            file_list[fm->file_count].icon_color = COLOR_GREEN;
        } else if (strstr(files[i], ".jpg") || strstr(files[i], ".png")) {
            file_list[fm->file_count].icon_color = COLOR_PURPLE;
        } else if (strstr(files[i], ".mp3") || strstr(files[i], ".wav")) {
            file_list[fm->file_count].icon_color = COLOR_ORANGE;
        } else {
            file_list[fm->file_count].icon_color = COLOR_GRAY;
        }
        
        fm->file_count++;
    }
}

/* Format file size */
void fm_format_size(u32 size, char* buffer) {
    if (size < 1024) {
        sprintf(buffer, "%d B", size);
    } else if (size < 1024 * 1024) {
        sprintf(buffer, "%d KB", size / 1024);
    } else {
        sprintf(buffer, "%d MB", size / (1024 * 1024));
    }
}

/* Draw file icon */
void fm_draw_file_icon(u32* buffer, u32 buffer_width, u32 x, u32 y, 
                      struct file_entry* file) {
    u32 icon_size = 16;
    
    /* Draw icon background */
    for (u32 dy = 0; dy < icon_size; dy++) {
        for (u32 dx = 0; dx < icon_size; dx++) {
            u32 px = x + dx;
            u32 py = y + dy;
            if (px < buffer_width && py < 400) {
                buffer[py * buffer_width + px] = file->icon_color;
            }
        }
    }
    
    /* Draw icon symbol */
    if (file->is_directory) {
        /* Folder icon - draw folder shape */
        for (u32 dx = 2; dx < 14; dx++) {
            for (u32 dy = 4; dy < 12; dy++) {
                u32 px = x + dx;
                u32 py = y + dy;
                if (px < buffer_width && py < 400) {
                    buffer[py * buffer_width + px] = COLOR_YELLOW;
                }
            }
        }
        /* Folder tab */
        for (u32 dx = 2; dx < 8; dx++) {
            for (u32 dy = 2; dy < 4; dy++) {
                u32 px = x + dx;
                u32 py = y + dy;
                if (px < buffer_width && py < 400) {
                    buffer[py * buffer_width + px] = COLOR_YELLOW;
                }
            }
        }
    } else {
        /* File icon - draw document shape */
        for (u32 dx = 4; dx < 12; dx++) {
            for (u32 dy = 2; dy < 14; dy++) {
                u32 px = x + dx;
                u32 py = y + dy;
                if (px < buffer_width && py < 400) {
                    buffer[py * buffer_width + px] = COLOR_WHITE;
                }
            }
        }
        /* Document lines */
        for (u32 dx = 6; dx < 10; dx++) {
            u32 px = x + dx;
            u32 py1 = y + 4;
            u32 py2 = y + 6;
            u32 py3 = y + 8;
            if (px < buffer_width) {
                if (py1 < 400) buffer[py1 * buffer_width + px] = COLOR_BLACK;
                if (py2 < 400) buffer[py2 * buffer_width + px] = COLOR_BLACK;
                if (py3 < 400) buffer[py3 * buffer_width + px] = COLOR_BLACK;
            }
        }
    }
}

/* Render file manager */
void fm_render(struct file_manager* fm) {
    if (!fm->active) return;
    
    u32* buffer = wm_get_window_buffer(fm->window_id);
    if (!buffer) return;
    
    u32 win_width = 700;
    u32 win_height = 500;
    
    /* Clear window buffer */
    for (u32 i = 0; i < win_width * win_height; i++) {
        buffer[i] = COLOR_WHITE;
    }
    
    /* Draw toolbar */
    for (u32 x = 0; x < win_width; x++) {
        for (u32 y = 0; y < 30; y++) {
            buffer[y * win_width + x] = COLOR_LIGHT_GRAY;
        }
    }
    
    /* Draw address bar */
    fm_draw_text_to_buffer(buffer, win_width, 10, 8, fm->current_path, COLOR_BLACK);
    
    /* Draw file list */
    u32 item_height = 24;
    u32 start_y = 40;
    
    for (u32 i = fm->scroll_offset; i < fm->file_count && i < fm->scroll_offset + 15; i++) {
        u32 y = start_y + (i - fm->scroll_offset) * item_height;
        
        /* Highlight selected item */
        if (i == fm->selected_file) {
            for (u32 x = 0; x < win_width; x++) {
                for (u32 dy = 0; dy < item_height; dy++) {
                    u32 py = y + dy;
                    if (py < win_height) {
                        buffer[py * win_width + x] = UBUNTU_LIGHT;
                    }
                }
            }
        }
        
        /* Draw file icon */
        fm_draw_file_icon(buffer, win_width, 10, y + 4, &file_list[i]);
        
        /* Draw file name */
        fm_draw_text_to_buffer(buffer, win_width, 35, y + 8, file_list[i].name, COLOR_BLACK);
        
        /* Draw file size for files */
        if (!file_list[i].is_directory) {
            char size_str[32];
            fm_format_size(file_list[i].size, size_str);
            fm_draw_text_to_buffer(buffer, win_width, 400, y + 8, size_str, COLOR_GRAY);
        }
    }
    
    /* Draw scrollbar if needed */
    if (fm->file_count > 15) {
        u32 scrollbar_x = win_width - 20;
        u32 scrollbar_height = 15 * item_height;
        u32 thumb_height = (15 * scrollbar_height) / fm->file_count;
        u32 thumb_y = start_y + (fm->scroll_offset * scrollbar_height) / fm->file_count;
        
        /* Scrollbar background */
        for (u32 x = scrollbar_x; x < scrollbar_x + 16; x++) {
            for (u32 y = start_y; y < start_y + scrollbar_height; y++) {
                if (x < win_width && y < win_height) {
                    buffer[y * win_width + x] = COLOR_LIGHT_GRAY;
                }
            }
        }
        
        /* Scrollbar thumb */
        for (u32 x = scrollbar_x + 2; x < scrollbar_x + 14; x++) {
            for (u32 y = thumb_y; y < thumb_y + thumb_height; y++) {
                if (x < win_width && y < win_height) {
                    buffer[y * win_width + x] = COLOR_GRAY;
                }
            }
        }
    }
}

/* Draw text to buffer (simplified) */
void fm_draw_text_to_buffer(u32* buffer, u32 buffer_width, u32 x, u32 y, 
                           const char* text, u32 color) {
    /* Simple text rendering - each character is 8x8 pixels */
    u32 pos_x = x;
    
    while (*text && pos_x < buffer_width - 8) {
        /* Simple character rendering */
        for (u32 dy = 0; dy < 8; dy++) {
            for (u32 dx = 0; dx < 8; dx++) {
                /* Simple pattern based on character */
                bool pixel = false;
                if (*text >= 'A' && *text <= 'Z') {
                    pixel = ((dx + dy + *text) % 3 == 0);
                } else if (*text >= 'a' && *text <= 'z') {
                    pixel = ((dx * dy + *text) % 4 == 0);
                } else if (*text >= '0' && *text <= '9') {
                    pixel = ((dx + dy * 2) % 3 == 0);
                } else if (*text == '.') {
                    pixel = (dx == 4 && dy == 6);
                } else if (*text == '/') {
                    pixel = (dx + dy == 7);
                } else if (*text != ' ') {
                    pixel = ((dx + dy) % 2 == 0);
                }
                
                if (pixel && pos_x + dx < buffer_width && y + dy < 400) {
                    buffer[(y + dy) * buffer_width + (pos_x + dx)] = color;
                }
            }
        }
        pos_x += 8;
        text++;
    }
}

/* Handle file manager input */
void fm_handle_input(struct file_manager* fm, u32 key) {
    switch (key) {
        case KEY_UP:
            if (fm->selected_file > 0) {
                fm->selected_file--;
                if (fm->selected_file < fm->scroll_offset) {
                    fm->scroll_offset = fm->selected_file;
                }
            }
            break;
            
        case KEY_DOWN:
            if (fm->selected_file < fm->file_count - 1) {
                fm->selected_file++;
                if (fm->selected_file >= fm->scroll_offset + 15) {
                    fm->scroll_offset = fm->selected_file - 14;
                }
            }
            break;
            
        case KEY_ENTER:
            /* Open selected file/directory */
            if (fm->selected_file < fm->file_count) {
                struct file_entry* entry = &file_list[fm->selected_file];
                if (entry->is_directory) {
                    if (strcmp(entry->name, "..") == 0) {
                        /* Go to parent directory */
                        char* last_slash = strrchr(fm->current_path, '/');
                        if (last_slash && last_slash != fm->current_path) {
                            *last_slash = '\0';
                        }
                    } else {
                        /* Enter directory */
                        if (strcmp(fm->current_path, "/") != 0) {
                            strcat(fm->current_path, "/");
                        }
                        strcat(fm->current_path, entry->name);
                    }
                    fm->selected_file = 0;
                    fm->scroll_offset = 0;
                    fm_load_directory(fm);
                }
            }
            break;
    }
}

/* Find file manager by window ID */
struct file_manager* fm_find_by_window(u32 window_id) {
    for (u32 i = 0; i < 2; i++) {
        if (file_managers[i].active && file_managers[i].window_id == window_id) {
            return &file_managers[i];
        }
    }
    return NULL;
}
