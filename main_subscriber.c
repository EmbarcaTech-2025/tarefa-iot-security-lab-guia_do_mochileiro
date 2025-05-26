// Bibliotecas necessárias
#include <string.h>             // Para funções de string como strlen()
#include "pico/stdlib.h"        // Biblioteca padrão do Pico (GPIO, tempo, etc.)
#include "pico/cyw43_arch.h"    // Driver WiFi para Pico W
#include "config/credentials.h" // Credenciais da rede WiFi e do broker MQTT
#include "wifi_conn.h"          // Funções personalizadas de conexão WiFi
#include "mqtt_comm.h"          // Funções personalizadas para MQTT
#include "xor_cipher.h"         // Funções de cifra XOR
#include "display.h"
#include "button.h"        // Button handling module
#include "joystick.h"      // Joystick handling module
#include "mbedtls/md.h"    // Para HMAC
#include "mbedtls/error.h" // Para mbedtls_strerror

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

// Forward declartion para os handlers de mensagens específicos de cada modo
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

// Handler para XOR_MODE
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

// Handler para HMAC_MODE
void on_message_hmac_mode(const char *topic, const uint8_t *payload, size_t len)
{
    if (len < HMAC_DIGEST_SIZE)
    {
        printf("[HMAC Sub] Payload muito curto. Len: %u, Esperado min: %d\n", len, HMAC_DIGEST_SIZE);
        display_text_in_line("HMAC Err: Curto", 1, 0);
        char len_str[20];
        snprintf(len_str, sizeof(len_str), "Len: %u", len);
        display_text_in_line(len_str, 2, 0);
        return;
    }

    const uint8_t *received_hmac = payload;
    const uint8_t *message_data_ptr = payload + HMAC_DIGEST_SIZE;
    size_t message_data_len = len - HMAC_DIGEST_SIZE;

    // Buffer for the message string part, ensure null termination for sscanf
    char extracted_message_str[PAYLOAD_MAX_LEN]; // Use a generous buffer
    if (message_data_len >= sizeof(extracted_message_str))
    {
        printf("[HMAC Sub] Parte da mensagem do payload muito longa. Len: %u\n", message_data_len);
        display_text_in_line("HMAC Err: MsgLng", 1, 0);
        return;
    }
    memcpy(extracted_message_str, message_data_ptr, message_data_len);
    extracted_message_str[message_data_len] = '\0'; // Null-terminate

    uint8_t calculated_hmac[HMAC_DIGEST_SIZE];
    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

    if (md_info == NULL)
    {
        printf("[HMAC Sub] Error: SHA256 não disponível.\n");
        display_text_in_line("HMAC Err: SHA256", 1, 0);
        display_text_in_line("Indisponivel", 2, 0);
        return;
    }

    int ret = mbedtls_md_hmac(md_info,
                              (const unsigned char *)HMAC_SECRET_KEY, strlen(HMAC_SECRET_KEY),
                              message_data_ptr, message_data_len, // Use pointer and actual length
                              calculated_hmac);

    if (ret != 0)
    {
        char error_buf[100];
        mbedtls_strerror(ret, error_buf, sizeof(error_buf));
        printf("[HMAC Sub] Error: Falha de calculo mbedtls_md_hmac: -0x%04X - %s\n", (unsigned int)-ret, error_buf);
        display_text_in_line("HMAC Err: Calc", 1, 0);
        snprintf(error_buf, sizeof(error_buf), "Code: -0x%04X", (unsigned int)-ret);
        display_text_in_line(error_buf, 2, 0);
        return;
    }

    char valor[32] = {0};
    uint64_t timestamp = 0;
    // Ensure sscanf does not read past the actual message data by using the null-terminated string
    sscanf(extracted_message_str, "%31[^,],%llu", valor, &timestamp);

    if (memcmp(received_hmac, calculated_hmac, HMAC_DIGEST_SIZE) == 0)
    {
        if (timestamp > global_last_timestamp)
        {
            global_last_timestamp = timestamp;
            printf("[HMAC Sub] Mensagem AUTENTICADA e NOVA: msg='%s', ts=%llu\n", extracted_message_str, timestamp);

            display_text_in_line("Msg Autenticada:", 1, 0);
            display_text_in_line(extracted_message_str, 2, 0);
            char ts_str[21];
            snprintf(ts_str, sizeof(ts_str), "TS: %llu", timestamp);
            display_text_in_line(ts_str, 3, 0);
            char hmac_ok_disp[20];
            sprintf(hmac_ok_disp, "HMAC OK: %02x%02x..", received_hmac[0], received_hmac[1]);
            display_text_in_line(hmac_ok_disp, 4, 0);
        }
        else
        {
            printf("[HMAC Sub] Replay detectado! Msg: '%s', ts=%llu. HMAC era válido.\n", extracted_message_str, timestamp);
            display_text_in_line("Replay Detectado!", 1, 0);
            display_text_in_line(extracted_message_str, 2, 0);
            char ts_str[21];
            snprintf(ts_str, sizeof(ts_str), "TS: %llu", timestamp);
            display_text_in_line(ts_str, 3, 0);
            display_text_in_line("(HMAC OK)", 4, 0);
        }
    }
    else
    {
        printf("[HMAC Sub] Falha na verificação do HMAC! Msg: '%s', ts=%llu\n", extracted_message_str, timestamp);

        display_text_in_line("Falha HMAC!", 1, 0);
        display_text_in_line(extracted_message_str, 2, 0);
        char ts_str[21];
        snprintf(ts_str, sizeof(ts_str), "TS: %llu", timestamp);
        display_text_in_line(ts_str, 3, 0);
        char hmac_fail_disp[20];
        sprintf(hmac_fail_disp, "Rec: %02x%02x Calc:%02x%02x", received_hmac[0], received_hmac[1], calculated_hmac[0], calculated_hmac[1]);
        display_text_in_line(hmac_fail_disp, 4, 0);
    }
}

// Placeholder para AES_MODE
void on_message_aes_mode(const char *topic, const uint8_t *payload, size_t len)
{
    printf("[AES] Mensagem recebida. Lógica de AES não implementada.\n");
    display_text_in_line("Msg AES (NI)", 1, 0);
    char len_str[16];
    snprintf(len_str, sizeof(len_str), "Len: %u", len);
    display_text_in_line(len_str, 2, 0);
    display_text_in_line("", 3, 0);
    display_text_in_line("", 4, 0);
    // TODO
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
                    display_clear(); 
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
            }
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
                display_text_in_line("Modo: Autent. HMAC", 0, 0);
                display_text_in_line("Aguardando msg...", 1, 0);
                display_text_in_line("", 2, 0);
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