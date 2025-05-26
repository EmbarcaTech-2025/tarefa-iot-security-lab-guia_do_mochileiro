#include "display.h"
#include "config/config.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <string.h> // Para sprintf

ssd1306_t display;

void display_init()
{
    // Inicialização I2C
    i2c_init(OLED_I2C_PORT, OLED_I2C_FREQUENCY); // 400kHz
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    display.external_vcc = false; // Define se o VCC é fornecido externamente

    // Inicialização do display SSD1306
    if (!ssd1306_init(&display, OLED_WIDTH, OLED_HEIGHT, OLED_I2C_ADDRESS, OLED_I2C_PORT))
    {
        printf("Falha ao inicializar SSD1306\n");
        while (1)
            ; // Parar em caso de erro
    }
    ssd1306_clear(&display);
    ssd1306_show(&display);
    printf("Display SSD1306 inicializado e limpo.\n");
    display_draw_initial_message();
}

void display_draw_initial_message()
{
    ssd1306_clear(&display);
    // Exibir mensagem inicial
    ssd1306_draw_string(&display, 5, 5, 1, "Carregando");
    ssd1306_draw_string(&display, 5, 20, 1, "Teste Seguranca");
    ssd1306_draw_string(&display, 5, 35, 1, "IoT...");
    ssd1306_show(&display);
    sleep_ms(1000); // Dê tempo para o usuário ver a mensagem
}

void display_text_in_line(const char *message, int line)
{
    if (line <= 1) {
    ssd1306_clear(&display);
    }
    // Exibir mensagem
    ssd1306_draw_string(&display, 5, 5 + ((line - 1) * OLED_LINE_HEIGHT), 1, message);
    ssd1306_show(&display);
}

/**
 * @brief Desenha um item de menu no OLED.
 * @param text Texto do item de menu.
 * @param y_pos Posição Y inicial do item.
 * @param selected true se o item estiver selecionado, false caso contrário.
 */
void draw_menu_item(const char *text, int y_pos, bool selected)
{ 
    if (selected)
    {
        // Destaca o item selecionado (ex: com um retângulo ou cursor)
        ssd1306_draw_empty_square(&display, 0, y_pos - 2, OLED_WIDTH - 1, OLED_LINE_HEIGHT + 2); // Retângulo em volta
        ssd1306_draw_string(&display, 5, y_pos, 1, text); // Texto com pequeno recuo
    }
    else
    {
        ssd1306_draw_string(&display, 5, y_pos, 1, text);
    }
}

/**
 * @brief Desenha um menu completo no OLED.
 * @param title Título do menu.
 * @param items Array de strings com os itens do menu.
 * @param item_count Número de itens no menu.
 * @param selected_idx Índice do item atualmente selecionado.
 */
void draw_menu(const char *title, const char **items, int item_count, int selected_idx)
{ 
    ssd1306_clear(&display);
    // Centraliza o título
    ssd1306_draw_string(&display, (OLED_WIDTH - (strlen(title) * 6 /* largura da fonte */)) / 2, 0, 1, title);
    ssd1306_draw_line(&display, 0, OLED_LINE_HEIGHT - 2, OLED_WIDTH, OLED_LINE_HEIGHT - 2); // Linha separadora

    int y_start = OLED_LINE_HEIGHT + 4; // Posição Y inicial para os itens
    for (int i = 0; i < item_count; ++i)
    {
        draw_menu_item(items[i], y_start + (i * (OLED_LINE_HEIGHT + 3 /* espaçamento */)), i == selected_idx);
    }
    ssd1306_show(&display); // Atualiza o display
}

/**
 * @brief Desenha um menu de configuração no OLED (ex: taxa de amostragem, duração).
 * @param title Título do menu de configuração.
 * @param labels Array de strings com os rótulos das opções.
 * @param option_count Número de opções.
 * @param selected_idx Índice da opção atualmente selecionada para navegação.
 * @param current_config_idx Índice da configuração atualmente ativa.
 */
void draw_config_menu(const char *title, const char **labels, int option_count, int selected_idx, uint current_config_idx)
{ 
    ssd1306_clear(&display);
    ssd1306_draw_string(&display, (OLED_WIDTH - (strlen(title) * 6)) / 2, 0, 1, title);
    ssd1306_draw_line(&display, 0, OLED_LINE_HEIGHT - 2, OLED_WIDTH, OLED_LINE_HEIGHT - 2);

    int y_start = OLED_LINE_HEIGHT + 4;
    for (int i = 0; i < option_count; ++i)
    {
        char item_text[32];
        // Adiciona um ">" para indicar a configuração ativa
        snprintf(item_text, sizeof(item_text), "%s %s", (i == current_config_idx) ? ">" : " ", labels[i]);
        draw_menu_item(item_text, y_start + (i * (OLED_LINE_HEIGHT + 3)), i == selected_idx);
    }
    ssd1306_show(&display);
}