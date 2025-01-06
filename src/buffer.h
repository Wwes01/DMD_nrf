#pragma once

#include <inttypes.h>

#define RESET_BUFFER_SIZE 5
#define SCTRL_BUFFER_SIZE 3
#define LOADB_BUFFER_SIZE 3
#define DATA_BUFFER_SIZE 14
#define SAC_B_BUFFER_SIZE 14

extern uint8_t tx_sctrl_buffer[SCTRL_BUFFER_SIZE];
extern uint8_t tx_loadb_buffer[LOADB_BUFFER_SIZE];

extern uint8_t tx_drc_b_0[DATA_BUFFER_SIZE];
extern uint8_t tx_drc_b_1[DATA_BUFFER_SIZE];
extern uint8_t tx_drc_b_buffer[DATA_BUFFER_SIZE];
                                                                                                    
extern uint8_t tx_drc_s_0[DATA_BUFFER_SIZE];
extern uint8_t tx_drc_s_1[DATA_BUFFER_SIZE];
extern uint8_t tx_drc_s_buffer[DATA_BUFFER_SIZE];

extern uint8_t tx_sac_b_buffer[SAC_B_BUFFER_SIZE];

/**
 * @brief Shifts the array to the right. 
 *        1's are shifted in from the left.
 * 
 * @param[in] arr   The array that will be shifted.
 * @param[in] size  The size arr.
 * @param[in] shift The array is shifted by this amount.
 */
void shift_right(uint8_t *arr, int size, int shift);