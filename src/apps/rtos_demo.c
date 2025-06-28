/*
 * RTOS Demo Application for Kronos OS
 * Demonstrates real-time features and capabilities
 */

#include "kernel/rtos.h"
#include "kernel/process.h"
#include "kernel/ipc.h"
#include "drivers/vga.h"
#include "lib/string.h"

/* Demo task priorities */
#define HIGH_PRIORITY_TASK      10
#define MEDIUM_PRIORITY_TASK    50
#define LOW_PRIORITY_TASK       90

/* Demo task periods (in milliseconds) */
#define FAST_TASK_PERIOD        100     /* 10 Hz */
#define MEDIUM_TASK_PERIOD      500     /* 2 Hz */
#define SLOW_TASK_PERIOD        1000    /* 1 Hz */

/* Global variables for demo */
static u32 task_counters[3] = {0, 0, 0};
static i32 demo_semaphore;
static i32 demo_mutex;
static i32 demo_event_flags;
static bool demo_running = true;

/* High Priority Real-Time Task */
void high_priority_task(void) {
    static u32 execution_count = 0;
    execution_count++;
    task_counters[0]++;
    
    /* Simulate critical real-time work */
    rtos_enter_critical();
    
    /* Critical section - simulate sensor reading */
    u32 sensor_data = execution_count * 42;  /* Simulated sensor value */
    
    rtos_exit_critical();
    
    /* Signal event to other tasks */
    if (execution_count % 5 == 0) {
        rtos_event_set(demo_event_flags, 0x01);
    }
    
    /* Display status every 50 executions */
    if (execution_count % 50 == 0) {
        vga_printf("High Priority Task: %u executions, sensor: %u\n", 
                   execution_count, sensor_data);
    }
}

/* Medium Priority Task */
void medium_priority_task(void) {
    static u32 execution_count = 0;
    execution_count++;
    task_counters[1]++;
    
    /* Wait for event from high priority task */
    i32 result = rtos_event_wait(demo_event_flags, 0x01, false, 100);
    
    if (result == RTOS_OK) {
        /* Process the event */
        vga_printf("Medium Priority Task: Received event (exec: %u)\n", execution_count);
        
        /* Clear the event flag */
        rtos_event_clear(demo_event_flags, 0x01);
        
        /* Acquire mutex for shared resource access */
        if (rtos_mutex_lock(demo_mutex, 200) == RTOS_OK) {
            /* Simulate shared resource processing */
            rtos_delay_ms(10);  /* Simulate work */
            
            rtos_mutex_unlock(demo_mutex);
        }
    } else if (result == RTOS_TIMEOUT) {
        vga_printf("Medium Priority Task: Event wait timeout\n");
    }
}

/* Low Priority Background Task */
void low_priority_task(void) {
    static u32 execution_count = 0;
    execution_count++;
    task_counters[2]++;
    
    /* Acquire semaphore for resource access */
    if (rtos_sem_wait_timeout(demo_semaphore, 500) == RTOS_OK) {
        /* Simulate background processing */
        vga_printf("Low Priority Task: Processing background work (exec: %u)\n", execution_count);
        
        /* Simulate longer processing time */
        rtos_delay_ms(50);
        
        /* Release semaphore */
        sem_post(demo_semaphore);
    } else {
        vga_printf("Low Priority Task: Semaphore timeout\n");
    }
    
    /* Display task statistics every 10 executions */
    if (execution_count % 10 == 0) {
        display_task_statistics();
    }
}

/* Display comprehensive task statistics */
void display_task_statistics(void) {
    struct rtos_timing_stats timing_stats;
    rtos_get_timing_stats(&timing_stats);
    
    vga_puts("\n=== RTOS Task Statistics ===\n");
    vga_printf("System Ticks: %llu\n", timing_stats.system_ticks);
    vga_printf("High Priority Task Executions: %u\n", task_counters[0]);
    vga_printf("Medium Priority Task Executions: %u\n", task_counters[1]);
    vga_printf("Low Priority Task Executions: %u\n", task_counters[2]);
    vga_printf("Max Interrupt Latency: %u us\n", timing_stats.max_interrupt_latency_us);
    vga_printf("Max Scheduling Latency: %u us\n", timing_stats.max_scheduling_latency_us);
    vga_printf("Active Timeouts: %u\n", timing_stats.active_timeouts);
    vga_printf("RT Processes Ready: %u\n", timing_stats.rt_processes_ready);
    vga_printf("Preemption: %s\n", timing_stats.preemption_enabled ? "Enabled" : "Disabled");
    vga_puts("============================\n\n");
}

/* Deadline monitoring task */
void deadline_monitor_task(void) {
    static u32 check_count = 0;
    check_count++;
    
    /* Check deadlines for all real-time tasks */
    for (u32 pid = 1; pid < 32; pid++) {
        i32 deadline_status = rtos_deadline_check(pid);
        if (deadline_status == RTOS_DEADLINE_MISSED) {
            vga_printf("WARNING: Deadline missed for PID %u\n", pid);
        }
    }
    
    /* Display deadline monitoring status */
    if (check_count % 20 == 0) {
        vga_printf("Deadline Monitor: %u checks completed\n", check_count);
    }
}

/* Performance monitoring task */
void performance_monitor_task(void) {
    static u32 monitor_count = 0;
    monitor_count++;
    
    /* Get CPU utilization */
    u32 cpu_utilization;
    rtos_get_cpu_utilization(&cpu_utilization);
    
    /* Display performance metrics */
    vga_printf("Performance Monitor: CPU Utilization: %u%%\n", cpu_utilization);
    
    /* Check for system overload */
    if (cpu_utilization > 90) {
        vga_puts("WARNING: System CPU utilization high!\n");
    }
}

/* Initialize RTOS demo */
void rtos_demo_init(void) {
    vga_puts("Initializing RTOS Demo Application...\n");
    
    /* Create IPC objects */
    demo_semaphore = sem_create(1);  /* Binary semaphore */
    demo_mutex = rtos_mutex_create(false);  /* Non-recursive mutex */
    demo_event_flags = rtos_event_create();
    
    if (demo_semaphore < 0 || demo_mutex < 0 || demo_event_flags < 0) {
        vga_puts("ERROR: Failed to create IPC objects\n");
        return;
    }
    
    /* Register periodic real-time tasks */
    i32 task1 = rtos_register_periodic_task(high_priority_task, FAST_TASK_PERIOD, HIGH_PRIORITY_TASK);
    i32 task2 = rtos_register_periodic_task(medium_priority_task, MEDIUM_TASK_PERIOD, MEDIUM_PRIORITY_TASK);
    i32 task3 = rtos_register_periodic_task(low_priority_task, SLOW_TASK_PERIOD, LOW_PRIORITY_TASK);
    
    /* Register monitoring tasks */
    i32 deadline_task = rtos_register_periodic_task(deadline_monitor_task, 250, HIGH_PRIORITY_TASK + 5);
    i32 perf_task = rtos_register_periodic_task(performance_monitor_task, 2000, LOW_PRIORITY_TASK - 10);
    
    if (task1 < 0 || task2 < 0 || task3 < 0 || deadline_task < 0 || perf_task < 0) {
        vga_puts("ERROR: Failed to register periodic tasks\n");
        return;
    }
    
    /* Enable preemption for real-time behavior */
    rtos_set_preemption(true);
    
    /* Initialize deadline monitoring */
    rtos_deadline_monitor_init();
    
    /* Enable performance monitoring */
    rtos_performance_monitor_start();
    
    vga_puts("RTOS Demo initialized successfully!\n");
    vga_puts("Real-time tasks are now running...\n\n");
}

/* Main demo loop */
void rtos_demo_run(void) {
    u32 demo_duration = 0;
    const u32 MAX_DEMO_DURATION = 60000;  /* 60 seconds */
    
    vga_puts("Starting RTOS Demo - Press any key to stop\n");
    
    while (demo_running && demo_duration < MAX_DEMO_DURATION) {
        /* Execute periodic tasks */
        rtos_execute_periodic_tasks();
        
        /* Check for user input to stop demo */
        if (keyboard_has_input()) {
            demo_running = false;
            break;
        }
        
        /* Yield to other tasks */
        rtos_yield();
        
        /* Update demo duration */
        rtos_delay_ms(10);
        demo_duration += 10;
    }
    
    vga_puts("\nRTOS Demo completed.\n");
    display_final_statistics();
}

/* Display final demo statistics */
void display_final_statistics(void) {
    vga_puts("\n=== Final RTOS Demo Statistics ===\n");
    
    /* Display task execution counts */
    display_task_statistics();
    
    /* Display timing analysis */
    struct rtos_timing_stats final_stats;
    rtos_get_timing_stats(&final_stats);
    
    vga_printf("Total Context Switches: %u\n", final_stats.context_switches);
    vga_printf("Missed Deadlines: %u\n", final_stats.missed_deadlines);
    
    /* Calculate task execution rates */
    u32 total_time_ms = RTOS_TICKS_TO_MS(final_stats.system_ticks);
    if (total_time_ms > 0) {
        vga_printf("High Priority Task Rate: %.2f Hz\n", 
                   (float)task_counters[0] * 1000.0f / total_time_ms);
        vga_printf("Medium Priority Task Rate: %.2f Hz\n", 
                   (float)task_counters[1] * 1000.0f / total_time_ms);
        vga_printf("Low Priority Task Rate: %.2f Hz\n", 
                   (float)task_counters[2] * 1000.0f / total_time_ms);
    }
    
    vga_puts("==================================\n");
}

/* Cleanup demo resources */
void rtos_demo_cleanup(void) {
    /* Stop performance monitoring */
    rtos_performance_monitor_stop();
    
    /* Destroy IPC objects */
    if (demo_semaphore >= 0) {
        sem_destroy(demo_semaphore);
    }
    if (demo_mutex >= 0) {
        rtos_mutex_destroy(demo_mutex);
    }
    if (demo_event_flags >= 0) {
        rtos_event_destroy(demo_event_flags);
    }
    
    vga_puts("RTOS Demo cleanup completed.\n");
}

/* Main entry point for RTOS demo */
int main(void) {
    vga_puts("Kronos OS - Real-Time Operating System Demo\n");
    vga_puts("==========================================\n\n");
    
    /* Initialize the demo */
    rtos_demo_init();
    
    /* Run the demo */
    rtos_demo_run();
    
    /* Cleanup */
    rtos_demo_cleanup();
    
    return 0;
}
