#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Here is a good place to include header files that are required across
your application. */
#include "FreeRTOSLPM.h"

/* Ensure definitions are only used by the compiler, and not by the assembler. */
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
  #include <stdint.h>
  extern uint32_t SystemCoreClock;
#endif

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configUSE_TICKLESS_IDLE                 2
#define configCPU_CLOCK_HZ                      ( SystemCoreClock )
//#define configSYSTICK_CLOCK_HZ                  1000000
#define configTICK_RATE_HZ                      1000
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                256
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   3
#define configUSE_MUTEXES                       0
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           0
#define configUSE_ALTERNATIVE_API               0 /* Deprecated! */
#define configQUEUE_REGISTRY_SIZE               10
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  0
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 2
#define configSTACK_DEPTH_TYPE                  uint16_t
#define configMESSAGE_BUFFER_LENGTH_TYPE        size_t

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   1024*16
#define configAPPLICATION_ALLOCATED_HEAP        0

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         1

/* Software timer related definitions. */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               3
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE

/* Define to trap errors during development. */
#define configASSERT(x) if(!(x)){while(1){};};

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     0
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     0
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_eTaskGetState                   0
#define INCLUDE_xEventGroupSetBitFromISR        1
#define INCLUDE_xTimerPendFunctionCall          0
#define INCLUDE_xTaskAbortDelay                 0
#define INCLUDE_xTaskGetHandle                  0
#define INCLUDE_xTaskResumeFromISR              1

/* CPU Specific defines */
#define configENABLE_FPU                         1

#define USE_FreeRTOS_HEAP_4

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
 /* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
 #define configPRIO_BITS         __NVIC_PRIO_BITS
#else
 #define configPRIO_BITS         4
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   15

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )


/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler    SVC_Handler
#define xPortPendSVHandler PendSV_Handler

#define xPortSysTickHandler SysTick_Handler


// Integrate lptimTick.c -- Start of Block
//
#if ( configUSE_TICKLESS_IDLE == 2 )

  //      Preprocessor code in lptimTick.c requires that configTICK_RATE_HZ be a preprocessor-friendly numeric
  // literal.  As a result, the application *ignores* the CubeMX configuration of the FreeRTOS tick rate.
  //
  #undef  configTICK_RATE_HZ
  #define configTICK_RATE_HZ  1000UL  // <-- Set FreeRTOS tick rate here, not in CubeMX.

  //      Don't bother installing xPortSysTickHandler() into the vector table or including it in the firmware
  // image at all.  FreeRTOS doesn't use the SysTick timer nor xPortSysTickHandler() when lptimTick.c is
  // providing the OS tick.
  //
  #undef  xPortSysTickHandler

#endif // configUSE_TICKLESS_IDLE == 2
//
// Integrate lptimTick.c -- End of Block

#if ( configUSE_TICKLESS_IDLE == 2 )

  //      Without pre- and post-sleep processing, lptimTick.c uses only basic sleep mode during tickless idle.
  // To utilize the stop modes and their dramatic reduction in power consumption, we employ an ultra-low-power
  // driver to handle the pre- and post-sleep hooks.
  //
  #define configPRE_SLEEP_PROCESSING(x)  lpmPreSleepProcessing()
  #define configPOST_SLEEP_PROCESSING(x) lpmPostSleepProcessing()

  //      Make sure the self-tests in our demo application capture tick-timing information as quickly as
  // possible after each tick.  This interrupt priority ensures the OS tick ISR preempts other ISRs.  Without
  // this configuration, interrupt latency causes a little extra jitter during the stress test.  See main.c.
  // To see the effect of interrupt latency in the terminal output, remove or comment out this line.
  //
  // #define configTICK_INTERRUPT_PRIORITY   (configLIBRARY_LOWEST_INTERRUPT_PRIORITY - 1)

  //      The Nucleo board gives us Vdd=3.3V, so we need only 2us minimum run time to work around erratum
  // 2.3.21.  Note that when we define this symbol, code in ulp.c owns the SysTick timer.  At the moment, our
  // demo application doesn't have any relevant ISRs short enough to violate this minimum run time, so we
  // don't enable the work around.  (The shortest possible run time between deep sleeps occurs with a very
  // short ISR that interrupts STOP mode but doesn't interact with FreeRTOS at all.  See lptimTick.c.)
  //
  // #define configMIN_RUN_BETWEEN_DEEP_SLEEPS  ( ( 2U * configCPU_CLOCK_HZ ) / 1000000 )

#endif // configUSE_TICKLESS_IDLE == 2

#endif /* FREERTOS_CONFIG_H */
