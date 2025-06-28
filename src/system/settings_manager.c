#include "kronos.h"

/* Advanced Settings Management System for Kronos OS */

#define MAX_SETTING_NAME 64
#define MAX_SETTING_VALUE 256
#define MAX_SETTINGS_PER_CATEGORY 50
#define MAX_CATEGORIES 20
#define SETTINGS_FILE_PATH "/etc/kronos/settings.conf"
#define USER_SETTINGS_PATH "/home/%s/.kronos/settings.conf"

/* Setting data types */
typedef enum {
    SETTING_TYPE_BOOLEAN,
    SETTING_TYPE_INTEGER,
    SETTING_TYPE_FLOAT,
    SETTING_TYPE_STRING,
    SETTING_TYPE_ENUM,
    SETTING_TYPE_COLOR,
    SETTING_TYPE_PATH,
    SETTING_TYPE_PASSWORD
} setting_type_t;

/* Setting access levels */
typedef enum {
    SETTING_ACCESS_USER,
    SETTING_ACCESS_ADMIN,
    SETTING_ACCESS_SYSTEM
} setting_access_t;

/* Setting categories */
typedef enum {
    SETTINGS_CATEGORY_APPEARANCE,
    SETTINGS_CATEGORY_DISPLAY,
    SETTINGS_CATEGORY_AUDIO,
    SETTINGS_CATEGORY_NETWORK,
    SETTINGS_CATEGORY_SECURITY,
    SETTINGS_CATEGORY_PRIVACY,
    SETTINGS_CATEGORY_PERFORMANCE,
    SETTINGS_CATEGORY_ACCESSIBILITY,
    SETTINGS_CATEGORY_KEYBOARD,
    SETTINGS_CATEGORY_MOUSE,
    SETTINGS_CATEGORY_POWER,
    SETTINGS_CATEGORY_STARTUP,
    SETTINGS_CATEGORY_UPDATES,
    SETTINGS_CATEGORY_USERS,
    SETTINGS_CATEGORY_SYSTEM,
    SETTINGS_CATEGORY_DEVELOPER
} settings_category_t;

/* Setting definition */
struct setting_definition {
    char name[MAX_SETTING_NAME];
    char display_name[MAX_SETTING_NAME];
    char description[256];
    setting_type_t type;
    setting_access_t access_level;
    settings_category_t category;
    
    /* Type-specific data */
    union {
        struct {
            bool default_value;
        } boolean;
        
        struct {
            i32 default_value;
            i32 min_value;
            i32 max_value;
        } integer;
        
        struct {
            float default_value;
            float min_value;
            float max_value;
        } floating;
        
        struct {
            char default_value[MAX_SETTING_VALUE];
            u32 max_length;
        } string;
        
        struct {
            char options[16][64];
            u32 option_count;
            u32 default_index;
        } enumeration;
        
        struct {
            u32 default_color;
        } color;
    } data;
    
    bool requires_restart;
    bool is_readonly;
    void (*change_callback)(const char* name, const void* value);
};

/* Setting value */
struct setting_value {
    char name[MAX_SETTING_NAME];
    setting_type_t type;
    bool is_user_setting;
    
    union {
        bool boolean_value;
        i32 integer_value;
        float float_value;
        char string_value[MAX_SETTING_VALUE];
        u32 enum_index;
        u32 color_value;
    } value;
    
    u64 last_modified;
    bool is_dirty;
};

/* Settings category */
struct settings_category {
    settings_category_t id;
    char name[64];
    char description[256];
    char icon[8];
    struct setting_definition* definitions[MAX_SETTINGS_PER_CATEGORY];
    u32 definition_count;
};

/* Settings manager state */
struct settings_manager {
    struct settings_category categories[MAX_CATEGORIES];
    u32 category_count;
    
    struct setting_value system_settings[500];
    u32 system_setting_count;
    
    struct setting_value user_settings[200];
    u32 user_setting_count;
    
    bool initialized;
    bool auto_save;
    u32 save_interval;  /* seconds */
    u64 last_save_time;
} settings_mgr;

/* Initialize settings system */
void settings_manager_init(void) {
    memset(&settings_mgr, 0, sizeof(settings_mgr));
    
    /* Initialize categories */
    init_settings_categories();
    
    /* Register default settings */
    register_default_settings();
    
    /* Load settings from files */
    load_system_settings();
    load_user_settings();
    
    settings_mgr.auto_save = true;
    settings_mgr.save_interval = 30;  /* Save every 30 seconds */
    settings_mgr.last_save_time = get_system_time();
    settings_mgr.initialized = true;
    
    vga_puts("Settings manager initialized\n");
}

/* Initialize settings categories */
static void init_settings_categories(void) {
    settings_mgr.category_count = 0;
    
    /* Appearance */
    add_category(SETTINGS_CATEGORY_APPEARANCE, "Appearance", 
                "Desktop theme, wallpaper, and visual settings", "🎨");
    
    /* Display */
    add_category(SETTINGS_CATEGORY_DISPLAY, "Display", 
                "Screen resolution, brightness, and monitor settings", "🖥️");
    
    /* Audio */
    add_category(SETTINGS_CATEGORY_AUDIO, "Audio", 
                "Sound settings, volume, and audio devices", "🔊");
    
    /* Network */
    add_category(SETTINGS_CATEGORY_NETWORK, "Network", 
                "WiFi, Ethernet, and network configuration", "🌐");
    
    /* Security */
    add_category(SETTINGS_CATEGORY_SECURITY, "Security", 
                "Firewall, passwords, and security policies", "🔒");
    
    /* Privacy */
    add_category(SETTINGS_CATEGORY_PRIVACY, "Privacy", 
                "Data collection, tracking, and privacy controls", "🛡️");
    
    /* Performance */
    add_category(SETTINGS_CATEGORY_PERFORMANCE, "Performance", 
                "CPU, memory, and system optimization settings", "⚡");
    
    /* Accessibility */
    add_category(SETTINGS_CATEGORY_ACCESSIBILITY, "Accessibility", 
                "Screen reader, magnifier, and accessibility features", "♿");
    
    /* Keyboard & Mouse */
    add_category(SETTINGS_CATEGORY_KEYBOARD, "Keyboard & Mouse", 
                "Input device settings and shortcuts", "⌨️");
    
    /* Power Management */
    add_category(SETTINGS_CATEGORY_POWER, "Power Management", 
                "Battery, sleep, and power saving settings", "🔋");
    
    /* Startup & Services */
    add_category(SETTINGS_CATEGORY_STARTUP, "Startup & Services", 
                "Boot options and system services", "🚀");
    
    /* System Updates */
    add_category(SETTINGS_CATEGORY_UPDATES, "Updates", 
                "Automatic updates and software management", "📦");
    
    /* User Accounts */
    add_category(SETTINGS_CATEGORY_USERS, "User Accounts", 
                "User management and account settings", "👤");
    
    /* System Information */
    add_category(SETTINGS_CATEGORY_SYSTEM, "System", 
                "Hardware information and system details", "ℹ️");
    
    /* Developer Options */
    add_category(SETTINGS_CATEGORY_DEVELOPER, "Developer", 
                "Development tools and debugging options", "🛠️");
}

/* Add settings category */
static void add_category(settings_category_t id, const char* name, 
                        const char* description, const char* icon) {
    if (settings_mgr.category_count >= MAX_CATEGORIES) return;
    
    struct settings_category* cat = &settings_mgr.categories[settings_mgr.category_count++];
    cat->id = id;
    strcpy(cat->name, name);
    strcpy(cat->description, description);
    strcpy(cat->icon, icon);
    cat->definition_count = 0;
}

/* Register default settings */
static void register_default_settings(void) {
    /* Appearance Settings */
    register_boolean_setting("appearance.theme_dark", "Dark Theme", 
                            "Use dark theme for the desktop", 
                            SETTINGS_CATEGORY_APPEARANCE, false, false);
    
    register_enum_setting("appearance.wallpaper", "Wallpaper", 
                         "Desktop background wallpaper",
                         SETTINGS_CATEGORY_APPEARANCE, 
                         (const char*[]){"Ocean Blue", "Ubuntu Orange", "Forest Green", 
                                       "Sunset Purple", "Deep Space", "Cherry Blossom"}, 
                         6, 0, false);
    
    register_boolean_setting("appearance.animations", "Window Animations", 
                            "Enable smooth window transitions", 
                            SETTINGS_CATEGORY_APPEARANCE, true, false);
    
    register_integer_setting("appearance.icon_size", "Icon Size", 
                            "Desktop icon size in pixels",
                            SETTINGS_CATEGORY_APPEARANCE, 48, 24, 128, false);
    
    /* Display Settings */
    register_enum_setting("display.resolution", "Screen Resolution", 
                         "Display resolution setting",
                         SETTINGS_CATEGORY_DISPLAY,
                         (const char*[]){"1920x1080", "1680x1050", "1440x900", 
                                       "1366x768", "1280x1024", "1024x768"}, 
                         6, 0, true);
    
    register_integer_setting("display.brightness", "Brightness", 
                            "Screen brightness level",
                            SETTINGS_CATEGORY_DISPLAY, 80, 10, 100, false);
    
    register_boolean_setting("display.auto_brightness", "Auto Brightness", 
                            "Automatically adjust brightness", 
                            SETTINGS_CATEGORY_DISPLAY, false, false);
    
    /* Audio Settings */
    register_integer_setting("audio.master_volume", "Master Volume", 
                            "System master volume level",
                            SETTINGS_CATEGORY_AUDIO, 75, 0, 100, false);
    
    register_boolean_setting("audio.system_sounds", "System Sounds", 
                            "Play sounds for system events", 
                            SETTINGS_CATEGORY_AUDIO, true, false);
    
    register_enum_setting("audio.output_device", "Output Device", 
                         "Audio output device selection",
                         SETTINGS_CATEGORY_AUDIO,
                         (const char*[]){"Built-in Speakers", "Headphones", "HDMI Audio"}, 
                         3, 0, false);
    
    /* Network Settings */
    register_boolean_setting("network.wifi_enabled", "WiFi", 
                            "Enable wireless networking", 
                            SETTINGS_CATEGORY_NETWORK, true, false);
    
    register_boolean_setting("network.ethernet_enabled", "Ethernet", 
                            "Enable wired networking", 
                            SETTINGS_CATEGORY_NETWORK, true, false);
    
    register_boolean_setting("network.firewall_enabled", "Firewall", 
                            "Enable network firewall protection", 
                            SETTINGS_CATEGORY_NETWORK, true, true);
    
    /* Security Settings */
    register_boolean_setting("security.auto_lock", "Auto Lock Screen", 
                            "Automatically lock screen when idle", 
                            SETTINGS_CATEGORY_SECURITY, true, false);
    
    register_integer_setting("security.lock_timeout", "Lock Timeout", 
                            "Minutes before auto-lock (0 = disabled)",
                            SETTINGS_CATEGORY_SECURITY, 15, 0, 120, false);
    
    register_boolean_setting("security.require_password", "Require Password", 
                            "Password required for login and unlock", 
                            SETTINGS_CATEGORY_SECURITY, true, true);
    
    /* Performance Settings */
    register_enum_setting("performance.power_mode", "Power Mode", 
                         "System performance and power balance",
                         SETTINGS_CATEGORY_PERFORMANCE,
                         (const char*[]){"Power Saver", "Balanced", "High Performance"}, 
                         3, 1, false);
    
    register_boolean_setting("performance.background_apps", "Background Apps", 
                            "Allow apps to run in background", 
                            SETTINGS_CATEGORY_PERFORMANCE, true, false);
    
    register_integer_setting("performance.max_processes", "Max Processes", 
                            "Maximum number of concurrent processes",
                            SETTINGS_CATEGORY_PERFORMANCE, 256, 64, 1024, true);
    
    /* Accessibility Settings */
    register_boolean_setting("accessibility.high_contrast", "High Contrast", 
                            "Use high contrast colors", 
                            SETTINGS_CATEGORY_ACCESSIBILITY, false, false);
    
    register_integer_setting("accessibility.text_size", "Text Size", 
                            "System text size scaling percentage",
                            SETTINGS_CATEGORY_ACCESSIBILITY, 100, 75, 200, false);
    
    register_boolean_setting("accessibility.screen_reader", "Screen Reader", 
                            "Enable screen reader for visually impaired", 
                            SETTINGS_CATEGORY_ACCESSIBILITY, false, false);
    
    /* Keyboard & Mouse Settings */
    register_integer_setting("input.key_repeat_delay", "Key Repeat Delay", 
                            "Delay before key repeat starts (ms)",
                            SETTINGS_CATEGORY_KEYBOARD, 500, 200, 2000, false);
    
    register_integer_setting("input.key_repeat_rate", "Key Repeat Rate", 
                            "Key repeat rate (keys per second)",
                            SETTINGS_CATEGORY_KEYBOARD, 25, 5, 50, false);
    
    register_integer_setting("input.mouse_sensitivity", "Mouse Sensitivity", 
                            "Mouse pointer sensitivity",
                            SETTINGS_CATEGORY_MOUSE, 50, 10, 100, false);
    
    /* Power Management Settings */
    register_integer_setting("power.sleep_timeout", "Sleep Timeout", 
                            "Minutes before system sleeps (0 = never)",
                            SETTINGS_CATEGORY_POWER, 30, 0, 240, false);
    
    register_boolean_setting("power.hibernate_enabled", "Hibernation", 
                            "Enable hibernation support", 
                            SETTINGS_CATEGORY_POWER, true, true);
    
    /* System Update Settings */
    register_boolean_setting("updates.auto_check", "Auto Check Updates", 
                            "Automatically check for system updates", 
                            SETTINGS_CATEGORY_UPDATES, true, false);
    
    register_enum_setting("updates.install_mode", "Update Installation", 
                         "How to handle system updates",
                         SETTINGS_CATEGORY_UPDATES,
                         (const char*[]){"Manual", "Download Only", "Auto Install"}, 
                         3, 1, false);
    
    /* Developer Settings */
    register_boolean_setting("developer.debug_mode", "Debug Mode", 
                            "Enable system debugging features", 
                            SETTINGS_CATEGORY_DEVELOPER, false, true);
    
    register_boolean_setting("developer.show_fps", "Show FPS", 
                            "Display frame rate counter", 
                            SETTINGS_CATEGORY_DEVELOPER, false, false);
}

/* Register boolean setting */
static void register_boolean_setting(const char* name, const char* display_name,
                                    const char* description, settings_category_t category,
                                    bool default_value, bool requires_restart) {
    struct setting_definition* def = create_setting_definition(name, display_name, description, 
                                                              SETTING_TYPE_BOOLEAN, category, requires_restart);
    if (def) {
        def->data.boolean.default_value = default_value;
        add_setting_to_category(category, def);
    }
}

/* Register integer setting */
static void register_integer_setting(const char* name, const char* display_name,
                                    const char* description, settings_category_t category,
                                    i32 default_value, i32 min_value, i32 max_value, 
                                    bool requires_restart) {
    struct setting_definition* def = create_setting_definition(name, display_name, description, 
                                                              SETTING_TYPE_INTEGER, category, requires_restart);
    if (def) {
        def->data.integer.default_value = default_value;
        def->data.integer.min_value = min_value;
        def->data.integer.max_value = max_value;
        add_setting_to_category(category, def);
    }
}

/* Register enum setting */
static void register_enum_setting(const char* name, const char* display_name,
                                 const char* description, settings_category_t category,
                                 const char* options[], u32 option_count, u32 default_index,
                                 bool requires_restart) {
    struct setting_definition* def = create_setting_definition(name, display_name, description, 
                                                              SETTING_TYPE_ENUM, category, requires_restart);
    if (def) {
        def->data.enumeration.option_count = option_count;
        def->data.enumeration.default_index = default_index;
        for (u32 i = 0; i < option_count && i < 16; i++) {
            strcpy(def->data.enumeration.options[i], options[i]);
        }
        add_setting_to_category(category, def);
    }
}

/* Create setting definition */
static struct setting_definition* create_setting_definition(const char* name, const char* display_name,
                                                           const char* description, setting_type_t type,
                                                           settings_category_t category, bool requires_restart) {
    struct setting_definition* def = (struct setting_definition*)kmalloc(sizeof(struct setting_definition));
    if (!def) return NULL;
    
    strcpy(def->name, name);
    strcpy(def->display_name, display_name);
    strcpy(def->description, description);
    def->type = type;
    def->access_level = SETTING_ACCESS_USER;
    def->category = category;
    def->requires_restart = requires_restart;
    def->is_readonly = false;
    def->change_callback = NULL;
    
    return def;
}

/* Add setting to category */
static void add_setting_to_category(settings_category_t category, struct setting_definition* def) {
    for (u32 i = 0; i < settings_mgr.category_count; i++) {
        if (settings_mgr.categories[i].id == category) {
            struct settings_category* cat = &settings_mgr.categories[i];
            if (cat->definition_count < MAX_SETTINGS_PER_CATEGORY) {
                cat->definitions[cat->definition_count++] = def;
            }
            break;
        }
    }
}

/* Get setting value */
bool settings_get_boolean(const char* name) {
    struct setting_value* value = find_setting_value(name);
    if (value && value->type == SETTING_TYPE_BOOLEAN) {
        return value->value.boolean_value;
    }
    
    /* Return default value */
    struct setting_definition* def = find_setting_definition(name);
    if (def && def->type == SETTING_TYPE_BOOLEAN) {
        return def->data.boolean.default_value;
    }
    
    return false;
}

i32 settings_get_integer(const char* name) {
    struct setting_value* value = find_setting_value(name);
    if (value && value->type == SETTING_TYPE_INTEGER) {
        return value->value.integer_value;
    }
    
    /* Return default value */
    struct setting_definition* def = find_setting_definition(name);
    if (def && def->type == SETTING_TYPE_INTEGER) {
        return def->data.integer.default_value;
    }
    
    return 0;
}

const char* settings_get_string(const char* name) {
    struct setting_value* value = find_setting_value(name);
    if (value && (value->type == SETTING_TYPE_STRING || value->type == SETTING_TYPE_ENUM)) {
        if (value->type == SETTING_TYPE_ENUM) {
            struct setting_definition* def = find_setting_definition(name);
            if (def && value->value.enum_index < def->data.enumeration.option_count) {
                return def->data.enumeration.options[value->value.enum_index];
            }
        } else {
            return value->value.string_value;
        }
    }
    
    /* Return default value */
    struct setting_definition* def = find_setting_definition(name);
    if (def) {
        if (def->type == SETTING_TYPE_STRING) {
            return def->data.string.default_value;
        } else if (def->type == SETTING_TYPE_ENUM) {
            u32 default_index = def->data.enumeration.default_index;
            if (default_index < def->data.enumeration.option_count) {
                return def->data.enumeration.options[default_index];
            }
        }
    }
    
    return "";
}

/* Set setting value */
bool settings_set_boolean(const char* name, bool value) {
    return set_setting_value(name, SETTING_TYPE_BOOLEAN, &value);
}

bool settings_set_integer(const char* name, i32 value) {
    return set_setting_value(name, SETTING_TYPE_INTEGER, &value);
}

bool settings_set_string(const char* name, const char* value) {
    return set_setting_value(name, SETTING_TYPE_STRING, value);
}

/* Set setting value (internal) */
static bool set_setting_value(const char* name, setting_type_t type, const void* value) {
    struct setting_value* setting = find_setting_value(name);
    
    if (!setting) {
        /* Create new setting value */
        setting = create_setting_value(name, type);
        if (!setting) return false;
    }
    
    if (setting->type != type) return false;
    
    /* Validate and set value */
    struct setting_definition* def = find_setting_definition(name);
    if (def && !validate_setting_value(def, value)) {
        return false;
    }
    
    /* Update value */
    switch (type) {
        case SETTING_TYPE_BOOLEAN:
            setting->value.boolean_value = *(bool*)value;
            break;
        case SETTING_TYPE_INTEGER:
            setting->value.integer_value = *(i32*)value;
            break;
        case SETTING_TYPE_STRING:
            strncpy(setting->value.string_value, (const char*)value, MAX_SETTING_VALUE - 1);
            setting->value.string_value[MAX_SETTING_VALUE - 1] = '\0';
            break;
        case SETTING_TYPE_ENUM:
            setting->value.enum_index = *(u32*)value;
            break;
        default:
            return false;
    }
    
    setting->last_modified = get_system_time();
    setting->is_dirty = true;
    
    /* Call change callback */
    if (def && def->change_callback) {
        def->change_callback(name, value);
    }
    
    /* Auto-save if enabled */
    if (settings_mgr.auto_save) {
        schedule_settings_save();
    }
    
    return true;
}

/* Find setting value */
static struct setting_value* find_setting_value(const char* name) {
    /* Check user settings first */
    for (u32 i = 0; i < settings_mgr.user_setting_count; i++) {
        if (strcmp(settings_mgr.user_settings[i].name, name) == 0) {
            return &settings_mgr.user_settings[i];
        }
    }
    
    /* Check system settings */
    for (u32 i = 0; i < settings_mgr.system_setting_count; i++) {
        if (strcmp(settings_mgr.system_settings[i].name, name) == 0) {
            return &settings_mgr.system_settings[i];
        }
    }
    
    return NULL;
}

/* Find setting definition */
static struct setting_definition* find_setting_definition(const char* name) {
    for (u32 i = 0; i < settings_mgr.category_count; i++) {
        struct settings_category* cat = &settings_mgr.categories[i];
        for (u32 j = 0; j < cat->definition_count; j++) {
            if (strcmp(cat->definitions[j]->name, name) == 0) {
                return cat->definitions[j];
            }
        }
    }
    return NULL;
}

/* Save settings to file */
void settings_save_all(void) {
    save_system_settings();
    save_user_settings();
    settings_mgr.last_save_time = get_system_time();
}

/* Load settings from files */
static void load_system_settings(void) {
    /* Implementation would load from SETTINGS_FILE_PATH */
    /* For now, use defaults */
}

static void load_user_settings(void) {
    /* Implementation would load from user-specific path */
    /* For now, use defaults */
}

/* Save settings to files */
static void save_system_settings(void) {
    /* Implementation would save to SETTINGS_FILE_PATH */
}

static void save_user_settings(void) {
    /* Implementation would save to user-specific path */
}

/* Settings update timer */
void settings_update(void) {
    if (!settings_mgr.initialized) return;
    
    u64 current_time = get_system_time();
    if (settings_mgr.auto_save && 
        (current_time - settings_mgr.last_save_time) >= (settings_mgr.save_interval * 1000000)) {
        settings_save_all();
    }
}

/* Get settings category */
struct settings_category* settings_get_category(settings_category_t category) {
    for (u32 i = 0; i < settings_mgr.category_count; i++) {
        if (settings_mgr.categories[i].id == category) {
            return &settings_mgr.categories[i];
        }
    }
    return NULL;
}

/* Get all categories */
struct settings_category* settings_get_all_categories(u32* count) {
    *count = settings_mgr.category_count;
    return settings_mgr.categories;
}
