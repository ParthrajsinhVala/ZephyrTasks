/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(button_poll, LOG_LEVEL_DBG);

/* Sleep time for polling loop */
#define POLL_INTERVAL_MS 50

/* DeviceTree aliases for LED and Button */
#define LED_NODE     DT_ALIAS(led0)
#define BUTTON_NODE  DT_ALIAS(sw0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

int main(void)
{
	int ret;
	bool led_state = false;

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

	LOG_INF("Polling button...");

	while (1) {
		int val = gpio_pin_get_dt(&button);
		if (val > 0) { /* Button pressed */
			led_state = !led_state;
			gpio_pin_set_dt(&led, led_state);
			LOG_DBG("Button pressed LED %s", led_state ? "ON" : "OFF");

			/* Debounce delay */
			k_msleep(200);
		}

		k_msleep(POLL_INTERVAL_MS);
	}
}
