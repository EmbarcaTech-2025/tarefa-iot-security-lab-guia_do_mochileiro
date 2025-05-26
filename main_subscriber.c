// Bibliotecas necessárias
#include <string.h>             // Para funções de string como strlen()
#include "pico/stdlib.h"        // Biblioteca padrão do Pico (GPIO, tempo, etc.)
#include "pico/cyw43_arch.h"    // Driver WiFi para Pico W
#include "config/credentials.h" // Credenciais da rede WiFi e do broker MQTT
#include "wifi_conn.h"          // Funções personalizadas de conexão WiFi
#include "mqtt_comm.h"          // Funções personalizadas para MQTT
#include "xor_cipher.h"         // Funções de cifra XOR
#include "display.h"
#include "button.h"     // Button handling module
#include "joystick.h"   // Joystick handling module
#include "mbedtls/md.h" // Para HMAC

/**
 * @brief Enumeração dos possíveis modos de operação.
 * Define os diferentes modos de operação de segurança.
 */
typedef enum
{
    MAIN_MENU,
    NORMAL_MODE,
    XOR_MODE,
    HMAC_MODE,
    AES_MODE
} OperationMode;

// --- Estado do Sistema e UI ---
OperationMode current_mode = MAIN_MENU;                                                               // Modo atual de operação.
int main_menu_selected_idx = 0;                                                                       // Índice do item selecionado no menu principal.
const char *main_menu_items[] = {"Sem seguranca", "Encriptacao XOR", "Autenticacao HMAC", "AES-GCM"}; // Itens do menu principal.
const int main_menu_count = sizeof(main_menu_items) / sizeof(main_menu_items[0]);                     // Número de itens no menu principal.
static volatile uint32_t last_btn_press_time = 0;

// Buffer para descriptografia
#define PAYLOAD_MAX_LEN 256
uint8_t decrypted_buffer[PAYLOAD_MAX_LEN];

static uint64_t global_last_timestamp = 0;

// Forward declarations for mode-specific message handlers
void on_message_normal_mode(const char *topic, const uint8_t *payload, size_t len);
void on_message_xor_mode(const char *topic, const uint8_t *payload, size_t len);
void on_message_hmac_mode(const char *topic, const uint8_t *payload, size_t len);
void on_message_aes_mode(const char *topic, const uint8_t *payload, size_t len);

// Handler para o NORMAL_MODE (Sem segurança)
void on_message_normal_mode(const char *topic, const uint8_t *payload, size_t len)
{
    char valor[32] = {0};
    uint64_t timestamp = 0;
    char mensagem[PAYLOAD_MAX_LEN];

    size_t copy_len = len < (PAYLOAD_MAX_LEN - 1) ? len : (PAYLOAD_MAX_LEN - 1);
    memcpy(mensagem, payload, copy_len);
    mensagem[copy_len] = '\0';

    sscanf(mensagem, "%31[^,],%llu", valor, &timestamp);

    if (timestamp > global_last_timestamp)
    {
        printf("[NORMAL] Mensagem NOVA recebida: valor=%s, timestamp=%llu\n", valor, timestamp);
        global_last_timestamp = timestamp;

        display_text_in_line("Msg Recebida:", 1, 0);
        display_text_in_line(mensagem, 2, 0);
        char ts_str[21];
        snprintf(ts_str, sizeof(ts_str), "TS: %llu", timestamp);
        display_text_in_line(ts_str, 3, 0);
        display_text_in_line("", 4, 0); // Clear last line
    }
    else
    {
        printf("[NORMAL] Replay detectado! Ignorando mensagem: valor=%s, timestamp=%llu\n", valor, timestamp);
        display_text_in_line("Replay Detectado!", 1, 0);
        display_text_in_line(valor, 2, 0);
        char ts_str[21];
        snprintf(ts_str, sizeof(ts_str), "TS: %llu", timestamp);
        display_text_in_line(ts_str, 3, 0);
        display_text_in_line("(Normal)", 4, 0);
    }
}

// Handler for XOR_MODE
void on_message_xor_mode(const char *topic, const uint8_t *payload, size_t len)
{
    char valor[32] = {0};
    uint64_t timestamp = 0;

    size_t process_len = len < PAYLOAD_MAX_LEN ? len : (PAYLOAD_MAX_LEN - 1);
    xor_encrypt(payload, decrypted_buffer, process_len, XOR_KEY);
    decrypted_buffer[process_len] = '\0';

    sscanf((char *)decrypted_buffer, "%31[^,],%llu", valor, &timestamp);

    if (timestamp > global_last_timestamp)
    {
        printf("[XOR] Mensagem NOVA (descriptografada): valor=%s, timestamp=%llu\n", valor, timestamp);
        global_last_timestamp = timestamp;

        char hex_string_buffer[2 * process_len + 1];
        for (size_t i = 0; i < process_len; ++i)
        {
            sprintf(hex_string_buffer + (i * 2), "%02x", payload[i]);
        }
        // sprintf null-terminates, but to be safe:
        hex_string_buffer[2 * process_len] = '\0';

        printf("Mensagem criptografada (hex): %s\n", hex_string_buffer);
        printf("Mensagem descriptografada: %s\n", decrypted_buffer);

        display_text_in_line("Msg Cript (XOR):", 1, 0);
        display_text_in_line(hex_string_buffer, 2, 0);
        display_text_in_line("Msg Descriptografada:", 3, 0);
        display_text_in_line((char *)decrypted_buffer, 4, 0);
    }
    else
    {
        printf("[XOR] Replay detectado! Ignorando (descriptografada): valor=%s, timestamp=%llu\n", valor, timestamp);
        display_text_in_line("Replay Detectado!", 1, 0);
        display_text_in_line(valor, 2, 0);
        char ts_str[21];
        snprintf(ts_str, sizeof(ts_str), "TS: %llu", timestamp);
        display_text_in_line(ts_str, 3, 0);
        display_text_in_line("(XOR)", 4, 0);
    }
}

// Placeholder for HMAC_MODE
void on_message_hmac_mode(const char *topic, const uint8_t *payload, size_t len)
{
    printf("[HMAC] Mensagem recebida. Lógica de HMAC não implementada.\n");
    display_text_in_line("Msg HMAC (NI)", 1, 0);
    char len_str[16];
    snprintf(len_str, sizeof(len_str), "Len: %u", len);
    display_text_in_line(len_str, 2, 0);
    display_text_in_line("", 3, 0);
    display_text_in_line("", 4, 0);
    // Implement HMAC verification and replay detection here
}

// Placeholder for AES_MODE
void on_message_aes_mode(const char *topic, const uint8_t *payload, size_t len)
{
    printf("[AES] Mensagem recebida. Lógica de AES não implementada.\n");
    display_text_in_line("Msg AES (NI)", 1, 0);
    char len_str[16];
    snprintf(len_str, sizeof(len_str), "Len: %u", len);
    display_text_in_line(len_str, 2, 0);
    display_text_in_line("", 3, 0);
    display_text_in_line("", 4, 0);
    // Implement AES decryption and replay detection here
}

int main()
{
    stdio_init_all();

    // Inicializa o display
    display_init();

    // Inicializa os botões
    button_init();

    // Inicializa o joystick
    joystick_init();

    sleep_ms(5000);

    // Conecta ao Wi-Fi
    display_text_in_line("Conectando Wi-Fi...", 1, 0);
    connect_to_wifi(WIFI_SSID, WIFI_PASSWORD);
    if (!wifi_comm_is_connected())
    {
        printf("Falha ao obter link da rede Wi-Fi. Abortando.\n");
        return -1;
    }
    printf("Link da rede Wi-Fi estabelecido.\n");
    display_text_in_line("Link estabecido!", 2, 0);

    // Inicializa e conecta ao MQTT
    mqtt_setup(MQTT_CLIENT_ID_SUBSCRIBER, MQTT_BROKER_IP, MQTT_USER, MQTT_PASS);
    sleep_ms(1000);
    display_text_in_line("Conectando MQTT...", 1, 0);

    printf("Aguardando conexao MQTT (3s)...\n"); // Tempo para o cliente MQTT conectar
    sleep_ms(3000);

    // Verifica se a conexão MQTT foi estabelecida
    if (!mqtt_comm_is_connected())
    {
        printf("Falha ao conectar ao broker MQTT. Abortando.\n");
        return -1;
    }
    printf("Conexão MQTT estabelecida.\n");
    display_text_in_line("MQTT Conectado!", 2, 0);
    display_text_in_line(MQTT_CLIENT_ID_SUBSCRIBER, 3, 0);
    sleep_ms(2000);

    // Inscreve no tópico de interesse
    mqtt_comm_subscribe(MQTT_TOPIC_SUBSCRIBE);

    printf("Aguardando mensagens no tópico: %s\n", MQTT_TOPIC_SUBSCRIBE);

    bool first_draw_for_state = true;

    while (true)
    {
        switch (current_mode)
        {
        case MAIN_MENU:
            if (first_draw_for_state)
            {
                // Consider display_clear() here if menu should always redraw on clean screen
                draw_menu("SUBSCRIBER", main_menu_items, main_menu_count, main_menu_selected_idx);
                first_draw_for_state = false;
            }
            int previous_main_menu_idx = main_menu_selected_idx;
            joystick_handle_menu_navigation(main_menu_count, &main_menu_selected_idx);
            if (previous_main_menu_idx != main_menu_selected_idx)
                first_draw_for_state = true;

            if (button_get_pressed_and_reset())
            {
                OperationMode previous_op_mode = current_mode;
                if (main_menu_selected_idx == 0)
                {
                    current_mode = NORMAL_MODE;
                    mqtt_comm_set_message_handler(on_message_normal_mode);
                    printf("Modo Normal selecionado.\n");
                }
                else if (main_menu_selected_idx == 1)
                {
                    current_mode = XOR_MODE;
                    mqtt_comm_set_message_handler(on_message_xor_mode);
                    printf("Modo XOR selecionado.\n");
                }
                else if (main_menu_selected_idx == 2)
                {
                    current_mode = HMAC_MODE;
                    mqtt_comm_set_message_handler(on_message_hmac_mode);
                    printf("Modo HMAC selecionado (NI).\n");
                }
                else if (main_menu_selected_idx == 3)
                {
                    current_mode = AES_MODE;
                    mqtt_comm_set_message_handler(on_message_aes_mode);
                    printf("Modo AES selecionado (NI).\n");
                }

                if (previous_op_mode == MAIN_MENU && current_mode != MAIN_MENU)
                {
                    display_clear(); // Clear menu before showing mode screen
                }
                first_draw_for_state = true;
            }
            break;
        case NORMAL_MODE:
        {
            if (first_draw_for_state)
            {
                display_clear();
                display_text_in_line("Modo: Sem Seguranca", 0, 0);
                display_text_in_line("Aguardando msg...", 1, 0);
                display_text_in_line("", 2, 0);
                display_text_in_line("", 3, 0);
                display_text_in_line("", 4, 0);
                first_draw_for_state = false;
            }
            if (button_get_pressed_and_reset())
            {
                current_mode = MAIN_MENU;
                main_menu_selected_idx = 0;
                first_draw_for_state = true;
                // No break here, outer loop handles switch to MAIN_MENU on next iteration
            }
            // MQTT messages handled by callback. Add other mode-specific periodic tasks if any.
            sleep_ms(100);
        }
        break;
        case XOR_MODE:
        {
            if (first_draw_for_state)
            {
                display_clear();
                display_text_in_line("Modo: Encriptacao XOR", 0, 0);
                display_text_in_line("Aguardando msg...", 1, 0);
                display_text_in_line("", 2, 0);
                display_text_in_line("", 3, 0);
                display_text_in_line("", 4, 0);
                first_draw_for_state = false;
            }
            if (button_get_pressed_and_reset())
            {
                current_mode = MAIN_MENU;
                main_menu_selected_idx = 1;
                first_draw_for_state = true;
            }
            sleep_ms(100);
        }
        break;
        case HMAC_MODE:
        {
            if (first_draw_for_state)
            {
                display_clear();
                display_text_in_line("Modo: HMAC", 0, 0);
                display_text_in_line("Nao implementado", 1, 0);
                display_text_in_line("Aguardando msg...", 2, 0);
                display_text_in_line("", 3, 0);
                display_text_in_line("", 4, 0);
                first_draw_for_state = false;
            }
            if (button_get_pressed_and_reset())
            {
                current_mode = MAIN_MENU;
                main_menu_selected_idx = 2;
                first_draw_for_state = true;
            }
            sleep_ms(100);
        }
        break;
        case AES_MODE:
        {
            if (first_draw_for_state)
            {
                display_clear();
                display_text_in_line("Modo: AES-GCM", 0, 0);
                display_text_in_line("Nao implementado", 1, 0);
                display_text_in_line("Aguardando msg...", 2, 0);
                display_text_in_line("", 3, 0);
                display_text_in_line("", 4, 0);
                first_draw_for_state = false;
            }
            if (button_get_pressed_and_reset())
            {
                current_mode = MAIN_MENU;
                main_menu_selected_idx = 3;
                first_draw_for_state = true;
            }
            sleep_ms(100);
        }
        break;
        }
        tight_loop_contents();
    }

    return 0;
}

/*
 * Comandos de terminal para testar o MQTT:
 *
 * Inicia o broker MQTT com logs detalhados:
 * mosquitto -c mosquitto.conf -v ou mosquitto -c "D:\Arquivos de Programa\mosquitto\config\mosquitto.conf.txt" -v
 *
 * Assina o tópico de temperatura (recebe mensagens):
 * mosquitto_sub -h localhost -p 1883 -t "escola/sala1/temperatura" -u "aluno" -P "senha123"
 *
 * Publica mensagem de teste no tópico de temperatura:
 * mosquitto_pub -h localhost -p 1883 -t "escola/sala1/temperatura" -u "aluno" -P "senha123" -m "26.6"
 */