// Tower Kit documentation https://tower.hardwario.com/
// SDK API description https://sdk.hardwario.com/
// Forum https://forum.hardwario.com/

#include <application.h>

// LED instance
twr_led_t led;

twr_scheduler_task_id_t lcd_print_task;
twr_scheduler_task_id_t tester_task;

test_state state = TEST_I2C_0;

// Proggress of the whole test sequence
int test_progress = 0;

twr_button_t button_left;
twr_button_t button_right;

twr_button_t button;

twr_tick_t next_pub;

twr_chester_x3_t x3;

// Initial state for the tester function
//test_state state = TEST_EXPANDER;

twr_scheduler_task_id_t lcd_print_task;
twr_scheduler_task_id_t tester_task;

twr_led_t lcdLedRed;
twr_led_t lcdLedGreen;
twr_led_t lcdLedBlue;

// Results of each test
bool test_results[2];

void tester();
void lcd_print_results();

// Task to print the results onto LCD
void lcd_print_results()
{
    if(!twr_module_lcd_is_ready())
    {
        twr_scheduler_plan_current_relative(20);
        return;
    }

    twr_system_pll_enable();

    twr_module_lcd_clear();

    twr_module_lcd_set_font(&twr_font_ubuntu_15);
    twr_module_lcd_draw_string(0, 5, "CHESTER TESTER X3", 1);
    twr_module_lcd_draw_string(0, 15, "--------------------------", 1);

    if(test_progress == 0)
    {
        twr_module_lcd_set_font(&twr_font_ubuntu_13);
        twr_module_lcd_draw_string(15, 43, "Insert chester X3", 1);
        twr_module_lcd_draw_string(40, 58, "and hold", 1);
        twr_module_lcd_draw_string(8, 73, "right button for X3A", 1);
        //twr_module_lcd_draw_string(12, 88, "left button for X0B", 1);

    }

    twr_module_lcd_set_font(&twr_font_ubuntu_13);

    if(test_progress > 0)
    {
        if(test_results[0])
        {
            twr_module_lcd_draw_string(0, 25, "I2C test 0: OK", 1);
        }
        else
        {
            twr_module_lcd_draw_string(3, 25, "I2C test 0: FAILED", 1);
        }
    }

    if(test_progress > 1)
    {
        if(test_results[1])
        {
            twr_module_lcd_draw_string(0, 35, "I2C test 1: OK", 1);
        }
        else
        {
            twr_module_lcd_draw_string(0, 35, "I2C test 1: FAILED", 1);
        }
    }

    if(test_progress > 2)
    {
        if(test_results[2])
        {
            twr_module_lcd_draw_string(0, 45, "DRDY 0 test: OK", 1);
        }
        else
        {
            twr_module_lcd_draw_string(0, 45, "DRDY 0 test: FAILED", 1);
        }
    }

    if(test_progress > 3)
    {
        twr_ads122c04_reset(&x3.ads122c04_1);
        twr_ads122c04_reset(&x3.ads122c04_2);

        if(test_results[3])
        {
            twr_module_lcd_draw_string(0, 55, "DRDY 1 test: OK", 1);
        }
        else
        {
            twr_module_lcd_draw_string(0, 55, "DRDY 1 test: FAILED", 1);
        }

        int numberOfErrors = 0;
        for(int i = 0; i < 2; i++)
        {
            if(!test_results[i])
            {
                numberOfErrors++;
            }
        }

        twr_led_set_mode(&lcdLedBlue, TWR_LED_MODE_OFF);

        if(numberOfErrors == 0)
        {
            twr_led_set_mode(&lcdLedGreen, TWR_LED_MODE_ON);
        }
        else
        {
            twr_led_set_mode(&lcdLedRed, TWR_LED_MODE_ON);
        }
    }

    twr_module_lcd_update();

    twr_system_pll_disable();

    twr_scheduler_plan_current_relative(200);
}

// LCD button handler. Left hold starts the test for Chester X0B (GPIO sub test 3 is skiped), Right hold starts the test for Chester X0A
void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;

    if(self == &button_right && event == TWR_BUTTON_EVENT_HOLD && test_progress == 0)
    {
        twr_led_set_mode(&lcdLedBlue, TWR_LED_MODE_ON);
        twr_scheduler_plan_now(tester_task);
    }
    else if(self == &button_right && event == TWR_BUTTON_EVENT_HOLD && test_progress > 3)
    {
        twr_led_set_mode(&lcdLedRed, TWR_LED_MODE_OFF);
        twr_led_set_mode(&lcdLedGreen, TWR_LED_MODE_OFF);
        twr_led_set_mode(&lcdLedBlue, TWR_LED_MODE_ON);

        test_progress = 0;
        state = TEST_I2C_0;
        twr_scheduler_plan_now(tester_task);
    }
}

void delay()
{
    twr_delay_us(60000);
    twr_delay_us(60000);
}

void x3_event_handler(twr_chester_x3_t *self, twr_chester_x3_event_t event, void *event_param)
{
    if (event == TWR_CHESTER_X3_EVENT_UPDATE)
    {
        //float temperature;
        //twr_chester_x3_get_temperature_1(self, &temperature);
        //twr_log_debug("Temperature: %.2f", temperature);
    }
}

// State machine of the whole tester
void tester()
{
    start:
    switch(state)
    {
        // Simple test if the expander is communicatig well
        case TEST_I2C_0:
            if(!twr_ads122c04_init(&x3.ads122c04_1, x3._i2c_channel, TWR_ADS122C04_ADDRESS_A))
            {
                test_results[test_progress] = false;

                state = TEST_I2C_1;
                test_progress++;
                twr_delay_us(60000);
                goto start;
            }
            else
            {
                test_results[test_progress] = true;

                state = TEST_I2C_1;
                test_progress++;
                twr_delay_us(60000);
                goto start;
            }
            break;
        case TEST_I2C_1:
            if(!twr_ads122c04_init(&x3.ads122c04_2, x3._i2c_channel, TWR_ADS122C04_ADDRESS_B))
            {
                test_results[test_progress] = false;

                state = TEST_DRDY_0;
                test_progress++;
                twr_delay_us(60000);
                goto start;
            }
            else
            {
                test_results[test_progress] = true;

                state = TEST_DRDY_0;
                test_progress++;
                twr_delay_us(60000);
                goto start;
            }
            break;

        case TEST_DRDY_0:
        {
            uint8_t rdry = 42;

            uint8_t cr2;
            if(!twr_ads122c04_register_read(&x3.ads122c04_1, 0x02, &cr2))
            {
                test_results[test_progress] = false;

                state = TEST_DRDY_1;
                test_progress++;
                twr_delay_us(60000);

                goto start;
            }

            rdry = (cr2 & 0x80) >> 7;
            twr_log_debug("READY : %d", rdry);

            if(rdry == 0)
            {
                twr_ads122c04_start_sync(&x3.ads122c04_1);
                for(int i = 0; i < 50; i++)
                {
                    uint8_t cr2;
                    twr_ads122c04_register_read(&x3.ads122c04_1, 0x02, &cr2);

                    rdry = (cr2 & 0x80) >> 7;

                    twr_log_debug("READY: %d", (cr2 & 0x80) >> 7);
                    if(rdry == 1)
                    {
                        break;
                    }
                    twr_delay_us(60000);
                }
                if(rdry == 1)
                {
                    test_results[test_progress] = true;

                    state = TEST_DRDY_1;
                    test_progress++;
                    twr_delay_us(60000);

                    goto start;
                }
                else
                {
                    test_results[test_progress] = false;

                    state = TEST_DRDY_1;
                    test_progress++;
                    twr_delay_us(60000);

                    goto start;
                }
            }
            else
            {
                test_results[test_progress] = false;

                state = TEST_DRDY_1;
                test_progress++;
                twr_delay_us(60000);

                goto start;
            }

            break;
        }

        case TEST_DRDY_1:
        {
            uint8_t rdry = 42;

            uint8_t cr2;
            if(!twr_ads122c04_register_read(&x3.ads122c04_2, 0x02, &cr2))
            {
                test_results[test_progress] = false;

                test_progress++;
                twr_delay_us(60000);
                return;
            }

            rdry = (cr2 & 0x80) >> 7;
            twr_log_debug("READY 2: %d", rdry);

            if(rdry == 0)
            {
                twr_ads122c04_start_sync(&x3.ads122c04_2);
                for(int i = 0; i < 50; i++)
                {
                    uint8_t cr2;
                    twr_ads122c04_register_read(&x3.ads122c04_2, 0x02, &cr2);

                    rdry = (cr2 & 0x80) >> 7;

                    twr_log_debug("READY 2: %d", (cr2 & 0x80) >> 7);
                    if(rdry == 1)
                    {
                        break;
                    }
                    twr_delay_us(60000);
                }
                if(rdry == 1)
                {
                    test_results[test_progress] = true;

                    test_progress++;
                    twr_delay_us(60000);
                }
                else
                {
                    test_results[test_progress] = false;

                    test_progress++;
                    twr_delay_us(60000);
                }
            }
            else
            {
                test_results[test_progress] = false;

                test_progress++;
                twr_delay_us(60000);
            }
            break;
        }

        default:
            return;
    }
}

// Application initialization function which is called once after boot
void application_init(void)
{
    // Initialize logging
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_ABS);

    // Initialize LED
    twr_led_init(&led, TWR_GPIO_LED, false, 0);
    twr_led_pulse(&led, 2000);

    twr_module_lcd_init();
    twr_module_lcd_set_font(&twr_font_ubuntu_13);
    twr_module_lcd_update();

    const twr_button_driver_t* lcdButtonDriver =  twr_module_lcd_get_button_driver();
    twr_button_init_virtual(&button_left, 0, lcdButtonDriver, 0);
    twr_button_init_virtual(&button_right, 1, lcdButtonDriver, 0);
    twr_button_set_event_handler(&button_left, button_event_handler, (int*)0);
    twr_button_set_event_handler(&button_right, button_event_handler, (int*)1);

    const twr_led_driver_t* driver = twr_module_lcd_get_led_driver();
    twr_led_init_virtual(&lcdLedRed, TWR_MODULE_LCD_LED_RED, driver, 1);
    twr_led_init_virtual(&lcdLedGreen, TWR_MODULE_LCD_LED_GREEN, driver, 1);
    twr_led_init_virtual(&lcdLedBlue, TWR_MODULE_LCD_LED_BLUE, driver, 1);

    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, 0);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    twr_button_set_hold_time(&button_left, 300);
    twr_button_set_hold_time(&button_right, 300);

    twr_button_set_debounce_time(&button_left, 30);
    twr_button_set_debounce_time(&button_right, 30);

    twr_chester_x3_init(&x3, TWR_I2C_I2C0, twr_chester_x3_I2C_ADDRESS);
    twr_ads122c04_reset(&x3.ads122c04_1);
    twr_ads122c04_reset(&x3.ads122c04_2);
    /*twr_chester_x3_set_event_handler(&x3, x3_event_handler, NULL);
    twr_chester_x3_set_update_interval(&x3, 5000);*/

    // Set up all the tasks
    lcd_print_task = twr_scheduler_register(lcd_print_results, NULL, 1000);
    tester_task = twr_scheduler_register(tester, NULL, TWR_TICK_INFINITY);

    twr_scheduler_plan_now(0);
}

void application_task()
{
    /*uint8_t cr2;
    twr_ads122c04_register_read(&x3.ads122c04_1, 0x02, &cr2);

    twr_log_debug("READY: %d", (cr2 & 0x80) >> 7);

    twr_scheduler_plan_current_relative(10);*/
}
