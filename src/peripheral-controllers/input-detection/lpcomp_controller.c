#include "lpcomp_controller.h"

volatile bool dmd_on = true;

// Checks if its light or dark
static void lpcomp_event_handler(nrf_lpcomp_event_t event)
{
    dmd_on = !dmd_on;
    NRF_LPCOMP->EVENTS_CROSS = 0;
}


void configure_lpcomp(void)
{
    IRQ_DIRECT_CONNECT(COMP_LPCOMP_IRQn, 1, lpcomp_event_handler, 0);
    nrfx_lpcomp_config_t config = {
        .config = { .reference = NRF_LPCOMP_REF_SUPPLY_4_8,
                    .detection = NRF_LPCOMP_DETECT_CROSS,
                    .hyst = LPCOMP_HYST_HYST_Disabled
                  },
        .input = (nrf_lpcomp_input_t)NRF_LPCOMP_INPUT_0,
        .interrupt_priority = NRFX_LPCOMP_DEFAULT_CONFIG_IRQ_PRIORITY
    };

    int err = nrfx_lpcomp_init(&config, lpcomp_event_handler);
    if (err != NRFX_SUCCESS)
    {
        return;
    }
}