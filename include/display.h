#ifndef DISPLAY_H
#define DISPLAY_H

#include "ssd1306.h"

void display_init();
void display_draw_initial_message();
void display_text_in_line(const char *message, int line);
void draw_menu(const char *title, const char *items[], int item_count, int selected_idx);

#endif // DISPLAY_H
