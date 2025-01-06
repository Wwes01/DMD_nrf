#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "peripheral-controllers/signals/spim_controller.h"
#include "peripheral-controllers/timing/timer_controller.h"

#include "../libcorrect/correct.h"

#include <zephyr/drivers/gpio.h>
#include "peripheral-controllers/input-detection/adc_controller.h"
#include "peripheral-controllers/interfacing/uarte_controller.h"

extern char frame[2000];
extern volatile uint16_t current_period_index;
extern volatile uint32_t factor_periods[10000];
extern volatile bool rgb_case;

/**
 * @brief Constructs the Manchester frame.
 */
void construct_frame(char * msg, uint8_t len);

/**
 * @brief Selects next compare value for Timer.
 * 
 * @param[in] paused True if there are no messages to be sent, false otherwise.
 * 
 * @param[out] end returns whether the end of the frame has been reached.
 */
bool toggle_dmd(bool paused);

/**
 * @brief Translates a byte of OOK to two bytes of Manchester.
 * 
 * @param[in] ook The byte of OOK.
 *
 * @param[out] manchester The two bytes of Manchester.
 */
uint16_t ook_to_manchester(uint8_t ook);

/**
 * @brief Increments bit_index and byte_index of the current message that is being sent.
 *        0 --> 1 0
 *        1 --> 0 1
 */
void construct_pauses(uint32_t size_of_frame);