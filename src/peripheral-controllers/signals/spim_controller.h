#pragma once

#include <nrfx_spim.h>
#include <zephyr/kernel.h>

typedef struct {
    uint8_t data_pin;
    uint8_t clk_pin;
    uint8_t frequency;
    nrfx_spim_evt_handler_t event_handler;
    uint32_t flags;
} spim_instance_config;

#define SPIM_DEFAULT_INSTANCE(_data_pin, _clk_pin, _frequency, _event_handler, _flags)           \
{                                                                                                \
    .data_pin       = _data_pin,                                                                 \
    .clk_pin        = _clk_pin,                                                                  \
    .frequency      = _frequency,                                                                \
    .event_handler  = _event_handler,                                                            \
    .flags          = _flags                                                                     \
}

/**
 * @brief   Initialises SPIM instance, and configures for a transfer.
 * 
 * @param[in] nrfx_spim     The NRFX SPIM instance.
 * @param[in] spim_inst     A SPIM configuration instance.
 * @param[in] tx_buffer     The tx buffer of the SPIM instance.
 * @param[in] buffer_size   The size of the tx buffer.
 */
void spim_init(nrfx_spim_t nrfx_spim, spim_instance_config spim_inst, uint8_t tx_buffer[], uint8_t buffer_size);

/**
 * @brief   Configures a SPIM transfer.
 * 
 * @param[in] nrfx_spim     The NRFX SPIM instance.
 * @param[in] spim_inst     A SPIM configuration instance.
 * @param[in] tx_buffer     The tx buffer of the SPIM instance.
 * @param[in] buffer_size   The size of the tx buffer.
 */
void spim_change_buffer(nrfx_spim_t nrfx_spim, spim_instance_config spim_inst, uint8_t tx_buffer[], uint8_t buffer_size);