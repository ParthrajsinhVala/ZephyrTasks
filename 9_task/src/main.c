/*
 * Copyright (c) 2022 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

/* change this to any other UART peripheral if desired */
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
#define LED0_NODE DT_ALIAS(led0)


static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE , gpios);
/* receive buffer used in UART ISR callback */


/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
uint8_t c;
int flag = 0;
void serial_cb(const struct device *dev, void *user_data)
{
	if (!uart_irq_update(uart_dev)) {
		return;
	}
	if (!uart_irq_rx_ready(uart_dev)) {
		return;
	}
	/* read until FIFO empty */
	uart_fifo_read(uart_dev, &c, 1);
	flag++;
}



int main(void)
{
	if (!device_is_ready(uart_dev)) {
		printk("UART device not found!");
		return 0;
	}
	
	if(!gpio_is_ready_dt(&led))
	{
		return 0;
	}
	
	/* configure interrupt and callback to receive data */
	int ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);

	if (ret < 0) {
		if (ret == -ENOTSUP) {
			printk("Interrupt-driven UART API support not enabled\n");
		} else if (ret == -ENOSYS) {
			printk("UART device does not support interrupt-driven API\n");
		} else {
			printk("Error setting UART callback: %d\n", ret);
		}
		return 0;
	}
	uart_irq_rx_enable(uart_dev);
	
	ret = gpio_pin_configure_dt(&led , GPIO_OUTPUT_ACTIVE);
	if(ret < 0)
	{
		return 0;
	}
	
	printf("1: LED ON \n2:LED OFF \n3:LED TOGGLE\n");
	while(1)
	{
		if( c == '1' && flag == 1)
		{
			gpio_pin_set_dt(&led,1);
			printf("LED ON\n\r");
			flag--;
		}
		else if(c == '2' && flag == 1)
		{
			gpio_pin_set_dt(&led,0);
			printf("LED OFF\n\r");
			flag--;
		}
		else if( c == '3'&& flag == 1)
		{
			gpio_pin_toggle_dt(&led);
			printf("LED TOGGLED\n\r");
			flag--;
		}
		else if( c != '1' && c != '2' && c != '3' && flag == 1)
		{
			printf("Invalid Option \n\r");
			flag--;
		}
		else
		{

		}
	}
	return 0;
}
