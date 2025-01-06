#pragma once

#include <nrfx_gpiote.h>
#include <helpers/nrfx_gppi.h>
#include <nrfx_dppi.h>
#include <nrfx_spim.h>

#include <nrfx_timer.h>
#include <zephyr/sys/printk.h>

#define ERROR_STATUS_CHECK(error) if(error != NRFX_SUCCESS){ printk("error code not 0! IS = %d", error);}

/**
 * @brief   Allocates a DPPI channel.
 * 
 * @param[out] Channel the bit mask of the channel that is allocated. 
 */
uint8_t allocate_dppi_channel(void);

/**
 * @brief   Links an event and task on a DPPI channel.
 * 
 * @param[in] dppi_channel  The DPPI channel.
 * @param[in] event_addr    The address of the event.
 * @param[in] task_addr     The address of the task.
 */
void setup_dppi(uint8_t dppi_channel, uint32_t event_addr, uint32_t task_addr);

/**
 * @brief   Disables a channel and unlinks a task and event.
 * 
 * @param[in] dppi_channel  The DPPI channel.
 * @param[in] event_addr    The address of the event.
 * @param[in] task_addr     The address of the task.
 */
void disable_dppi(uint8_t dppi_channel, uint32_t event_addr, uint32_t task_addr);