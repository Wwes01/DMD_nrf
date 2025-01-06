#pragma once

#include <hal/nrf_comp.h>
#include <hal/nrf_lpcomp.h>
#include <drivers/include/nrfx_lpcomp.h>

extern volatile bool dmd_on;

void configure_lpcomp(void);