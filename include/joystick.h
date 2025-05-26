#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "pico/stdlib.h"
#include "hardware/adc.h"

/**
 * @brief Initializes the ADC for joystick input.
 */
void joystick_init(void);

/**
 * @brief Processes joystick input for menu navigation.
 * @param item_count Total number of items in the current menu.
 * @param selected_idx_ptr Pointer to the currently selected item index (will be modified).
 */
void joystick_handle_menu_navigation(int item_count, int *selected_idx_ptr);

#endif // JOYSTICK_H
