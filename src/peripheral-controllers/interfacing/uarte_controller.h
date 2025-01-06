#pragma once

#include <inttypes.h>

#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>

#include "../../common.h"
#include "../../manchester.h"

#define UART_BUFFER_SIZE 512

extern uint8_t buffer[UART_BUFFER_SIZE];
extern volatile bool messages_paused;

#ifdef USER_INTERFACE
void start_uarte(void);
#endif