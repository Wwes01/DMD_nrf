#include "gpio_controller.h"

void setup_gpio_dt(const struct gpio_dt_spec *spec, gpio_flags_t flags)
{
    int ret;
    if (!gpio_is_ready_dt(spec)) {
        printk("Device was not ready");
        return;
    }

    ret = gpio_pin_configure_dt(spec, flags);
    if(ret < 0) printk("failed to configure led");
}

void setup_gpio(const struct device *port, gpio_pin_t pin, gpio_flags_t flags)
{
    int ret = gpio_pin_configure(port, pin, flags);
    if(ret < 0) printk("failed to configure led");
}