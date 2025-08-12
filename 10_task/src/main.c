/*
 * Zephyr: Thread Coordination with Semaphore
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* Define semaphore */
struct k_sem my_sem;

/* Thread stack sizes and priorities */
#define STACK_SIZE 1024
#define PRIORITY 5

/* Declare thread functions */
void thread_a(void *, void *, void *);
void thread_b(void *, void *, void *);
void thread_c(void *, void *, void *);

/* Define thread stacks */
K_THREAD_STACK_DEFINE(thread_a_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_b_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_c_stack, STACK_SIZE);

/* Thread control blocks */
struct k_thread thread_a_data;
struct k_thread thread_b_data;
struct k_thread thread_c_data;

void thread_a(void *arg1, void *arg2, void *arg3) {
    printk("1\n");
    k_sem_take(&my_sem, K_FOREVER);
    printk("4\n");
    k_sem_give(&my_sem);
    k_sem_take(&my_sem, K_FOREVER);
    printk("7\n");
    k_sem_give(&my_sem);
    k_sem_take(&my_sem, K_FOREVER);
}

void thread_b(void *arg1, void *arg2, void *arg3) {
    printk("2\n");
    // k_sleep(K_SECONDS(3));
    k_sem_give(&my_sem);
    k_sem_take(&my_sem, K_FOREVER);
    printk("5\n");
    k_sem_give(&my_sem);
    k_sem_take(&my_sem, K_FOREVER);
    printk("8\n");
    k_sem_give(&my_sem);
    k_sem_take(&my_sem, K_FOREVER);

}

void thread_c(void *arg1, void *arg2, void *arg3) {
    printk("3\n");
    k_sem_give(&my_sem);
    k_sem_take(&my_sem, K_FOREVER);
    printk("6\n");
    k_sem_give(&my_sem);
    k_sem_take(&my_sem, K_FOREVER);
    printk("9\n");
    k_sem_give(&my_sem);
}

void main(void) {
    printk("Main: Initializing semaphore and threads...\n");

    /* Initialize semaphore with count = 0 */
    k_sem_init(&my_sem, 0, 1);

    /* Create threads */
    k_thread_create(&thread_a_data, thread_a_stack, STACK_SIZE,
                    thread_a, NULL, NULL, NULL,
                    PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&thread_b_data, thread_b_stack, STACK_SIZE,
                    thread_b, NULL, NULL, NULL,
                    PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&thread_c_data, thread_c_stack, STACK_SIZE,
                    thread_c, NULL, NULL, NULL,
                    PRIORITY, 0, K_NO_WAIT);

    k_sleep(K_SECONDS(1));
    k_thread_abort(thread_a);

    /* Optional: Reset semaphore */
    k_sem_reset(&my_sem);
    printk("Main: Ending semaphore and threads...\n");
}