#include "kronos.h"

/* Inter-Process Communication for Kronos OS */

#define MAX_PIPES 128
#define MAX_MESSAGE_QUEUES 64
#define MAX_SEMAPHORES 128
#define MAX_SHARED_MEMORY 64
#define PIPE_BUFFER_SIZE 4096
#define MSG_QUEUE_SIZE 1024
#define MAX_MSG_SIZE 256

/* Signal definitions */
#define SIGHUP    1   /* Hangup */
#define SIGINT    2   /* Interrupt */
#define SIGQUIT   3   /* Quit */
#define SIGILL    4   /* Illegal instruction */
#define SIGTRAP   5   /* Trace trap */
#define SIGABRT   6   /* Abort */
#define SIGBUS    7   /* Bus error */
#define SIGFPE    8   /* Floating point exception */
#define SIGKILL   9   /* Kill */
#define SIGUSR1   10  /* User signal 1 */
#define SIGSEGV   11  /* Segmentation violation */
#define SIGUSR2   12  /* User signal 2 */
#define SIGPIPE   13  /* Broken pipe */
#define SIGALRM   14  /* Alarm */
#define SIGTERM   15  /* Termination */
#define SIGCHLD   17  /* Child status changed */
#define SIGCONT   18  /* Continue */
#define SIGSTOP   19  /* Stop */

/* Pipe structure */
struct pipe {
    u32 id;
    u32 read_fd;
    u32 write_fd;
    char buffer[PIPE_BUFFER_SIZE];
    u32 read_pos;
    u32 write_pos;
    u32 data_size;
    bool in_use;
    struct process* readers[MAX_PROCESSES];
    struct process* writers[MAX_PROCESSES];
    u32 reader_count;
    u32 writer_count;
} pipes[MAX_PIPES];

/* Message queue structure */
struct message {
    u32 type;
    u32 size;
    char data[MAX_MSG_SIZE];
};

struct message_queue {
    u32 id;
    u32 key;
    struct message messages[MSG_QUEUE_SIZE];
    u32 head;
    u32 tail;
    u32 count;
    u32 max_messages;
    u32 max_message_size;
    bool in_use;
    struct process* waiting_senders[MAX_PROCESSES];
    struct process* waiting_receivers[MAX_PROCESSES];
    u32 sender_count;
    u32 receiver_count;
} message_queues[MAX_MESSAGE_QUEUES];

/* Semaphore structure */
struct semaphore {
    u32 id;
    u32 key;
    i32 value;
    i32 max_value;
    bool in_use;
    struct process* waiting_processes[MAX_PROCESSES];
    u32 waiting_count;
} semaphores[MAX_SEMAPHORES];

/* Shared memory structure */
struct shared_memory {
    u32 id;
    u32 key;
    void* address;
    u32 size;
    u32 permissions;
    bool in_use;
    struct process* attached_processes[MAX_PROCESSES];
    u32 attach_count;
    u32 creator_pid;
} shared_memory_segments[MAX_SHARED_MEMORY];

/* Signal handler structure */
struct signal_handler {
    void (*handler)(int);
    u32 mask;
    u32 flags;
};

/* Process signal state */
struct signal_state {
    u32 pending_signals;
    u32 blocked_signals;
    struct signal_handler handlers[32];
};

/* RTOS timing and priority structures */
struct rtos_timeout {
    u64 deadline_ticks;
    bool has_timeout;
    struct process* waiting_process;
};

struct priority_queue {
    struct process* processes[MAX_PROCESSES];
    u32 priorities[MAX_PROCESSES];
    u32 count;
};

/* IPC system state with RTOS enhancements */
static struct {
    u32 next_pipe_id;
    u32 next_msgq_id;
    u32 next_sem_id;
    u32 next_shm_id;
    bool initialized;

    /* RTOS features */
    u64 system_ticks;
    struct rtos_timeout active_timeouts[MAX_PROCESSES];
    u32 timeout_count;
    struct priority_queue rt_ready_queue;
    bool preemption_enabled;
    u32 max_interrupt_latency_us;
    u32 max_scheduling_latency_us;
} ipc_system;

/* RTOS Helper Functions */

/* Get current system time in ticks */
static u64 rtos_get_ticks(void) {
    return ipc_system.system_ticks;
}

/* Convert milliseconds to ticks (assuming 1000 Hz timer) */
static u64 rtos_ms_to_ticks(u32 ms) {
    return (u64)ms;  /* 1 ms = 1 tick at 1000 Hz */
}

/* Priority queue operations for real-time scheduling */
static void priority_queue_insert(struct priority_queue* pq, struct process* proc, u32 priority) {
    if (pq->count >= MAX_PROCESSES) return;

    /* Insert in priority order (higher priority = lower number) */
    u32 i = pq->count;
    while (i > 0 && pq->priorities[i-1] > priority) {
        pq->processes[i] = pq->processes[i-1];
        pq->priorities[i] = pq->priorities[i-1];
        i--;
    }

    pq->processes[i] = proc;
    pq->priorities[i] = priority;
    pq->count++;
}

static struct process* priority_queue_remove_highest(struct priority_queue* pq) {
    if (pq->count == 0) return NULL;

    struct process* proc = pq->processes[0];

    /* Shift remaining processes */
    for (u32 i = 0; i < pq->count - 1; i++) {
        pq->processes[i] = pq->processes[i + 1];
        pq->priorities[i] = pq->priorities[i + 1];
    }
    pq->count--;

    return proc;
}

/* Add timeout for RTOS operations */
static void rtos_add_timeout(struct process* proc, u32 timeout_ms) {
    if (ipc_system.timeout_count >= MAX_PROCESSES) return;

    struct rtos_timeout* timeout = &ipc_system.active_timeouts[ipc_system.timeout_count++];
    timeout->deadline_ticks = rtos_get_ticks() + rtos_ms_to_ticks(timeout_ms);
    timeout->has_timeout = true;
    timeout->waiting_process = proc;
}

/* Check and handle expired timeouts */
static void rtos_check_timeouts(void) {
    u64 current_ticks = rtos_get_ticks();

    for (u32 i = 0; i < ipc_system.timeout_count; i++) {
        struct rtos_timeout* timeout = &ipc_system.active_timeouts[i];

        if (timeout->has_timeout && current_ticks >= timeout->deadline_ticks) {
            /* Timeout expired - wake up process */
            if (timeout->waiting_process->state == PROCESS_BLOCKED) {
                timeout->waiting_process->state = PROCESS_READY;
                priority_queue_insert(&ipc_system.rt_ready_queue,
                                    timeout->waiting_process,
                                    timeout->waiting_process->priority);
            }

            /* Remove timeout */
            for (u32 j = i; j < ipc_system.timeout_count - 1; j++) {
                ipc_system.active_timeouts[j] = ipc_system.active_timeouts[j + 1];
            }
            ipc_system.timeout_count--;
            i--; /* Recheck this index */
        }
    }
}

/* Initialize IPC system with RTOS features */
void ipc_init(void) {
    /* Clear all IPC structures */
    for (u32 i = 0; i < MAX_PIPES; i++) {
        pipes[i].in_use = false;
    }

    for (u32 i = 0; i < MAX_MESSAGE_QUEUES; i++) {
        message_queues[i].in_use = false;
    }

    for (u32 i = 0; i < MAX_SEMAPHORES; i++) {
        semaphores[i].in_use = false;
    }

    for (u32 i = 0; i < MAX_SHARED_MEMORY; i++) {
        shared_memory_segments[i].in_use = false;
    }

    /* Initialize RTOS features */
    ipc_system.next_pipe_id = 1;
    ipc_system.next_msgq_id = 1;
    ipc_system.next_sem_id = 1;
    ipc_system.next_shm_id = 1;
    ipc_system.system_ticks = 0;
    ipc_system.timeout_count = 0;
    ipc_system.rt_ready_queue.count = 0;
    ipc_system.preemption_enabled = true;
    ipc_system.max_interrupt_latency_us = 10;  /* 10 microseconds max */
    ipc_system.max_scheduling_latency_us = 100; /* 100 microseconds max */
    ipc_system.initialized = true;

    vga_puts("RTOS-enhanced IPC system initialized\n");
    vga_puts("Real-time features: Priority scheduling, Timeouts, Preemption\n");
}

/* PIPES */

/* Create pipe */
i32 pipe_create(i32 pipefd[2]) {
    /* Find free pipe slot */
    struct pipe* p = NULL;
    for (u32 i = 0; i < MAX_PIPES; i++) {
        if (!pipes[i].in_use) {
            p = &pipes[i];
            break;
        }
    }
    
    if (!p) {
        return -1;  /* No free pipes */
    }
    
    /* Initialize pipe */
    p->id = ipc_system.next_pipe_id++;
    p->read_fd = fd_allocate();
    p->write_fd = fd_allocate();
    p->read_pos = 0;
    p->write_pos = 0;
    p->data_size = 0;
    p->reader_count = 0;
    p->writer_count = 0;
    p->in_use = true;
    
    /* Set up file descriptors */
    pipefd[0] = p->read_fd;   /* Read end */
    pipefd[1] = p->write_fd;  /* Write end */
    
    /* Register with current process */
    struct process* current = get_current_process();
    if (current) {
        current->fd_table[p->read_fd] = (struct file_descriptor*)p;
        current->fd_table[p->write_fd] = (struct file_descriptor*)p;
    }
    
    return 0;
}

/* Read from pipe */
i32 pipe_read(u32 pipe_id, void* buffer, u32 size) {
    struct pipe* p = &pipes[pipe_id];
    if (!p->in_use) {
        return -1;
    }
    
    /* Block if no data available */
    while (p->data_size == 0) {
        if (p->writer_count == 0) {
            return 0;  /* EOF - no writers */
        }
        
        /* Add to waiting readers */
        struct process* current = get_current_process();
        p->readers[p->reader_count++] = current;
        current->state = PROCESS_BLOCKED;
        schedule();  /* Block until data available */
    }
    
    /* Read data */
    u32 bytes_read = 0;
    char* buf = (char*)buffer;
    
    while (bytes_read < size && p->data_size > 0) {
        buf[bytes_read++] = p->buffer[p->read_pos];
        p->read_pos = (p->read_pos + 1) % PIPE_BUFFER_SIZE;
        p->data_size--;
    }
    
    /* Wake up waiting writers */
    for (u32 i = 0; i < p->writer_count; i++) {
        p->writers[i]->state = PROCESS_READY;
        cfs_enqueue_task(p->writers[i]);
    }
    p->writer_count = 0;
    
    return bytes_read;
}

/* Write to pipe */
i32 pipe_write(u32 pipe_id, const void* buffer, u32 size) {
    struct pipe* p = &pipes[pipe_id];
    if (!p->in_use) {
        return -1;
    }
    
    if (p->reader_count == 0) {
        /* No readers - send SIGPIPE */
        signal_send(get_current_process()->pid, SIGPIPE);
        return -1;
    }
    
    /* Block if pipe full */
    while (p->data_size >= PIPE_BUFFER_SIZE - size) {
        struct process* current = get_current_process();
        p->writers[p->writer_count++] = current;
        current->state = PROCESS_BLOCKED;
        schedule();  /* Block until space available */
    }
    
    /* Write data */
    u32 bytes_written = 0;
    const char* buf = (const char*)buffer;
    
    while (bytes_written < size && p->data_size < PIPE_BUFFER_SIZE) {
        p->buffer[p->write_pos] = buf[bytes_written++];
        p->write_pos = (p->write_pos + 1) % PIPE_BUFFER_SIZE;
        p->data_size++;
    }
    
    /* Wake up waiting readers */
    for (u32 i = 0; i < p->reader_count; i++) {
        p->readers[i]->state = PROCESS_READY;
        cfs_enqueue_task(p->readers[i]);
    }
    p->reader_count = 0;
    
    return bytes_written;
}

/* MESSAGE QUEUES */

/* Create message queue */
i32 msgget(u32 key, i32 flags) {
    /* Find existing queue with key */
    for (u32 i = 0; i < MAX_MESSAGE_QUEUES; i++) {
        if (message_queues[i].in_use && message_queues[i].key == key) {
            return message_queues[i].id;
        }
    }
    
    /* Create new queue if IPC_CREAT flag set */
    if (!(flags & IPC_CREAT)) {
        return -1;
    }
    
    /* Find free slot */
    struct message_queue* mq = NULL;
    for (u32 i = 0; i < MAX_MESSAGE_QUEUES; i++) {
        if (!message_queues[i].in_use) {
            mq = &message_queues[i];
            break;
        }
    }
    
    if (!mq) {
        return -1;
    }
    
    /* Initialize message queue */
    mq->id = ipc_system.next_msgq_id++;
    mq->key = key;
    mq->head = 0;
    mq->tail = 0;
    mq->count = 0;
    mq->max_messages = MSG_QUEUE_SIZE;
    mq->max_message_size = MAX_MSG_SIZE;
    mq->sender_count = 0;
    mq->receiver_count = 0;
    mq->in_use = true;
    
    return mq->id;
}

/* Send message */
i32 msgsnd(i32 msgqid, const void* msgp, u32 msgsz, i32 msgflg) {
    struct message_queue* mq = &message_queues[msgqid];
    if (!mq->in_use) {
        return -1;
    }
    
    if (msgsz > MAX_MSG_SIZE) {
        return -1;
    }
    
    /* Block if queue full */
    while (mq->count >= mq->max_messages) {
        if (msgflg & IPC_NOWAIT) {
            return -1;
        }
        
        struct process* current = get_current_process();
        mq->waiting_senders[mq->sender_count++] = current;
        current->state = PROCESS_BLOCKED;
        schedule();
    }
    
    /* Add message to queue */
    struct message* msg = &mq->messages[mq->tail];
    const struct message* input_msg = (const struct message*)msgp;
    
    msg->type = input_msg->type;
    msg->size = msgsz;
    memcpy(msg->data, input_msg->data, msgsz);
    
    mq->tail = (mq->tail + 1) % mq->max_messages;
    mq->count++;
    
    /* Wake up waiting receivers */
    for (u32 i = 0; i < mq->receiver_count; i++) {
        mq->waiting_receivers[i]->state = PROCESS_READY;
        cfs_enqueue_task(mq->waiting_receivers[i]);
    }
    mq->receiver_count = 0;
    
    return 0;
}

/* Receive message */
i32 msgrcv(i32 msgqid, void* msgp, u32 msgsz, i32 msgtyp, i32 msgflg) {
    struct message_queue* mq = &message_queues[msgqid];
    if (!mq->in_use) {
        return -1;
    }
    
    /* Block if no messages */
    while (mq->count == 0) {
        if (msgflg & IPC_NOWAIT) {
            return -1;
        }
        
        struct process* current = get_current_process();
        mq->waiting_receivers[mq->receiver_count++] = current;
        current->state = PROCESS_BLOCKED;
        schedule();
    }
    
    /* Find message of requested type */
    u32 msg_index = mq->head;
    struct message* msg = &mq->messages[msg_index];
    
    if (msgtyp > 0) {
        /* Look for specific type */
        bool found = false;
        for (u32 i = 0; i < mq->count; i++) {
            u32 index = (mq->head + i) % mq->max_messages;
            if (mq->messages[index].type == msgtyp) {
                msg_index = index;
                msg = &mq->messages[index];
                found = true;
                break;
            }
        }
        
        if (!found) {
            return -1;
        }
    }
    
    /* Copy message to user buffer */
    struct message* output_msg = (struct message*)msgp;
    output_msg->type = msg->type;
    
    u32 copy_size = (msg->size < msgsz) ? msg->size : msgsz;
    memcpy(output_msg->data, msg->data, copy_size);
    
    /* Remove message from queue */
    if (msg_index == mq->head) {
        mq->head = (mq->head + 1) % mq->max_messages;
    } else {
        /* Shift messages if not head */
        for (u32 i = msg_index; i != mq->tail; i = (i + 1) % mq->max_messages) {
            u32 next = (i + 1) % mq->max_messages;
            mq->messages[i] = mq->messages[next];
        }
        mq->tail = (mq->tail - 1 + mq->max_messages) % mq->max_messages;
    }
    
    mq->count--;
    
    /* Wake up waiting senders */
    for (u32 i = 0; i < mq->sender_count; i++) {
        mq->waiting_senders[i]->state = PROCESS_READY;
        cfs_enqueue_task(mq->waiting_senders[i]);
    }
    mq->sender_count = 0;
    
    return copy_size;
}

/* SEMAPHORES */

/* Create semaphore */
i32 semget(u32 key, i32 nsems, i32 flags) {
    /* Find existing semaphore */
    for (u32 i = 0; i < MAX_SEMAPHORES; i++) {
        if (semaphores[i].in_use && semaphores[i].key == key) {
            return semaphores[i].id;
        }
    }
    
    /* Create new semaphore */
    if (!(flags & IPC_CREAT)) {
        return -1;
    }
    
    struct semaphore* sem = NULL;
    for (u32 i = 0; i < MAX_SEMAPHORES; i++) {
        if (!semaphores[i].in_use) {
            sem = &semaphores[i];
            break;
        }
    }
    
    if (!sem) {
        return -1;
    }
    
    sem->id = ipc_system.next_sem_id++;
    sem->key = key;
    sem->value = 1;  /* Binary semaphore by default */
    sem->max_value = 1;
    sem->waiting_count = 0;
    sem->in_use = true;
    
    return sem->id;
}

/* RTOS-enhanced semaphore wait with timeout and priority inheritance */
i32 sem_wait_timeout(i32 semid, u32 timeout_ms) {
    struct semaphore* sem = &semaphores[semid];
    if (!sem->in_use) {
        return -1;
    }

    struct process* current = get_current_process();
    u64 start_ticks = rtos_get_ticks();
    u64 deadline = start_ticks + rtos_ms_to_ticks(timeout_ms);

    /* Try to acquire semaphore */
    if (sem->value > 0) {
        sem->value--;
        return 0;  /* Success */
    }

    /* Need to wait - implement priority inheritance */
    if (sem->waiting_count > 0) {
        /* Find highest priority waiting process */
        u32 highest_priority = current->priority;
        for (u32 i = 0; i < sem->waiting_count; i++) {
            if (sem->waiting_processes[i]->priority < highest_priority) {
                highest_priority = sem->waiting_processes[i]->priority;
            }
        }

        /* Boost current holder's priority if needed */
        if (current->priority > highest_priority) {
            current->original_priority = current->priority;
            current->priority = highest_priority;
            current->priority_inherited = true;
        }
    }

    /* Add to waiting queue in priority order */
    u32 insert_pos = sem->waiting_count;
    for (u32 i = 0; i < sem->waiting_count; i++) {
        if (current->priority < sem->waiting_processes[i]->priority) {
            insert_pos = i;
            break;
        }
    }

    /* Shift processes to make room */
    for (u32 i = sem->waiting_count; i > insert_pos; i--) {
        sem->waiting_processes[i] = sem->waiting_processes[i-1];
    }
    sem->waiting_processes[insert_pos] = current;
    sem->waiting_count++;

    /* Set up timeout if specified */
    if (timeout_ms > 0) {
        rtos_add_timeout(current, timeout_ms);
    }

    /* Block and wait */
    current->state = PROCESS_BLOCKED;

    /* Wait loop with timeout checking */
    while (sem->value <= 0) {
        if (timeout_ms > 0 && rtos_get_ticks() >= deadline) {
            /* Timeout - remove from waiting queue */
            for (u32 i = 0; i < sem->waiting_count; i++) {
                if (sem->waiting_processes[i] == current) {
                    for (u32 j = i; j < sem->waiting_count - 1; j++) {
                        sem->waiting_processes[j] = sem->waiting_processes[j + 1];
                    }
                    sem->waiting_count--;
                    break;
                }
            }
            current->state = PROCESS_READY;
            return -2;  /* Timeout error */
        }

        schedule();
        rtos_check_timeouts();
    }

    sem->value--;
    return 0;
}

/* Standard semaphore wait (infinite timeout) */
i32 sem_wait(i32 semid) {
    return sem_wait_timeout(semid, 0);  /* 0 = infinite timeout */
}

/* Semaphore signal (V operation) */
i32 sem_signal(i32 semid) {
    struct semaphore* sem = &semaphores[semid];
    if (!sem->in_use) {
        return -1;
    }
    
    sem->value++;
    
    /* Wake up one waiting process */
    if (sem->waiting_count > 0) {
        struct process* proc = sem->waiting_processes[0];
        proc->state = PROCESS_READY;
        cfs_enqueue_task(proc);
        
        /* Shift waiting queue */
        for (u32 i = 0; i < sem->waiting_count - 1; i++) {
            sem->waiting_processes[i] = sem->waiting_processes[i + 1];
        }
        sem->waiting_count--;
    }
    
    return 0;
}

/* SIGNALS */

/* Send signal to process */
i32 signal_send(u32 pid, i32 signal) {
    struct process* target = get_process_by_pid(pid);
    if (!target) {
        return -1;
    }
    
    /* Add signal to pending signals */
    target->signal_state.pending_signals |= (1 << signal);
    
    /* If process is blocked, wake it up for signal handling */
    if (target->state == PROCESS_BLOCKED) {
        target->state = PROCESS_READY;
        cfs_enqueue_task(target);
    }
    
    return 0;
}

/* Handle pending signals */
void signal_handle_pending(struct process* proc) {
    u32 pending = proc->signal_state.pending_signals & ~proc->signal_state.blocked_signals;
    
    for (i32 sig = 1; sig < 32; sig++) {
        if (pending & (1 << sig)) {
            /* Clear pending signal */
            proc->signal_state.pending_signals &= ~(1 << sig);
            
            /* Call signal handler */
            struct signal_handler* handler = &proc->signal_state.handlers[sig];
            if (handler->handler) {
                handler->handler(sig);
            } else {
                /* Default signal handling */
                switch (sig) {
                    case SIGKILL:
                    case SIGTERM:
                        process_exit(128 + sig);
                        break;
                    case SIGSTOP:
                        proc->state = PROCESS_BLOCKED;
                        break;
                    default:
                        /* Ignore signal */
                        break;
                }
            }
        }
    }
}

/* RTOS SYSTEM FUNCTIONS */

/* System tick handler - called from timer interrupt */
void rtos_tick_handler(void) {
    ipc_system.system_ticks++;

    /* Check for expired timeouts */
    rtos_check_timeouts();

    /* Update process time slices for round-robin within priorities */
    struct process* current = get_current_process();
    if (current && current->time_slice > 0) {
        current->time_slice--;

        /* If time slice expired, trigger preemption */
        if (current->time_slice == 0 && ipc_system.preemption_enabled) {
            current->time_slice = current->default_time_slice;
            schedule();  /* Preemptive scheduling */
        }
    }
}

/* Real-time scheduler - priority-based with round-robin within priorities */
struct process* rtos_schedule_next(void) {
    /* First check real-time ready queue */
    struct process* rt_proc = priority_queue_remove_highest(&ipc_system.rt_ready_queue);
    if (rt_proc) {
        return rt_proc;
    }

    /* Fall back to regular CFS scheduler */
    return cfs_pick_next_task();
}

/* Set process as real-time with priority */
i32 rtos_set_realtime_priority(u32 pid, u32 priority) {
    struct process* proc = get_process_by_pid(pid);
    if (!proc) {
        return -1;
    }

    /* Real-time priorities: 0-99 (0 = highest) */
    if (priority > 99) {
        return -1;
    }

    proc->priority = priority;
    proc->is_realtime = true;
    proc->time_slice = 10;  /* Shorter time slice for RT processes */
    proc->default_time_slice = 10;

    /* If process is ready, add to RT queue */
    if (proc->state == PROCESS_READY) {
        priority_queue_insert(&ipc_system.rt_ready_queue, proc, priority);
    }

    return 0;
}

/* Get system timing statistics */
void rtos_get_timing_stats(struct rtos_timing_stats* stats) {
    stats->system_ticks = ipc_system.system_ticks;
    stats->max_interrupt_latency_us = ipc_system.max_interrupt_latency_us;
    stats->max_scheduling_latency_us = ipc_system.max_scheduling_latency_us;
    stats->active_timeouts = ipc_system.timeout_count;
    stats->rt_processes_ready = ipc_system.rt_ready_queue.count;
    stats->preemption_enabled = ipc_system.preemption_enabled;
}

/* Enable/disable preemption */
void rtos_set_preemption(bool enabled) {
    ipc_system.preemption_enabled = enabled;
}

/* Critical section management */
static u32 critical_section_count = 0;

void rtos_enter_critical(void) {
    disable_interrupts();
    critical_section_count++;
}

void rtos_exit_critical(void) {
    if (critical_section_count > 0) {
        critical_section_count--;
        if (critical_section_count == 0) {
            enable_interrupts();
        }
    }
}

/* High-precision delay for real-time applications */
void rtos_delay_us(u32 microseconds) {
    u64 start_ticks = rtos_get_ticks();
    u64 delay_ticks = (microseconds * 1000) / 1000000;  /* Convert to ticks */

    while ((rtos_get_ticks() - start_ticks) < delay_ticks) {
        /* Busy wait for precise timing */
        __asm__ volatile ("nop");
    }
}

/* Periodic task support */
struct periodic_task {
    void (*task_function)(void);
    u32 period_ms;
    u64 next_execution;
    bool active;
    u32 priority;
};

static struct periodic_task periodic_tasks[32];
static u32 periodic_task_count = 0;

/* Register periodic task */
i32 rtos_register_periodic_task(void (*task_func)(void), u32 period_ms, u32 priority) {
    if (periodic_task_count >= 32) {
        return -1;
    }

    struct periodic_task* task = &periodic_tasks[periodic_task_count++];
    task->task_function = task_func;
    task->period_ms = period_ms;
    task->next_execution = rtos_get_ticks() + rtos_ms_to_ticks(period_ms);
    task->active = true;
    task->priority = priority;

    return periodic_task_count - 1;
}

/* Execute periodic tasks - called from scheduler */
void rtos_execute_periodic_tasks(void) {
    u64 current_ticks = rtos_get_ticks();

    for (u32 i = 0; i < periodic_task_count; i++) {
        struct periodic_task* task = &periodic_tasks[i];

        if (task->active && current_ticks >= task->next_execution) {
            /* Execute task */
            task->task_function();

            /* Schedule next execution */
            task->next_execution = current_ticks + rtos_ms_to_ticks(task->period_ms);
        }
    }
}
