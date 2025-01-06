#include "timer_controller.h"

nrfx_timer_t init_timer(nrfx_timer_t timer, nrfx_timer_event_handler_t ih, uint32_t frequency)
{
	nrfx_err_t err;
	nrfx_timer_config_t timer_cfg = NRFX_TIMER_DEFAULT_CONFIG(frequency);
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    
    err = nrfx_timer_init(&timer, &timer_cfg, ih);
	ERROR_STATUS_CHECK(err);
    return timer;	
}

void set_timer(timer_instance timer_inst, uint32_t desired_ticks, bool ih) 
{
    if(nrfx_timer_is_enabled(&timer_inst.timer)) nrfx_timer_disable(&timer_inst.timer);
    
    nrfx_timer_clear(&timer_inst.timer);
    nrfx_timer_extended_compare(&timer_inst.timer, timer_inst.cc_channel, desired_ticks, timer_inst.timer_mask, ih);   
}