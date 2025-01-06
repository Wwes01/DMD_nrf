#pragma once

#include <nrfx_gpiote.h>
#include <zephyr/sys/printk.h>

#define GPIOTE_INTERRUPT_PRIORITY 0

/**
 * @brief   Allocates a GPIOTE channel, and sets up a pin with the toggle task.
 * 
 * @param[in]  pin   The pin to which the toggle task needs to be configured.
 * @param[out] state    The state of the pin at start.
 */
uint8_t setup_gpiote_toggle(nrfx_gpiote_pin_t pin, nrf_gpiote_outinit_t state);