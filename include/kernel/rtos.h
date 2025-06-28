#ifndef RTOS_H
#define RTOS_H

#include "types.h"
#include "process.h"

/* RTOS Configuration */
#define RTOS_MAX_RT_PRIORITY    99      /* Real-time priority range: 0-99 */
#define RTOS_TICK_RATE_HZ       1000    /* System tick frequency */
#define RTOS_MAX_PERIODIC_TASKS 32      /* Maximum periodic tasks */

/* RTOS Process States */
#define RTOS_TASK_READY         0
#define RTOS_TASK_RUNNING       1
#define RTOS_TASK_BLOCKED       2
#define RTOS_TASK_SUSPENDED     3
#define RTOS_TASK_TERMINATED    4

/* RTOS Priority Classes */
#define RTOS_PRIORITY_IDLE      255     /* Idle task priority */
#define RTOS_PRIORITY_LOW       200     /* Low priority tasks */
#define RTOS_PRIORITY_NORMAL    100     /* Normal priority tasks */
#define RTOS_PRIORITY_HIGH      50      /* High priority tasks */
#define RTOS_PRIORITY_REALTIME  0       /* Highest real-time priority */

/* RTOS Timing Statistics */
struct rtos_timing_stats {
    u64 system_ticks;                   /* Total system ticks */
    u32 max_interrupt_latency_us;       /* Maximum interrupt latency */
    u32 max_scheduling_latency_us;      /* Maximum scheduling latency */
    u32 active_timeouts;                /* Number of active timeouts */
    u32 rt_processes_ready;             /* Real-time processes ready */
    bool preemption_enabled;            /* Preemption status */
    u32 context_switches;               /* Total context switches */
    u32 missed_deadlines;               /* Missed real-time deadlines */
};

/* RTOS Task Control Block Extensions */
struct rtos_task_info {
    u32 period_ms;                      /* Task period in milliseconds */
    u64 next_deadline;                  /* Next deadline in ticks */
    u64 worst_case_execution_time;      /* WCET in ticks */
    u32 deadline_misses;                /* Number of missed deadlines */
    bool is_periodic;                   /* Is this a periodic task? */
    bool deadline_monitoring;           /* Enable deadline monitoring */
};

/* RTOS Memory Pool for Real-Time Allocation */
struct rtos_memory_pool {
    void* pool_start;                   /* Start of memory pool */
    u32 block_size;                     /* Size of each block */
    u32 total_blocks;                   /* Total number of blocks */
    u32 free_blocks;                    /* Number of free blocks */
    u8* allocation_bitmap;              /* Bitmap for block allocation */
    bool initialized;                   /* Pool initialization status */
};

/* RTOS Mutex with Priority Inheritance */
struct rtos_mutex {
    u32 owner_pid;                      /* Process ID of current owner */
    u32 original_priority;              /* Original priority of owner */
    u32 inherited_priority;             /* Inherited priority */
    bool priority_inherited;            /* Priority inheritance active */
    struct process* waiting_queue[32];  /* Priority-ordered waiting queue */
    u32 waiting_count;                  /* Number of waiting processes */
    bool recursive;                     /* Allow recursive locking */
    u32 lock_count;                     /* Recursive lock count */
};

/* RTOS Event Flags */
struct rtos_event_flags {
    u32 flags;                          /* Current flag state */
    struct process* waiting_processes[32]; /* Processes waiting for events */
    u32 wait_conditions[32];            /* Wait conditions for each process */
    u32 waiting_count;                  /* Number of waiting processes */
    bool auto_clear;                    /* Auto-clear flags on wait */
};

/* Function Declarations */

/* Core RTOS Functions */
void rtos_init(void);
void rtos_start_scheduler(void);
void rtos_tick_handler(void);
struct process* rtos_schedule_next(void);

/* Real-Time Task Management */
i32 rtos_set_realtime_priority(u32 pid, u32 priority);
i32 rtos_create_periodic_task(void (*task_func)(void), u32 period_ms, u32 priority);
i32 rtos_register_periodic_task(void (*task_func)(void), u32 period_ms, u32 priority);
void rtos_execute_periodic_tasks(void);
i32 rtos_set_deadline(u32 pid, u32 deadline_ms);
i32 rtos_monitor_deadline(u32 pid, bool enable);

/* Timing and Synchronization */
void rtos_get_timing_stats(struct rtos_timing_stats* stats);
void rtos_delay_us(u32 microseconds);
void rtos_delay_ms(u32 milliseconds);
u64 rtos_get_tick_count(void);
u32 rtos_ticks_to_ms(u64 ticks);
u64 rtos_ms_to_ticks(u32 ms);

/* Critical Sections and Preemption */
void rtos_enter_critical(void);
void rtos_exit_critical(void);
void rtos_set_preemption(bool enabled);
bool rtos_is_preemption_enabled(void);
void rtos_yield(void);

/* Enhanced IPC with RTOS Features */
i32 rtos_sem_wait_timeout(i32 semid, u32 timeout_ms);
i32 rtos_mutex_create(bool recursive);
i32 rtos_mutex_lock(i32 mutex_id, u32 timeout_ms);
i32 rtos_mutex_unlock(i32 mutex_id);
i32 rtos_mutex_destroy(i32 mutex_id);

/* Event Flags */
i32 rtos_event_create(void);
i32 rtos_event_set(i32 event_id, u32 flags);
i32 rtos_event_clear(i32 event_id, u32 flags);
i32 rtos_event_wait(i32 event_id, u32 flags, bool wait_all, u32 timeout_ms);
i32 rtos_event_destroy(i32 event_id);

/* Memory Management */
i32 rtos_pool_create(u32 block_size, u32 num_blocks);
void* rtos_pool_alloc(i32 pool_id);
i32 rtos_pool_free(i32 pool_id, void* ptr);
i32 rtos_pool_destroy(i32 pool_id);

/* Deadline Monitoring */
void rtos_deadline_monitor_init(void);
i32 rtos_deadline_check(u32 pid);
void rtos_deadline_violation_handler(u32 pid);

/* Performance Monitoring */
void rtos_performance_monitor_start(void);
void rtos_performance_monitor_stop(void);
void rtos_get_cpu_utilization(u32* utilization_percent);
void rtos_get_task_statistics(u32 pid, struct rtos_task_info* stats);

/* Power Management for RTOS */
void rtos_enter_low_power_mode(void);
void rtos_exit_low_power_mode(void);
void rtos_set_cpu_frequency(u32 frequency_mhz);

/* Interrupt Management */
void rtos_register_isr(u32 irq, void (*handler)(void));
void rtos_enable_irq(u32 irq);
void rtos_disable_irq(u32 irq);
u32 rtos_get_interrupt_latency(void);

/* Debug and Tracing */
void rtos_trace_enable(bool enable);
void rtos_trace_task_switch(u32 from_pid, u32 to_pid);
void rtos_trace_interrupt(u32 irq);
void rtos_dump_task_info(void);
void rtos_dump_timing_stats(void);

/* RTOS Configuration */
void rtos_set_tick_rate(u32 hz);
void rtos_set_max_interrupt_latency(u32 microseconds);
void rtos_set_max_scheduling_latency(u32 microseconds);

/* Utility Macros */
#define RTOS_TICKS_PER_SECOND   RTOS_TICK_RATE_HZ
#define RTOS_MS_TO_TICKS(ms)    ((ms) * RTOS_TICK_RATE_HZ / 1000)
#define RTOS_TICKS_TO_MS(ticks) ((ticks) * 1000 / RTOS_TICK_RATE_HZ)
#define RTOS_US_TO_TICKS(us)    ((us) * RTOS_TICK_RATE_HZ / 1000000)

/* RTOS Error Codes */
#define RTOS_OK                 0
#define RTOS_ERROR              -1
#define RTOS_TIMEOUT            -2
#define RTOS_INVALID_PARAM      -3
#define RTOS_NO_MEMORY          -4
#define RTOS_DEADLINE_MISSED    -5
#define RTOS_PRIORITY_INVALID   -6

/* RTOS Assert Macro for Debug Builds */
#ifdef RTOS_DEBUG
#define RTOS_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            rtos_panic("RTOS Assert failed: " #condition " at " __FILE__ ":" __LINE__); \
        } \
    } while(0)
#else
#define RTOS_ASSERT(condition) ((void)0)
#endif

/* RTOS Panic Function */
void rtos_panic(const char* message);

#endif /* RTOS_H */
