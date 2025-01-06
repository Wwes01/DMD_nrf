#include "uarte_controller.h"

uint8_t buffer[UART_BUFFER_SIZE];
volatile bool messages_paused = true;

#ifdef USER_INTERFACE
static void interrupt_handler(const struct device *dev, void *user_data)
{
	ARG_UNUSED(user_data);

	while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {
		if (uart_irq_rx_ready(dev)) {
			int rx_len;

			rx_len = uart_fifo_read(dev, buffer, UART_BUFFER_SIZE);
			if (rx_len < 0) {
				printk("Failed to read UART FIFO");
				rx_len = 0;
			};
            while(buffer[rx_len-1] == '\n' || buffer[rx_len-1] == '\r') buffer[--rx_len] = '\0';

            construct_frame(buffer, rx_len);
            memset(buffer, 0, UART_BUFFER_SIZE);
			messages_paused = false;
		}
	}
}

void start_uarte(void)
{
	const struct device *dev;
	uint32_t baudrate, dtr = 0U;
	int ret;

	dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);
	if (!device_is_ready(dev)) {
		printk("CDC ACM device not ready");
		return;
	}


	ret = usb_enable(NULL);
	if (ret != 0) {
		printk("Failed to enable USB");
		return;
	}

	printk("Wait for DTR");

	while (true) {
		ret = uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		if (dtr) {
			break;
		} else {
			/* Give CPU resources to low priority threads. */
			k_sleep(K_MSEC(100));
		}
	}

	printk("DTR set");

	/* They are optional, we use them to test the interrupt endpoint */
	ret = uart_line_ctrl_set(dev, UART_LINE_CTRL_DCD, 1);
	if (ret) {
		printk("Failed to set DCD, ret code %d", ret);
	}

	ret = uart_line_ctrl_set(dev, UART_LINE_CTRL_DSR, 1);
	if (ret) {
		printk("Failed to set DSR, ret code %d", ret);
	}

	/* Wait 100ms for the host to do all settings */
	k_msleep(100);

	ret = uart_line_ctrl_get(dev, UART_LINE_CTRL_BAUD_RATE, &baudrate);
	if (ret) {
		printk("Failed to get baudrate, ret code %d", ret);
	} else {
		printk("Baudrate detected: %d", baudrate);
	}

	uart_irq_callback_set(dev, interrupt_handler);

	/* Enable rx interrupts */
	uart_irq_rx_enable(dev);
}

#endif