/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

/* Enable logging for this module at debug level */
LOG_MODULE_REGISTER(led_blink, LOG_LEVEL_DBG);

/* 750 msec delay */
#define SLEEP_TIME_MS   750

/* The devicetree node identifiers for the LEDs */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED3_NODE DT_ALIAS(led3)
#define LED4_NODE DT_ALIAS(led4)

/* GPIO specifications for each LED */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec led4 = GPIO_DT_SPEC_GET(LED4_NODE, gpios);

int main(void)
{
	int ret;
	bool led_state = true;

	/* Verify all LEDs are ready */
	if (!gpio_is_ready_dt(&led0) || !gpio_is_ready_dt(&led1) || !gpio_is_ready_dt(&led3) || !gpio_is_ready_dt(&led4)) {
		LOG_ERR("One or more LEDs not ready");
		return 0;
	}

	/* Configure each LED pin as output */
	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED0");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED1");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED3");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led4, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED4");
		return 0;
	}

	while (1) {
		/* Turn LEDs ON */
		gpio_pin_set_dt(&led0, 1);
		LOG_DBG("LED0 ON");

		gpio_pin_set_dt(&led1, 1);
		LOG_DBG("LED1 ON");

		gpio_pin_set_dt(&led3, 1);
		LOG_DBG("LED3 ON");

		gpio_pin_set_dt(&led4, 1);
		LOG_DBG("LED4 ON");

		k_msleep(SLEEP_TIME_MS);

		/* Turn LEDs OFF */
		gpio_pin_set_dt(&led0, 0);
		LOG_DBG("LED0 OFF");

		gpio_pin_set_dt(&led1, 0);
		LOG_DBG("LED1 OFF");

		gpio_pin_set_dt(&led3, 0);
		LOG_DBG("LED3 OFF");

		gpio_pin_set_dt(&led4, 0);
		LOG_DBG("LED4 OFF");
		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
