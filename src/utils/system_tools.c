#include "kronos.h"

/* System Utilities for Kronos OS */

/* Process status command (ps) */
void cmd_ps(char* args) {
    vga_puts("  PID  PPID STAT  TIME COMMAND\n");
    vga_puts("===== ===== ===== ===== =======\n");
    
    for (u32 i = 0; i < MAX_PROCESSES; i++) {
        struct process* proc = &processes[i];
        if (proc->in_use) {
            char state_char;
            switch (proc->state) {
                case PROCESS_RUNNING: state_char = 'R'; break;
                case PROCESS_READY: state_char = 'S'; break;
                case PROCESS_BLOCKED: state_char = 'D'; break;
                case PROCESS_ZOMBIE: state_char = 'Z'; break;
                default: state_char = '?'; break;
            }
            
            u64 cpu_time_sec = proc->total_cpu_time / 1000000;
            vga_printf("%5d %5d   %c   %3d %s\n", 
                      proc->pid, proc->ppid, state_char, 
                      cpu_time_sec, proc->name);
        }
    }
}

/* Process tree command (pstree) */
void cmd_pstree(char* args) {
    vga_puts("Process Tree:\n");
    print_process_tree(0, 0);  /* Start with init process */
}

static void print_process_tree(u32 pid, u32 depth) {
    struct process* proc = get_process_by_pid(pid);
    if (!proc) return;
    
    /* Print indentation */
    for (u32 i = 0; i < depth; i++) {
        vga_puts("  ");
    }
    
    vga_printf("├─ %s (%d)\n", proc->name, proc->pid);
    
    /* Print children */
    for (u32 i = 0; i < proc->child_count; i++) {
        if (proc->children[i]) {
            print_process_tree(proc->children[i]->pid, depth + 1);
        }
    }
}

/* Top command - real-time process monitor */
void cmd_top(char* args) {
    vga_clear();
    
    while (1) {
        vga_set_cursor(0, 0);
        
        /* Header */
        vga_puts("Kronos OS - System Monitor\n");
        vga_puts("==========================\n\n");
        
        /* System stats */
        struct process_stats stats;
        get_process_stats(&stats);
        
        struct memory_stats mem_stats;
        get_memory_stats(&mem_stats.total, &mem_stats.used, &mem_stats.free);
        
        vga_printf("Processes: %d total, %d running\n", 
                  stats.total_processes, stats.running_processes);
        vga_printf("Memory: %d KB total, %d KB used, %d KB free\n",
                  mem_stats.total / 1024, mem_stats.used / 1024, mem_stats.free / 1024);
        vga_printf("Uptime: %d seconds\n\n", get_uptime());
        
        /* Process list header */
        vga_puts("  PID USER     %CPU %MEM    VSZ   RSS STAT COMMAND\n");
        vga_puts("===== ======== ==== ==== ====== ===== ==== =======\n");
        
        /* Process list */
        for (u32 i = 0; i < MAX_PROCESSES && i < 15; i++) {
            struct process* proc = &processes[i];
            if (proc->in_use) {
                char state_char;
                switch (proc->state) {
                    case PROCESS_RUNNING: state_char = 'R'; break;
                    case PROCESS_READY: state_char = 'S'; break;
                    case PROCESS_BLOCKED: state_char = 'D'; break;
                    case PROCESS_ZOMBIE: state_char = 'Z'; break;
                    default: state_char = '?'; break;
                }
                
                u32 cpu_percent = (proc->total_cpu_time * 100) / get_uptime();
                if (cpu_percent > 100) cpu_percent = 100;
                
                u32 mem_percent = (proc->virtual_memory_size * 100) / mem_stats.total;
                
                vga_printf("%5d %-8s %3d%% %3d%% %6d %5d  %c   %s\n",
                          proc->pid, "user", cpu_percent, mem_percent,
                          proc->virtual_memory_size / 1024,
                          proc->virtual_memory_size / 1024,
                          state_char, proc->name);
            }
        }
        
        vga_puts("\nPress 'q' to quit, 'r' to refresh\n");
        
        /* Check for input */
        if (keyboard_has_input()) {
            char c = keyboard_getchar();
            if (c == 'q' || c == 'Q') {
                break;
            }
        }
        
        /* Wait before refresh */
        timer_sleep(1000000);  /* 1 second */
    }
    
    vga_clear();
}

/* Kill process command */
void cmd_kill(char* args) {
    if (!args || strlen(args) == 0) {
        vga_puts("Usage: kill <pid> [signal]\n");
        return;
    }
    
    u32 pid = 0;
    i32 signal = SIGTERM;  /* Default signal */
    
    /* Parse arguments */
    char* pid_str = strtok(args, " ");
    char* sig_str = strtok(NULL, " ");
    
    if (pid_str) {
        pid = atoi(pid_str);
    }
    
    if (sig_str) {
        signal = atoi(sig_str);
    }
    
    if (pid == 0) {
        vga_puts("Invalid PID\n");
        return;
    }
    
    if (signal_send(pid, signal) == 0) {
        vga_printf("Signal %d sent to process %d\n", signal, pid);
    } else {
        vga_printf("Failed to send signal to process %d\n", pid);
    }
}

/* List files command (ls) */
void cmd_ls(char* args) {
    char* path = args && strlen(args) > 0 ? args : ".";
    
    struct directory* dir = vfs_opendir(path);
    if (!dir) {
        vga_printf("ls: cannot access '%s': No such file or directory\n", path);
        return;
    }
    
    struct dirent* entry;
    while ((entry = vfs_readdir(dir)) != NULL) {
        char permissions[11];
        format_permissions(entry->mode, permissions);
        
        char size_str[16];
        format_file_size(entry->size, size_str);
        
        char time_str[20];
        format_time(entry->mtime, time_str);
        
        vga_printf("%s %8s %s %s\n", 
                  permissions, size_str, time_str, entry->name);
    }
    
    vfs_closedir(dir);
}

/* Copy files command (cp) */
void cmd_cp(char* args) {
    if (!args) {
        vga_puts("Usage: cp <source> <destination>\n");
        return;
    }
    
    char* src = strtok(args, " ");
    char* dst = strtok(NULL, " ");
    
    if (!src || !dst) {
        vga_puts("Usage: cp <source> <destination>\n");
        return;
    }
    
    if (file_copy(src, dst) == 0) {
        vga_printf("'%s' copied to '%s'\n", src, dst);
    } else {
        vga_printf("cp: failed to copy '%s' to '%s'\n", src, dst);
    }
}

/* Move/rename files command (mv) */
void cmd_mv(char* args) {
    if (!args) {
        vga_puts("Usage: mv <source> <destination>\n");
        return;
    }
    
    char* src = strtok(args, " ");
    char* dst = strtok(NULL, " ");
    
    if (!src || !dst) {
        vga_puts("Usage: mv <source> <destination>\n");
        return;
    }
    
    if (vfs_rename(src, dst) == 0) {
        vga_printf("'%s' moved to '%s'\n", src, dst);
    } else {
        vga_printf("mv: failed to move '%s' to '%s'\n", src, dst);
    }
}

/* Remove files command (rm) */
void cmd_rm(char* args) {
    if (!args) {
        vga_puts("Usage: rm <file>\n");
        return;
    }
    
    char* filename = strtok(args, " ");
    while (filename) {
        if (vfs_unlink(filename) == 0) {
            vga_printf("'%s' removed\n", filename);
        } else {
            vga_printf("rm: cannot remove '%s'\n", filename);
        }
        filename = strtok(NULL, " ");
    }
}

/* Make directory command (mkdir) */
void cmd_mkdir(char* args) {
    if (!args) {
        vga_puts("Usage: mkdir <directory>\n");
        return;
    }
    
    if (vfs_mkdir(args, 0755) == 0) {
        vga_printf("Directory '%s' created\n", args);
    } else {
        vga_printf("mkdir: cannot create directory '%s'\n", args);
    }
}

/* Remove directory command (rmdir) */
void cmd_rmdir(char* args) {
    if (!args) {
        vga_puts("Usage: rmdir <directory>\n");
        return;
    }
    
    if (vfs_rmdir(args) == 0) {
        vga_printf("Directory '%s' removed\n", args);
    } else {
        vga_printf("rmdir: cannot remove directory '%s'\n", args);
    }
}

/* Change directory command (cd) */
void cmd_cd(char* args) {
    char* path = args && strlen(args) > 0 ? args : "/home/user";
    
    if (vfs_chdir(path) == 0) {
        vga_printf("Changed to directory '%s'\n", path);
    } else {
        vga_printf("cd: cannot change to directory '%s'\n", path);
    }
}

/* Print working directory command (pwd) */
void cmd_pwd(char* args) {
    char cwd[256];
    if (vfs_getcwd(cwd, sizeof(cwd)) != NULL) {
        vga_puts(cwd);
        vga_putchar('\n');
    } else {
        vga_puts("pwd: error getting current directory\n");
    }
}

/* Find files command (find) */
void cmd_find(char* args) {
    char* path = ".";
    char* pattern = "*";
    
    if (args) {
        char* arg1 = strtok(args, " ");
        char* arg2 = strtok(NULL, " ");
        
        if (arg1) path = arg1;
        if (arg2) pattern = arg2;
    }
    
    find_files_recursive(path, pattern);
}

static void find_files_recursive(const char* path, const char* pattern) {
    struct directory* dir = vfs_opendir(path);
    if (!dir) return;
    
    struct dirent* entry;
    while ((entry = vfs_readdir(dir)) != NULL) {
        if (strcmp(entry->name, ".") == 0 || strcmp(entry->name, "..") == 0) {
            continue;
        }
        
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->name);
        
        if (match_pattern(entry->name, pattern)) {
            vga_puts(full_path);
            vga_putchar('\n');
        }
        
        if (entry->type == DT_DIR) {
            find_files_recursive(full_path, pattern);
        }
    }
    
    vfs_closedir(dir);
}

/* Disk usage command (du) */
void cmd_du(char* args) {
    char* path = args && strlen(args) > 0 ? args : ".";
    
    u64 total_size = calculate_directory_size(path);
    
    vga_printf("%d KB\t%s\n", total_size / 1024, path);
}

static u64 calculate_directory_size(const char* path) {
    struct directory* dir = vfs_opendir(path);
    if (!dir) return 0;
    
    u64 total_size = 0;
    struct dirent* entry;
    
    while ((entry = vfs_readdir(dir)) != NULL) {
        if (strcmp(entry->name, ".") == 0 || strcmp(entry->name, "..") == 0) {
            continue;
        }
        
        if (entry->type == DT_DIR) {
            char subdir_path[512];
            snprintf(subdir_path, sizeof(subdir_path), "%s/%s", path, entry->name);
            total_size += calculate_directory_size(subdir_path);
        } else {
            total_size += entry->size;
        }
    }
    
    vfs_closedir(dir);
    return total_size;
}

/* Disk free command (df) */
void cmd_df(char* args) {
    vga_puts("Filesystem     1K-blocks    Used Available Use% Mounted on\n");
    vga_puts("============== ========= ======= ========= ==== ==========\n");
    
    struct filesystem_info fs_info;
    get_filesystem_info("/", &fs_info);
    
    u32 used_percent = (fs_info.used_blocks * 100) / fs_info.total_blocks;
    
    vga_printf("%-14s %9d %7d %9d %3d%% %s\n",
              fs_info.device_name,
              fs_info.total_blocks,
              fs_info.used_blocks,
              fs_info.free_blocks,
              used_percent,
              fs_info.mount_point);
}

/* Mount filesystem command */
void cmd_mount(char* args) {
    if (!args) {
        /* Show mounted filesystems */
        show_mounted_filesystems();
        return;
    }
    
    char* device = strtok(args, " ");
    char* mount_point = strtok(NULL, " ");
    char* fs_type = strtok(NULL, " ");
    
    if (!device || !mount_point) {
        vga_puts("Usage: mount <device> <mount_point> [filesystem_type]\n");
        return;
    }
    
    if (!fs_type) {
        fs_type = "ext4";  /* Default filesystem */
    }
    
    if (vfs_mount(device, mount_point, fs_type, 0, NULL) == 0) {
        vga_printf("'%s' mounted on '%s'\n", device, mount_point);
    } else {
        vga_printf("mount: failed to mount '%s' on '%s'\n", device, mount_point);
    }
}

/* Unmount filesystem command */
void cmd_umount(char* args) {
    if (!args) {
        vga_puts("Usage: umount <mount_point>\n");
        return;
    }
    
    if (vfs_umount(args) == 0) {
        vga_printf("'%s' unmounted\n", args);
    } else {
        vga_printf("umount: failed to unmount '%s'\n", args);
    }
}

/* System information command */
void cmd_sysinfo(char* args) {
    struct sysinfo info;
    sys_sysinfo(&info);
    
    vga_puts("System Information:\n");
    vga_puts("==================\n");
    vga_printf("Uptime: %d seconds\n", info.uptime);
    vga_printf("Load averages: %.2f %.2f %.2f\n", 
              info.loads[0] / 65536.0, 
              info.loads[1] / 65536.0, 
              info.loads[2] / 65536.0);
    vga_printf("Total RAM: %d KB\n", info.totalram / 1024);
    vga_printf("Free RAM: %d KB\n", info.freeram / 1024);
    vga_printf("Shared RAM: %d KB\n", info.sharedram / 1024);
    vga_printf("Buffer RAM: %d KB\n", info.bufferram / 1024);
    vga_printf("Total swap: %d KB\n", info.totalswap / 1024);
    vga_printf("Free swap: %d KB\n", info.freeswap / 1024);
    vga_printf("Number of processes: %d\n", info.procs);
}

/* Format file permissions */
static void format_permissions(mode_t mode, char* buffer) {
    buffer[0] = (mode & S_IFDIR) ? 'd' : '-';
    buffer[1] = (mode & S_IRUSR) ? 'r' : '-';
    buffer[2] = (mode & S_IWUSR) ? 'w' : '-';
    buffer[3] = (mode & S_IXUSR) ? 'x' : '-';
    buffer[4] = (mode & S_IRGRP) ? 'r' : '-';
    buffer[5] = (mode & S_IWGRP) ? 'w' : '-';
    buffer[6] = (mode & S_IXGRP) ? 'x' : '-';
    buffer[7] = (mode & S_IROTH) ? 'r' : '-';
    buffer[8] = (mode & S_IWOTH) ? 'w' : '-';
    buffer[9] = (mode & S_IXOTH) ? 'x' : '-';
    buffer[10] = '\0';
}

/* Format file size */
static void format_file_size(u64 size, char* buffer) {
    if (size < 1024) {
        sprintf(buffer, "%dB", size);
    } else if (size < 1024 * 1024) {
        sprintf(buffer, "%dK", size / 1024);
    } else if (size < 1024 * 1024 * 1024) {
        sprintf(buffer, "%dM", size / (1024 * 1024));
    } else {
        sprintf(buffer, "%dG", size / (1024 * 1024 * 1024));
    }
}

/* Format time */
static void format_time(time_t time, char* buffer) {
    /* Simple time formatting - in real implementation would use proper time functions */
    sprintf(buffer, "Jan 01 12:00");
}
