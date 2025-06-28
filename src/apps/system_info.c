#include "kronos.h"

/* Advanced System Information Application for Kronos OS */

#define MAX_INFO_CATEGORIES 10
#define MAX_EXPORT_SIZE 8192

/* System info view modes */
typedef enum {
    SYSINFO_VIEW_OVERVIEW,
    SYSINFO_VIEW_HARDWARE,
    SYSINFO_VIEW_SOFTWARE,
    SYSINFO_VIEW_PERFORMANCE,
    SYSINFO_VIEW_NETWORK,
    SYSINFO_VIEW_STORAGE,
    SYSINFO_VIEW_PROCESSES
} sysinfo_view_t;

/* System info categories */
typedef enum {
    SYSINFO_CAT_OVERVIEW,
    SYSINFO_CAT_CPU,
    SYSINFO_CAT_MEMORY,
    SYSINFO_CAT_STORAGE,
    SYSINFO_CAT_GRAPHICS,
    SYSINFO_CAT_NETWORK,
    SYSINFO_CAT_SYSTEM,
    SYSINFO_CAT_PERFORMANCE
} sysinfo_category_t;

/* System info application state */
struct system_info_app {
    u32 window_id;
    sysinfo_view_t current_view;
    sysinfo_category_t selected_category;
    u32 scroll_position;
    bool auto_refresh;
    u32 refresh_interval;  /* seconds */
    u64 last_refresh;
    bool show_advanced;
    char export_buffer[MAX_EXPORT_SIZE];
    bool active;
} sysinfo_apps[2];

static u32 sysinfo_count = 0;

/* Initialize system info application */
void app_system_info_init(void) {
    for (u32 i = 0; i < 2; i++) {
        sysinfo_apps[i].active = false;
        sysinfo_apps[i].window_id = 0;
    }
    sysinfo_count = 0;
}

/* Launch system info application */
void app_launch_system_info(u32 window_id) {
    if (sysinfo_count >= 2) return;

    u32 slot = sysinfo_count++;
    sysinfo_apps[slot].window_id = window_id;
    sysinfo_apps[slot].current_view = SYSINFO_VIEW_OVERVIEW;
    sysinfo_apps[slot].selected_category = SYSINFO_CAT_OVERVIEW;
    sysinfo_apps[slot].scroll_position = 0;
    sysinfo_apps[slot].auto_refresh = true;
    sysinfo_apps[slot].refresh_interval = 2;  /* 2 seconds */
    sysinfo_apps[slot].last_refresh = get_system_time();
    sysinfo_apps[slot].show_advanced = false;
    sysinfo_apps[slot].active = true;

    /* Initialize system info backend if not already done */
    system_info_init();
}

/* Draw system info sidebar */
static void draw_sysinfo_sidebar(u32* buffer, u32 buffer_width, struct system_info_app* app) {
    u32 sidebar_width = 200;

    /* Draw sidebar background */
    for (u32 y = 0; y < 600; y++) {
        for (u32 x = 0; x < sidebar_width; x++) {
            if (x < buffer_width && y < 600) {
                buffer[y * buffer_width + x] = COLOR_LIGHT_GRAY;
            }
        }
    }

    /* Category items */
    const char* categories[] = {
        "📊 Overview",
        "🖥️ CPU",
        "💾 Memory",
        "💿 Storage",
        "🎮 Graphics",
        "🌐 Network",
        "ℹ️ System",
        "⚡ Performance"
    };

    for (u32 i = 0; i < 8; i++) {
        u32 item_y = 20 + i * 50;
        u32 item_color = (i == app->selected_category) ? UBUNTU_ORANGE : COLOR_LIGHT_GRAY;
        u32 text_color = (i == app->selected_category) ? COLOR_WHITE : COLOR_BLACK;

        /* Draw selection background */
        if (i == app->selected_category) {
            for (u32 y = item_y; y < item_y + 40; y++) {
                for (u32 x = 5; x < sidebar_width - 5; x++) {
                    if (x < buffer_width && y < 600) {
                        buffer[y * buffer_width + x] = item_color;
                    }
                }
            }
        }

        /* Draw category text */
        sysinfo_draw_text_to_buffer(buffer, buffer_width, 15, item_y + 15,
                                   categories[i], text_color);
    }

    /* Draw refresh controls */
    u32 controls_y = 450;
    sysinfo_draw_text_to_buffer(buffer, buffer_width, 15, controls_y,
                               "Auto Refresh:", COLOR_BLACK);

    const char* refresh_text = app->auto_refresh ? "ON" : "OFF";
    u32 refresh_color = app->auto_refresh ? COLOR_GREEN : COLOR_RED;
    sysinfo_draw_text_to_buffer(buffer, buffer_width, 15, controls_y + 20,
                               refresh_text, refresh_color);

    /* Draw export button */
    sysinfo_draw_button(buffer, buffer_width, 15, controls_y + 50, 150, 25,
                       "Export Info", COLOR_BLUE);

    /* Draw sidebar border */
    for (u32 y = 0; y < 600; y++) {
        if (sidebar_width < buffer_width && y < 600) {
            buffer[y * buffer_width + sidebar_width] = COLOR_GRAY;
        }
    }
}

/* Draw overview content */
static void draw_overview_content(u32* buffer, u32 buffer_width, struct system_info_app* app) {
    u32 content_x = 220;
    struct system_info* info = get_system_info();

    /* Clear content area */
    for (u32 y = 0; y < 600; y++) {
        for (u32 x = content_x; x < buffer_width; x++) {
            buffer[y * buffer_width + x] = COLOR_WHITE;
        }
    }

    /* Draw title */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, 20,
                               "System Overview", COLOR_BLACK);

    /* System info grid */
    u32 start_y = 60;
    u32 col1_x = content_x + 20;
    u32 col2_x = content_x + 300;

    /* Operating System */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col1_x, start_y,
                               "Operating System:", COLOR_BLACK);
    char os_info[128];
    snprintf(os_info, sizeof(os_info), "%s %s", info->os_name, info->os_version);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col1_x, start_y + 15,
                               os_info, COLOR_BLUE);

    /* Hostname */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col2_x, start_y,
                               "Computer Name:", COLOR_BLACK);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col2_x, start_y + 15,
                               info->hostname, COLOR_BLUE);

    /* CPU */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col1_x, start_y + 50,
                               "Processor:", COLOR_BLACK);
    char cpu_info[128];
    snprintf(cpu_info, sizeof(cpu_info), "%u cores @ %.1fGHz",
             info->cpu.cores, info->cpu.base_frequency / 1000.0f);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col1_x, start_y + 65,
                               cpu_info, COLOR_BLUE);

    /* Memory */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col2_x, start_y + 50,
                               "Memory:", COLOR_BLACK);
    char mem_info[64];
    format_bytes(info->memory.total_physical, mem_info, sizeof(mem_info));
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col2_x, start_y + 65,
                               mem_info, COLOR_BLUE);

    /* Uptime */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col1_x, start_y + 100,
                               "Uptime:", COLOR_BLACK);
    char uptime_str[64];
    format_uptime(info->uptime, uptime_str, sizeof(uptime_str));
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col1_x, start_y + 115,
                               uptime_str, COLOR_BLUE);

    /* Load Average */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col2_x, start_y + 100,
                               "Load Average:", COLOR_BLACK);
    char load_str[64];
    snprintf(load_str, sizeof(load_str), "%.2f %.2f %.2f",
             info->cpu_load_1min, info->cpu_load_5min, info->cpu_load_15min);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col2_x, start_y + 115,
                               load_str, COLOR_BLUE);

    /* Resource usage bars */
    u32 bars_y = start_y + 160;

    /* CPU Usage Bar */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col1_x, bars_y,
                               "CPU Usage:", COLOR_BLACK);
    draw_usage_bar(buffer, buffer_width, col1_x, bars_y + 20, 200, 15,
                   info->cpu.usage_percent, COLOR_GREEN);

    char cpu_usage[32];
    snprintf(cpu_usage, sizeof(cpu_usage), "%.1f%%", info->cpu.usage_percent);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col1_x + 210, bars_y + 20,
                               cpu_usage, COLOR_BLACK);

    /* Memory Usage Bar */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col1_x, bars_y + 50,
                               "Memory Usage:", COLOR_BLACK);
    float mem_usage = get_memory_usage();
    draw_usage_bar(buffer, buffer_width, col1_x, bars_y + 70, 200, 15,
                   mem_usage, COLOR_BLUE);

    char mem_usage_str[32];
    snprintf(mem_usage_str, sizeof(mem_usage_str), "%.1f%%", mem_usage);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col1_x + 210, bars_y + 70,
                               mem_usage_str, COLOR_BLACK);

    /* Storage Usage Bar */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col1_x, bars_y + 100,
                               "Storage Usage:", COLOR_BLACK);
    float storage_usage = get_storage_usage(0);  /* Primary drive */
    draw_usage_bar(buffer, buffer_width, col1_x, bars_y + 120, 200, 15,
                   storage_usage, COLOR_ORANGE);

    char storage_usage_str[32];
    snprintf(storage_usage_str, sizeof(storage_usage_str), "%.1f%%", storage_usage);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, col1_x + 210, bars_y + 120,
                               storage_usage_str, COLOR_BLACK);
}

/* Draw CPU details */
static void draw_cpu_content(u32* buffer, u32 buffer_width, struct system_info_app* app) {
    u32 content_x = 220;
    struct system_info* info = get_system_info();
    struct cpu_info* cpu = &info->cpu;

    /* Clear content area */
    for (u32 y = 0; y < 600; y++) {
        for (u32 x = content_x; x < buffer_width; x++) {
            buffer[y * buffer_width + x] = COLOR_WHITE;
        }
    }

    /* Draw title */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, 20,
                               "CPU Information", COLOR_BLACK);

    u32 start_y = 60;
    u32 line_height = 25;
    u32 current_y = start_y;

    /* CPU details */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y,
                               "Processor Model:", COLOR_BLACK);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y + 15,
                               cpu->model, COLOR_BLUE);
    current_y += 40;

    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y,
                               "Vendor:", COLOR_BLACK);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 120, current_y,
                               cpu->vendor, COLOR_BLUE);
    current_y += line_height;

    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y,
                               "Architecture:", COLOR_BLACK);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 120, current_y,
                               cpu->architecture, COLOR_BLUE);
    current_y += line_height;

    char cores_str[32];
    snprintf(cores_str, sizeof(cores_str), "%u cores, %u threads", cpu->cores, cpu->threads);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y,
                               "Cores/Threads:", COLOR_BLACK);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 120, current_y,
                               cores_str, COLOR_BLUE);
    current_y += line_height;

    char freq_str[64];
    snprintf(freq_str, sizeof(freq_str), "%u MHz (base) / %u MHz (max)",
             cpu->base_frequency, cpu->max_frequency);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y,
                               "Frequency:", COLOR_BLACK);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y + 15,
                               freq_str, COLOR_BLUE);
    current_y += 40;

    char cache_str[64];
    snprintf(cache_str, sizeof(cache_str), "L1: %u KB, L2: %u KB, L3: %u KB",
             cpu->cache_l1, cpu->cache_l2, cpu->cache_l3);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y,
                               "Cache:", COLOR_BLACK);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y + 15,
                               cache_str, COLOR_BLUE);
    current_y += 40;

    /* Real-time stats */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y,
                               "Current Usage:", COLOR_BLACK);
    draw_usage_bar(buffer, buffer_width, content_x + 20, current_y + 20, 300, 20,
                   cpu->usage_percent, COLOR_GREEN);

    char usage_str[32];
    snprintf(usage_str, sizeof(usage_str), "%.1f%%", cpu->usage_percent);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 330, current_y + 20,
                               usage_str, COLOR_BLACK);
    current_y += 50;

    char temp_str[32];
    snprintf(temp_str, sizeof(temp_str), "%.1f°C", cpu->temperature);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y,
                               "Temperature:", COLOR_BLACK);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 120, current_y,
                               temp_str, COLOR_RED);
    current_y += line_height;

    /* CPU Features */
    if (app->show_advanced) {
        sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y,
                                   "Features:", COLOR_BLACK);
        sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y + 15,
                                   cpu->features, COLOR_GRAY);
    }
}

/* Draw usage bar */
static void draw_usage_bar(u32* buffer, u32 buffer_width, u32 x, u32 y,
                          u32 width, u32 height, float percentage, u32 color) {
    /* Draw background */
    for (u32 dy = 0; dy < height; dy++) {
        for (u32 dx = 0; dx < width; dx++) {
            if (x + dx < buffer_width && y + dy < 600) {
                buffer[(y + dy) * buffer_width + (x + dx)] = COLOR_LIGHT_GRAY;
            }
        }
    }

    /* Draw filled portion */
    u32 filled_width = (u32)((percentage / 100.0f) * width);
    for (u32 dy = 0; dy < height; dy++) {
        for (u32 dx = 0; dx < filled_width; dx++) {
            if (x + dx < buffer_width && y + dy < 600) {
                buffer[(y + dy) * buffer_width + (x + dx)] = color;
            }
        }
    }

    /* Draw border */
    for (u32 dx = 0; dx < width; dx++) {
        if (x + dx < buffer_width) {
            if (y < 600) buffer[y * buffer_width + (x + dx)] = COLOR_BLACK;
            if (y + height - 1 < 600) buffer[(y + height - 1) * buffer_width + (x + dx)] = COLOR_BLACK;
        }
    }
    for (u32 dy = 0; dy < height; dy++) {
        if (y + dy < 600) {
            if (x < buffer_width) buffer[(y + dy) * buffer_width + x] = COLOR_BLACK;
            if (x + width - 1 < buffer_width) buffer[(y + dy) * buffer_width + (x + width - 1)] = COLOR_BLACK;
        }
    }
}

/* Draw system info content based on category */
static void draw_sysinfo_content(u32* buffer, u32 buffer_width, struct system_info_app* app) {
    switch (app->selected_category) {
        case SYSINFO_CAT_OVERVIEW:
            draw_overview_content(buffer, buffer_width, app);
            break;
        case SYSINFO_CAT_CPU:
            draw_cpu_content(buffer, buffer_width, app);
            break;
        case SYSINFO_CAT_MEMORY:
            draw_memory_content(buffer, buffer_width, app);
            break;
        case SYSINFO_CAT_STORAGE:
            draw_storage_content(buffer, buffer_width, app);
            break;
        case SYSINFO_CAT_GRAPHICS:
            draw_graphics_content(buffer, buffer_width, app);
            break;
        case SYSINFO_CAT_NETWORK:
            draw_network_content(buffer, buffer_width, app);
            break;
        case SYSINFO_CAT_SYSTEM:
            draw_system_content(buffer, buffer_width, app);
            break;
        case SYSINFO_CAT_PERFORMANCE:
            draw_performance_content(buffer, buffer_width, app);
            break;
    }
}

/* Draw memory content */
static void draw_memory_content(u32* buffer, u32 buffer_width, struct system_info_app* app) {
    u32 content_x = 220;
    struct system_info* info = get_system_info();
    struct memory_info* mem = &info->memory;

    /* Clear content area */
    for (u32 y = 0; y < 600; y++) {
        for (u32 x = content_x; x < buffer_width; x++) {
            buffer[y * buffer_width + x] = COLOR_WHITE;
        }
    }

    /* Draw title */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, 20,
                               "Memory Information", COLOR_BLACK);

    u32 start_y = 60;
    u32 current_y = start_y;

    /* Physical Memory */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y,
                               "Physical Memory:", COLOR_BLACK);
    current_y += 30;

    char total_str[64], used_str[64], avail_str[64];
    format_bytes(mem->total_physical, total_str, sizeof(total_str));
    format_bytes(mem->used_physical, used_str, sizeof(used_str));
    format_bytes(mem->available_physical, avail_str, sizeof(avail_str));

    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 40, current_y,
                               "Total:", COLOR_BLACK);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 120, current_y,
                               total_str, COLOR_BLUE);
    current_y += 20;

    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 40, current_y,
                               "Used:", COLOR_BLACK);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 120, current_y,
                               used_str, COLOR_RED);
    current_y += 20;

    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 40, current_y,
                               "Available:", COLOR_BLACK);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 120, current_y,
                               avail_str, COLOR_GREEN);
    current_y += 40;

    /* Memory usage bar */
    float mem_usage = get_memory_usage();
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y,
                               "Memory Usage:", COLOR_BLACK);
    draw_usage_bar(buffer, buffer_width, content_x + 20, current_y + 20, 300, 20,
                   mem_usage, COLOR_BLUE);

    char usage_str[32];
    snprintf(usage_str, sizeof(usage_str), "%.1f%%", mem_usage);
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 330, current_y + 20,
                               usage_str, COLOR_BLACK);
    current_y += 60;

    /* Memory modules */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y,
                               "Memory Modules:", COLOR_BLACK);
    current_y += 30;

    for (u32 i = 0; i < mem->module_count; i++) {
        char module_str[128];
        char size_str[32];
        format_bytes(mem->modules[i].size, size_str, sizeof(size_str));

        snprintf(module_str, sizeof(module_str), "Module %u: %s %s @ %u MHz (%s)",
                 i + 1, size_str, mem->modules[i].type, mem->modules[i].speed,
                 mem->modules[i].manufacturer);

        sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 40, current_y,
                                   module_str, COLOR_BLUE);
        current_y += 20;
    }
}

/* Draw storage content */
static void draw_storage_content(u32* buffer, u32 buffer_width, struct system_info_app* app) {
    u32 content_x = 220;
    struct system_info* info = get_system_info();
    struct storage_info* storage = &info->storage;

    /* Clear content area */
    for (u32 y = 0; y < 600; y++) {
        for (u32 x = content_x; x < buffer_width; x++) {
            buffer[y * buffer_width + x] = COLOR_WHITE;
        }
    }

    /* Draw title */
    sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, 20,
                               "Storage Information", COLOR_BLACK);

    u32 start_y = 60;
    u32 current_y = start_y;

    for (u32 i = 0; i < storage->device_count; i++) {
        struct storage_info* dev = &storage->devices[i];

        /* Device header */
        char device_header[128];
        snprintf(device_header, sizeof(device_header), "Device %u: %s", i + 1, dev->name);
        sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 20, current_y,
                                   device_header, COLOR_BLACK);
        current_y += 30;

        /* Device details */
        sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 40, current_y,
                                   "Type:", COLOR_BLACK);
        sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 120, current_y,
                                   dev->type, COLOR_BLUE);
        current_y += 20;

        sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 40, current_y,
                                   "Interface:", COLOR_BLACK);
        sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 120, current_y,
                                   dev->interface, COLOR_BLUE);
        current_y += 20;

        char total_str[32], free_str[32];
        format_bytes(dev->total_size, total_str, sizeof(total_str));
        format_bytes(dev->free_size, free_str, sizeof(free_str));

        sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 40, current_y,
                                   "Capacity:", COLOR_BLACK);
        sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 120, current_y,
                                   total_str, COLOR_BLUE);
        current_y += 20;

        sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 40, current_y,
                                   "Free Space:", COLOR_BLACK);
        sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 120, current_y,
                                   free_str, COLOR_GREEN);
        current_y += 20;

        /* Usage bar */
        float usage = get_storage_usage(i);
        draw_usage_bar(buffer, buffer_width, content_x + 40, current_y, 250, 15,
                       usage, COLOR_ORANGE);

        char usage_str[32];
        snprintf(usage_str, sizeof(usage_str), "%.1f%%", usage);
        sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 300, current_y,
                                   usage_str, COLOR_BLACK);
        current_y += 40;

        /* Advanced info */
        if (app->show_advanced) {
            char model_str[128];
            snprintf(model_str, sizeof(model_str), "%s %s", dev->manufacturer, dev->model);
            sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 40, current_y,
                                       "Model:", COLOR_BLACK);
            sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 120, current_y,
                                       model_str, COLOR_GRAY);
            current_y += 20;

            char temp_str[32];
            snprintf(temp_str, sizeof(temp_str), "%.1f°C", dev->temperature);
            sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 40, current_y,
                                       "Temperature:", COLOR_BLACK);
            sysinfo_draw_text_to_buffer(buffer, buffer_width, content_x + 120, current_y,
                                       temp_str, COLOR_RED);
            current_y += 20;
        }

        current_y += 20;  /* Spacing between devices */
    }
}

/* Render system info application */
void sysinfo_render(struct system_info_app* app) {
    if (!app->active) return;

    u32* buffer = wm_get_window_buffer(app->window_id);
    if (!buffer) return;

    u32 win_width = 800;
    u32 win_height = 600;

    /* Clear window buffer */
    for (u32 i = 0; i < win_width * win_height; i++) {
        buffer[i] = COLOR_WHITE;
    }

    /* Auto-refresh check */
    if (app->auto_refresh) {
        u64 current_time = get_system_time();
        if ((current_time - app->last_refresh) >= (app->refresh_interval * 1000000)) {
            system_info_update();
            app->last_refresh = current_time;
        }
    }

    /* Draw components */
    draw_sysinfo_sidebar(buffer, win_width, app);
    draw_sysinfo_content(buffer, win_width, app);
}

/* Handle system info click */
void sysinfo_handle_click(struct system_info_app* app, u32 x, u32 y) {
    /* Check sidebar clicks */
    if (x < 200) {
        u32 category_index = (y - 20) / 50;
        if (category_index < 8) {
            app->selected_category = category_index;
        }

        /* Check refresh toggle */
        if (y >= 450 && y < 490 && x >= 15 && x < 165) {
            app->auto_refresh = !app->auto_refresh;
        }

        /* Check export button */
        if (y >= 500 && y < 525 && x >= 15 && x < 165) {
            export_system_info(app->export_buffer, sizeof(app->export_buffer));
            /* In a real implementation, this would save to file or copy to clipboard */
        }
        return;
    }
}

/* Draw button */
static void sysinfo_draw_button(u32* buffer, u32 buffer_width, u32 x, u32 y,
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
    sysinfo_draw_text_to_buffer(buffer, buffer_width, text_x, text_y, text, COLOR_WHITE);
}

/* Draw text to buffer */
void sysinfo_draw_text_to_buffer(u32* buffer, u32 buffer_width, u32 x, u32 y,
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

/* Find system info app by window ID */
struct system_info_app* sysinfo_find_by_window(u32 window_id) {
    for (u32 i = 0; i < 2; i++) {
        if (sysinfo_apps[i].active && sysinfo_apps[i].window_id == window_id) {
            return &sysinfo_apps[i];
        }
    }
    return NULL;
}