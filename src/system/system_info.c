#include "kronos.h"

/* Comprehensive System Information for Kronos OS */

#define MAX_CPU_CORES 32
#define MAX_MEMORY_MODULES 8
#define MAX_STORAGE_DEVICES 16
#define MAX_NETWORK_INTERFACES 8
#define MAX_USB_DEVICES 32

/* CPU Information */
struct cpu_info {
    char vendor[32];
    char model[128];
    char architecture[16];
    u32 cores;
    u32 threads;
    u32 base_frequency;     /* MHz */
    u32 max_frequency;      /* MHz */
    u32 cache_l1;          /* KB */
    u32 cache_l2;          /* KB */
    u32 cache_l3;          /* KB */
    char features[256];     /* CPU features */
    float temperature;      /* Celsius */
    float usage_percent;    /* Current usage */
};

/* Memory Information */
struct memory_info {
    u64 total_physical;     /* Bytes */
    u64 available_physical; /* Bytes */
    u64 used_physical;      /* Bytes */
    u64 total_virtual;      /* Bytes */
    u64 available_virtual;  /* Bytes */
    u64 used_virtual;       /* Bytes */
    u32 module_count;
    struct {
        u64 size;           /* Bytes */
        u32 speed;          /* MHz */
        char type[16];      /* DDR4, DDR5, etc */
        char manufacturer[32];
    } modules[MAX_MEMORY_MODULES];
};

/* Storage Information */
struct storage_info {
    u32 device_count;
    struct {
        char name[64];
        char type[16];      /* SSD, HDD, NVMe */
        char interface[16]; /* SATA, PCIe, USB */
        u64 total_size;     /* Bytes */
        u64 free_size;      /* Bytes */
        u32 rpm;           /* For HDDs */
        char manufacturer[32];
        char model[64];
        char serial[32];
        float temperature;  /* Celsius */
        u64 read_speed;    /* Bytes/sec */
        u64 write_speed;   /* Bytes/sec */
    } devices[MAX_STORAGE_DEVICES];
};

/* Graphics Information */
struct graphics_info {
    char vendor[32];
    char model[128];
    char driver_version[32];
    u64 memory_total;      /* Bytes */
    u64 memory_used;       /* Bytes */
    u32 core_clock;        /* MHz */
    u32 memory_clock;      /* MHz */
    float temperature;     /* Celsius */
    float usage_percent;   /* Current usage */
    bool hardware_acceleration;
};

/* Network Information */
struct network_info {
    u32 interface_count;
    struct {
        char name[32];
        char type[16];      /* Ethernet, WiFi, Bluetooth */
        char mac_address[18];
        char ip_address[16];
        char subnet_mask[16];
        char gateway[16];
        u64 bytes_sent;
        u64 bytes_received;
        u32 link_speed;     /* Mbps */
        bool connected;
    } interfaces[MAX_NETWORK_INTERFACES];
};

/* System Information */
struct system_info {
    char os_name[64];
    char os_version[32];
    char kernel_version[32];
    char hostname[64];
    char username[32];
    u64 boot_time;         /* Unix timestamp */
    u64 uptime;           /* Seconds */
    u32 process_count;
    u32 thread_count;
    float cpu_load_1min;
    float cpu_load_5min;
    float cpu_load_15min;
    
    struct cpu_info cpu;
    struct memory_info memory;
    struct storage_info storage;
    struct graphics_info graphics;
    struct network_info network;
};

/* Global system information */
static struct system_info sys_info;
static bool info_initialized = false;

/* Initialize system information */
void system_info_init(void) {
    memset(&sys_info, 0, sizeof(sys_info));
    
    /* Initialize basic system info */
    strcpy(sys_info.os_name, "Kronos OS");
    strcpy(sys_info.os_version, "2.0 Advanced");
    strcpy(sys_info.kernel_version, "5.15.0-kronos");
    strcpy(sys_info.hostname, "kronos-desktop");
    strcpy(sys_info.username, "user");
    
    sys_info.boot_time = get_system_time() - 172800000000ULL; /* 2 days ago */
    
    /* Initialize CPU information */
    init_cpu_info();
    
    /* Initialize memory information */
    init_memory_info();
    
    /* Initialize storage information */
    init_storage_info();
    
    /* Initialize graphics information */
    init_graphics_info();
    
    /* Initialize network information */
    init_network_info();
    
    info_initialized = true;
    vga_puts("System information initialized\n");
}

/* Initialize CPU information */
static void init_cpu_info(void) {
    struct cpu_info* cpu = &sys_info.cpu;
    
    strcpy(cpu->vendor, "Intel Corporation");
    strcpy(cpu->model, "Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz");
    strcpy(cpu->architecture, "x86_64");
    cpu->cores = 4;
    cpu->threads = 8;
    cpu->base_frequency = 1800;
    cpu->max_frequency = 4000;
    cpu->cache_l1 = 256;    /* 64KB per core * 4 cores */
    cpu->cache_l2 = 1024;   /* 256KB per core * 4 cores */
    cpu->cache_l3 = 8192;   /* 8MB shared */
    strcpy(cpu->features, "SSE SSE2 SSE3 SSSE3 SSE4.1 SSE4.2 AVX AVX2 AES-NI VT-x");
    cpu->temperature = 45.0f;
    cpu->usage_percent = 15.0f;
}

/* Initialize memory information */
static void init_memory_info(void) {
    struct memory_info* mem = &sys_info.memory;
    
    mem->total_physical = 8ULL * 1024 * 1024 * 1024;      /* 8GB */
    mem->used_physical = 2ULL * 1024 * 1024 * 1024;       /* 2GB used */
    mem->available_physical = mem->total_physical - mem->used_physical;
    
    mem->total_virtual = 16ULL * 1024 * 1024 * 1024;      /* 16GB virtual */
    mem->used_virtual = 3ULL * 1024 * 1024 * 1024;        /* 3GB used */
    mem->available_virtual = mem->total_virtual - mem->used_virtual;
    
    mem->module_count = 2;
    
    /* Module 1 */
    mem->modules[0].size = 4ULL * 1024 * 1024 * 1024;     /* 4GB */
    mem->modules[0].speed = 2400;
    strcpy(mem->modules[0].type, "DDR4");
    strcpy(mem->modules[0].manufacturer, "Samsung");
    
    /* Module 2 */
    mem->modules[1].size = 4ULL * 1024 * 1024 * 1024;     /* 4GB */
    mem->modules[1].speed = 2400;
    strcpy(mem->modules[1].type, "DDR4");
    strcpy(mem->modules[1].manufacturer, "Samsung");
}

/* Initialize storage information */
static void init_storage_info(void) {
    struct storage_info* storage = &sys_info.storage;
    
    storage->device_count = 2;
    
    /* Primary SSD */
    strcpy(storage->devices[0].name, "/dev/nvme0n1");
    strcpy(storage->devices[0].type, "NVMe SSD");
    strcpy(storage->devices[0].interface, "PCIe 3.0");
    storage->devices[0].total_size = 256ULL * 1024 * 1024 * 1024;  /* 256GB */
    storage->devices[0].free_size = 128ULL * 1024 * 1024 * 1024;   /* 128GB free */
    storage->devices[0].rpm = 0;  /* SSDs don't have RPM */
    strcpy(storage->devices[0].manufacturer, "Samsung");
    strcpy(storage->devices[0].model, "980 PRO");
    strcpy(storage->devices[0].serial, "S5P2NG0N123456");
    storage->devices[0].temperature = 42.0f;
    storage->devices[0].read_speed = 3500ULL * 1024 * 1024;   /* 3.5 GB/s */
    storage->devices[0].write_speed = 3000ULL * 1024 * 1024;  /* 3.0 GB/s */
    
    /* Secondary HDD */
    strcpy(storage->devices[1].name, "/dev/sda1");
    strcpy(storage->devices[1].type, "HDD");
    strcpy(storage->devices[1].interface, "SATA 3.0");
    storage->devices[1].total_size = 1000ULL * 1024 * 1024 * 1024; /* 1TB */
    storage->devices[1].free_size = 750ULL * 1024 * 1024 * 1024;   /* 750GB free */
    storage->devices[1].rpm = 7200;
    strcpy(storage->devices[1].manufacturer, "Western Digital");
    strcpy(storage->devices[1].model, "WD Blue");
    strcpy(storage->devices[1].serial, "WD-WCC6Y7123456");
    storage->devices[1].temperature = 35.0f;
    storage->devices[1].read_speed = 150ULL * 1024 * 1024;    /* 150 MB/s */
    storage->devices[1].write_speed = 140ULL * 1024 * 1024;   /* 140 MB/s */
}

/* Initialize graphics information */
static void init_graphics_info(void) {
    struct graphics_info* gpu = &sys_info.graphics;
    
    strcpy(gpu->vendor, "Intel Corporation");
    strcpy(gpu->model, "Intel UHD Graphics 620");
    strcpy(gpu->driver_version, "27.20.100.8681");
    gpu->memory_total = 2ULL * 1024 * 1024 * 1024;    /* 2GB shared */
    gpu->memory_used = 512ULL * 1024 * 1024;          /* 512MB used */
    gpu->core_clock = 300;      /* Base: 300MHz */
    gpu->memory_clock = 1067;   /* 1067MHz */
    gpu->temperature = 55.0f;
    gpu->usage_percent = 25.0f;
    gpu->hardware_acceleration = true;
}

/* Initialize network information */
static void init_network_info(void) {
    struct network_info* net = &sys_info.network;
    
    net->interface_count = 3;
    
    /* Ethernet interface */
    strcpy(net->interfaces[0].name, "eth0");
    strcpy(net->interfaces[0].type, "Ethernet");
    strcpy(net->interfaces[0].mac_address, "00:1B:44:11:3A:B7");
    strcpy(net->interfaces[0].ip_address, "192.168.1.100");
    strcpy(net->interfaces[0].subnet_mask, "255.255.255.0");
    strcpy(net->interfaces[0].gateway, "192.168.1.1");
    net->interfaces[0].bytes_sent = 1024ULL * 1024 * 1024;     /* 1GB */
    net->interfaces[0].bytes_received = 5ULL * 1024 * 1024 * 1024; /* 5GB */
    net->interfaces[0].link_speed = 1000;  /* 1 Gbps */
    net->interfaces[0].connected = true;
    
    /* WiFi interface */
    strcpy(net->interfaces[1].name, "wlan0");
    strcpy(net->interfaces[1].type, "WiFi");
    strcpy(net->interfaces[1].mac_address, "A4:34:D9:12:34:56");
    strcpy(net->interfaces[1].ip_address, "192.168.1.101");
    strcpy(net->interfaces[1].subnet_mask, "255.255.255.0");
    strcpy(net->interfaces[1].gateway, "192.168.1.1");
    net->interfaces[1].bytes_sent = 512ULL * 1024 * 1024;      /* 512MB */
    net->interfaces[1].bytes_received = 2ULL * 1024 * 1024 * 1024; /* 2GB */
    net->interfaces[1].link_speed = 300;   /* 300 Mbps */
    net->interfaces[1].connected = false;
    
    /* Loopback interface */
    strcpy(net->interfaces[2].name, "lo");
    strcpy(net->interfaces[2].type, "Loopback");
    strcpy(net->interfaces[2].mac_address, "00:00:00:00:00:00");
    strcpy(net->interfaces[2].ip_address, "127.0.0.1");
    strcpy(net->interfaces[2].subnet_mask, "255.0.0.0");
    strcpy(net->interfaces[2].gateway, "0.0.0.0");
    net->interfaces[2].bytes_sent = 1024 * 1024;       /* 1MB */
    net->interfaces[2].bytes_received = 1024 * 1024;   /* 1MB */
    net->interfaces[2].link_speed = 0;
    net->interfaces[2].connected = true;
}

/* Update real-time system information */
void system_info_update(void) {
    if (!info_initialized) return;
    
    /* Update uptime */
    sys_info.uptime = (get_system_time() - sys_info.boot_time) / 1000000; /* Convert to seconds */
    
    /* Update CPU usage (simulated) */
    sys_info.cpu.usage_percent = 10.0f + (rand() % 30);
    sys_info.cpu.temperature = 40.0f + (rand() % 20);
    
    /* Update memory usage */
    u64 base_used = 2ULL * 1024 * 1024 * 1024;  /* 2GB base */
    u64 variable_used = (rand() % 1024) * 1024 * 1024; /* 0-1GB variable */
    sys_info.memory.used_physical = base_used + variable_used;
    sys_info.memory.available_physical = sys_info.memory.total_physical - sys_info.memory.used_physical;
    
    /* Update GPU usage */
    sys_info.graphics.usage_percent = 5.0f + (rand() % 40);
    sys_info.graphics.temperature = 50.0f + (rand() % 15);
    
    /* Update load averages (simulated) */
    sys_info.cpu_load_1min = 0.5f + ((float)(rand() % 100) / 100.0f);
    sys_info.cpu_load_5min = 0.4f + ((float)(rand() % 80) / 100.0f);
    sys_info.cpu_load_15min = 0.3f + ((float)(rand() % 60) / 100.0f);
    
    /* Update process counts */
    sys_info.process_count = 150 + (rand() % 50);
    sys_info.thread_count = sys_info.process_count * 3;
}

/* Get system information */
struct system_info* get_system_info(void) {
    if (!info_initialized) {
        system_info_init();
    }
    system_info_update();
    return &sys_info;
}

/* Format bytes to human readable */
void format_bytes(u64 bytes, char* buffer, size_t buffer_size) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size = (double)bytes;
    
    while (size >= 1024.0 && unit_index < 4) {
        size /= 1024.0;
        unit_index++;
    }
    
    if (unit_index == 0) {
        snprintf(buffer, buffer_size, "%llu %s", bytes, units[unit_index]);
    } else {
        snprintf(buffer, buffer_size, "%.2f %s", size, units[unit_index]);
    }
}

/* Format uptime to human readable */
void format_uptime(u64 seconds, char* buffer, size_t buffer_size) {
    u64 days = seconds / 86400;
    u64 hours = (seconds % 86400) / 3600;
    u64 minutes = (seconds % 3600) / 60;
    u64 secs = seconds % 60;
    
    if (days > 0) {
        snprintf(buffer, buffer_size, "%llu days, %02llu:%02llu:%02llu", days, hours, minutes, secs);
    } else {
        snprintf(buffer, buffer_size, "%02llu:%02llu:%02llu", hours, minutes, secs);
    }
}

/* Get CPU usage percentage */
float get_cpu_usage(void) {
    return sys_info.cpu.usage_percent;
}

/* Get memory usage percentage */
float get_memory_usage(void) {
    if (sys_info.memory.total_physical == 0) return 0.0f;
    return ((float)sys_info.memory.used_physical / (float)sys_info.memory.total_physical) * 100.0f;
}

/* Get storage usage percentage */
float get_storage_usage(u32 device_index) {
    if (device_index >= sys_info.storage.device_count) return 0.0f;
    
    struct storage_info* storage = &sys_info.storage;
    u64 used = storage->devices[device_index].total_size - storage->devices[device_index].free_size;
    
    if (storage->devices[device_index].total_size == 0) return 0.0f;
    return ((float)used / (float)storage->devices[device_index].total_size) * 100.0f;
}

/* Export system information to string */
void export_system_info(char* buffer, size_t buffer_size) {
    struct system_info* info = get_system_info();
    char temp_buffer[256];
    
    snprintf(buffer, buffer_size,
        "=== KRONOS OS SYSTEM INFORMATION ===\n\n"
        "Operating System: %s %s\n"
        "Kernel Version: %s\n"
        "Hostname: %s\n"
        "Username: %s\n",
        info->os_name, info->os_version, info->kernel_version,
        info->hostname, info->username);
    
    format_uptime(info->uptime, temp_buffer, sizeof(temp_buffer));
    strncat(buffer, "Uptime: ", buffer_size - strlen(buffer) - 1);
    strncat(buffer, temp_buffer, buffer_size - strlen(buffer) - 1);
    strncat(buffer, "\n\n", buffer_size - strlen(buffer) - 1);
    
    /* CPU Information */
    strncat(buffer, "=== CPU INFORMATION ===\n", buffer_size - strlen(buffer) - 1);
    snprintf(temp_buffer, sizeof(temp_buffer),
        "Model: %s\n"
        "Architecture: %s\n"
        "Cores: %u\n"
        "Threads: %u\n"
        "Base Frequency: %u MHz\n"
        "Max Frequency: %u MHz\n"
        "Usage: %.1f%%\n"
        "Temperature: %.1f°C\n\n",
        info->cpu.model, info->cpu.architecture,
        info->cpu.cores, info->cpu.threads,
        info->cpu.base_frequency, info->cpu.max_frequency,
        info->cpu.usage_percent, info->cpu.temperature);
    strncat(buffer, temp_buffer, buffer_size - strlen(buffer) - 1);
    
    /* Memory Information */
    strncat(buffer, "=== MEMORY INFORMATION ===\n", buffer_size - strlen(buffer) - 1);
    char total_mem[64], used_mem[64], avail_mem[64];
    format_bytes(info->memory.total_physical, total_mem, sizeof(total_mem));
    format_bytes(info->memory.used_physical, used_mem, sizeof(used_mem));
    format_bytes(info->memory.available_physical, avail_mem, sizeof(avail_mem));
    
    snprintf(temp_buffer, sizeof(temp_buffer),
        "Total Physical: %s\n"
        "Used Physical: %s\n"
        "Available Physical: %s\n"
        "Usage: %.1f%%\n\n",
        total_mem, used_mem, avail_mem, get_memory_usage());
    strncat(buffer, temp_buffer, buffer_size - strlen(buffer) - 1);
}

/* System information timer update */
void system_info_timer_update(void) {
    static u64 last_update = 0;
    u64 current_time = get_system_time();
    
    /* Update every second */
    if (current_time - last_update >= 1000000) {
        system_info_update();
        last_update = current_time;
    }
}
