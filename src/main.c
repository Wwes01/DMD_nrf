#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/drivers/gpio.h>
#include <nrfx_clock.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>
#include <nrf5340_application.h>

#include "peripheral-controllers/signals/spim_controller.h"
#include "peripheral-controllers/signals/spis_controller.h"
#include "peripheral-controllers/timing/timer_controller.h"
#include "peripheral-controllers/interfacing/gpio_controller.h"
#include "peripheral-controllers/signals/gpiote_controller.h"
#include "peripheral-controllers/timing/dppi_controller.h"
#include "manchester.h"
#include "peripheral-controllers/input-detection/adc_controller.h"
#include "peripheral-controllers/interfacing/uarte_controller.h"
#include "peripheral-controllers/input-detection/lpcomp_controller.h"

// Message setup
char message[] = "Hello World!";
uint8_t length_message = 12;

// char message[] = "Testing right now!";
// uint8_t length_message = 18;

// #define LED

// Speed configuration
#define SINGLE_SPEED 8
#ifdef HIGH_SPEED
#define DOUBLE_SPEED 16
#else
#define DOUBLE_SPEED 8
#endif

// GPIO Pin assignments
#define LED_CTRL 30
#define CSN_PIN  6

#define SCTRL_PIN_CLK  8
#define SCTRL_PIN 9

#define DRC_S_PIN_CLK  25
#define DRC_S_CSN_PIN  26
#define DRC_S_PIN 12

#define Shutdown1 28

// Function prototypes for handling timers and SPI. Also the functions to turn off/on the LED. 
static void timer_handler(nrf_timer_event_t event_type, void * p_context);
static void spis_handler(nrfx_spis_evt_t const * p_event, void * p_context);
void turn_off_DMD(void);
void turn_on_DMD(void);
void turn_off_LED(void);
void turn_on_LED(void);
void turn_DMD_fully_off(void);

const nrfx_timer_t timer1 = NRFX_TIMER_INSTANCE(1);

timer_instance timer_trigger_dmd_toggle = TIMER_DEFAULT_INSTANCE(NRFX_TIMER_INSTANCE(1), NRF_TIMER_CC_CHANNEL0, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, NRF_TIMER_EVENT_COMPARE0);

nrfx_spis_t spis1 = NRFX_SPIS_INSTANCE(1);
spis_instance spis_drc_s = SPIS_DEFAULT_INSTANCE(DRC_S_PIN, DRC_S_PIN_CLK, DRC_S_CSN_PIN, spis_handler, &spis1, tx_sac_b_buffer);

nrfx_spim_t spim_sctrl = NRFX_SPIM_INSTANCE(4);
spim_instance_config spim_sctrl_config = SPIM_DEFAULT_INSTANCE(SCTRL_PIN, SCTRL_PIN_CLK, DOUBLE_SPEED, NULL, NRFX_SPIM_FLAG_REPEATED_XFER | NRFX_SPIM_FLAG_NO_XFER_EVT_HANDLER | NRFX_SPIM_FLAG_HOLD_XFER);

uint8_t dppi_channel_spi;
uint8_t led_channel; 
uint8_t loadb_channel;
uint8_t dppi_loadb_channel;

// Flags to manage LED and DMD states
bool led_paused = true;
volatile bool dmd_paused = true;

bool end_of_message = false;

#define SW0_NODE	DT_ALIAS(sw0)
static struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback cb;

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

#define LED1_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);

#define LED2_NODE DT_ALIAS(led2)
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);




/**
 * Interrupt handler of SPIS instance.
 * Toggles buffers for DMD, and sets SPIS buffer.
 */
static void spis_handler(nrfx_spis_evt_t const * p_event, void * p_context)
{
    if(p_event->evt_type == NRFX_SPIS_XFER_DONE) {
        spis_instance* temp = p_context;
        
        toggle_sent_signal(LED_CTRL);
        nrfx_spis_buffers_set(temp->spis_inst, temp->tx_buffer, SAC_B_BUFFER_SIZE, NULL, 0);
    }
}


/**
 * Interrupt Handler of Timer instance.
 * Sets new compare value for Timer, tests if DMD needs to be turned on/off, 
 * and selects new compare value.
*/
static void timer_handler(nrf_timer_event_t event_type, void * p_context)
{
    if(event_type == NRF_TIMER_EVENT_COMPARE0)  
    {
        nrfx_timer_extended_compare(&timer_trigger_dmd_toggle.timer, NRF_TIMER_CC_CHANNEL0, factor_periods[current_period_index], NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);   

        // Messag paused is for when its too dark
        // if((end_of_message || messages_paused)) {
        //     if (dmd_on && dmd_paused) {
        //         turn_on_DMD();
        //     } else if(!dmd_on && !dmd_paused) {
        //         turn_off_DMD();
        //     } 
        // }
        end_of_message = toggle_dmd(messages_paused);
    }
}

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    messages_paused = false;  
}

/**
 * Turns off all peripherals that are used to send signals to DMD.
 * Can occur to pause in between messages, or because PD senses too little light.
*/
void turn_off_DMD(void)
{
    if(dmd_paused) return;
    dmd_paused = true;

    nrfx_timer_pause(&timer_trigger_dmd_toggle.timer);
    nrfx_timer_disable(&timer_trigger_dmd_toggle.timer);

    nrfx_gpiote_out_task_disable(CSN_PIN);

    // //! DRC_STROBE - SPIM4 + DPPI + TIMER
    nrfx_spim_abort(&spim_sctrl);
    nrfx_gppi_channel_endpoints_clear(dppi_loadb_channel, nrfx_spim_start_task_address_get(&spim_sctrl), nrfx_timer_event_address_get(&timer1, NRF_TIMER_EVENT_COMPARE0));

    #ifdef LED
    if(led_paused)
    {
        //! Disconnect LED from DRC_B or DRC_S and disable GPIOTE task, turn off timer
        nrfx_gpiote_out_task_disable(LED_CTRL);
        nrfx_gppi_channel_endpoints_clear(dppi_channel_spi, nrfx_gpiote_set_task_address_get(CSN_PIN), nrfx_spim_end_event_address_get(&spim_sctrl));
        disable_dppi(dppi_channel_spi, nrfx_gpiote_out_task_address_get(LED_CTRL), nrfx_spim_start_task_address_get(&spim_sctrl));
        disable_dppi(dppi_loadb_channel, nrfx_gpiote_clr_task_address_get(CSN_PIN), nrfx_spim_start_task_address_get(&spim_sctrl));
    }
    else
    {
        //! Disconnect LED from DRC_S or DRC_B and connect it to timer, leave timer on
        nrfx_gppi_channel_endpoints_clear(dppi_loadb_channel, nrfx_gpiote_clr_task_address_get(CSN_PIN), nrfx_spim_start_task_address_get(&spim_sctrl));
        nrfx_gppi_channel_endpoints_clear(dppi_channel_spi, nrfx_gpiote_out_task_address_get(LED_CTRL), nrfx_spim_start_task_address_get(&spim_sctrl));
        disable_dppi(dppi_channel_spi, nrfx_gpiote_set_task_address_get(CSN_PIN), nrfx_spim_end_event_address_get(&spim_sctrl));
        setup_dppi(dppi_loadb_channel, nrfx_gpiote_out_task_address_get(LED_CTRL), nrfx_timer_event_address_get(&timer1, NRF_TIMER_EVENT_COMPARE0));

        if(!nrfx_timer_is_enabled(&timer1)) nrfx_timer_enable(&timer1);
    } 
    #else
    disable_dppi(dppi_channel_spi, nrfx_gpiote_set_task_address_get(CSN_PIN), nrfx_spim_end_event_address_get(&spim_sctrl));
    disable_dppi(dppi_loadb_channel, nrfx_gpiote_clr_task_address_get(CSN_PIN), nrfx_spim_start_task_address_get(&spim_sctrl));
    #endif
}

/**
 * Turns on all peripherals that are used to send signals to DMD.
*/
void turn_on_DMD(void)
{
    if(!dmd_paused) return;
    dmd_paused = false;

    #ifdef LED
    if(led_paused) 
    {
        //! connect LED to end of DRC_B or DRC_S 
        nrfx_gpiote_out_task_enable(LED_CTRL);
    }
    else 
    {
        //! Disconnect LED from Timer, connect to DRC_B or DRC_S instead
        nrfx_gppi_channel_endpoints_clear(dppi_loadb_channel, nrfx_gpiote_out_task_address_get(LED_CTRL),  nrfx_timer_event_address_get(&timer1, NRF_TIMER_EVENT_COMPARE0));
    }
    #endif

    set_timer(timer_trigger_dmd_toggle, US_BETWEEN_BITS, true);

    //! DRC_STROBE - SPIM4 + DPPI + TIMER
    spim_change_buffer(spim_sctrl, spim_sctrl_config, tx_drc_b_buffer, DATA_BUFFER_SIZE);

    setup_dppi(dppi_loadb_channel, nrfx_spim_start_task_address_get(&spim_sctrl), nrfx_timer_event_address_get(&timer1, NRF_TIMER_EVENT_COMPARE0));

    setup_dppi(dppi_loadb_channel, nrfx_gpiote_clr_task_address_get(CSN_PIN), nrfx_spim_start_task_address_get(&spim_sctrl));
    setup_dppi(dppi_channel_spi, nrfx_gpiote_set_task_address_get(CSN_PIN), nrfx_spim_end_event_address_get(&spim_sctrl));

    //! DRC_BUS - SPIS + DPPI + GPIOTE
    nrfx_gpiote_out_task_enable(CSN_PIN);

    #ifdef LED
    setup_dppi(dppi_channel_spi, nrfx_gpiote_out_task_address_get(LED_CTRL), nrfx_spim_end_event_address_get(&spim_sctrl));
    #endif

    if(!nrfx_timer_is_enabled(&timer1)) nrfx_timer_enable(&timer1);
}


#ifdef LED
void turn_off_LED(void)
{
    if(!dmd_on) 
    {
        nrfx_timer_pause(&timer1);
        nrfx_timer_disable(&timer1);

        nrfx_gpiote_out_task_disable(LED_CTRL);
        disable_dppi(dppi_loadb_channel, nrfx_gpiote_out_task_address_get(LED_CTRL), nrfx_spim_start_task_address_get(&spim_sctrl));
        nrfx_gpiote_channel_free(dppi_loadb_channel);
    }
}

void turn_on_LED(void)
{
    if(!dmd_on) 
    {
        nrfx_gpiote_out_task_enable(LED_CTRL);
        setup_dppi(dppi_loadb_channel, nrfx_gpiote_out_task_address_get(LED_CTRL), nrfx_timer_event_address_get(&timer1, NRF_TIMER_EVENT_COMPARE0));
        if(!nrfx_timer_is_enabled(&timer1)) nrfx_timer_enable(&timer1);
    }
}
#endif

void turn_DMD_fully_off(void)
{
    nrfx_gppi_channel_endpoints_clear(dppi_loadb_channel, nrfx_spim_start_task_address_get(&spim_sctrl), nrfx_timer_event_address_get(&timer1, NRF_TIMER_EVENT_COMPARE0));
    disable_dppi(dppi_channel_spi, nrfx_gpiote_set_task_address_get(CSN_PIN), nrfx_spim_end_event_address_get(&spim_sctrl));
    disable_dppi(dppi_loadb_channel, nrfx_gpiote_clr_task_address_get(CSN_PIN), nrfx_spim_start_task_address_get(&spim_sctrl));

    nrfx_timer_pause(&timer_trigger_dmd_toggle.timer);
    nrfx_timer_disable(&timer_trigger_dmd_toggle.timer);
    nrfx_timer_uninit(&timer_trigger_dmd_toggle.timer);

    nrfx_spim_abort(&spim_sctrl);
    nrfx_spim_uninit(&spim_sctrl);
    nrfx_spis_uninit(&spis1);

    nrfx_gpiote_out_task_disable(CSN_PIN);
    nrfx_gpiote_pin_uninit(CSN_PIN);
    nrfx_gpiote_uninit();
}

int main(void)
{
    // //! Setup for the shutdown pin to for the correct start of the DMD

    int ret;

	if (!gpio_is_ready_dt(&led1)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

    if (!gpio_is_ready_dt(&led2)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

    if (!gpio_is_ready_dt(&led3)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}


    // Toggle the GPIO pin in a loop
    // while (1) {
    //     ret = gpio_pin_toggle_dt(&led1);
	// 	if (ret < 0) {
	// 		return 0;
	// 	}

    //     ret = gpio_pin_toggle_dt(&led2);
	// 	if (ret < 0) {
	// 		return 0;
	// 	}

    //     ret = gpio_pin_toggle_dt(&led3);
	// 	if (ret < 0) {
	// 		return 0;
	// 	}

    //     k_msleep(100);  // 1-second delay
    // }

    
    //! Set clock to high frequency
    #ifdef HIGH_SPEED
    nrfx_clock_divider_set(NRF_CLOCK_DOMAIN_HFCLK, NRF_CLOCK_HFCLK_DIV_1);
    #endif

    #ifdef USER_INTERFACE
    //! Start User Interface
    start_uarte();
    #endif

    //! Construct frame, and shift buffers
    construct_frame(message, length_message);
    shift_right(tx_sac_b_buffer, SAC_B_BUFFER_SIZE, 2);

    //! Setup button
    setup_gpio_dt(&button, GPIO_INPUT);
    setup_button(&button, button_pressed, &cb);
    
    // configure_lpcomp();
    // ! Enable peripheral interrupts
    #if defined(__ZEPHYR__)
        IRQ_DIRECT_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_SPIS_INST_GET(1)), IRQ_PRIO_LOWEST,
                    NRFX_SPIS_INST_HANDLER_GET(1), 0);
        IRQ_DIRECT_CONNECT(NRFX_IRQ_NUMBER_GET(NRF_TIMER_INST_GET(1)), IRQ_PRIO_LOWEST,
                    NRFX_TIMER_INST_HANDLER_GET(1), 0);       
    #endif

    //! Initialise peripherals
    nrfx_gpiote_init(GPIOTE_INTERRUPT_PRIORITY);
    
    //! TRIGGER - Set up trigger for US_BETWEEN_BITS clock pulses
    init_timer(timer1, timer_handler, 1000000);
    set_timer(timer_trigger_dmd_toggle, US_BETWEEN_BITS, true); //TODO: test if i can remove it

    //! SCTRL - SPIM4
    spim_init(spim_sctrl, spim_sctrl_config, tx_drc_b_buffer, DATA_BUFFER_SIZE);

    setup_gpiote_toggle(CSN_PIN, NRF_GPIOTE_INITIAL_VALUE_HIGH);
    nrfx_gpiote_out_task_trigger(CSN_PIN);

    #ifdef LED
    led_channel = setup_gpiote_toggle(LED_CTRL, NRF_GPIOTE_INITIAL_VALUE_HIGH);
    #endif


    // nrfx_lpcomp_enable();
    spis_init(spis1,  spis_drc_s, tx_sac_b_buffer, SAC_B_BUFFER_SIZE);
    dppi_loadb_channel = allocate_dppi_channel();
    dppi_channel_spi = allocate_dppi_channel();
    if(dmd_on) turn_on_DMD();
    #ifdef LED
    turn_on_LED();
    led_paused = false;
    #endif    

    k_msleep(1000);
    // messages_paused = false;
    char rgb_c[4];
    rgb_c[0] = '#';
    rgb_c[1] = 254;
    rgb_c[2] = 0;
    rgb_c[3] = 0;

    while(!rgb_case) {
        k_msleep(2000);
    }
    printk("Hello RGB!\n");

    while (true) {
        while(rgb_c[1] > 0) {
            while(!messages_paused){k_usleep(100);}
            construct_frame(rgb_c, 4);
            rgb_c[1] -= 2;
            rgb_c[2] += 2;
        }
        while(rgb_c[2] > 0) {
            while(!messages_paused){k_usleep(100);}
            construct_frame(rgb_c, 4);
            rgb_c[2] -= 2;
            rgb_c[3] += 2;
        }
        while(rgb_c[3] > 0) {
            while(!messages_paused){k_usleep(100);}
            construct_frame(rgb_c, 4);
            rgb_c[3] -= 2;
            rgb_c[1] += 2;
        }
    }

    return 0;
}