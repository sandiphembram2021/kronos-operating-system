#include "kronos.h"

/* Package Manager Application for Kronos OS */

#define MAX_PACKAGES 500
#define MAX_REPOSITORIES 10
#define MAX_PACKAGE_NAME 64
#define MAX_PACKAGE_DESC 256

/* Package status */
typedef enum {
    PKG_NOT_INSTALLED,
    PKG_INSTALLED,
    PKG_UPGRADABLE,
    PKG_BROKEN
} package_status_t;

/* Package category */
typedef enum {
    PKG_SYSTEM,
    PKG_DEVELOPMENT,
    PKG_GRAPHICS,
    PKG_MULTIMEDIA,
    PKG_OFFICE,
    PKG_GAMES,
    PKG_NETWORK,
    PKG_UTILITIES
} package_category_t;

/* Package information */
struct package_info {
    char name[MAX_PACKAGE_NAME];
    char description[MAX_PACKAGE_DESC];
    char version[32];
    char maintainer[64];
    u64 size;
    package_status_t status;
    package_category_t category;
    bool essential;
    char dependencies[256];
    u32 download_progress;
    bool in_use;
};

/* Repository information */
struct repository {
    char name[64];
    char url[256];
    bool enabled;
    bool secure;
    u32 package_count;
};

/* Package manager view modes */
typedef enum {
    VIEW_ALL_PACKAGES,
    VIEW_INSTALLED,
    VIEW_UPGRADABLE,
    VIEW_CATEGORIES,
    VIEW_SEARCH_RESULTS
} view_mode_t;

/* Package manager state */
struct package_manager {
    u32 window_id;
    struct package_info packages[MAX_PACKAGES];
    u32 package_count;
    struct repository repositories[MAX_REPOSITORIES];
    u32 repository_count;
    view_mode_t current_view;
    package_category_t selected_category;
    char search_query[128];
    u32 selected_package;
    u32 scroll_position;
    bool show_details;
    bool updating_cache;
    bool active;
} package_managers[2];

static u32 pm_count = 0;

/* Sample packages */
static struct package_info sample_packages[] = {
    {"gcc", "GNU Compiler Collection", "11.2.0", "GNU Project", 45000000, PKG_INSTALLED, PKG_DEVELOPMENT, false, "libc6, binutils", 0, true},
    {"python3", "Python 3 programming language", "3.9.7", "Python Foundation", 25000000, PKG_INSTALLED, PKG_DEVELOPMENT, false, "libc6", 0, true},
    {"git", "Distributed version control system", "2.34.1", "Git Team", 8000000, PKG_INSTALLED, PKG_DEVELOPMENT, false, "libc6, curl", 0, true},
    {"vim", "Vi IMproved text editor", "8.2.3458", "Vim Team", 3500000, PKG_INSTALLED, PKG_UTILITIES, false, "libc6", 0, true},
    {"firefox", "Mozilla Firefox web browser", "95.0.1", "Mozilla", 85000000, PKG_NOT_INSTALLED, PKG_NETWORK, false, "gtk3, dbus", 0, true},
    {"libreoffice", "Office productivity suite", "7.2.3", "LibreOffice Team", 350000000, PKG_NOT_INSTALLED, PKG_OFFICE, false, "gtk3, java", 0, true},
    {"gimp", "GNU Image Manipulation Program", "2.10.28", "GIMP Team", 45000000, PKG_NOT_INSTALLED, PKG_GRAPHICS, false, "gtk3, gegl", 0, true},
    {"vlc", "VLC media player", "3.0.16", "VideoLAN", 25000000, PKG_INSTALLED, PKG_MULTIMEDIA, false, "ffmpeg, alsa", 0, true},
    {"blender", "3D creation suite", "3.0.0", "Blender Foundation", 180000000, PKG_NOT_INSTALLED, PKG_GRAPHICS, false, "opengl, python3", 0, true},
    {"steam", "Gaming platform", "1.0.0.70", "Valve", 120000000, PKG_NOT_INSTALLED, PKG_GAMES, false, "mesa, pulseaudio", 0, true},
    {"docker", "Container platform", "20.10.12", "Docker Inc", 75000000, PKG_UPGRADABLE, PKG_DEVELOPMENT, false, "systemd, iptables", 0, true},
    {"nodejs", "JavaScript runtime", "16.13.1", "Node.js Foundation", 35000000, PKG_INSTALLED, PKG_DEVELOPMENT, false, "libc6, openssl", 0, true},
    {"code", "Visual Studio Code", "1.63.2", "Microsoft", 95000000, PKG_NOT_INSTALLED, PKG_DEVELOPMENT, false, "electron, gtk3", 0, true},
    {"thunderbird", "Email client", "91.4.1", "Mozilla", 65000000, PKG_NOT_INSTALLED, PKG_NETWORK, false, "gtk3, dbus", 0, true},
    {"audacity", "Audio editor", "3.1.3", "Audacity Team", 28000000, PKG_NOT_INSTALLED, PKG_MULTIMEDIA, false, "alsa, gtk3", 0, true}
};

/* Initialize package manager */
void app_package_manager_init(void) {
    for (u32 i = 0; i < 2; i++) {
        package_managers[i].active = false;
        package_managers[i].window_id = 0;
    }
    pm_count = 0;
}

/* Launch package manager */
void app_launch_package_manager(u32 window_id) {
    if (pm_count >= 2) return;
    
    u32 slot = pm_count++;
    package_managers[slot].window_id = window_id;
    package_managers[slot].current_view = VIEW_ALL_PACKAGES;
    package_managers[slot].selected_category = PKG_SYSTEM;
    package_managers[slot].selected_package = 0;
    package_managers[slot].scroll_position = 0;
    package_managers[slot].show_details = false;
    package_managers[slot].updating_cache = false;
    package_managers[slot].active = true;
    
    /* Initialize package list */
    package_managers[slot].package_count = sizeof(sample_packages) / sizeof(sample_packages[0]);
    for (u32 i = 0; i < package_managers[slot].package_count; i++) {
        package_managers[slot].packages[i] = sample_packages[i];
    }
    
    /* Initialize repositories */
    package_managers[slot].repository_count = 3;
    strcpy(package_managers[slot].repositories[0].name, "Kronos Main");
    strcpy(package_managers[slot].repositories[0].url, "http://packages.kronos-os.org/main");
    package_managers[slot].repositories[0].enabled = true;
    package_managers[slot].repositories[0].secure = true;
    
    strcpy(package_managers[slot].repositories[1].name, "Kronos Universe");
    strcpy(package_managers[slot].repositories[1].url, "http://packages.kronos-os.org/universe");
    package_managers[slot].repositories[1].enabled = true;
    package_managers[slot].repositories[1].secure = true;
    
    strcpy(package_managers[slot].repositories[2].name, "Third Party");
    strcpy(package_managers[slot].repositories[2].url, "http://ppa.kronos-os.org/");
    package_managers[slot].repositories[2].enabled = false;
    package_managers[slot].repositories[2].secure = false;
    
    strcpy(package_managers[slot].search_query, "");
}

/* Get package status icon */
static const char* get_status_icon(package_status_t status) {
    switch (status) {
        case PKG_INSTALLED: return "✓";
        case PKG_UPGRADABLE: return "↑";
        case PKG_BROKEN: return "✗";
        default: return " ";
    }
}

/* Get category name */
static const char* get_category_name(package_category_t category) {
    switch (category) {
        case PKG_SYSTEM: return "System";
        case PKG_DEVELOPMENT: return "Development";
        case PKG_GRAPHICS: return "Graphics";
        case PKG_MULTIMEDIA: return "Multimedia";
        case PKG_OFFICE: return "Office";
        case PKG_GAMES: return "Games";
        case PKG_NETWORK: return "Network";
        case PKG_UTILITIES: return "Utilities";
        default: return "Other";
    }
}

/* Format file size */
static void format_package_size(u64 size, char* buffer) {
    if (size < 1024) {
        sprintf(buffer, "%d B", size);
    } else if (size < 1024 * 1024) {
        sprintf(buffer, "%d KB", size / 1024);
    } else if (size < 1024 * 1024 * 1024) {
        sprintf(buffer, "%d MB", size / (1024 * 1024));
    } else {
        sprintf(buffer, "%.1f GB", size / (1024.0 * 1024.0 * 1024.0));
    }
}

/* Draw package manager toolbar */
static void draw_pm_toolbar(u32* buffer, u32 buffer_width, struct package_manager* pm) {
    u32 toolbar_height = 50;
    
    /* Draw toolbar background */
    for (u32 y = 0; y < toolbar_height; y++) {
        for (u32 x = 0; x < buffer_width; x++) {
            buffer[y * buffer_width + x] = COLOR_LIGHT_GRAY;
        }
    }
    
    /* Draw buttons */
    pm_draw_button(buffer, buffer_width, 10, 10, 80, 30, "Update", COLOR_BLUE);
    pm_draw_button(buffer, buffer_width, 100, 10, 80, 30, "Upgrade", COLOR_GREEN);
    pm_draw_button(buffer, buffer_width, 190, 10, 80, 30, "Install", COLOR_ORANGE);
    pm_draw_button(buffer, buffer_width, 280, 10, 80, 30, "Remove", COLOR_RED);
    
    /* Draw search box */
    u32 search_x = 400;
    u32 search_width = 200;
    
    for (u32 y = 15; y < 35; y++) {
        for (u32 x = search_x; x < search_x + search_width; x++) {
            if (x < buffer_width) {
                buffer[y * buffer_width + x] = COLOR_WHITE;
            }
        }
    }
    
    pm_draw_text_to_buffer(buffer, buffer_width, search_x + 5, 20, "Search packages...", COLOR_GRAY);
    
    /* Draw view selector */
    pm_draw_button(buffer, buffer_width, buffer_width - 150, 10, 60, 30, "All", COLOR_GRAY);
    pm_draw_button(buffer, buffer_width, buffer_width - 80, 10, 60, 30, "Installed", COLOR_GRAY);
}

/* Draw package list */
static void draw_package_list(u32* buffer, u32 buffer_width, struct package_manager* pm) {
    u32 list_y = 50;
    u32 list_height = 450;
    u32 item_height = 60;
    
    /* Clear list area */
    for (u32 y = list_y; y < list_y + list_height; y++) {
        for (u32 x = 0; x < buffer_width; x++) {
            if (y < 600) {
                buffer[y * buffer_width + x] = COLOR_WHITE;
            }
        }
    }
    
    /* Draw package items */
    u32 visible_items = list_height / item_height;
    for (u32 i = pm->scroll_position; i < pm->package_count && i < pm->scroll_position + visible_items; i++) {
        struct package_info* pkg = &pm->packages[i];
        u32 item_y = list_y + (i - pm->scroll_position) * item_height;
        
        /* Highlight selected item */
        if (i == pm->selected_package) {
            for (u32 y = item_y; y < item_y + item_height; y++) {
                for (u32 x = 0; x < buffer_width; x++) {
                    if (y < 600) {
                        buffer[y * buffer_width + x] = UBUNTU_LIGHT;
                    }
                }
            }
        }
        
        /* Draw status icon */
        pm_draw_text_to_buffer(buffer, buffer_width, 10, item_y + 10, 
                              get_status_icon(pkg->status), COLOR_GREEN);
        
        /* Draw package name */
        pm_draw_text_to_buffer(buffer, buffer_width, 30, item_y + 10, pkg->name, COLOR_BLACK);
        
        /* Draw version */
        pm_draw_text_to_buffer(buffer, buffer_width, 200, item_y + 10, pkg->version, COLOR_BLUE);
        
        /* Draw description */
        pm_draw_text_to_buffer(buffer, buffer_width, 30, item_y + 30, pkg->description, COLOR_GRAY);
        
        /* Draw size */
        char size_str[32];
        format_package_size(pkg->size, size_str);
        pm_draw_text_to_buffer(buffer, buffer_width, buffer_width - 100, item_y + 10, size_str, COLOR_GRAY);
        
        /* Draw category */
        pm_draw_text_to_buffer(buffer, buffer_width, buffer_width - 100, item_y + 30, 
                              get_category_name(pkg->category), COLOR_PURPLE);
        
        /* Draw separator line */
        for (u32 x = 10; x < buffer_width - 10; x++) {
            if (item_y + item_height - 1 < 600) {
                buffer[(item_y + item_height - 1) * buffer_width + x] = COLOR_LIGHT_GRAY;
            }
        }
    }
}

/* Draw package details panel */
static void draw_package_details(u32* buffer, u32 buffer_width, struct package_manager* pm) {
    if (!pm->show_details) return;
    
    u32 panel_x = buffer_width - 300;
    u32 panel_width = 290;
    u32 panel_height = 450;
    u32 panel_y = 50;
    
    /* Draw panel background */
    for (u32 y = panel_y; y < panel_y + panel_height; y++) {
        for (u32 x = panel_x; x < panel_x + panel_width; x++) {
            if (x < buffer_width && y < 600) {
                buffer[y * buffer_width + x] = COLOR_LIGHT_GRAY;
            }
        }
    }
    
    if (pm->selected_package < pm->package_count) {
        struct package_info* pkg = &pm->packages[pm->selected_package];
        
        /* Draw package details */
        pm_draw_text_to_buffer(buffer, buffer_width, panel_x + 10, panel_y + 10, 
                              "Package Details", COLOR_BLACK);
        
        pm_draw_text_to_buffer(buffer, buffer_width, panel_x + 10, panel_y + 40, 
                              "Name:", COLOR_BLACK);
        pm_draw_text_to_buffer(buffer, buffer_width, panel_x + 60, panel_y + 40, 
                              pkg->name, COLOR_BLUE);
        
        pm_draw_text_to_buffer(buffer, buffer_width, panel_x + 10, panel_y + 60, 
                              "Version:", COLOR_BLACK);
        pm_draw_text_to_buffer(buffer, buffer_width, panel_x + 70, panel_y + 60, 
                              pkg->version, COLOR_BLACK);
        
        pm_draw_text_to_buffer(buffer, buffer_width, panel_x + 10, panel_y + 80, 
                              "Maintainer:", COLOR_BLACK);
        pm_draw_text_to_buffer(buffer, buffer_width, panel_x + 10, panel_y + 100, 
                              pkg->maintainer, COLOR_BLACK);
        
        char size_str[32];
        format_package_size(pkg->size, size_str);
        pm_draw_text_to_buffer(buffer, buffer_width, panel_x + 10, panel_y + 120, 
                              "Size:", COLOR_BLACK);
        pm_draw_text_to_buffer(buffer, buffer_width, panel_x + 50, panel_y + 120, 
                              size_str, COLOR_BLACK);
        
        pm_draw_text_to_buffer(buffer, buffer_width, panel_x + 10, panel_y + 140, 
                              "Category:", COLOR_BLACK);
        pm_draw_text_to_buffer(buffer, buffer_width, panel_x + 10, panel_y + 160, 
                              get_category_name(pkg->category), COLOR_PURPLE);
        
        pm_draw_text_to_buffer(buffer, buffer_width, panel_x + 10, panel_y + 180, 
                              "Dependencies:", COLOR_BLACK);
        pm_draw_text_to_buffer(buffer, buffer_width, panel_x + 10, panel_y + 200, 
                              pkg->dependencies, COLOR_GRAY);
        
        /* Action buttons */
        if (pkg->status == PKG_NOT_INSTALLED) {
            pm_draw_button(buffer, buffer_width, panel_x + 10, panel_y + 250, 100, 30, "Install", COLOR_GREEN);
        } else if (pkg->status == PKG_INSTALLED) {
            pm_draw_button(buffer, buffer_width, panel_x + 10, panel_y + 250, 100, 30, "Remove", COLOR_RED);
        } else if (pkg->status == PKG_UPGRADABLE) {
            pm_draw_button(buffer, buffer_width, panel_x + 10, panel_y + 250, 100, 30, "Upgrade", COLOR_BLUE);
        }
    }
}

/* Draw button */
static void pm_draw_button(u32* buffer, u32 buffer_width, u32 x, u32 y, 
                          u32 width, u32 height, const char* text, u32 color) {
    /* Draw button background */
    for (u32 dy = 0; dy < height; dy++) {
        for (u32 dx = 0; dx < width; dx++) {
            if (x + dx < buffer_width && y + dy < 600) {
                buffer[(y + dy) * buffer_width + (x + dx)] = color;
            }
        }
    }
    
    /* Draw button text */
    u32 text_x = x + (width - strlen(text) * 8) / 2;
    u32 text_y = y + (height - 8) / 2;
    pm_draw_text_to_buffer(buffer, buffer_width, text_x, text_y, text, COLOR_WHITE);
}

/* Render package manager */
void package_manager_render(struct package_manager* pm) {
    if (!pm->active) return;
    
    u32* buffer = wm_get_window_buffer(pm->window_id);
    if (!buffer) return;
    
    u32 win_width = 900;
    u32 win_height = 600;
    
    /* Clear window buffer */
    for (u32 i = 0; i < win_width * win_height; i++) {
        buffer[i] = COLOR_WHITE;
    }
    
    /* Draw components */
    draw_pm_toolbar(buffer, win_width, pm);
    draw_package_list(buffer, win_width, pm);
    draw_package_details(buffer, win_width, pm);
}

/* Handle package manager click */
void package_manager_handle_click(struct package_manager* pm, u32 x, u32 y) {
    /* Check toolbar clicks */
    if (y < 50) {
        if (x >= 10 && x < 90) {
            /* Update button */
            pm_update_package_cache(pm);
        } else if (x >= 100 && x < 180) {
            /* Upgrade button */
            pm_upgrade_packages(pm);
        } else if (x >= 190 && x < 270) {
            /* Install button */
            pm_install_selected_package(pm);
        } else if (x >= 280 && x < 360) {
            /* Remove button */
            pm_remove_selected_package(pm);
        }
        return;
    }
    
    /* Check package list clicks */
    if (y >= 50 && y < 500) {
        u32 item_index = (y - 50) / 60 + pm->scroll_position;
        if (item_index < pm->package_count) {
            pm->selected_package = item_index;
            pm->show_details = true;
        }
        return;
    }
}

/* Package operations */
static void pm_update_package_cache(struct package_manager* pm) {
    pm->updating_cache = true;
    /* Simulate cache update */
    timer_sleep(3000000);  /* 3 seconds */
    pm->updating_cache = false;
}

static void pm_upgrade_packages(struct package_manager* pm) {
    /* Upgrade all upgradable packages */
    for (u32 i = 0; i < pm->package_count; i++) {
        if (pm->packages[i].status == PKG_UPGRADABLE) {
            pm->packages[i].status = PKG_INSTALLED;
        }
    }
}

static void pm_install_selected_package(struct package_manager* pm) {
    if (pm->selected_package < pm->package_count) {
        struct package_info* pkg = &pm->packages[pm->selected_package];
        if (pkg->status == PKG_NOT_INSTALLED) {
            pkg->status = PKG_INSTALLED;
        }
    }
}

static void pm_remove_selected_package(struct package_manager* pm) {
    if (pm->selected_package < pm->package_count) {
        struct package_info* pkg = &pm->packages[pm->selected_package];
        if (pkg->status == PKG_INSTALLED && !pkg->essential) {
            pkg->status = PKG_NOT_INSTALLED;
        }
    }
}

/* Draw text to buffer */
void pm_draw_text_to_buffer(u32* buffer, u32 buffer_width, u32 x, u32 y, 
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

/* Find package manager by window ID */
struct package_manager* package_manager_find_by_window(u32 window_id) {
    for (u32 i = 0; i < 2; i++) {
        if (package_managers[i].active && package_managers[i].window_id == window_id) {
            return &package_managers[i];
        }
    }
    return NULL;
}
