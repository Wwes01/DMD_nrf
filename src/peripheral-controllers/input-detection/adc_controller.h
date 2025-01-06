#pragma once

#include "nrfx_saadc.h"
#include "../../common.h"

/**
 * @brief Initialise and configure the ADC peripheral.
 *
 * @param[in] buffer    The buffer the ADC writes samples into.
 * @param[in] channel   Configure ADC on this channel.
 * @param[in] ih        Interrupt handler that is called when ADC is completed. 
 *                      If NULL, it runs blocking.
 */
void setup_adc(nrf_saadc_value_t * buffer, const nrfx_saadc_channel_t channel, nrfx_saadc_event_handler_t ih);

void take_sample(nrf_saadc_value_t * buff);