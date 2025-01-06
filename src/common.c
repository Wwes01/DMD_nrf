#include "common.h"

volatile bool sending_0 = true;

void toggle_sent_signal(nrfx_gpiote_pin_t led_pin)
{
    if (sending_0)
    {
        if(nrfx_gpiote_in_is_set(led_pin)) nrfx_gpiote_clr_task_trigger(led_pin);
        sending_0 = false;
        tx_drc_b_buffer[3] = tx_drc_b_0[3];
        tx_drc_b_buffer[6] = tx_drc_b_0[6];
        tx_drc_b_buffer[8] = tx_drc_b_0[8];
    } 
    else 
    {
        if(!nrfx_gpiote_in_is_set(led_pin)) nrfx_gpiote_set_task_trigger(led_pin);

        sending_0 = true;
        tx_drc_b_buffer[3] = tx_drc_b_1[3];
        tx_drc_b_buffer[6] = tx_drc_b_1[6];
        tx_drc_b_buffer[8] = tx_drc_b_1[8];
    }
}

void setup_button(struct gpio_dt_spec *button, gpio_callback_handler_t ih, struct gpio_callback *cb)
{   
    int ret = gpio_pin_interrupt_configure_dt(button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error failed to configure button interrupt");
		return;
	}

	gpio_init_callback(cb, ih, BIT(button->pin));
	gpio_add_callback(button->port, cb);
}