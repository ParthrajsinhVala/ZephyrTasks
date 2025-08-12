/*
 * SPDX-License-Identifier: Apache-2.0
 * Task 7: UART TX & RX using polling
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <string.h>

/* Change this to any other UART peripheral if desired */
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)

#define MSG_SIZE 32

static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

/* Simple UART transmit (polling) */
void print_uart(const char *buf)
{
    for (int i = 0; buf[i] != '\0'; i++) {
        uart_poll_out(uart_dev, buf[i]);
    }
}

int main(void)
{
    char rx_buf[MSG_SIZE];
    int rx_pos = 0;
    uint8_t c;

    if (!device_is_ready(uart_dev)) {
        printk("UART device not found!\n");
        return 0;
    }

    /* Startup messages */
    print_uart("Hello! I'm your echo bot (Polling Mode).\r\n");
    print_uart("Tell me something and press enter:\r\n");

    while (1) {
        /* Poll for a character */
        if (uart_poll_in(uart_dev, &c) == 0) {
            if ((c == '\n' || c == '\r') && rx_pos > 0) {
                rx_buf[rx_pos] = '\0'; // terminate string

                /* Echo back */
                print_uart("Echo: ");
                print_uart(rx_buf);
                print_uart("\r\n");

                rx_pos = 0; // reset buffer
            } 
            else if (rx_pos < (MSG_SIZE - 1)) {
                rx_buf[rx_pos++] = c; // store character
            }
            /* else: drop character if buffer full */
        } 
        else {
            k_sleep(K_MSEC(10)); // avoid busy-waiting
        }
    }

    return 0;
}
