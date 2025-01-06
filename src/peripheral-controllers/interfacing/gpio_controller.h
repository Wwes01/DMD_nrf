#pragma once 

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/**
 * @brief   Setup a GPIO.
 * 
 * @param[in] spec  The gpio_dt_spec of the GPIO.
 * @param[in] flags The flags for the GPIO.
 */
void setup_gpio_dt(const struct gpio_dt_spec *spec, gpio_flags_t flags);

/**
 * @brief   Setup a GPIO.
 * 
 * @param[in] port  The port of the GPIO.
 * @param[in] pin   The pin of the GPIO.
 * @param[in] flags The flags for the GPIO.
 */
void setup_gpio(const struct device *port, gpio_pin_t pin, gpio_flags_t flags);