#include "kronos.h"

/* Advanced System Settings Application for Kronos OS */

#define MAX_SETTINGS_CATEGORIES 20
#define MAX_SETTINGS_ITEMS 50
#define MAX_SEARCH_RESULTS 100

/* Settings view modes */
typedef enum {
    SETTINGS_VIEW_CATEGORIES,
    SETTINGS_VIEW_SEARCH,
    SETTINGS_VIEW_FAVORITES,
    SETTINGS_VIEW_RECENT
} settings_view_mode_t;

/* Settings categories */
typedef enum {
    SETTINGS_APPEARANCE,
    SETTINGS_DISPLAY,
    SETTINGS_AUDIO,
    SETTINGS_NETWORK,
    SETTINGS_SECURITY,
    SETTINGS_PRIVACY,
    SETTINGS_PERFORMANCE,
    SETTINGS_ACCESSIBILITY,
    SETTINGS_KEYBOARD_MOUSE,
    SETTINGS_POWER,
    SETTINGS_STARTUP,
    SETTINGS_UPDATES,
    SETTINGS_USERS,
    SETTINGS_SYSTEM,
    SETTINGS_DEVELOPER,
    SETTINGS_ABOUT
} settings_category_t;

/* Settings item types */
typedef enum {
    SETTING_TOGGLE,
    SETTING_SLIDER,
    SETTING_DROPDOWN,
    SETTING_TEXT,
    SETTING_BUTTON,
    SETTING_COLOR
} setting_type_t;

/* Settings item */
struct setting_item {
    char name[64];
    char description[128];
    setting_type_t type;
    union {
        bool toggle_value;
        u32 slider_value;
        u32 dropdown_index;
        char text_value[64];
        u32 color_value;
    } value;
    u32 min_value;
    u32 max_value;
    char options[8][32];  /* For dropdown */
    u32 option_count;
};

/* Settings category */
struct settings_category {
    char name[32];
    char icon[8];
    struct setting_item items[MAX_SETTINGS_ITEMS];
    u32 item_count;
};

/* Search result */
struct search_result {
    char setting_name[64];
    char category_name[32];
    settings_category_t category;
    u32 relevance_score;
};

/* Settings application state */
struct settings_app {
    u32 window_id;
    settings_category_t current_category;
    settings_view_mode_t view_mode;
    u32 selected_item;
    u32 scroll_position;

    struct settings_category categories[MAX_SETTINGS_CATEGORIES];
    u32 category_count;

    char search_query[128];
    struct search_result search_results[MAX_SEARCH_RESULTS];
    u32 search_result_count;

    char favorites[50][64];
    u32 favorite_count;

    char recent_settings[20][64];
    u32 recent_count;

    bool show_advanced;
    bool preview_changes;
    bool unsaved_changes;

    bool active;
} settings_apps[2];

static u32 settings_count = 0;

/* Initialize settings application */
void app_settings_init(void) {
    for (u32 i = 0; i < 2; i++) {
        settings_apps[i].active = false;
        settings_apps[i].window_id = 0;
    }
    settings_count = 0;
}

/* Initialize default settings */
static void init_default_settings(struct settings_app* app) {
    app->category_count = 0;
    
    /* Appearance Settings */
    struct settings_category* appearance = &app->categories[app->category_count++];
    strcpy(appearance->name, "Appearance");
    strcpy(appearance->icon, "🎨");
    appearance->item_count = 0;
    
    /* Theme setting */
    struct setting_item* theme = &appearance->items[appearance->item_count++];
    strcpy(theme->name, "Theme");
    strcpy(theme->description, "Choose system theme");
    theme->type = SETTING_DROPDOWN;
    theme->option_count = 3;
    strcpy(theme->options[0], "Light");
    strcpy(theme->options[1], "Dark");
    strcpy(theme->options[2], "Auto");
    theme->value.dropdown_index = 0;
    
    /* Wallpaper setting */
    struct setting_item* wallpaper = &appearance->items[appearance->item_count++];
    strcpy(wallpaper->name, "Wallpaper");
    strcpy(wallpaper->description, "Desktop background image");
    wallpaper->type = SETTING_BUTTON;
    strcpy(wallpaper->value.text_value, "Change...");
    
    /* Window animations */
    struct setting_item* animations = &appearance->items[appearance->item_count++];
    strcpy(animations->name, "Window Animations");
    strcpy(animations->description, "Enable smooth window transitions");
    animations->type = SETTING_TOGGLE;
    animations->value.toggle_value = true;
    
    /* System Settings */
    struct settings_category* system = &app->categories[app->category_count++];
    strcpy(system->name, "System");
    strcpy(system->icon, "⚙️");
    system->item_count = 0;
    
    /* Auto-start applications */
    struct setting_item* autostart = &system->items[system->item_count++];
    strcpy(autostart->name, "Auto-start Applications");
    strcpy(autostart->description, "Programs to start automatically");
    autostart->type = SETTING_BUTTON;
    strcpy(autostart->value.text_value, "Manage...");
    
    /* System sounds */
    struct setting_item* sounds = &system->items[system->item_count++];
    strcpy(sounds->name, "System Sounds");
    strcpy(sounds->description, "Play sounds for system events");
    sounds->type = SETTING_TOGGLE;
    sounds->value.toggle_value = true;
    
    /* Performance mode */
    struct setting_item* performance = &system->items[system->item_count++];
    strcpy(performance->name, "Performance Mode");
    strcpy(performance->description, "Optimize system for performance");
    performance->type = SETTING_DROPDOWN;
    performance->option_count = 3;
    strcpy(performance->options[0], "Balanced");
    strcpy(performance->options[1], "Performance");
    strcpy(performance->options[2], "Power Saver");
    performance->value.dropdown_index = 0;
    
    /* Network Settings */
    struct settings_category* network = &app->categories[app->category_count++];
    strcpy(network->name, "Network");
    strcpy(network->icon, "🌐");
    network->item_count = 0;
    
    /* WiFi */
    struct setting_item* wifi = &network->items[network->item_count++];
    strcpy(wifi->name, "WiFi");
    strcpy(wifi->description, "Wireless network connection");
    wifi->type = SETTING_TOGGLE;
    wifi->value.toggle_value = true;
    
    /* Ethernet */
    struct setting_item* ethernet = &network->items[network->item_count++];
    strcpy(ethernet->name, "Ethernet");
    strcpy(ethernet->description, "Wired network connection");
    ethernet->type = SETTING_TOGGLE;
    ethernet->value.toggle_value = false;
    
    /* Firewall */
    struct setting_item* firewall = &network->items[network->item_count++];
    strcpy(firewall->name, "Firewall");
    strcpy(firewall->description, "Network security protection");
    firewall->type = SETTING_TOGGLE;
    firewall->value.toggle_value = true;
    
    /* Users & Accounts */
    struct settings_category* users = &app->categories[app->category_count++];
    strcpy(users->name, "Users & Accounts");
    strcpy(users->icon, "👤");
    users->item_count = 0;
    
    /* Current user */
    struct setting_item* current_user = &users->items[users->item_count++];
    strcpy(current_user->name, "Current User");
    strcpy(current_user->description, "Manage current user account");
    current_user->type = SETTING_TEXT;
    strcpy(current_user->value.text_value, "user");
    
    /* Add user */
    struct setting_item* add_user = &users->items[users->item_count++];
    strcpy(add_user->name, "Add User");
    strcpy(add_user->description, "Create new user account");
    add_user->type = SETTING_BUTTON;
    strcpy(add_user->value.text_value, "Add...");
    
    /* Privacy & Security */
    struct settings_category* privacy = &app->categories[app->category_count++];
    strcpy(privacy->name, "Privacy & Security");
    strcpy(privacy->icon, "🔒");
    privacy->item_count = 0;
    
    /* Screen lock */
    struct setting_item* screen_lock = &privacy->items[privacy->item_count++];
    strcpy(screen_lock->name, "Screen Lock");
    strcpy(screen_lock->description, "Automatically lock screen");
    screen_lock->type = SETTING_TOGGLE;
    screen_lock->value.toggle_value = true;
    
    /* Password required */
    struct setting_item* password = &privacy->items[privacy->item_count++];
    strcpy(password->name, "Require Password");
    strcpy(password->description, "Password required for login");
    password->type = SETTING_TOGGLE;
    password->value.toggle_value = true;
    
    /* About */
    struct settings_category* about = &app->categories[app->category_count++];
    strcpy(about->name, "About");
    strcpy(about->icon, "ℹ️");
    about->item_count = 0;
    
    /* System info */
    struct setting_item* system_info = &about->items[about->item_count++];
    strcpy(system_info->name, "System Information");
    strcpy(system_info->description, "Kronos OS v2.0 Advanced");
    system_info->type = SETTING_TEXT;
    strcpy(system_info->value.text_value, "View Details...");
    
    /* Check for updates */
    struct setting_item* updates = &about->items[about->item_count++];
    strcpy(updates->name, "Check for Updates");
    strcpy(updates->description, "Keep your system up to date");
    updates->type = SETTING_BUTTON;
    strcpy(updates->value.text_value, "Check Now");
}

/* Launch settings application */
void app_launch_settings(u32 window_id) {
    if (settings_count >= 2) return;
    
    u32 slot = settings_count++;
    settings_apps[slot].window_id = window_id;
    settings_apps[slot].current_category = SETTINGS_APPEARANCE;
    settings_apps[slot].selected_item = 0;
    settings_apps[slot].active = true;
    
    init_default_settings(&settings_apps[slot]);
}

/* Draw settings sidebar */
static void draw_settings_sidebar(u32* buffer, u32 buffer_width, struct settings_app* app) {
    u32 sidebar_width = 200;
    
    /* Draw sidebar background */
    for (u32 y = 0; y < 600; y++) {
        for (u32 x = 0; x < sidebar_width; x++) {
            if (x < buffer_width && y < 600) {
                buffer[y * buffer_width + x] = COLOR_LIGHT_GRAY;
            }
        }
    }
    
    /* Draw category items */
    for (u32 i = 0; i < app->category_count; i++) {
        u32 item_y = 20 + i * 50;
        u32 item_color = (i == app->current_category) ? UBUNTU_ORANGE : COLOR_LIGHT_GRAY;
        u32 text_color = (i == app->current_category) ? COLOR_WHITE : COLOR_BLACK;
        
        /* Draw selection background */
        if (i == app->current_category) {
            for (u32 y = item_y; y < item_y + 40; y++) {
                for (u32 x = 5; x < sidebar_width - 5; x++) {
                    if (x < buffer_width && y < 600) {
                        buffer[y * buffer_width + x] = item_color;
                    }
                }
            }
        }
        
        /* Draw icon */
        settings_draw_text_to_buffer(buffer, buffer_width, 15, item_y + 5, 
                                   app->categories[i].icon, text_color);
        
        /* Draw category name */
        settings_draw_text_to_buffer(buffer, buffer_width, 45, item_y + 15, 
                                   app->categories[i].name, text_color);
    }
    
    /* Draw sidebar border */
    for (u32 y = 0; y < 600; y++) {
        if (sidebar_width < buffer_width && y < 600) {
            buffer[y * buffer_width + sidebar_width] = COLOR_GRAY;
        }
    }
}

/* Draw settings content */
static void draw_settings_content(u32* buffer, u32 buffer_width, struct settings_app* app) {
    u32 content_x = 220;
    u32 content_width = buffer_width - content_x;
    
    /* Clear content area */
    for (u32 y = 0; y < 600; y++) {
        for (u32 x = content_x; x < buffer_width; x++) {
            buffer[y * buffer_width + x] = COLOR_WHITE;
        }
    }
    
    /* Draw category title */
    struct settings_category* category = &app->categories[app->current_category];
    settings_draw_text_to_buffer(buffer, buffer_width, content_x + 20, 20, 
                               category->name, COLOR_BLACK);
    
    /* Draw settings items */
    for (u32 i = 0; i < category->item_count; i++) {
        struct setting_item* item = &category->items[i];
        u32 item_y = 60 + i * 80;
        
        /* Draw item background */
        if (i == app->selected_item) {
            for (u32 y = item_y; y < item_y + 70; y++) {
                for (u32 x = content_x + 10; x < buffer_width - 10; x++) {
                    if (x < buffer_width && y < 600) {
                        buffer[y * buffer_width + x] = 0x00F0F0F0;
                    }
                }
            }
        }
        
        /* Draw item name */
        settings_draw_text_to_buffer(buffer, buffer_width, content_x + 20, item_y + 10, 
                                   item->name, COLOR_BLACK);
        
        /* Draw item description */
        settings_draw_text_to_buffer(buffer, buffer_width, content_x + 20, item_y + 30, 
                                   item->description, COLOR_GRAY);
        
        /* Draw item control based on type */
        u32 control_x = buffer_width - 150;
        
        switch (item->type) {
            case SETTING_TOGGLE:
                /* Draw toggle switch */
                draw_toggle_switch(buffer, buffer_width, control_x, item_y + 15, 
                                 item->value.toggle_value);
                break;
                
            case SETTING_DROPDOWN:
                /* Draw dropdown */
                settings_draw_text_to_buffer(buffer, buffer_width, control_x, item_y + 20, 
                                           item->options[item->value.dropdown_index], COLOR_BLUE);
                settings_draw_text_to_buffer(buffer, buffer_width, control_x + 80, item_y + 20, 
                                           "▼", COLOR_BLUE);
                break;
                
            case SETTING_BUTTON:
                /* Draw button */
                draw_settings_button(buffer, buffer_width, control_x, item_y + 15, 
                                   item->value.text_value);
                break;
                
            case SETTING_TEXT:
                /* Draw text field */
                settings_draw_text_to_buffer(buffer, buffer_width, control_x, item_y + 20, 
                                           item->value.text_value, COLOR_BLACK);
                break;
                
            case SETTING_SLIDER:
                /* Draw slider */
                draw_slider(buffer, buffer_width, control_x, item_y + 20, 
                          item->value.slider_value, item->min_value, item->max_value);
                break;
        }
    }
}

/* Draw toggle switch */
static void draw_toggle_switch(u32* buffer, u32 buffer_width, u32 x, u32 y, bool enabled) {
    u32 switch_width = 50;
    u32 switch_height = 25;
    u32 bg_color = enabled ? COLOR_GREEN : COLOR_GRAY;
    u32 knob_color = COLOR_WHITE;
    u32 knob_x = enabled ? x + switch_width - 20 : x + 5;
    
    /* Draw switch background */
    for (u32 dy = 0; dy < switch_height; dy++) {
        for (u32 dx = 0; dx < switch_width; dx++) {
            if (x + dx < buffer_width && y + dy < 600) {
                buffer[(y + dy) * buffer_width + (x + dx)] = bg_color;
            }
        }
    }
    
    /* Draw knob */
    for (u32 dy = 2; dy < switch_height - 2; dy++) {
        for (u32 dx = 0; dx < 15; dx++) {
            if (knob_x + dx < buffer_width && y + dy < 600) {
                buffer[(y + dy) * buffer_width + (knob_x + dx)] = knob_color;
            }
        }
    }
}

/* Draw settings button */
static void draw_settings_button(u32* buffer, u32 buffer_width, u32 x, u32 y, const char* text) {
    u32 btn_width = 100;
    u32 btn_height = 30;
    
    /* Draw button background */
    for (u32 dy = 0; dy < btn_height; dy++) {
        for (u32 dx = 0; dx < btn_width; dx++) {
            if (x + dx < buffer_width && y + dy < 600) {
                buffer[(y + dy) * buffer_width + (x + dx)] = COLOR_BLUE;
            }
        }
    }
    
    /* Draw button text */
    u32 text_x = x + (btn_width - strlen(text) * 8) / 2;
    u32 text_y = y + (btn_height - 8) / 2;
    settings_draw_text_to_buffer(buffer, buffer_width, text_x, text_y, text, COLOR_WHITE);
}

/* Draw slider */
static void draw_slider(u32* buffer, u32 buffer_width, u32 x, u32 y, u32 value, u32 min_val, u32 max_val) {
    u32 slider_width = 100;
    u32 slider_height = 10;
    u32 knob_pos = (value - min_val) * slider_width / (max_val - min_val);
    
    /* Draw slider track */
    for (u32 dy = 0; dy < slider_height; dy++) {
        for (u32 dx = 0; dx < slider_width; dx++) {
            if (x + dx < buffer_width && y + dy < 600) {
                buffer[(y + dy) * buffer_width + (x + dx)] = COLOR_GRAY;
            }
        }
    }
    
    /* Draw slider knob */
    for (u32 dy = 0; dy < slider_height + 10; dy++) {
        for (u32 dx = 0; dx < 10; dx++) {
            if (x + knob_pos + dx < buffer_width && y + dy - 5 < 600) {
                buffer[(y + dy - 5) * buffer_width + (x + knob_pos + dx)] = COLOR_BLUE;
            }
        }
    }
}

/* Render settings application */
void settings_render(struct settings_app* app) {
    if (!app->active) return;
    
    u32* buffer = wm_get_window_buffer(app->window_id);
    if (!buffer) return;
    
    u32 win_width = 800;
    u32 win_height = 600;
    
    /* Clear window buffer */
    for (u32 i = 0; i < win_width * win_height; i++) {
        buffer[i] = COLOR_WHITE;
    }
    
    /* Draw sidebar */
    draw_settings_sidebar(buffer, win_width, app);
    
    /* Draw content */
    draw_settings_content(buffer, win_width, app);
}

/* Handle settings click */
void settings_handle_click(struct settings_app* app, u32 x, u32 y) {
    /* Check sidebar clicks */
    if (x < 200) {
        u32 category_index = (y - 20) / 50;
        if (category_index < app->category_count) {
            app->current_category = category_index;
            app->selected_item = 0;
        }
        return;
    }
    
    /* Check content area clicks */
    if (x > 220) {
        u32 item_index = (y - 60) / 80;
        struct settings_category* category = &app->categories[app->current_category];
        
        if (item_index < category->item_count) {
            app->selected_item = item_index;
            
            struct setting_item* item = &category->items[item_index];
            
            /* Handle control clicks */
            if (x > 650) {  /* Control area */
                switch (item->type) {
                    case SETTING_TOGGLE:
                        item->value.toggle_value = !item->value.toggle_value;
                        break;
                        
                    case SETTING_DROPDOWN:
                        item->value.dropdown_index = (item->value.dropdown_index + 1) % item->option_count;
                        break;
                        
                    case SETTING_BUTTON:
                        /* Handle button click */
                        settings_handle_button_click(app, item);
                        break;
                }
            }
        }
    }
}

/* Handle button clicks */
static void settings_handle_button_click(struct settings_app* app, struct setting_item* item) {
    if (strcmp(item->name, "Wallpaper") == 0) {
        /* Open wallpaper selector */
        desktop_launch_app("wallpaper-selector");
    } else if (strcmp(item->name, "Check for Updates") == 0) {
        /* Check for system updates */
        strcpy(item->value.text_value, "Checking...");
        /* Simulate update check */
        timer_sleep(2000000);  /* 2 seconds */
        strcpy(item->value.text_value, "Up to date");
    } else if (strcmp(item->name, "Add User") == 0) {
        /* Open user creation dialog */
        /* This would open a dialog in a real implementation */
    }
}

/* Draw text to buffer (simplified) */
void settings_draw_text_to_buffer(u32* buffer, u32 buffer_width, u32 x, u32 y, 
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
                
                if (pixel && pos_x + dx < buffer_width && y + dy < 600) {
                    buffer[(y + dy) * buffer_width + (pos_x + dx)] = color;
                }
            }
        }
        pos_x += 8;
        text++;
    }
}

/* Find settings app by window ID */
struct settings_app* settings_find_by_window(u32 window_id) {
    for (u32 i = 0; i < 2; i++) {
        if (settings_apps[i].active && settings_apps[i].window_id == window_id) {
            return &settings_apps[i];
        }
    }
    return NULL;
}
