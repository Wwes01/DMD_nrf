#pragma once

#include <nrfx_timer.h>
#include <zephyr/sys/printk.h>

#define ERROR_STATUS_CHECK(error) if(error != NRFX_SUCCESS){ printk("error code not 0! IS = %d", error);}

typedef struct {
    nrfx_timer_t timer;
    nrf_timer_cc_channel_t cc_channel;
    nrf_timer_short_mask_t timer_mask;
    nrf_timer_event_t timer_event;
    
} timer_instance;

#define TIMER_DEFAULT_INSTANCE(_timer, _cc_channel, _timer_mask, _timer_event)                    \
{                                                                                                \
    .timer       = _timer,                                                                       \
    .cc_channel  = _cc_channel,                                                                  \
    .timer_mask  = _timer_mask,                                                                  \
    .timer_event = _timer_event                                                                  \
}

/**
 * @brief   Initialises a Timer instance.
 * 
 * @param[in] timer     The NRFX Timer instance that will be initialised. 
 * @param[in] ih        The interrupt handler of the Timer.
 * @param[in] frequency The frequency the Timer will be initialised to.
 * 
 * @param[out]
 */
nrfx_timer_t init_timer(nrfx_timer_t timer, nrfx_timer_event_handler_t ih, uint32_t frequency);

/**
 * @brief   Sets a new interval after which the callback function is called.
 * 
 * @param[in] timer_inst    The configuration of the Timer. 
 * @param[in] desired_tick  The ticks after which the interrupt handler is called.
 * @param[in] ih            If true, interrupt handler is called after Timer triggers. 
 *                          If false, no interrupt handler is called.  
 */
void set_timer(timer_instance timer_inst, uint32_t desired_ticks, bool ih);
