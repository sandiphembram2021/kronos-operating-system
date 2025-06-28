#include "kronos.h"

/* Web Browser Application for Kronos OS */

#define MAX_URL_LENGTH 512
#define MAX_PAGE_CONTENT 8192
#define MAX_BOOKMARKS 50
#define MAX_HISTORY 100
#define MAX_TABS 8

/* Browser tab */
struct browser_tab {
    char url[MAX_URL_LENGTH];
    char title[128];
    char content[MAX_PAGE_CONTENT];
    bool loading;
    bool active;
    u32 scroll_position;
};

/* Bookmark */
struct bookmark {
    char url[MAX_URL_LENGTH];
    char title[128];
    bool in_use;
};

/* History entry */
struct history_entry {
    char url[MAX_URL_LENGTH];
    char title[128];
    u64 timestamp;
    bool in_use;
};

/* Web browser state */
struct web_browser {
    u32 window_id;
    struct browser_tab tabs[MAX_TABS];
    u32 active_tab;
    u32 tab_count;
    char address_bar[MAX_URL_LENGTH];
    struct bookmark bookmarks[MAX_BOOKMARKS];
    u32 bookmark_count;
    struct history_entry history[MAX_HISTORY];
    u32 history_count;
    bool show_bookmarks;
    bool show_history;
    bool active;
} browsers[2];

static u32 browser_count = 0;

/* Sample web pages */
static const char* sample_pages[][3] = {
    {"https://kronos-os.org", "Kronos OS - Official Website", 
     "Welcome to Kronos OS!\n\nKronos OS is an advanced 64-bit operating system with Ubuntu-like features.\n\nFeatures:\n- Advanced multitasking with CFS scheduler\n- Complete GUI desktop environment\n- Full application suite\n- Ubuntu-compatible commands\n- Modern security features\n\nDownload Kronos OS today and experience the future of computing!"},
    
    {"https://github.com/kronos-os", "Kronos OS - GitHub Repository",
     "Kronos OS Source Code\n\nOpen source operating system development.\n\nRepositories:\n- kronos-kernel: Core kernel and drivers\n- kronos-desktop: GUI and applications\n- kronos-tools: Development tools\n\nContribute to Kronos OS development!"},
    
    {"https://docs.kronos-os.org", "Kronos OS Documentation",
     "Kronos OS Documentation\n\nUser Guide:\n- Getting Started\n- System Administration\n- Application Development\n\nDeveloper Guide:\n- Kernel Development\n- Driver Development\n- GUI Programming\n\nAPI Reference:\n- System Calls\n- Library Functions"},
    
    {"https://news.kronos-os.org", "Kronos OS News",
     "Latest News\n\nKronos OS v2.0 Released!\nJanuary 15, 2025\n\nNew features include advanced multitasking, improved GUI, and enhanced security.\n\nKronos OS Wins Innovation Award\nDecember 20, 2024\n\nRecognized for outstanding OS design and implementation."},
    
    {"https://forum.kronos-os.org", "Kronos OS Community Forum",
     "Community Forum\n\nWelcome to the Kronos OS community!\n\nCategories:\n- General Discussion\n- Technical Support\n- Development\n- Feature Requests\n\nJoin thousands of Kronos OS users and developers!"}
};

/* Initialize web browser application */
void app_web_browser_init(void) {
    for (u32 i = 0; i < 2; i++) {
        browsers[i].active = false;
        browsers[i].window_id = 0;
    }
    browser_count = 0;
}

/* Launch web browser */
void app_launch_web_browser(u32 window_id) {
    if (browser_count >= 2) return;
    
    u32 slot = browser_count++;
    browsers[slot].window_id = window_id;
    browsers[slot].active_tab = 0;
    browsers[slot].tab_count = 1;
    browsers[slot].bookmark_count = 0;
    browsers[slot].history_count = 0;
    browsers[slot].show_bookmarks = false;
    browsers[slot].show_history = false;
    browsers[slot].active = true;
    
    /* Initialize first tab */
    struct browser_tab* tab = &browsers[slot].tabs[0];
    strcpy(tab->url, "https://kronos-os.org");
    strcpy(tab->title, "Kronos OS");
    tab->loading = false;
    tab->active = true;
    tab->scroll_position = 0;
    
    /* Load home page */
    browser_load_page(&browsers[slot], tab->url);
    
    /* Add default bookmarks */
    browser_add_bookmark(&browsers[slot], "https://kronos-os.org", "Kronos OS");
    browser_add_bookmark(&browsers[slot], "https://github.com/kronos-os", "GitHub");
    browser_add_bookmark(&browsers[slot], "https://docs.kronos-os.org", "Documentation");
    
    strcpy(browsers[slot].address_bar, tab->url);
}

/* Load web page */
static void browser_load_page(struct web_browser* browser, const char* url) {
    struct browser_tab* tab = &browser->tabs[browser->active_tab];
    
    tab->loading = true;
    strcpy(tab->url, url);
    strcpy(browser->address_bar, url);
    
    /* Find matching sample page */
    bool found = false;
    for (u32 i = 0; i < 5; i++) {
        if (strcmp(url, sample_pages[i][0]) == 0) {
            strcpy(tab->title, sample_pages[i][1]);
            strcpy(tab->content, sample_pages[i][2]);
            found = true;
            break;
        }
    }
    
    if (!found) {
        strcpy(tab->title, "Page Not Found");
        strcpy(tab->content, "404 - Page Not Found\n\nThe requested page could not be found.\n\nPlease check the URL and try again.");
    }
    
    tab->loading = false;
    tab->scroll_position = 0;
    
    /* Add to history */
    browser_add_to_history(browser, url, tab->title);
}

/* Add bookmark */
static void browser_add_bookmark(struct web_browser* browser, const char* url, const char* title) {
    if (browser->bookmark_count >= MAX_BOOKMARKS) return;
    
    struct bookmark* bookmark = &browser->bookmarks[browser->bookmark_count++];
    strcpy(bookmark->url, url);
    strcpy(bookmark->title, title);
    bookmark->in_use = true;
}

/* Add to history */
static void browser_add_to_history(struct web_browser* browser, const char* url, const char* title) {
    if (browser->history_count >= MAX_HISTORY) {
        /* Remove oldest entry */
        for (u32 i = 0; i < MAX_HISTORY - 1; i++) {
            browser->history[i] = browser->history[i + 1];
        }
        browser->history_count--;
    }
    
    struct history_entry* entry = &browser->history[browser->history_count++];
    strcpy(entry->url, url);
    strcpy(entry->title, title);
    entry->timestamp = get_system_time();
    entry->in_use = true;
}

/* Create new tab */
static void browser_new_tab(struct web_browser* browser) {
    if (browser->tab_count >= MAX_TABS) return;
    
    u32 new_tab = browser->tab_count++;
    struct browser_tab* tab = &browser->tabs[new_tab];
    
    strcpy(tab->url, "https://kronos-os.org");
    strcpy(tab->title, "New Tab");
    strcpy(tab->content, "");
    tab->loading = false;
    tab->active = false;
    tab->scroll_position = 0;
    
    /* Switch to new tab */
    browser->tabs[browser->active_tab].active = false;
    browser->active_tab = new_tab;
    tab->active = true;
    
    browser_load_page(browser, tab->url);
}

/* Close tab */
static void browser_close_tab(struct web_browser* browser, u32 tab_index) {
    if (browser->tab_count <= 1) return;  /* Keep at least one tab */
    
    /* Shift tabs */
    for (u32 i = tab_index; i < browser->tab_count - 1; i++) {
        browser->tabs[i] = browser->tabs[i + 1];
    }
    
    browser->tab_count--;
    
    /* Adjust active tab */
    if (browser->active_tab >= tab_index) {
        if (browser->active_tab > 0) {
            browser->active_tab--;
        }
    }
    
    browser->tabs[browser->active_tab].active = true;
}

/* Draw browser toolbar */
static void draw_browser_toolbar(u32* buffer, u32 buffer_width, struct web_browser* browser) {
    u32 toolbar_height = 40;
    
    /* Draw toolbar background */
    for (u32 y = 0; y < toolbar_height; y++) {
        for (u32 x = 0; x < buffer_width; x++) {
            buffer[y * buffer_width + x] = COLOR_LIGHT_GRAY;
        }
    }
    
    /* Draw navigation buttons */
    browser_draw_button(buffer, buffer_width, 10, 8, 25, 25, "←", COLOR_BLUE);  /* Back */
    browser_draw_button(buffer, buffer_width, 40, 8, 25, 25, "→", COLOR_BLUE);  /* Forward */
    browser_draw_button(buffer, buffer_width, 70, 8, 25, 25, "↻", COLOR_GREEN); /* Refresh */
    
    /* Draw address bar */
    u32 addr_x = 110;
    u32 addr_width = buffer_width - 200;
    
    for (u32 y = 8; y < 32; y++) {
        for (u32 x = addr_x; x < addr_x + addr_width; x++) {
            if (x < buffer_width) {
                buffer[y * buffer_width + x] = COLOR_WHITE;
            }
        }
    }
    
    /* Draw address text */
    browser_draw_text_to_buffer(buffer, buffer_width, addr_x + 5, 15, 
                               browser->address_bar, COLOR_BLACK);
    
    /* Draw menu button */
    browser_draw_button(buffer, buffer_width, buffer_width - 80, 8, 25, 25, "☰", COLOR_GRAY);
    
    /* Draw bookmark button */
    browser_draw_button(buffer, buffer_width, buffer_width - 50, 8, 25, 25, "★", COLOR_ORANGE);
}

/* Draw browser tabs */
static void draw_browser_tabs(u32* buffer, u32 buffer_width, struct web_browser* browser) {
    u32 tab_y = 40;
    u32 tab_height = 30;
    u32 tab_width = 150;
    
    for (u32 i = 0; i < browser->tab_count; i++) {
        struct browser_tab* tab = &browser->tabs[i];
        u32 tab_x = i * tab_width;
        u32 tab_color = tab->active ? COLOR_WHITE : COLOR_LIGHT_GRAY;
        u32 text_color = tab->active ? COLOR_BLACK : COLOR_GRAY;
        
        /* Draw tab background */
        for (u32 y = tab_y; y < tab_y + tab_height; y++) {
            for (u32 x = tab_x; x < tab_x + tab_width && x < buffer_width; x++) {
                buffer[y * buffer_width + x] = tab_color;
            }
        }
        
        /* Draw tab border */
        for (u32 x = tab_x; x < tab_x + tab_width && x < buffer_width; x++) {
            buffer[tab_y * buffer_width + x] = COLOR_GRAY;
            buffer[(tab_y + tab_height - 1) * buffer_width + x] = COLOR_GRAY;
        }
        
        /* Draw tab title */
        char short_title[20];
        strncpy(short_title, tab->title, 15);
        short_title[15] = '\0';
        if (strlen(tab->title) > 15) {
            strcat(short_title, "...");
        }
        
        browser_draw_text_to_buffer(buffer, buffer_width, tab_x + 5, tab_y + 10, 
                                   short_title, text_color);
        
        /* Draw close button */
        browser_draw_text_to_buffer(buffer, buffer_width, tab_x + tab_width - 20, tab_y + 10, 
                                   "×", COLOR_RED);
    }
    
    /* Draw new tab button */
    u32 new_tab_x = browser->tab_count * tab_width;
    if (new_tab_x + 30 < buffer_width) {
        browser_draw_button(buffer, buffer_width, new_tab_x, tab_y + 5, 25, 20, "+", COLOR_GREEN);
    }
}

/* Draw page content */
static void draw_page_content(u32* buffer, u32 buffer_width, struct web_browser* browser) {
    u32 content_y = 70;
    u32 content_height = 530;
    struct browser_tab* tab = &browser->tabs[browser->active_tab];
    
    /* Clear content area */
    for (u32 y = content_y; y < content_y + content_height; y++) {
        for (u32 x = 0; x < buffer_width; x++) {
            if (y < 600) {
                buffer[y * buffer_width + x] = COLOR_WHITE;
            }
        }
    }
    
    /* Draw loading indicator */
    if (tab->loading) {
        browser_draw_text_to_buffer(buffer, buffer_width, 20, content_y + 20, 
                                   "Loading...", COLOR_BLUE);
        return;
    }
    
    /* Draw page content */
    char* content = tab->content;
    u32 line_y = content_y + 20 - tab->scroll_position;
    char* line_start = content;
    
    while (*content && line_y < content_y + content_height) {
        if (*content == '\n' || *content == '\0') {
            /* Draw line */
            char line[256];
            u32 line_len = content - line_start;
            if (line_len > 255) line_len = 255;
            
            strncpy(line, line_start, line_len);
            line[line_len] = '\0';
            
            if (line_y >= content_y && line_y < content_y + content_height) {
                browser_draw_text_to_buffer(buffer, buffer_width, 20, line_y, line, COLOR_BLACK);
            }
            
            line_y += 16;
            line_start = content + 1;
        }
        content++;
    }
}

/* Draw bookmarks panel */
static void draw_bookmarks_panel(u32* buffer, u32 buffer_width, struct web_browser* browser) {
    if (!browser->show_bookmarks) return;
    
    u32 panel_x = buffer_width - 250;
    u32 panel_width = 240;
    u32 panel_height = 400;
    u32 panel_y = 70;
    
    /* Draw panel background */
    for (u32 y = panel_y; y < panel_y + panel_height; y++) {
        for (u32 x = panel_x; x < panel_x + panel_width; x++) {
            if (x < buffer_width && y < 600) {
                buffer[y * buffer_width + x] = COLOR_LIGHT_GRAY;
            }
        }
    }
    
    /* Draw panel title */
    browser_draw_text_to_buffer(buffer, buffer_width, panel_x + 10, panel_y + 10, 
                               "Bookmarks", COLOR_BLACK);
    
    /* Draw bookmarks */
    for (u32 i = 0; i < browser->bookmark_count && i < 15; i++) {
        u32 item_y = panel_y + 40 + i * 25;
        browser_draw_text_to_buffer(buffer, buffer_width, panel_x + 10, item_y, 
                                   browser->bookmarks[i].title, COLOR_BLUE);
    }
}

/* Draw button */
static void browser_draw_button(u32* buffer, u32 buffer_width, u32 x, u32 y, 
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
    browser_draw_text_to_buffer(buffer, buffer_width, text_x, text_y, text, COLOR_WHITE);
}

/* Render web browser */
void browser_render(struct web_browser* browser) {
    if (!browser->active) return;
    
    u32* buffer = wm_get_window_buffer(browser->window_id);
    if (!buffer) return;
    
    u32 win_width = 900;
    u32 win_height = 600;
    
    /* Clear window buffer */
    for (u32 i = 0; i < win_width * win_height; i++) {
        buffer[i] = COLOR_WHITE;
    }
    
    /* Draw browser components */
    draw_browser_toolbar(buffer, win_width, browser);
    draw_browser_tabs(buffer, win_width, browser);
    draw_page_content(buffer, win_width, browser);
    draw_bookmarks_panel(buffer, win_width, browser);
}

/* Handle browser click */
void browser_handle_click(struct web_browser* browser, u32 x, u32 y) {
    /* Check toolbar clicks */
    if (y < 40) {
        if (x >= 10 && x < 35) {
            /* Back button */
            browser_navigate_back(browser);
        } else if (x >= 40 && x < 65) {
            /* Forward button */
            browser_navigate_forward(browser);
        } else if (x >= 70 && x < 95) {
            /* Refresh button */
            browser_refresh(browser);
        } else if (x >= 110 && x < 800) {
            /* Address bar click */
            browser_focus_address_bar(browser);
        }
        return;
    }
    
    /* Check tab clicks */
    if (y >= 40 && y < 70) {
        u32 tab_index = x / 150;
        if (tab_index < browser->tab_count) {
            /* Switch tab */
            browser->tabs[browser->active_tab].active = false;
            browser->active_tab = tab_index;
            browser->tabs[tab_index].active = true;
            strcpy(browser->address_bar, browser->tabs[tab_index].url);
        } else if (x >= browser->tab_count * 150 && x < browser->tab_count * 150 + 30) {
            /* New tab button */
            browser_new_tab(browser);
        }
        return;
    }
    
    /* Check bookmarks panel */
    if (browser->show_bookmarks && x >= 650) {
        u32 bookmark_index = (y - 110) / 25;
        if (bookmark_index < browser->bookmark_count) {
            browser_load_page(browser, browser->bookmarks[bookmark_index].url);
        }
        return;
    }
}

/* Navigation functions */
static void browser_navigate_back(struct web_browser* browser) {
    /* Simple back navigation - in real implementation would use history stack */
    browser_load_page(browser, "https://kronos-os.org");
}

static void browser_navigate_forward(struct web_browser* browser) {
    /* Forward navigation */
}

static void browser_refresh(struct web_browser* browser) {
    struct browser_tab* tab = &browser->tabs[browser->active_tab];
    browser_load_page(browser, tab->url);
}

static void browser_focus_address_bar(struct web_browser* browser) {
    /* Focus address bar for editing */
}

/* Draw text to buffer */
void browser_draw_text_to_buffer(u32* buffer, u32 buffer_width, u32 x, u32 y, 
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

/* Handle keyboard input for address bar */
void browser_handle_keyboard(struct web_browser* browser, char c) {
    if (c == '\n' || c == '\r') {
        /* Navigate to URL */
        browser_load_page(browser, browser->address_bar);
    } else if (c == '\b') {
        /* Backspace */
        u32 len = strlen(browser->address_bar);
        if (len > 0) {
            browser->address_bar[len - 1] = '\0';
        }
    } else if (c >= 32 && c <= 126) {
        /* Printable character */
        u32 len = strlen(browser->address_bar);
        if (len < MAX_URL_LENGTH - 1) {
            browser->address_bar[len] = c;
            browser->address_bar[len + 1] = '\0';
        }
    }
}

/* Find browser by window ID */
struct web_browser* browser_find_by_window(u32 window_id) {
    for (u32 i = 0; i < 2; i++) {
        if (browsers[i].active && browsers[i].window_id == window_id) {
            return &browsers[i];
        }
    }
    return NULL;
}
