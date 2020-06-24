#include <assert.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"


#define BLINK_TASK_PRI         (99)
#define BLINK_STACK_SIZE       (1024)
struct os_task blink_task;
os_stack_t blink_task_stack[BLINK_STACK_SIZE];

void blink_task_fn(void *arg) {
    uint8_t count = 0;

    hal_gpio_init_out(LED1_PIN, 0);
    hal_gpio_init_out(LED2_PIN, 0);
 
    while(1) {
        os_time_delay(OS_TICKS_PER_SEC/4);
        if (count & 1) {
            hal_gpio_toggle(LED1_PIN);
        }
        
        if (count & 2) {
            hal_gpio_toggle(LED2_PIN);
        }

        console_printf("%d\n", count);
        count++;
    }

}

int
main(int argc, char **argv)
{
    sysinit();

    os_task_init(
        &blink_task,
        "blink_task",
        blink_task_fn,
        NULL,
        BLINK_TASK_PRI,
        OS_WAIT_FOREVER,
        blink_task_stack,
        BLINK_STACK_SIZE);

    while(1) {
        os_eventq_run(os_eventq_dflt_get());
    }

    assert(0);

    return 0;
}

