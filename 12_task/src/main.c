/*
 * Ping-Pong Threads Example in Zephyr
 * Two threads alternate printing "Ping" and "Pong" using semaphores.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stdint.h>

#define STACK_SIZE 1024
#define THREAD_PRIORITY 5
#define NUM_ROUNDS 5   // Number of Ping-Pong exchanges

/* Semaphores for synchronization */
static struct k_sem sem_ping;
static struct k_sem sem_pong;

/* Ping thread */
void ping_thread(void *arg1, void *arg2, void *arg3)
{
    for (int i = 0; i < NUM_ROUNDS; i++) {
        k_sem_take(&sem_ping, K_FOREVER);  // Wait for turn
        printk("Ping\n");
        k_sleep(K_SECONDS(1));             // 1 sec delay
        k_sem_give(&sem_pong);              // Signal Pong thread
    }
}

/* Pong thread */
void pong_thread(void *arg1, void *arg2, void *arg3)
{
    for (int i = 0; i < NUM_ROUNDS; i++) {
        k_sem_take(&sem_pong, K_FOREVER);  // Wait for turn
        printk("Pong\n");
        k_sleep(K_SECONDS(1));              // 1 sec delay
        k_sem_give(&sem_ping);              // Signal Ping thread
    }
}

/* Thread stack and data */
K_THREAD_STACK_DEFINE(ping_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(pong_stack, STACK_SIZE);

static struct k_thread ping_data;
static struct k_thread pong_data;

int main(void)
{
    /* Initialize semaphores */
    k_sem_init(&sem_ping, 1, 1);  // Start with Ping's turn
    k_sem_init(&sem_pong, 0, 1);  // Pong waits initially

    /* Create threads */
    k_thread_create(&ping_data, ping_stack,
                    K_THREAD_STACK_SIZEOF(ping_stack),
                    ping_thread,
                    NULL, NULL, NULL,
                    THREAD_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&pong_data, pong_stack,
                    K_THREAD_STACK_SIZEOF(pong_stack),
                    pong_thread,
                    NULL, NULL, NULL,
                    THREAD_PRIORITY, 0, K_NO_WAIT);

    return 0;
}
