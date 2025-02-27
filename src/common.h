#pragma once 

#include <inttypes.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <nrfx_clock.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <nrfx_gpiote.h>

#include "peripheral-controllers/timing/timer_controller.h"
#include "buffer.h"

// USER_INTERFACE: enables the use of the nRF Connect Serial Terminal through UART.
// #define USER_INTERFACE

// INFINITE: the same message is sent on repeat, no pause or waiting for new input.
#define INFINITE

// Number of messages to send
// #define NUMBER_OF_MESSAGES_TO_SEND 5000

// HIGH_SPEED: for speeds that require a higher clock rate.
// #define HIGH_SPEED

#define US_BETWEEN_BITS 500

#define NR_PULSES_BETWEEN_CONSECUTIVE_MESSAGES 6 // ! Needs to be even !
#define US_PULSES_BETWEEN_CONSECUTIVE_MESSAGES (2*US_BETWEEN_BITS)


#define BITS_PREAMBLE 8
#define BITS_LENGTH 8
#define BYTES_RS 17
#define BYTES_FRAME (BITS_PREAMBLE / 8) + (BITS_LENGTH / 8) + BYTES_RS
#define MANCHESTER_BYTES_FRAME ((BYTES_FRAME - (BITS_PREAMBLE / 8)) * 2) + (BITS_PREAMBLE / 8)

extern volatile bool sending_0;
extern char message[];
extern uint8_t length_message;

/**
 * @brief   Swaps out the part of the buffers that sends signals to the DMD, to toggle the DMD.
 *          Also toggles LED if necessary.
 * 
 * @param[in] led_pin     The pin of the LED.
 */
void toggle_sent_signal(nrfx_gpiote_pin_t led_pin);

/**
 * @brief   Setup a button with interrupt handler and gpio callback.
 * 
 * @param[in] button    The gpio_dt_spec of the button that needs to be configured.
 * @param[in] ih        The interrupt handler that is called when the button is pressed.
 * @param[in] cb        The gpio callback for the button.
 */
void setup_button(struct gpio_dt_spec *button, gpio_callback_handler_t ih, struct gpio_callback *cb);