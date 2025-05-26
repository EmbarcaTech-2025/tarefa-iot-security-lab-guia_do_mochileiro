#include "joystick.h"
#include "config/config.h" // Required for joystick pins, ADC channels, and thresholds

static uint32_t last_joy_y_move_time = 0;
static bool joystick_y_action_taken = false;

void joystick_init(void)
{
    adc_init();
    adc_gpio_init(JOYSTICK_VRY_PIN); // Enable ADC on joystick Y pin
}

void joystick_handle_menu_navigation(int item_count, int *selected_idx_ptr)
{
    uint32_t now = time_us_32();
    adc_select_input(ADC_JOYSTICK_Y_CHANNEL); // Select ADC channel for Y-axis
    uint16_t joy_y_val = adc_read();          // Read ADC value

    // Anti-repeat logic for joystick movement
    if (!joystick_y_action_taken && (now - last_joy_y_move_time > 200000 /* 200ms delay */))
    {
        // Physical UP movement (HIGH ADC value) -> MENU selection UP
        if (joy_y_val > JOYSTICK_Y_MOVE_UP_THRESHOLD)
        {
            if (*selected_idx_ptr > 0)
                (*selected_idx_ptr)--;
            else
                *selected_idx_ptr = item_count - 1; // Wrap to the last item
            joystick_y_action_taken = true;
            last_joy_y_move_time = now;
        }
        // Physical DOWN movement (LOW ADC value) -> MENU selection DOWN
        else if (joy_y_val < JOYSTICK_Y_MOVE_DOWN_THRESHOLD)
        {
            if (*selected_idx_ptr < item_count - 1)
                (*selected_idx_ptr)++;
            else
                *selected_idx_ptr = 0; // Wrap to the first item
            joystick_y_action_taken = true;
            last_joy_y_move_time = now;
        }
    }
    else
    {
        // Check if joystick returned to neutral to allow new action
        if (joy_y_val < JOYSTICK_Y_NEUTRAL_DEADZONE_HIGH &&
            joy_y_val > JOYSTICK_Y_NEUTRAL_DEADZONE_LOW)
        {
            joystick_y_action_taken = false;
        }
        // Timeout to reset action flag if joystick gets stuck
        if (now - last_joy_y_move_time > 500000 /* 500ms timeout */)
        {
            joystick_y_action_taken = false;
        }
    }
}
