/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(button_irq, LOG_LEVEL_DBG);

/* DeviceTree aliases for LED and Button */
#define LED_NODE     DT_ALIAS(led0)
#define BUTTON_NODE  DT_ALIAS(sw0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

static struct gpio_callback button_cb_data;
static bool led_state = false;

/* ISR: Called when button is pressed/released */
static void button_pressed_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	led_state = !led_state;
	gpio_pin_set_dt(&led, led_state);
	LOG_DBG("Interrupt: Button → LED %s", led_state ? "ON" : "OFF");
}

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&button)) {
		LOG_ERR("LED or button not ready");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED");
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Failed to configure button");
		return 0;
	}

	/* Configure button interrupt on rising and falling edge */
	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
	if (ret < 0) {
		LOG_ERR("Failed to configure button interrupt");
		return 0;
	}

	/* Initialize and add the callback */
	gpio_init_callback(&button_cb_data, button_pressed_isr, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);

	LOG_INF("Button interrupt example running");

	while (1) {
		k_sleep(K_FOREVER); /* CPU sleeps until ISR wakes it */
	}
}
