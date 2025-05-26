#ifndef DISPLAY_H
#define DISPLAY_H

#include "ssd1306.h"

void display_init();
void display_draw_initial_message();
void display_text_in_line(const char *message, int line, bool is_publisher);
void draw_menu(const char *title, const char *items[], int item_count, int selected_idx);
void draw_top_title_publisher();
void draw_top_title_subscriber();

#endif // DISPLAY_H