#include "kronos.h"

/* Advanced Multitasking & Completely Fair Scheduler (CFS) for Kronos OS */

#define MAX_PROCESSES 256
#define PROCESS_STACK_SIZE 8192
#define TIME_SLICE_MS 10
#define CFS_PERIOD_NS 6000000  /* 6ms period */
#define CFS_MIN_GRANULARITY_NS 750000  /* 0.75ms minimum */

/* Process states */
typedef enum {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_ZOMBIE,
    PROCESS_TERMINATED
} process_state_t;

/* Process priority levels */
typedef enum {
    PRIORITY_REALTIME = 0,
    PRIORITY_HIGH = 1,
    PRIORITY_NORMAL = 2,
    PRIORITY_LOW = 3,
    PRIORITY_IDLE = 4
} process_priority_t;

/* CPU registers for context switching */
struct cpu_context {
    u64 rax, rbx, rcx, rdx;
    u64 rsi, rdi, rbp, rsp;
    u64 r8, r9, r10, r11;
    u64 r12, r13, r14, r15;
    u64 rip, rflags;
    u64 cr3;  /* Page directory */
} __attribute__((packed));

/* Process Control Block (PCB) */
struct process {
    u32 pid;                    /* Process ID */
    u32 ppid;                   /* Parent Process ID */
    char name[64];              /* Process name */
    process_state_t state;      /* Current state */
    process_priority_t priority; /* Process priority */
    
    /* CPU context for context switching */
    struct cpu_context context;
    
    /* Memory management */
    u64 virtual_memory_base;
    u64 virtual_memory_size;
    u64 stack_base;
    u64 heap_base;
    u64 heap_size;
    
    /* CFS scheduling data */
    u64 vruntime;              /* Virtual runtime */
    u64 exec_start;            /* When process started executing */
    u64 sum_exec_runtime;      /* Total execution time */
    u64 nice_value;            /* Nice value (-20 to 19) */
    u64 weight;                /* Scheduling weight */
    
    /* Time accounting */
    u64 creation_time;
    u64 last_scheduled;
    u64 total_cpu_time;
    
    /* File descriptors */
    struct file_descriptor* fd_table[MAX_FD_PER_PROCESS];
    
    /* Process tree */
    struct process* parent;
    struct process* children[MAX_CHILD_PROCESSES];
    u32 child_count;
    
    /* Synchronization */
    u32 exit_code;
    bool in_use;
} processes[MAX_PROCESSES];

/* CFS Red-Black Tree for runqueue */
struct rb_node {
    struct process* process;
    u64 key;  /* vruntime */
    struct rb_node* left;
    struct rb_node* right;
    struct rb_node* parent;
    bool red;
};

/* Scheduler state */
static struct {
    struct process* current_process;
    struct process* idle_process;
    struct rb_node* cfs_runqueue;  /* Red-black tree root */
    u64 total_weight;
    u64 min_vruntime;
    u32 nr_running;
    u32 next_pid;
    bool scheduler_enabled;
} scheduler;

/* Timer for preemption */
static u64 scheduler_timer = 0;

/* Initialize scheduler */
void scheduler_init(void) {
    /* Clear process table */
    for (u32 i = 0; i < MAX_PROCESSES; i++) {
        processes[i].in_use = false;
        processes[i].pid = 0;
    }
    
    scheduler.current_process = NULL;
    scheduler.idle_process = NULL;
    scheduler.cfs_runqueue = NULL;
    scheduler.total_weight = 0;
    scheduler.min_vruntime = 0;
    scheduler.nr_running = 0;
    scheduler.next_pid = 1;
    scheduler.scheduler_enabled = false;
    
    /* Create idle process */
    create_idle_process();
    
    /* Enable scheduler */
    scheduler.scheduler_enabled = true;
    
    vga_puts("CFS Scheduler initialized\n");
}

/* Create idle process */
static void create_idle_process(void) {
    struct process* idle = &processes[0];
    
    idle->pid = 0;
    idle->ppid = 0;
    strcpy(idle->name, "idle");
    idle->state = PROCESS_READY;
    idle->priority = PRIORITY_IDLE;
    idle->vruntime = 0;
    idle->nice_value = 19;  /* Lowest priority */
    idle->weight = 15;      /* Minimum weight */
    idle->in_use = true;
    
    scheduler.idle_process = idle;
}

/* Calculate process weight based on nice value */
static u64 calculate_weight(i32 nice) {
    /* Weight table for nice values */
    static const u64 nice_to_weight[40] = {
        88761, 71755, 56483, 46273, 36291,  /* -20 to -16 */
        29154, 23254, 18705, 14949, 11916,  /* -15 to -11 */
        9548, 7620, 6100, 4904, 3906,       /* -10 to -6 */
        3121, 2501, 1991, 1586, 1277,       /* -5 to -1 */
        1024, 820, 655, 526, 423,           /* 0 to 4 */
        335, 272, 215, 172, 137,             /* 5 to 9 */
        110, 87, 70, 56, 45,                 /* 10 to 14 */
        36, 29, 23, 18, 15                   /* 15 to 19 */
    };
    
    if (nice < -20) nice = -20;
    if (nice > 19) nice = 19;
    
    return nice_to_weight[nice + 20];
}

/* Create new process */
u32 process_create(const char* name, void* entry_point, process_priority_t priority) {
    /* Find free process slot */
    struct process* proc = NULL;
    for (u32 i = 1; i < MAX_PROCESSES; i++) {
        if (!processes[i].in_use) {
            proc = &processes[i];
            break;
        }
    }
    
    if (!proc) {
        return 0;  /* No free slots */
    }
    
    /* Initialize process */
    proc->pid = scheduler.next_pid++;
    proc->ppid = scheduler.current_process ? scheduler.current_process->pid : 0;
    strcpy(proc->name, name);
    proc->state = PROCESS_READY;
    proc->priority = priority;
    proc->nice_value = (priority == PRIORITY_HIGH) ? -5 : 
                      (priority == PRIORITY_LOW) ? 5 : 0;
    proc->weight = calculate_weight(proc->nice_value);
    
    /* Allocate virtual memory */
    proc->virtual_memory_base = 0x400000;  /* 4MB base */
    proc->virtual_memory_size = 0x100000;  /* 1MB size */
    proc->stack_base = proc->virtual_memory_base + proc->virtual_memory_size - PROCESS_STACK_SIZE;
    proc->heap_base = proc->virtual_memory_base + 0x10000;  /* 64KB offset */
    proc->heap_size = 0;
    
    /* Initialize CPU context */
    memset(&proc->context, 0, sizeof(struct cpu_context));
    proc->context.rip = (u64)entry_point;
    proc->context.rsp = proc->stack_base + PROCESS_STACK_SIZE - 8;
    proc->context.rflags = 0x202;  /* Enable interrupts */
    
    /* CFS initialization */
    proc->vruntime = scheduler.min_vruntime;
    proc->exec_start = 0;
    proc->sum_exec_runtime = 0;
    
    /* Time accounting */
    proc->creation_time = get_system_time();
    proc->last_scheduled = 0;
    proc->total_cpu_time = 0;
    
    /* Process tree */
    proc->parent = scheduler.current_process;
    proc->child_count = 0;
    
    /* File descriptors */
    for (u32 i = 0; i < MAX_FD_PER_PROCESS; i++) {
        proc->fd_table[i] = NULL;
    }
    
    proc->in_use = true;
    
    /* Add to runqueue */
    cfs_enqueue_task(proc);
    
    return proc->pid;
}

/* CFS Red-Black Tree operations */
static void cfs_enqueue_task(struct process* proc) {
    if (proc->state != PROCESS_READY) {
        proc->state = PROCESS_READY;
    }
    
    /* Insert into red-black tree based on vruntime */
    rb_insert(&scheduler.cfs_runqueue, proc, proc->vruntime);
    
    scheduler.total_weight += proc->weight;
    scheduler.nr_running++;
}

static void cfs_dequeue_task(struct process* proc) {
    rb_remove(&scheduler.cfs_runqueue, proc->vruntime);
    
    scheduler.total_weight -= proc->weight;
    scheduler.nr_running--;
}

/* Calculate time slice for process */
static u64 calculate_time_slice(struct process* proc) {
    if (scheduler.nr_running == 0) {
        return CFS_PERIOD_NS;
    }
    
    u64 slice = (CFS_PERIOD_NS * proc->weight) / scheduler.total_weight;
    
    /* Ensure minimum granularity */
    if (slice < CFS_MIN_GRANULARITY_NS) {
        slice = CFS_MIN_GRANULARITY_NS;
    }
    
    return slice;
}

/* Update process virtual runtime */
static void update_vruntime(struct process* proc, u64 delta_exec) {
    /* vruntime = runtime / weight */
    proc->vruntime += (delta_exec * 1024) / proc->weight;
    proc->sum_exec_runtime += delta_exec;
    
    /* Update minimum vruntime */
    if (proc->vruntime < scheduler.min_vruntime) {
        scheduler.min_vruntime = proc->vruntime;
    }
}

/* Pick next task to run (leftmost in RB tree) */
static struct process* cfs_pick_next_task(void) {
    if (!scheduler.cfs_runqueue) {
        return scheduler.idle_process;
    }
    
    /* Find leftmost node (minimum vruntime) */
    struct rb_node* leftmost = rb_leftmost(scheduler.cfs_runqueue);
    if (leftmost) {
        return leftmost->process;
    }
    
    return scheduler.idle_process;
}

/* Main scheduler function */
void schedule(void) {
    if (!scheduler.scheduler_enabled) {
        return;
    }
    
    struct process* prev = scheduler.current_process;
    struct process* next = cfs_pick_next_task();
    
    if (!next) {
        next = scheduler.idle_process;
    }
    
    if (prev == next) {
        return;  /* No context switch needed */
    }
    
    /* Update previous process statistics */
    if (prev && prev->state == PROCESS_RUNNING) {
        u64 now = get_system_time();
        u64 delta_exec = now - prev->exec_start;
        
        update_vruntime(prev, delta_exec);
        prev->total_cpu_time += delta_exec;
        prev->state = PROCESS_READY;
        
        /* Re-enqueue if still runnable */
        if (prev->state == PROCESS_READY) {
            cfs_enqueue_task(prev);
        }
    }
    
    /* Dequeue next process */
    if (next != scheduler.idle_process) {
        cfs_dequeue_task(next);
    }
    
    /* Update next process */
    next->state = PROCESS_RUNNING;
    next->exec_start = get_system_time();
    next->last_scheduled = next->exec_start;
    
    scheduler.current_process = next;
    
    /* Perform context switch */
    if (prev && prev != next) {
        context_switch(prev, next);
    }
}

/* Timer interrupt handler for preemption */
void scheduler_timer_interrupt(void) {
    scheduler_timer++;
    
    if (scheduler_timer % TIME_SLICE_MS == 0) {
        /* Time slice expired, trigger reschedule */
        schedule();
    }
}

/* Process termination */
void process_exit(u32 exit_code) {
    struct process* proc = scheduler.current_process;
    if (!proc || proc == scheduler.idle_process) {
        return;
    }
    
    proc->state = PROCESS_ZOMBIE;
    proc->exit_code = exit_code;
    
    /* Remove from runqueue */
    cfs_dequeue_task(proc);
    
    /* Notify parent */
    if (proc->parent) {
        /* Send SIGCHLD signal to parent */
        signal_send(proc->parent->pid, SIGCHLD);
    }
    
    /* Reschedule */
    schedule();
}

/* Get current process */
struct process* get_current_process(void) {
    return scheduler.current_process;
}

/* Get process by PID */
struct process* get_process_by_pid(u32 pid) {
    for (u32 i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].in_use && processes[i].pid == pid) {
            return &processes[i];
        }
    }
    return NULL;
}

/* Process statistics */
void get_process_stats(struct process_stats* stats) {
    stats->total_processes = 0;
    stats->running_processes = scheduler.nr_running;
    stats->zombie_processes = 0;
    
    for (u32 i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].in_use) {
            stats->total_processes++;
            if (processes[i].state == PROCESS_ZOMBIE) {
                stats->zombie_processes++;
            }
        }
    }
    
    stats->current_pid = scheduler.current_process ? scheduler.current_process->pid : 0;
    stats->scheduler_enabled = scheduler.scheduler_enabled;
}
