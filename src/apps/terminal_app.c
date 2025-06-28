#include "kronos.h"

/* Terminal Application for Kronos OS GUI */

#define TERM_COLS 75
#define TERM_ROWS 25
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

struct terminal_app {
    u32 window_id;
    char buffer[TERM_ROWS][TERM_COLS];
    u32 cursor_x, cursor_y;
    u32 scroll_offset;
    char input_line[256];
    u32 input_pos;
    bool active;
} terminals[4];  /* Support up to 4 terminal instances */

static u32 terminal_count = 0;

/* Initialize terminal application */
void app_terminal_init(void) {
    for (u32 i = 0; i < 4; i++) {
        terminals[i].active = false;
        terminals[i].window_id = 0;
    }
    terminal_count = 0;
}

/* Launch new terminal */
void app_launch_terminal(u32 window_id) {
    if (terminal_count >= 4) return;
    
    u32 slot = terminal_count++;
    terminals[slot].window_id = window_id;
    terminals[slot].cursor_x = 0;
    terminals[slot].cursor_y = 0;
    terminals[slot].scroll_offset = 0;
    terminals[slot].input_pos = 0;
    terminals[slot].active = true;
    
    /* Clear terminal buffer */
    for (u32 y = 0; y < TERM_ROWS; y++) {
        for (u32 x = 0; x < TERM_COLS; x++) {
            terminals[slot].buffer[y][x] = ' ';
        }
    }
    
    /* Add welcome message */
    const char* welcome[] = {
        "Kronos OS Terminal v1.0",
        "Type 'help' for available commands.",
        ""
    };
    
    for (u32 i = 0; i < 3; i++) {
        terminal_print_line(&terminals[slot], welcome[i]);
    }
    
    terminal_print_prompt(&terminals[slot]);
}

/* Print line to terminal */
void terminal_print_line(struct terminal_app* term, const char* line) {
    u32 len = strlen(line);
    
    for (u32 i = 0; i < len && term->cursor_x < TERM_COLS; i++) {
        term->buffer[term->cursor_y][term->cursor_x++] = line[i];
    }
    
    /* Move to next line */
    term->cursor_x = 0;
    term->cursor_y++;
    
    /* Scroll if necessary */
    if (term->cursor_y >= TERM_ROWS) {
        terminal_scroll(term);
        term->cursor_y = TERM_ROWS - 1;
    }
}

/* Print prompt */
void terminal_print_prompt(struct terminal_app* term) {
    terminal_print_line(term, "kronos$ ");
    term->cursor_x = 8;  /* Position after prompt */
}

/* Scroll terminal up */
void terminal_scroll(struct terminal_app* term) {
    /* Move all lines up */
    for (u32 y = 0; y < TERM_ROWS - 1; y++) {
        for (u32 x = 0; x < TERM_COLS; x++) {
            term->buffer[y][x] = term->buffer[y + 1][x];
        }
    }
    
    /* Clear bottom line */
    for (u32 x = 0; x < TERM_COLS; x++) {
        term->buffer[TERM_ROWS - 1][x] = ' ';
    }
}

/* Handle terminal input */
void terminal_handle_input(struct terminal_app* term, char c) {
    if (c == '\n' || c == '\r') {
        /* Execute command */
        term->input_line[term->input_pos] = '\0';
        
        /* Move cursor to end of line */
        term->cursor_x = TERM_COLS;
        terminal_print_line(term, "");
        
        /* Execute command */
        terminal_execute_command(term, term->input_line);
        
        /* Reset input */
        term->input_pos = 0;
        terminal_print_prompt(term);
    } else if (c == '\b') {
        /* Backspace */
        if (term->input_pos > 0) {
            term->input_pos--;
            term->cursor_x--;
            term->buffer[term->cursor_y][term->cursor_x] = ' ';
        }
    } else if (c >= 32 && c <= 126) {
        /* Printable character */
        if (term->input_pos < 255 && term->cursor_x < TERM_COLS) {
            term->input_line[term->input_pos++] = c;
            term->buffer[term->cursor_y][term->cursor_x++] = c;
        }
    }
}

/* Execute terminal command */
void terminal_execute_command(struct terminal_app* term, const char* command) {
    if (strlen(command) == 0) return;
    
    if (strcmp(command, "help") == 0) {
        terminal_print_line(term, "Available commands:");
        terminal_print_line(term, "  help     - Show this help");
        terminal_print_line(term, "  clear    - Clear screen");
        terminal_print_line(term, "  ls       - List files");
        terminal_print_line(term, "  pwd      - Print working directory");
        terminal_print_line(term, "  echo     - Echo text");
        terminal_print_line(term, "  uname    - System information");
        terminal_print_line(term, "  exit     - Close terminal");
    } else if (strcmp(command, "clear") == 0) {
        /* Clear terminal */
        for (u32 y = 0; y < TERM_ROWS; y++) {
            for (u32 x = 0; x < TERM_COLS; x++) {
                term->buffer[y][x] = ' ';
            }
        }
        term->cursor_x = 0;
        term->cursor_y = 0;
    } else if (strcmp(command, "ls") == 0) {
        terminal_print_line(term, "Desktop/");
        terminal_print_line(term, "Documents/");
        terminal_print_line(term, "Downloads/");
        terminal_print_line(term, "Pictures/");
        terminal_print_line(term, "Music/");
        terminal_print_line(term, "Videos/");
    } else if (strcmp(command, "pwd") == 0) {
        terminal_print_line(term, "/home/user");
    } else if (strncmp(command, "echo ", 5) == 0) {
        terminal_print_line(term, command + 5);
    } else if (strcmp(command, "uname") == 0) {
        terminal_print_line(term, "Kronos OS 1.0 x86_64");
    } else if (strcmp(command, "exit") == 0) {
        /* Close terminal window */
        wm_destroy_window(term->window_id);
        desktop_remove_taskbar_item(term->window_id);
        term->active = false;
    } else {
        terminal_print_line(term, "Command not found. Type 'help' for available commands.");
    }
}

/* Render terminal */
void terminal_render(struct terminal_app* term) {
    if (!term->active) return;
    
    u32* buffer = wm_get_window_buffer(term->window_id);
    if (!buffer) return;
    
    /* Clear window buffer */
    u32 win_width = 600;  /* Assuming window size */
    u32 win_height = 400;
    
    for (u32 i = 0; i < win_width * win_height; i++) {
        buffer[i] = COLOR_BLACK;
    }
    
    /* Render terminal text */
    for (u32 y = 0; y < TERM_ROWS; y++) {
        for (u32 x = 0; x < TERM_COLS; x++) {
            char c = term->buffer[y][x];
            if (c != ' ') {
                /* Draw character to window buffer */
                terminal_draw_char_to_buffer(buffer, win_width, 
                                           x * CHAR_WIDTH, y * CHAR_HEIGHT, 
                                           c, COLOR_GREEN, COLOR_BLACK);
            }
        }
    }
    
    /* Draw cursor */
    if (term->cursor_y < TERM_ROWS) {
        u32 cursor_x = term->cursor_x * CHAR_WIDTH;
        u32 cursor_y = term->cursor_y * CHAR_HEIGHT;
        
        /* Draw cursor as white rectangle */
        for (u32 y = cursor_y; y < cursor_y + CHAR_HEIGHT && y < win_height; y++) {
            for (u32 x = cursor_x; x < cursor_x + CHAR_WIDTH && x < win_width; x++) {
                buffer[y * win_width + x] = COLOR_WHITE;
            }
        }
    }
}

/* Draw character to buffer */
void terminal_draw_char_to_buffer(u32* buffer, u32 buffer_width, 
                                 u32 x, u32 y, char c, u32 fg_color, u32 bg_color) {
    /* Simple 8x16 font rendering */
    for (u32 row = 0; row < CHAR_HEIGHT; row++) {
        for (u32 col = 0; col < CHAR_WIDTH; col++) {
            /* Simple character pattern - in real implementation, 
               this would use a proper font */
            bool pixel = false;
            
            if (c >= 'A' && c <= 'Z') {
                /* Simple pattern for letters */
                pixel = ((row + col) % 3 == 0);
            } else if (c >= '0' && c <= '9') {
                /* Simple pattern for numbers */
                pixel = ((row * col) % 4 == 0);
            } else if (c == '$') {
                /* Special pattern for prompt */
                pixel = (col == 2 || col == 5 || row == 8);
            } else if (c != ' ') {
                /* Default pattern for other characters */
                pixel = ((row + col) % 2 == 0);
            }
            
            u32 pixel_x = x + col;
            u32 pixel_y = y + row;
            
            if (pixel_x < buffer_width && pixel_y < 400) {  /* Assuming height */
                buffer[pixel_y * buffer_width + pixel_x] = pixel ? fg_color : bg_color;
            }
        }
    }
}

/* Find terminal by window ID */
struct terminal_app* terminal_find_by_window(u32 window_id) {
    for (u32 i = 0; i < 4; i++) {
        if (terminals[i].active && terminals[i].window_id == window_id) {
            return &terminals[i];
        }
    }
    return NULL;
}

/* Handle keyboard input for terminal */
void terminal_handle_keyboard(u32 window_id, char c) {
    struct terminal_app* term = terminal_find_by_window(window_id);
    if (term) {
        terminal_handle_input(term, c);
        terminal_render(term);
    }
}
