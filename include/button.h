#ifndef BUTTON_H
#define BUTTON_H

#include "pico/stdlib.h"
#include <stdbool.h>

/**
 * @brief Initializes the button GPIO and sets up the interrupt.
 */
void button_init(void);

/**
 * @brief Checks if the button was pressed and resets the flag.
 * 
 * @return true if the button was pressed since the last call, false otherwise.
 */
bool button_get_pressed_and_reset(void);

#endif // BUTTON_H
