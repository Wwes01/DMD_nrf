#pragma once

#include <nrfx_spis.h>
#include <zephyr/kernel.h>

typedef struct {
    uint8_t data_pin;
    uint8_t clk_pin;
    uint8_t csn_pin;
    nrfx_spis_event_handler_t event_handler;
    nrfx_spis_t * spis_inst;
    uint8_t * tx_buffer;
} spis_instance;

#define SPIS_DEFAULT_INSTANCE(_data_pin, _clk_pin, _csn_pin, _event_handler, _spis_inst, _tx_buffer) \
{                                                                                                \
    .data_pin       = _data_pin,                                                                 \
    .clk_pin        = _clk_pin,                                                                  \
    .csn_pin        = _csn_pin,                                                                  \
    .event_handler  = _event_handler,                                                            \
    .spis_inst      = _spis_inst,                                                                \
    .tx_buffer      = _tx_buffer                                                                 \
}

void spis_init(nrfx_spis_t nrfx_spis, spis_instance spis_inst, uint8_t tx_buffer[], uint8_t buffer_size);