#include "manchester.h"

char frame[2000];

volatile uint32_t factor_periods[10000];
uint16_t factor_periods_length = 0;
volatile uint16_t current_period_index = 0;
uint32_t messages_send_started = 0;

volatile bool rgb_case = true;


void construct_frame(char * msg, uint8_t len) {

    if (!rgb_case && msg[0] == '#') {
        if(memcmp(msg, "#rgb", 4) == 0) {
            rgb_case = true;
            msg[0] = '#';
            msg[0] = 255;
            msg[0] = 0;
            msg[0] = 0;
        } else {
            msg[0] = '#';
            char* token;
            token = strtok(msg + 1, ", ");
            msg[1] = atoi(token);
            for(int i = 2; i < 4; i++) {
                token = strtok(NULL, ", ");
                msg[i] = atoi(token);
            }
        }
        len = 4;
    }

    memset(frame, 0, sizeof(frame));
    messages_paused = true;
    uint16_t frame_index = 0;

    // Add preamble to frame
    for(int i = 0; i < BITS_PREAMBLE; i+=8) {
        frame[frame_index++] = 0x71;
    }

    // Add length to frame
    uint16_t manchester_length = ook_to_manchester(len);
    frame[frame_index++] = manchester_length >> 8;
    frame[frame_index++] = manchester_length & 0x00FF; 

    // Add message to frame
    uint8_t message_RS[512];
    memset(message_RS, 0, sizeof(message_RS));
    for(int i = 0; i < len; i++) {
        message_RS[i] = msg[i];
    }

    static correct_reed_solomon* rs_code = NULL;
    if (rs_code == NULL) rs_code = correct_reed_solomon_create(correct_rs_primitive_polynomial_8_4_3_2_0, 1, 1, 4);
    int rs_bytes = correct_reed_solomon_encode(rs_code, (const uint8_t*) message_RS , (size_t) len, message_RS);

    // Add message and reed-solomon to frame
    for(int i = 0; i < len + rs_bytes; i++) {
        if(i >= len && message_RS[i] == 0) {
            rs_bytes = i - len;
            break;
        }
        uint16_t manchester_char = ook_to_manchester(message_RS[i]);
        frame[frame_index++] = manchester_char >> 8;
        frame[frame_index++] = manchester_char & 0x00FF; 
    }

    construct_pauses(frame_index);
}

void construct_pauses(uint32_t size_of_frame) 
{
    uint16_t periods_index = 0;
    uint32_t byte_index = 0;
    uint8_t bit_index = 7;
    while(byte_index < size_of_frame) {
        // Extract bit and compare to next bit to decide to send for 1 or 2 periods
        uint8_t extracted_bit = frame[byte_index] & (1 << bit_index);
        uint8_t next_bit = extracted_bit;
        uint8_t periods_counter = 0;
        while(((extracted_bit && next_bit) || !(extracted_bit || next_bit)) 
                && (byte_index < size_of_frame)) {
            if(bit_index == 0) bit_index = 7;
            else bit_index = bit_index - 1;
            byte_index = (byte_index + (bit_index == 7));
            next_bit = (byte_index == size_of_frame)? !extracted_bit : frame[byte_index] & (1 << bit_index);
            periods_counter++;
        }
        if (periods_counter == 0 || periods_counter > 5) {
            printk("Found weird period in a message (period factor %d periods at index %d)\n", periods_counter, periods_index);
        }
        factor_periods[periods_index++] = periods_counter * US_BETWEEN_BITS;
    }
    
    // To make sure it starts with the right symbol
    factor_periods_length = periods_index;
    current_period_index = periods_index;
    messages_send_started = 0;
    factor_periods[factor_periods_length] = US_PULSES_BETWEEN_CONSECUTIVE_MESSAGES;
    messages_paused = false;
}

bool toggle_dmd(bool paused)
{
    static uint32_t count_down_to_next_msg = 0;

    // If there are no messages.
    if(paused) 
    {
        return true;
    }

    // If we are at the end of a frame.
    if(current_period_index < factor_periods_length) 
    {
        current_period_index++;
        return false;
    }

    #ifndef INFINITE
    // If all messages were sent.
    if(messages_send_started >= NUMBER_OF_MESSAGES_TO_SEND)
    {
        messages_send_started = 0;
        messages_paused = true;
        current_period_index = factor_periods_length;
    } else {
    #endif
        // Count down for next message if applicable.
        if(count_down_to_next_msg == 0) 
        {
            if(sending_0) count_down_to_next_msg = NR_PULSES_BETWEEN_CONSECUTIVE_MESSAGES - 1;
            else          count_down_to_next_msg = NR_PULSES_BETWEEN_CONSECUTIVE_MESSAGES;
        } else {
            count_down_to_next_msg--;
            if(count_down_to_next_msg == 0) {
                current_period_index = 0;
                messages_send_started++;
            }
            return false;
        }
    #ifndef INFINITE
    }
    #endif
    return true;
}

uint16_t ook_to_manchester(uint8_t ook)
{
    uint16_t res = 0;

    for(int i = 7; i >= 0; i--) 
    {
        if(ook & (1 << i)) res ^= 1 <<  (2*i);      // The actual   bit
        else               res ^= 1 << ((2*i) + 1); // The opposite bit
    }
    return res;
}