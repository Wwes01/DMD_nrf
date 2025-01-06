#include "spim_controller.h"

void spim_init(nrfx_spim_t nrfx_spim, spim_instance_config spim_inst, uint8_t tx_buffer[], uint8_t buffer_size)
{
    nrfx_err_t status;
    nrfx_spim_config_t spim_config = NRFX_SPIM_DEFAULT_CONFIG(spim_inst.clk_pin, 
                                                              spim_inst.data_pin,
                                                              NRF_SPIM_PIN_NOT_CONNECTED,
                                                              NRF_SPIM_PIN_NOT_CONNECTED);
    spim_config.frequency = NRFX_MHZ_TO_HZ(spim_inst.frequency);
    spim_config.mode = NRF_SPIM_MODE_2;

    status = nrfx_spim_init(&nrfx_spim, &spim_config, spim_inst.event_handler, NULL);
    NRFX_ASSERT(status == NRFX_SUCCESS || status == NRFX_ERROR_INVALID_STATE);
}

void spim_change_buffer(nrfx_spim_t nrfx_spim, spim_instance_config spim_inst, uint8_t tx_buffer[], uint8_t buffer_size)
{
    nrfx_err_t status;
    nrfx_spim_xfer_desc_t spim_xfer_desc = NRFX_SPIM_XFER_TRX(tx_buffer, buffer_size,
                                                              NULL, 0);
    status = nrfx_spim_xfer(&nrfx_spim, &spim_xfer_desc, spim_inst.flags);
    if( status != NRFX_SUCCESS )
    {
        printk("err_code not 0 for nrfx_spim_xfer");
    } 
}