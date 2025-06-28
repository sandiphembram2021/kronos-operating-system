#include "kronos.h"

/* System Call Interface for Kronos OS */

/* System call numbers */
#define SYS_READ        0
#define SYS_WRITE       1
#define SYS_OPEN        2
#define SYS_CLOSE       3
#define SYS_STAT        4
#define SYS_FSTAT       5
#define SYS_LSTAT       6
#define SYS_POLL        7
#define SYS_LSEEK       8
#define SYS_MMAP        9
#define SYS_MPROTECT    10
#define SYS_MUNMAP      11
#define SYS_BRK         12
#define SYS_RT_SIGACTION 13
#define SYS_RT_SIGPROCMASK 14
#define SYS_RT_SIGRETURN 15
#define SYS_IOCTL       16
#define SYS_PREAD64     17
#define SYS_PWRITE64    18
#define SYS_READV       19
#define SYS_WRITEV      20
#define SYS_ACCESS      21
#define SYS_PIPE        22
#define SYS_SELECT      23
#define SYS_SCHED_YIELD 24
#define SYS_MREMAP      25
#define SYS_MSYNC       26
#define SYS_MINCORE     27
#define SYS_MADVISE     28
#define SYS_SHMGET      29
#define SYS_SHMAT       30
#define SYS_SHMCTL      31
#define SYS_DUP         32
#define SYS_DUP2        33
#define SYS_PAUSE       34
#define SYS_NANOSLEEP   35
#define SYS_GETITIMER   36
#define SYS_ALARM       37
#define SYS_SETITIMER   38
#define SYS_GETPID      39
#define SYS_SENDFILE    40
#define SYS_SOCKET      41
#define SYS_CONNECT     42
#define SYS_ACCEPT      43
#define SYS_SENDTO      44
#define SYS_RECVFROM    45
#define SYS_SENDMSG     46
#define SYS_RECVMSG     47
#define SYS_SHUTDOWN    48
#define SYS_BIND        49
#define SYS_LISTEN      50
#define SYS_GETSOCKNAME 51
#define SYS_GETPEERNAME 52
#define SYS_SOCKETPAIR  53
#define SYS_SETSOCKOPT  54
#define SYS_GETSOCKOPT  55
#define SYS_CLONE       56
#define SYS_FORK        57
#define SYS_VFORK       58
#define SYS_EXECVE      59
#define SYS_EXIT        60
#define SYS_WAIT4       61
#define SYS_KILL        62
#define SYS_UNAME       63
#define SYS_SEMGET      64
#define SYS_SEMOP       65
#define SYS_SEMCTL      66
#define SYS_SHMDT       67
#define SYS_MSGGET      68
#define SYS_MSGSND      69
#define SYS_MSGRCV      70
#define SYS_MSGCTL      71
#define SYS_FCNTL       72
#define SYS_FLOCK       73
#define SYS_FSYNC       74
#define SYS_FDATASYNC   75
#define SYS_TRUNCATE    76
#define SYS_FTRUNCATE   77
#define SYS_GETDENTS    78
#define SYS_GETCWD      79
#define SYS_CHDIR       80
#define SYS_FCHDIR      81
#define SYS_RENAME      82
#define SYS_MKDIR       83
#define SYS_RMDIR       84
#define SYS_CREAT       85
#define SYS_LINK        86
#define SYS_UNLINK      87
#define SYS_SYMLINK     88
#define SYS_READLINK    89
#define SYS_CHMOD       90
#define SYS_FCHMOD      91
#define SYS_CHOWN       92
#define SYS_FCHOWN      93
#define SYS_LCHOWN      94
#define SYS_UMASK       95
#define SYS_GETTIMEOFDAY 96
#define SYS_GETRLIMIT   97
#define SYS_GETRUSAGE   98
#define SYS_SYSINFO     99
#define SYS_TIMES       100

/* Maximum number of system calls */
#define MAX_SYSCALLS    256

/* System call handler function pointer */
typedef i64 (*syscall_handler_t)(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);

/* System call table */
static syscall_handler_t syscall_table[MAX_SYSCALLS];

/* System call implementations */

/* File operations */
i64 sys_read(i32 fd, void* buf, size_t count) {
    struct process* current = get_current_process();
    if (!current || fd < 0 || fd >= MAX_FD_PER_PROCESS) {
        return -EBADF;
    }
    
    struct file_descriptor* file = current->fd_table[fd];
    if (!file) {
        return -EBADF;
    }
    
    return file_read(file->file, file->offset, buf, count);
}

i64 sys_write(i32 fd, const void* buf, size_t count) {
    struct process* current = get_current_process();
    if (!current || fd < 0 || fd >= MAX_FD_PER_PROCESS) {
        return -EBADF;
    }
    
    struct file_descriptor* file = current->fd_table[fd];
    if (!file) {
        return -EBADF;
    }
    
    i64 result = file_write(file->file, file->offset, buf, count);
    if (result > 0) {
        file->offset += result;
    }
    
    return result;
}

i64 sys_open(const char* pathname, i32 flags, mode_t mode) {
    struct process* current = get_current_process();
    if (!current) {
        return -ESRCH;
    }
    
    /* Find free file descriptor */
    i32 fd = -1;
    for (i32 i = 0; i < MAX_FD_PER_PROCESS; i++) {
        if (!current->fd_table[i]) {
            fd = i;
            break;
        }
    }
    
    if (fd == -1) {
        return -EMFILE;  /* Too many open files */
    }
    
    /* Open file */
    struct file* file = vfs_open(pathname, flags, mode);
    if (!file) {
        return -ENOENT;
    }
    
    /* Create file descriptor */
    struct file_descriptor* fd_entry = (struct file_descriptor*)kmalloc(sizeof(struct file_descriptor));
    fd_entry->file = file;
    fd_entry->offset = 0;
    fd_entry->flags = flags;
    
    current->fd_table[fd] = fd_entry;
    
    return fd;
}

i64 sys_close(i32 fd) {
    struct process* current = get_current_process();
    if (!current || fd < 0 || fd >= MAX_FD_PER_PROCESS) {
        return -EBADF;
    }
    
    struct file_descriptor* file = current->fd_table[fd];
    if (!file) {
        return -EBADF;
    }
    
    vfs_close(file->file);
    kfree(file);
    current->fd_table[fd] = NULL;
    
    return 0;
}

/* Process operations */
i64 sys_fork(void) {
    struct process* parent = get_current_process();
    if (!parent) {
        return -ESRCH;
    }
    
    /* Create child process */
    u32 child_pid = process_fork(parent);
    if (child_pid == 0) {
        return -ENOMEM;  /* Fork failed */
    }
    
    return child_pid;
}

i64 sys_execve(const char* filename, char* const argv[], char* const envp[]) {
    return process_execve(filename, argv, envp);
}

i64 sys_exit(i32 status) {
    process_exit(status);
    return 0;  /* Should not return */
}

i64 sys_wait4(pid_t pid, i32* status, i32 options, struct rusage* rusage) {
    return process_wait(pid, status, options, rusage);
}

i64 sys_getpid(void) {
    struct process* current = get_current_process();
    return current ? current->pid : -1;
}

i64 sys_getppid(void) {
    struct process* current = get_current_process();
    return current ? current->ppid : -1;
}

/* Memory operations */
i64 sys_mmap(void* addr, size_t length, i32 prot, i32 flags, i32 fd, off_t offset) {
    return (i64)mmap(addr, length, prot, flags, fd, offset);
}

i64 sys_munmap(void* addr, size_t length) {
    return munmap(addr, length);
}

i64 sys_brk(void* addr) {
    struct process* current = get_current_process();
    if (!current) {
        return -ESRCH;
    }
    
    if (!addr) {
        return current->heap_base + current->heap_size;  /* Return current break */
    }
    
    u64 new_break = (u64)addr;
    u64 old_break = current->heap_base + current->heap_size;
    
    if (new_break > old_break) {
        /* Expand heap */
        u64 expand_size = new_break - old_break;
        void* result = mmap((void*)old_break, expand_size, PROT_READ | PROT_WRITE, 
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
        if (result == MAP_FAILED) {
            return -ENOMEM;
        }
        current->heap_size += expand_size;
    } else if (new_break < old_break) {
        /* Shrink heap */
        u64 shrink_size = old_break - new_break;
        munmap((void*)new_break, shrink_size);
        current->heap_size -= shrink_size;
    }
    
    return new_break;
}

/* Signal operations */
i64 sys_kill(pid_t pid, i32 sig) {
    return signal_send(pid, sig);
}

i64 sys_rt_sigaction(i32 signum, const struct sigaction* act, struct sigaction* oldact) {
    return signal_action(signum, act, oldact);
}

/* IPC operations */
i64 sys_pipe(i32 pipefd[2]) {
    return pipe_create(pipefd);
}

i64 sys_msgget(key_t key, i32 msgflg) {
    return msgget(key, msgflg);
}

i64 sys_msgsnd(i32 msqid, const void* msgp, size_t msgsz, i32 msgflg) {
    return msgsnd(msqid, msgp, msgsz, msgflg);
}

i64 sys_msgrcv(i32 msqid, void* msgp, size_t msgsz, i32 msgtyp, i32 msgflg) {
    return msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
}

i64 sys_semget(key_t key, i32 nsems, i32 semflg) {
    return semget(key, nsems, semflg);
}

/* Filesystem operations */
i64 sys_stat(const char* pathname, struct stat* statbuf) {
    return vfs_stat(pathname, statbuf);
}

i64 sys_mkdir(const char* pathname, mode_t mode) {
    return vfs_mkdir(pathname, mode);
}

i64 sys_rmdir(const char* pathname) {
    return vfs_rmdir(pathname);
}

i64 sys_unlink(const char* pathname) {
    return vfs_unlink(pathname);
}

i64 sys_chdir(const char* path) {
    return vfs_chdir(path);
}

i64 sys_getcwd(char* buf, size_t size) {
    return vfs_getcwd(buf, size);
}

/* Time operations */
i64 sys_gettimeofday(struct timeval* tv, struct timezone* tz) {
    if (tv) {
        u64 time = get_system_time();
        tv->tv_sec = time / 1000000;
        tv->tv_usec = time % 1000000;
    }
    return 0;
}

i64 sys_nanosleep(const struct timespec* req, struct timespec* rem) {
    u64 nanoseconds = req->tv_sec * 1000000000ULL + req->tv_nsec;
    return timer_sleep(nanoseconds);
}

/* System information */
i64 sys_uname(struct utsname* buf) {
    strcpy(buf->sysname, "Kronos");
    strcpy(buf->nodename, "kronos-system");
    strcpy(buf->release, "1.0.0");
    strcpy(buf->version, "Kronos OS v1.0");
    strcpy(buf->machine, "x86_64");
    return 0;
}

i64 sys_sysinfo(struct sysinfo* info) {
    get_system_info(info);
    return 0;
}

/* Scheduler operations */
i64 sys_sched_yield(void) {
    schedule();
    return 0;
}

/* Initialize system call table */
void syscall_init(void) {
    /* Clear syscall table */
    for (u32 i = 0; i < MAX_SYSCALLS; i++) {
        syscall_table[i] = NULL;
    }
    
    /* Register system calls */
    syscall_table[SYS_READ] = (syscall_handler_t)sys_read;
    syscall_table[SYS_WRITE] = (syscall_handler_t)sys_write;
    syscall_table[SYS_OPEN] = (syscall_handler_t)sys_open;
    syscall_table[SYS_CLOSE] = (syscall_handler_t)sys_close;
    syscall_table[SYS_STAT] = (syscall_handler_t)sys_stat;
    syscall_table[SYS_MMAP] = (syscall_handler_t)sys_mmap;
    syscall_table[SYS_MUNMAP] = (syscall_handler_t)sys_munmap;
    syscall_table[SYS_BRK] = (syscall_handler_t)sys_brk;
    syscall_table[SYS_PIPE] = (syscall_handler_t)sys_pipe;
    syscall_table[SYS_SCHED_YIELD] = (syscall_handler_t)sys_sched_yield;
    syscall_table[SYS_GETPID] = (syscall_handler_t)sys_getpid;
    syscall_table[SYS_FORK] = (syscall_handler_t)sys_fork;
    syscall_table[SYS_EXECVE] = (syscall_handler_t)sys_execve;
    syscall_table[SYS_EXIT] = (syscall_handler_t)sys_exit;
    syscall_table[SYS_WAIT4] = (syscall_handler_t)sys_wait4;
    syscall_table[SYS_KILL] = (syscall_handler_t)sys_kill;
    syscall_table[SYS_UNAME] = (syscall_handler_t)sys_uname;
    syscall_table[SYS_MSGGET] = (syscall_handler_t)sys_msgget;
    syscall_table[SYS_MSGSND] = (syscall_handler_t)sys_msgsnd;
    syscall_table[SYS_MSGRCV] = (syscall_handler_t)sys_msgrcv;
    syscall_table[SYS_SEMGET] = (syscall_handler_t)sys_semget;
    syscall_table[SYS_MKDIR] = (syscall_handler_t)sys_mkdir;
    syscall_table[SYS_RMDIR] = (syscall_handler_t)sys_rmdir;
    syscall_table[SYS_UNLINK] = (syscall_handler_t)sys_unlink;
    syscall_table[SYS_CHDIR] = (syscall_handler_t)sys_chdir;
    syscall_table[SYS_GETCWD] = (syscall_handler_t)sys_getcwd;
    syscall_table[SYS_GETTIMEOFDAY] = (syscall_handler_t)sys_gettimeofday;
    syscall_table[SYS_NANOSLEEP] = (syscall_handler_t)sys_nanosleep;
    syscall_table[SYS_SYSINFO] = (syscall_handler_t)sys_sysinfo;
    
    vga_puts("System call interface initialized\n");
}

/* System call dispatcher */
i64 syscall_handler(u64 syscall_num, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
    if (syscall_num >= MAX_SYSCALLS || !syscall_table[syscall_num]) {
        return -ENOSYS;  /* System call not implemented */
    }
    
    /* Call the system call handler */
    return syscall_table[syscall_num](arg1, arg2, arg3, arg4, arg5, arg6);
}
