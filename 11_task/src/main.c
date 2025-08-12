#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stdint.h>

#define STACK_SIZE 1024
#define THREAD_A_PRIORITY 5
#define THREAD_B_PRIORITY 5
#define INCREMENTS 1000

static int32_t counter = 0;

static struct k_mutex counter_mutex;

void thread_fn(void *arg1, void *arg2, void *arg3)
{
    for (int i = 0; i < INCREMENTS; i++) {
        k_mutex_lock(&counter_mutex, K_FOREVER);  
        counter++;
        k_mutex_unlock(&counter_mutex);           
    }
}

K_THREAD_STACK_DEFINE(thread_a_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_b_stack, STACK_SIZE);

static struct k_thread thread_a_data;
static struct k_thread thread_b_data;

int main(void)
{
    k_mutex_init(&counter_mutex);

    k_thread_create(&thread_a_data, thread_a_stack,
                    K_THREAD_STACK_SIZEOF(thread_a_stack),
                    thread_fn,
                    NULL, NULL, NULL,
                    THREAD_A_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&thread_b_data, thread_b_stack,
                    K_THREAD_STACK_SIZEOF(thread_b_stack),
                    thread_fn,
                    NULL, NULL, NULL,
                    THREAD_B_PRIORITY, 0, K_NO_WAIT);

    k_msleep(100);

    printk("Final counter value: %d\n", counter);

    return 0;
}
