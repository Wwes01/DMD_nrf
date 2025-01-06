#include "dppi_controller.h"

uint8_t allocate_dppi_channel(void)
{
    nrfx_err_t err;
    uint8_t dppi_channel;

    err =  nrfx_dppi_channel_alloc(&dppi_channel);
    if( err != NRFX_SUCCESS ) printk("err_code not 0 for nrfx_dppi_channel_alloc");
    return dppi_channel;
}

void setup_dppi(uint8_t dppi_channel, uint32_t event_addr, uint32_t task_addr)
{
    nrfx_gppi_channel_endpoints_setup(dppi_channel, event_addr, task_addr);
    nrfx_gppi_channels_enable(BIT(dppi_channel));
}

void disable_dppi(uint8_t dppi_channel, uint32_t event_addr, uint32_t task_addr)
{
    nrfx_gppi_channel_endpoints_clear(dppi_channel, event_addr, task_addr);
    nrfx_gppi_channels_disable(BIT(dppi_channel));
    nrfx_gppi_channel_free(BIT(dppi_channel));
}
