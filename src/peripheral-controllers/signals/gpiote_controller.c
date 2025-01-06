#include "gpiote_controller.h"

uint8_t setup_gpiote_toggle(nrfx_gpiote_pin_t pin, nrf_gpiote_outinit_t state)
{
	nrfx_err_t err;

    // Allocate a GPIOTE channel
    uint8_t gpiote_channel;
    err = nrfx_gpiote_channel_alloc(&gpiote_channel);
    if( err != NRFX_SUCCESS ) printk("err_code not 0 for nrfx_gpiote_channel_alloc");

    // Configure output pin
    const nrfx_gpiote_output_config_t config = {
        .drive = NRF_GPIO_PIN_S0S1, 
        .input_connect = NRF_GPIO_PIN_INPUT_DISCONNECT,
        .pull = NRF_GPIO_PIN_NOPULL
    };

    const nrfx_gpiote_task_config_t task = {
        .task_ch = gpiote_channel, 
        .polarity = NRF_GPIOTE_POLARITY_TOGGLE,
        .init_val = state};

    err = nrfx_gpiote_output_configure(pin,  &config, &task);

    if( err != NRFX_SUCCESS ) printk("err_code not 0 for nrf_drv_gpiote_in_init");

    return gpiote_channel;
}