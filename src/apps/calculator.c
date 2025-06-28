#include "kronos.h"

/* Advanced Calculator Application for Kronos OS */

#define CALC_DISPLAY_WIDTH 300
#define CALC_DISPLAY_HEIGHT 60
#define CALC_BUTTON_WIDTH 60
#define CALC_BUTTON_HEIGHT 50
#define MAX_DISPLAY_CHARS 20

/* Calculator modes */
typedef enum {
    CALC_MODE_BASIC,
    CALC_MODE_SCIENTIFIC,
    CALC_MODE_PROGRAMMER
} calc_mode_t;

/* Calculator operations */
typedef enum {
    OP_NONE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_POWER,
    OP_SQRT,
    OP_SIN,
    OP_COS,
    OP_TAN,
    OP_LOG,
    OP_LN,
    OP_FACTORIAL
} calc_operation_t;

/* Calculator state */
struct calculator {
    u32 window_id;
    calc_mode_t mode;
    char display[MAX_DISPLAY_CHARS + 1];
    double current_value;
    double stored_value;
    calc_operation_t pending_operation;
    bool new_number;
    bool has_decimal;
    bool error_state;
    bool active;
} calculators[2];

static u32 calc_count = 0;

/* Calculator button layout */
struct calc_button {
    char label[8];
    u32 x, y;
    u32 color;
    char action;
    bool is_operation;
};

/* Basic calculator buttons */
static struct calc_button basic_buttons[] = {
    /* Row 1 */
    {"C", 10, 80, COLOR_RED, 'C', false},
    {"CE", 80, 80, COLOR_ORANGE, 'E', false},
    {"⌫", 150, 80, COLOR_ORANGE, 'B', false},
    {"÷", 220, 80, COLOR_BLUE, '/', true},
    
    /* Row 2 */
    {"7", 10, 140, COLOR_GRAY, '7', false},
    {"8", 80, 140, COLOR_GRAY, '8', false},
    {"9", 150, 140, COLOR_GRAY, '9', false},
    {"×", 220, 140, COLOR_BLUE, '*', true},
    
    /* Row 3 */
    {"4", 10, 200, COLOR_GRAY, '4', false},
    {"5", 80, 200, COLOR_GRAY, '5', false},
    {"6", 150, 200, COLOR_GRAY, '6', false},
    {"-", 220, 200, COLOR_BLUE, '-', true},
    
    /* Row 4 */
    {"1", 10, 260, COLOR_GRAY, '1', false},
    {"2", 80, 260, COLOR_GRAY, '2', false},
    {"3", 150, 260, COLOR_GRAY, '3', false},
    {"+", 220, 260, COLOR_BLUE, '+', true},
    
    /* Row 5 */
    {"±", 10, 320, COLOR_LIGHT_GRAY, 'N', false},
    {"0", 80, 320, COLOR_GRAY, '0', false},
    {".", 150, 320, COLOR_GRAY, '.', false},
    {"=", 220, 320, COLOR_GREEN, '=', true}
};

/* Scientific calculator additional buttons */
static struct calc_button scientific_buttons[] = {
    {"sin", 290, 140, COLOR_PURPLE, 's', true},
    {"cos", 360, 140, COLOR_PURPLE, 'c', true},
    {"tan", 430, 140, COLOR_PURPLE, 't', true},
    {"log", 290, 200, COLOR_PURPLE, 'l', true},
    {"ln", 360, 200, COLOR_PURPLE, 'n', true},
    {"x²", 430, 200, COLOR_PURPLE, 'q', true},
    {"√", 290, 260, COLOR_PURPLE, 'r', true},
    {"x^y", 360, 260, COLOR_PURPLE, '^', true},
    {"!", 430, 260, COLOR_PURPLE, '!', true},
    {"π", 290, 320, COLOR_PURPLE, 'p', false},
    {"e", 360, 320, COLOR_PURPLE, 'e', false},
    {"(", 430, 320, COLOR_PURPLE, '(', false}
};

/* Initialize calculator application */
void app_calculator_init(void) {
    for (u32 i = 0; i < 2; i++) {
        calculators[i].active = false;
        calculators[i].window_id = 0;
    }
    calc_count = 0;
}

/* Launch calculator */
void app_launch_calculator(u32 window_id) {
    if (calc_count >= 2) return;
    
    u32 slot = calc_count++;
    calculators[slot].window_id = window_id;
    calculators[slot].mode = CALC_MODE_BASIC;
    calculators[slot].current_value = 0.0;
    calculators[slot].stored_value = 0.0;
    calculators[slot].pending_operation = OP_NONE;
    calculators[slot].new_number = true;
    calculators[slot].has_decimal = false;
    calculators[slot].error_state = false;
    calculators[slot].active = true;
    
    strcpy(calculators[slot].display, "0");
}

/* Update calculator display */
static void update_display(struct calculator* calc) {
    if (calc->error_state) {
        strcpy(calc->display, "Error");
        return;
    }
    
    /* Format number for display */
    if (calc->current_value == (long long)calc->current_value) {
        /* Integer display */
        snprintf(calc->display, MAX_DISPLAY_CHARS, "%.0f", calc->current_value);
    } else {
        /* Decimal display */
        snprintf(calc->display, MAX_DISPLAY_CHARS, "%.8g", calc->current_value);
    }
    
    /* Truncate if too long */
    if (strlen(calc->display) > MAX_DISPLAY_CHARS - 1) {
        strcpy(calc->display, "Overflow");
    }
}

/* Perform calculation */
static void perform_calculation(struct calculator* calc) {
    double result = calc->stored_value;
    
    switch (calc->pending_operation) {
        case OP_ADD:
            result = calc->stored_value + calc->current_value;
            break;
        case OP_SUBTRACT:
            result = calc->stored_value - calc->current_value;
            break;
        case OP_MULTIPLY:
            result = calc->stored_value * calc->current_value;
            break;
        case OP_DIVIDE:
            if (calc->current_value == 0.0) {
                calc->error_state = true;
                return;
            }
            result = calc->stored_value / calc->current_value;
            break;
        case OP_POWER:
            result = pow(calc->stored_value, calc->current_value);
            break;
        default:
            result = calc->current_value;
            break;
    }
    
    calc->current_value = result;
    calc->stored_value = 0.0;
    calc->pending_operation = OP_NONE;
    calc->new_number = true;
    calc->has_decimal = false;
}

/* Perform scientific operations */
static void perform_scientific_operation(struct calculator* calc, calc_operation_t op) {
    double result = calc->current_value;
    
    switch (op) {
        case OP_SQRT:
            if (calc->current_value < 0) {
                calc->error_state = true;
                return;
            }
            result = sqrt(calc->current_value);
            break;
        case OP_SIN:
            result = sin(calc->current_value * M_PI / 180.0);  /* Convert to radians */
            break;
        case OP_COS:
            result = cos(calc->current_value * M_PI / 180.0);
            break;
        case OP_TAN:
            result = tan(calc->current_value * M_PI / 180.0);
            break;
        case OP_LOG:
            if (calc->current_value <= 0) {
                calc->error_state = true;
                return;
            }
            result = log10(calc->current_value);
            break;
        case OP_LN:
            if (calc->current_value <= 0) {
                calc->error_state = true;
                return;
            }
            result = log(calc->current_value);
            break;
        case OP_FACTORIAL:
            if (calc->current_value < 0 || calc->current_value != (int)calc->current_value) {
                calc->error_state = true;
                return;
            }
            result = 1;
            for (int i = 2; i <= (int)calc->current_value; i++) {
                result *= i;
            }
            break;
        default:
            return;
    }
    
    calc->current_value = result;
    calc->new_number = true;
    calc->has_decimal = false;
}

/* Handle calculator input */
void calculator_handle_input(struct calculator* calc, char input) {
    if (calc->error_state && input != 'C') {
        return;  /* Only clear can reset error state */
    }
    
    switch (input) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if (calc->new_number) {
                calc->current_value = input - '0';
                calc->new_number = false;
                calc->has_decimal = false;
            } else {
                if (strlen(calc->display) < MAX_DISPLAY_CHARS - 1) {
                    calc->current_value = calc->current_value * 10 + (input - '0');
                }
            }
            break;
            
        case '.':
            if (!calc->has_decimal) {
                calc->has_decimal = true;
                if (calc->new_number) {
                    calc->current_value = 0.0;
                    calc->new_number = false;
                }
            }
            break;
            
        case '+':
            if (calc->pending_operation != OP_NONE) {
                perform_calculation(calc);
            }
            calc->stored_value = calc->current_value;
            calc->pending_operation = OP_ADD;
            calc->new_number = true;
            break;
            
        case '-':
            if (calc->pending_operation != OP_NONE) {
                perform_calculation(calc);
            }
            calc->stored_value = calc->current_value;
            calc->pending_operation = OP_SUBTRACT;
            calc->new_number = true;
            break;
            
        case '*':
            if (calc->pending_operation != OP_NONE) {
                perform_calculation(calc);
            }
            calc->stored_value = calc->current_value;
            calc->pending_operation = OP_MULTIPLY;
            calc->new_number = true;
            break;
            
        case '/':
            if (calc->pending_operation != OP_NONE) {
                perform_calculation(calc);
            }
            calc->stored_value = calc->current_value;
            calc->pending_operation = OP_DIVIDE;
            calc->new_number = true;
            break;
            
        case '^':
            if (calc->pending_operation != OP_NONE) {
                perform_calculation(calc);
            }
            calc->stored_value = calc->current_value;
            calc->pending_operation = OP_POWER;
            calc->new_number = true;
            break;
            
        case '=':
            if (calc->pending_operation != OP_NONE) {
                perform_calculation(calc);
            }
            break;
            
        case 'C':  /* Clear all */
            calc->current_value = 0.0;
            calc->stored_value = 0.0;
            calc->pending_operation = OP_NONE;
            calc->new_number = true;
            calc->has_decimal = false;
            calc->error_state = false;
            break;
            
        case 'E':  /* Clear entry */
            calc->current_value = 0.0;
            calc->new_number = true;
            calc->has_decimal = false;
            break;
            
        case 'B':  /* Backspace */
            if (!calc->new_number && !calc->error_state) {
                calc->current_value = floor(calc->current_value / 10);
            }
            break;
            
        case 'N':  /* Negate */
            calc->current_value = -calc->current_value;
            break;
            
        /* Scientific operations */
        case 's': perform_scientific_operation(calc, OP_SIN); break;
        case 'c': perform_scientific_operation(calc, OP_COS); break;
        case 't': perform_scientific_operation(calc, OP_TAN); break;
        case 'l': perform_scientific_operation(calc, OP_LOG); break;
        case 'n': perform_scientific_operation(calc, OP_LN); break;
        case 'r': perform_scientific_operation(calc, OP_SQRT); break;
        case 'q': calc->current_value = calc->current_value * calc->current_value; break;
        case '!': perform_scientific_operation(calc, OP_FACTORIAL); break;
        case 'p': calc->current_value = M_PI; calc->new_number = true; break;
        case 'e': calc->current_value = M_E; calc->new_number = true; break;
    }
    
    update_display(calc);
}

/* Draw calculator button */
static void draw_calc_button(u32* buffer, u32 buffer_width, struct calc_button* btn, bool pressed) {
    u32 btn_color = pressed ? COLOR_LIGHT_GRAY : btn->color;
    u32 text_color = COLOR_BLACK;
    
    /* Draw button background */
    for (u32 y = btn->y; y < btn->y + CALC_BUTTON_HEIGHT; y++) {
        for (u32 x = btn->x; x < btn->x + CALC_BUTTON_WIDTH; x++) {
            if (x < buffer_width && y < 400) {
                buffer[y * buffer_width + x] = btn_color;
            }
        }
    }
    
    /* Draw button border */
    u32 border_color = pressed ? COLOR_DARK_GRAY : COLOR_WHITE;
    
    /* Top and bottom borders */
    for (u32 x = btn->x; x < btn->x + CALC_BUTTON_WIDTH; x++) {
        if (x < buffer_width) {
            if (btn->y < 400) buffer[btn->y * buffer_width + x] = border_color;
            if (btn->y + CALC_BUTTON_HEIGHT - 1 < 400) {
                buffer[(btn->y + CALC_BUTTON_HEIGHT - 1) * buffer_width + x] = border_color;
            }
        }
    }
    
    /* Left and right borders */
    for (u32 y = btn->y; y < btn->y + CALC_BUTTON_HEIGHT; y++) {
        if (y < 400) {
            if (btn->x < buffer_width) buffer[y * buffer_width + btn->x] = border_color;
            if (btn->x + CALC_BUTTON_WIDTH - 1 < buffer_width) {
                buffer[y * buffer_width + (btn->x + CALC_BUTTON_WIDTH - 1)] = border_color;
            }
        }
    }
    
    /* Draw button text */
    u32 text_x = btn->x + (CALC_BUTTON_WIDTH - strlen(btn->label) * 8) / 2;
    u32 text_y = btn->y + (CALC_BUTTON_HEIGHT - 8) / 2;
    
    calc_draw_text_to_buffer(buffer, buffer_width, text_x, text_y, btn->label, text_color);
}

/* Render calculator */
void calculator_render(struct calculator* calc) {
    if (!calc->active) return;
    
    u32* buffer = wm_get_window_buffer(calc->window_id);
    if (!buffer) return;
    
    u32 win_width = (calc->mode == CALC_MODE_SCIENTIFIC) ? 500 : 300;
    u32 win_height = 400;
    
    /* Clear window buffer */
    for (u32 i = 0; i < win_width * win_height; i++) {
        buffer[i] = COLOR_LIGHT_GRAY;
    }
    
    /* Draw display */
    for (u32 y = 10; y < 10 + CALC_DISPLAY_HEIGHT; y++) {
        for (u32 x = 10; x < win_width - 10; x++) {
            buffer[y * win_width + x] = COLOR_WHITE;
        }
    }
    
    /* Draw display border */
    for (u32 x = 10; x < win_width - 10; x++) {
        buffer[10 * win_width + x] = COLOR_BLACK;
        buffer[(10 + CALC_DISPLAY_HEIGHT - 1) * win_width + x] = COLOR_BLACK;
    }
    for (u32 y = 10; y < 10 + CALC_DISPLAY_HEIGHT; y++) {
        buffer[y * win_width + 10] = COLOR_BLACK;
        buffer[y * win_width + (win_width - 11)] = COLOR_BLACK;
    }
    
    /* Draw display text */
    u32 text_x = win_width - 20 - strlen(calc->display) * 8;
    u32 text_y = 30;
    calc_draw_text_to_buffer(buffer, win_width, text_x, text_y, calc->display, COLOR_BLACK);
    
    /* Draw basic calculator buttons */
    for (u32 i = 0; i < sizeof(basic_buttons) / sizeof(basic_buttons[0]); i++) {
        draw_calc_button(buffer, win_width, &basic_buttons[i], false);
    }
    
    /* Draw scientific buttons if in scientific mode */
    if (calc->mode == CALC_MODE_SCIENTIFIC) {
        for (u32 i = 0; i < sizeof(scientific_buttons) / sizeof(scientific_buttons[0]); i++) {
            draw_calc_button(buffer, win_width, &scientific_buttons[i], false);
        }
    }
    
    /* Draw mode selector */
    const char* mode_text = (calc->mode == CALC_MODE_BASIC) ? "Basic" : "Scientific";
    calc_draw_text_to_buffer(buffer, win_width, 10, win_height - 20, mode_text, COLOR_BLUE);
}

/* Handle calculator click */
void calculator_handle_click(struct calculator* calc, u32 x, u32 y) {
    /* Check basic buttons */
    for (u32 i = 0; i < sizeof(basic_buttons) / sizeof(basic_buttons[0]); i++) {
        struct calc_button* btn = &basic_buttons[i];
        if (x >= btn->x && x < btn->x + CALC_BUTTON_WIDTH &&
            y >= btn->y && y < btn->y + CALC_BUTTON_HEIGHT) {
            calculator_handle_input(calc, btn->action);
            return;
        }
    }
    
    /* Check scientific buttons if in scientific mode */
    if (calc->mode == CALC_MODE_SCIENTIFIC) {
        for (u32 i = 0; i < sizeof(scientific_buttons) / sizeof(scientific_buttons[0]); i++) {
            struct calc_button* btn = &scientific_buttons[i];
            if (x >= btn->x && x < btn->x + CALC_BUTTON_WIDTH &&
                y >= btn->y && y < btn->y + CALC_BUTTON_HEIGHT) {
                calculator_handle_input(calc, btn->action);
                return;
            }
        }
    }
    
    /* Check mode toggle */
    if (x >= 10 && x < 100 && y >= 380 && y < 400) {
        calc->mode = (calc->mode == CALC_MODE_BASIC) ? CALC_MODE_SCIENTIFIC : CALC_MODE_BASIC;
        
        /* Resize window based on mode */
        u32 new_width = (calc->mode == CALC_MODE_SCIENTIFIC) ? 500 : 300;
        wm_resize_window(calc->window_id, new_width, 400);
    }
}

/* Draw text to buffer (simplified) */
void calc_draw_text_to_buffer(u32* buffer, u32 buffer_width, u32 x, u32 y, 
                             const char* text, u32 color) {
    u32 pos_x = x;
    
    while (*text && pos_x < buffer_width - 8) {
        /* Simple character rendering */
        for (u32 dy = 0; dy < 8; dy++) {
            for (u32 dx = 0; dx < 8; dx++) {
                bool pixel = false;
                
                /* Character patterns */
                if (*text >= '0' && *text <= '9') {
                    pixel = ((dx + dy + *text) % 3 == 0);
                } else if (*text >= 'A' && *text <= 'Z') {
                    pixel = ((dx * 2 + dy + *text) % 4 == 0);
                } else if (*text >= 'a' && *text <= 'z') {
                    pixel = ((dx + dy * 2 + *text) % 4 == 0);
                } else {
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

/* Find calculator by window ID */
struct calculator* calculator_find_by_window(u32 window_id) {
    for (u32 i = 0; i < 2; i++) {
        if (calculators[i].active && calculators[i].window_id == window_id) {
            return &calculators[i];
        }
    }
    return NULL;
}
