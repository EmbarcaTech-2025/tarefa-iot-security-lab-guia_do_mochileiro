#include "button.h"
#include "config/config.h" // Required for BTN_A_PIN, BTN_DEBOUNCE_TIME_MS

static volatile uint32_t last_btn_press_time = 0;
static volatile bool button_pressed_flag = false;

// IRQ callback function for button A press
static void button_a_irq_callback(uint gpio, uint32_t events)
{
    if (events == GPIO_IRQ_EDGE_FALL && gpio == BTN_A_PIN && !button_pressed_flag)
    {
        uint32_t now = time_us_32();
        if ((int32_t)(now - last_btn_press_time) > BTN_DEBOUNCE_TIME_MS * 1000 || last_btn_press_time == 0)
        {
            button_pressed_flag = true;
            last_btn_press_time = now;
        }
    }
}

void button_init(void)
{
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &button_a_irq_callback);
}

bool button_get_pressed_and_reset(void)
{
    if (button_pressed_flag)
    {
        button_pressed_flag = false; // Reset the flag atomically (assuming single core access or IRQ priorities handle it)
        return true;
    }
    return false;
}
