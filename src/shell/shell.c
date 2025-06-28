#include "kronos.h"

/* Kronos Shell (ksh) */

#define MAX_COMMAND_LENGTH 256
#define MAX_ARGS 16

static char command_buffer[MAX_COMMAND_LENGTH];
static size_t command_pos = 0;

/* Shell commands */
static void cmd_help(void);
static void cmd_clear(void);
static void cmd_reboot(void);
static void cmd_halt(void);
static void cmd_meminfo(void);
static void cmd_uptime(void);
static void cmd_echo(char* args);

/* Command structure */
struct command {
    const char* name;
    const char* description;
    void (*handler)(char* args);
};

/* Available commands */
static struct command commands[] = {
    {"help", "Show this help message", (void(*)(char*))cmd_help},
    {"clear", "Clear the screen", (void(*)(char*))cmd_clear},
    {"reboot", "Reboot the system", (void(*)(char*))cmd_reboot},
    {"halt", "Halt the system", (void(*)(char*))cmd_halt},
    {"meminfo", "Show memory information", (void(*)(char*))cmd_meminfo},
    {"uptime", "Show system uptime", (void(*)(char*))cmd_uptime},
    {"echo", "Echo arguments", cmd_echo},
    {"gui", "Start graphical user interface", (void(*)(char*))cmd_gui},
    {"desktop", "Launch desktop environment", (void(*)(char*))cmd_desktop},
    {"demo", "Show GUI demo", (void(*)(char*))cmd_gui_demo},
    {NULL, NULL, NULL}
};

/* Initialize shell */
void shell_init(void) {
    command_pos = 0;
    memset(command_buffer, 0, MAX_COMMAND_LENGTH);
}

/* Print shell prompt */
static void print_prompt(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("kronos");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("$ ");
}

/* Execute command */
static void execute_command(const char* cmd_line) {
    if (strlen(cmd_line) == 0) {
        return;
    }
    
    /* Make a copy of the command line for tokenization */
    char cmd_copy[MAX_COMMAND_LENGTH];
    strcpy(cmd_copy, cmd_line);
    
    /* Parse command and arguments */
    char* cmd = strtok(cmd_copy, " \t");
    char* args = strtok(NULL, "");
    
    if (!cmd) {
        return;
    }
    
    /* Find and execute command */
    for (int i = 0; commands[i].name; i++) {
        if (strcmp(cmd, commands[i].name) == 0) {
            commands[i].handler(args);
            return;
        }
    }
    
    /* Command not found */
    vga_printf("ksh: command not found: %s\n", cmd);
    vga_puts("Type 'help' for available commands.\n");
}

/* Main shell loop */
void shell_run(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Welcome to Kronos Shell (ksh)\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    print_prompt();
    
    while (1) {
        if (keyboard_has_input()) {
            char c = keyboard_getchar();
            
            if (c == '\n') {
                vga_putchar('\n');
                command_buffer[command_pos] = '\0';
                execute_command(command_buffer);
                command_pos = 0;
                memset(command_buffer, 0, MAX_COMMAND_LENGTH);
                print_prompt();
            } else if (c == '\b') {
                if (command_pos > 0) {
                    command_pos--;
                    command_buffer[command_pos] = '\0';
                    vga_putchar('\b');
                }
            } else if (c >= 32 && c <= 126) { /* Printable characters */
                if (command_pos < MAX_COMMAND_LENGTH - 1) {
                    command_buffer[command_pos++] = c;
                    vga_putchar(c);
                }
            }
        }
        
        /* Halt CPU until next interrupt */
        __asm__ volatile ("hlt");
    }
}

/* Command implementations */

static void cmd_help(void) {
    vga_puts("Kronos OS v1.0 - Available Commands:\n");
    vga_puts("====================================\n");
    
    for (int i = 0; commands[i].name; i++) {
        vga_printf("  %-10s - %s\n", commands[i].name, commands[i].description);
    }
    vga_putchar('\n');
}

static void cmd_clear(void) {
    vga_clear();
}

static void cmd_reboot(void) {
    vga_puts("Rebooting system...\n");
    system_reboot();
}

static void cmd_halt(void) {
    vga_puts("Halting system...\n");
    system_halt();
}

static void cmd_meminfo(void) {
    size_t total, used, free;
    get_memory_stats(&total, &used, &free);
    
    vga_puts("Memory Information:\n");
    vga_puts("==================\n");
    vga_printf("Total:  %d KB\n", total / 1024);
    vga_printf("Used:   %d KB\n", used / 1024);
    vga_printf("Free:   %d KB\n", free / 1024);
    vga_printf("Usage:  %d%%\n", total > 0 ? (used * 100) / total : 0);
    vga_putchar('\n');
}

static void cmd_uptime(void) {
    u64 uptime = get_uptime();
    vga_printf("System uptime: %d seconds\n", uptime);
}

static void cmd_echo(char* args) {
    if (args) {
        vga_puts(args);
    }
    vga_putchar('\n');
}

static void cmd_gui(void) {
    vga_puts("Starting Kronos OS Graphical User Interface...\n");
    vga_puts("Features:\n");
    vga_puts("- Ubuntu-like desktop environment\n");
    vga_puts("- Window manager with taskbar\n");
    vga_puts("- File manager application\n");
    vga_puts("- Text editor with syntax highlighting\n");
    vga_puts("- Terminal emulator\n");
    vga_puts("- Start menu and desktop icons\n\n");

    gui_init();
}

static void cmd_desktop(void) {
    vga_puts("Launching desktop environment...\n");
    gui_show_demo();
}

static void cmd_gui_demo(void) {
    vga_puts("Starting GUI demonstration...\n");
    vga_puts("This will show:\n");
    vga_puts("- Desktop with wallpaper and icons\n");
    vga_puts("- Taskbar with start menu\n");
    vga_puts("- Multiple application windows\n");
    vga_puts("- Window management features\n");
    vga_puts("- File manager with directory browsing\n");
    vga_puts("- Text editor with file editing\n");
    vga_puts("- Terminal with command execution\n\n");

    gui_show_demo();
}
