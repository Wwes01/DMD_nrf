#include "adc_controller.h"

void setup_adc(nrf_saadc_value_t * buffer, const nrfx_saadc_channel_t channel, nrfx_saadc_event_handler_t ih)
{
    nrfx_err_t status;

	status = nrfx_saadc_init(NRFX_SAADC_DEFAULT_CONFIG_IRQ_PRIORITY);
	NRFX_ASSERT(status == NRFX_SUCCESS);

	status = nrfx_saadc_channel_config(&channel);
	NRFX_ASSERT(status == NRFX_SUCCESS);

	uint32_t channels_mask = nrfx_saadc_channels_configured_get();

	nrfx_saadc_simple_mode_set(channels_mask, NRF_SAADC_RESOLUTION_14BIT, NRF_SAADC_OVERSAMPLE_DISABLED, ih);
	NRFX_ASSERT(status == NRFX_SUCCESS);

	status = nrfx_saadc_buffer_set(buffer, 1);
	NRFX_ASSERT(status == NRFX_SUCCESS);

    status = nrfx_saadc_offset_calibrate(ih);
	NRFX_ASSERT(status == NRFX_SUCCESS);
}

void take_sample(nrf_saadc_value_t* buff) 
{
	nrfx_err_t status;

	status = nrfx_saadc_buffer_set(buff, 1);
	NRFX_ASSERT(status == NRFX_SUCCESS);

    status = nrfx_saadc_offset_calibrate(NULL);
	NRFX_ASSERT(status == NRFX_SUCCESS);

	status = nrfx_saadc_mode_trigger();
    NRFX_ASSERT(status == NRFX_SUCCESS);
}