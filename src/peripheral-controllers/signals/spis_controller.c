#include "spis_controller.h"
#include <nrf5340_application.h>

void spis_init(nrfx_spis_t nrfx_spis, spis_instance spis_inst, uint8_t tx_buffer[], uint8_t buffer_size)
{
    nrfx_err_t status;
    nrfx_spis_config_t spis_config = NRFX_SPIS_DEFAULT_CONFIG(spis_inst.clk_pin,
                            NRF_SPIS_PIN_NOT_CONNECTED,
                            spis_inst.data_pin,
                            spis_inst.csn_pin);

    spis_config.miso_drive = NRF_GPIO_PIN_H0H1;
    spis_config.def = 0x00;
    spis_config.orc = 0x00;
    
    void * p_context = &spis_inst;
    nrf_spis_configure(nrfx_spis.p_reg, NRF_SPIS_MODE_2, NRF_SPIS_BIT_ORDER_MSB_FIRST);
    status = nrfx_spis_init(&nrfx_spis, &spis_config, spis_inst.event_handler, p_context);
    NRFX_ASSERT(status == NRFX_SUCCESS);

    nrfx_spis_buffers_set(&nrfx_spis, tx_buffer, buffer_size, NULL, 0);
    NRFX_ASSERT(status == NRFX_SUCCESS);
}