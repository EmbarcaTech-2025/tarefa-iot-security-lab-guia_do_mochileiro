// Bibliotecas necessárias
#include <string.h>             // Para funções de string como strlen()
#include "pico/stdlib.h"        // Biblioteca padrão do Pico (GPIO, tempo, etc.)
#include "pico/cyw43_arch.h"    // Driver WiFi para Pico W
#include "config/credentials.h" // Credenciais da rede WiFi e do broker MQTT
#include "wifi_conn.h"          // Funções personalizadas de conexão WiFi
#include "mqtt_comm.h"          // Funções personalizadas para MQTT
#include "xor_cipher.h"         // Funções de cifra XOR
#include "display.h"            // Funções de exibição no display SSD1306
#include "button.h"             // Button handling module
#include "joystick.h"           // Joystick handling module
#include "mbedtls/md.h"         // Para HMAC
#include "mbedtls/error.h"      // Para mbedtls_strerror
#include "mbedtls/gcm.h"

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

int main()
{
    // Inicializa todas as interfaces de I/O padrão (USB serial, etc.)
    stdio_init_all();

    // Inicializa o display
    display_init();

    // Inicializa os botões
    button_init();

    // Inicializa o joystick
    joystick_init();

    // Aguarda inicialização do terminal serial
    sleep_ms(5000);

    // Conecta à rede WiFi
    // Parâmetros em credentials.h
    display_text_in_line("Conectando Wi-Fi...", 1, 1);
    connect_to_wifi(WIFI_SSID, WIFI_PASSWORD);

    // Testa o link da conexão Wi-Fi
    if (!wifi_comm_is_connected())
    {
        printf("Falha ao obter link da rede Wi-Fi. Abortando.\n");
        return -1;
    }
    printf("Link da rede Wi-Fi estabelecido.\n");
    display_text_in_line("Link estabecido!", 2, 1);

    // Configura o cliente MQTT
    // Parâmetros em credentials.h
    mqtt_setup(MQTT_CLIENT_ID_PUBLISHER, MQTT_BROKER_IP, MQTT_USER, MQTT_PASS);
    sleep_ms(1000);
    display_text_in_line("Conectando MQTT...", 1, 1);

    printf("Aguardando conexao MQTT (3s)...\n"); // Tempo para o cliente MQTT conectar
    sleep_ms(3000);

    // Verifica se a conexão MQTT foi estabelecida
    if (!mqtt_comm_is_connected())
    {
        printf("Falha ao conectar ao broker MQTT. Abortando.\n");
        return -1;
    }
    printf("Conexão MQTT estabelecida.\n");
    display_text_in_line("MQTT Conectado!", 2, 1);
    display_text_in_line(MQTT_CLIENT_ID_PUBLISHER, 3, 1);
    sleep_ms(2000);

    bool first_draw_for_state = true; // Flag para indicar se é o primeiro desenho do estado atual

    while (true)
    {
        switch (current_mode)
        {
        // --- MODO: MENU PRINCIPAL ---
        case MAIN_MENU:
            if (first_draw_for_state)
            {
                draw_menu("PUBLISHER", main_menu_items, main_menu_count, main_menu_selected_idx);
                first_draw_for_state = false;
            }
            int previous_main_menu_idx = main_menu_selected_idx;
            joystick_handle_menu_navigation(main_menu_count, &main_menu_selected_idx);
            if (previous_main_menu_idx != main_menu_selected_idx) // Se seleção mudou, redesenha
                first_draw_for_state = true;

            if (button_get_pressed_and_reset()) // Se botão do joystick pressionado
            {
                OperationMode previous_op_mode = current_mode;
                if (main_menu_selected_idx == 0) // Sem seguranca
                {
                    current_mode = NORMAL_MODE;
                }
                else if (main_menu_selected_idx == 1) // Encriptacao XOR
                {
                    current_mode = XOR_MODE;
                }
                else if (main_menu_selected_idx == 2) // Autenticacao HMAC
                {
                    current_mode = HMAC_MODE;
                }
                else if (main_menu_selected_idx == 3) // AES-GCM
                {
                    current_mode = AES_MODE;
                }

                if (previous_op_mode == MAIN_MENU && current_mode != MAIN_MENU)
                {
                    display_clear(); // Limpa o display ao entrar em um novo modo
                }
                first_draw_for_state = true; // Força redesenho do novo estado
            }
            break;
        case NORMAL_MODE:
        {
            if (first_draw_for_state)
            {
                display_text_in_line("Modo: Sem Seguranca", 0, 1);
                display_text_in_line("Enviando msg...", 1, 1);
                display_text_in_line("", 2, 1);
                display_text_in_line("", 3, 1);
                display_text_in_line("", 4, 1);
                first_draw_for_state = false;
            }
            // Loop principal do programa
            if (button_get_pressed_and_reset()) // Botão do joystick para sair do modo operacional
            {
                current_mode = MAIN_MENU;
                main_menu_selected_idx = 0; // Volta para "Modo Normal"
                first_draw_for_state = true;
                break; // Sai do case NORMAL_MODE para redesenhar o menu imediatamente
            }

            // mensagem com timestamp
            uint64_t timestamp = to_us_since_boot(get_absolute_time());
            char mensagem[64];
            snprintf(mensagem, sizeof(mensagem), "26.5,%llu", timestamp);

            // Publica a mensagem original (não criptografada)
            mqtt_comm_publish(MQTT_TOPIC_SUBSCRIBE, (uint8_t *)mensagem, strlen(mensagem));

            display_text_in_line("Msg Enviada:", 1, 1);
            display_text_in_line(mensagem, 2, 1);
            char ts_str[21];
            snprintf(ts_str, sizeof(ts_str), "TS: %llu", timestamp);
            display_text_in_line(ts_str, 3, 1);
            display_text_in_line("", 4, 1);

            // Aguarda 5 segundos antes da próxima publicação
            sleep_ms(5000);
        }
        break;
        case XOR_MODE:
        {
            if (first_draw_for_state)
            {
                display_text_in_line("Modo: Encriptacao XOR", 0, 1);
                display_text_in_line("Enviando msg...", 1, 1);
                display_text_in_line("", 2, 1);
                display_text_in_line("", 3, 1);
                display_text_in_line("", 4, 1);
                first_draw_for_state = false;
            }
            if (button_get_pressed_and_reset()) // Botão do joystick para sair do modo operacional
            {
                current_mode = MAIN_MENU;
                main_menu_selected_idx = 1; // Volta para "Encriptacao XOR"
                first_draw_for_state = true;
                break; // Sai do case XOR_MODE para redesenhar o menu imediatamente
            }

            // mensagem com timestamp
            uint64_t timestamp = to_us_since_boot(get_absolute_time());
            char mensagem[64];
            snprintf(mensagem, sizeof(mensagem), "26.5,%llu", timestamp);

            size_t mensagem_len = strlen(mensagem);
            char hex_string_buffer[2 * mensagem_len + 1];
            hex_string_buffer[0] = '\0'; // Inicializa o buffer como string vazia

            // Publica a mensagem criptografada
            uint8_t criptografada[64]; // Garante que o  buffer é suficiente para a mensagem criptografada
            xor_encrypt((uint8_t *)mensagem, criptografada, mensagem_len, XOR_KEY);

            mqtt_comm_publish(MQTT_TOPIC_SUBSCRIBE, criptografada, mensagem_len);
            printf("Mensagem original: %s\n", mensagem);
            printf("Mensagem criptografada (hex): ");
            for (size_t i = 0; i < mensagem_len; ++i)
            {
                printf("%02x", criptografada[i]);
                sprintf(hex_string_buffer + (i * 2), "%02x", criptografada[i]);
            }
            printf("\n");
            hex_string_buffer[2 * mensagem_len] = '\0'; // Garante terminação nula

            display_text_in_line("Msg Original:", 1, 1);
            display_text_in_line(mensagem, 2, 1);
            display_text_in_line("Msg Cript (XOR):", 3, 1);
            display_text_in_line(hex_string_buffer, 4, 1); // Exibe a string hexadecimal

            // Aguarda 5 segundos antes da próxima publicação
            sleep_ms(5000);
        }
        break;
        case HMAC_MODE:
        {
            if (first_draw_for_state)
            {
                display_text_in_line("Modo: Autent. HMAC", 0, 1);
                display_text_in_line("Enviando msg...", 1, 1);
                display_text_in_line("", 2, 1);
                display_text_in_line("", 3, 1);
                display_text_in_line("", 4, 1);
                first_draw_for_state = false;
            }

            if (button_get_pressed_and_reset())
            {
                current_mode = MAIN_MENU;
                main_menu_selected_idx = 2;
                first_draw_for_state = true;
                break;
            }

            // Mensagem com timestamp
            uint64_t timestamp = to_us_since_boot(get_absolute_time());
            char mensagem_original[64];
            snprintf(mensagem_original, sizeof(mensagem_original), "26.5,%llu", timestamp);
            size_t mensagem_original_len = strlen(mensagem_original);

            uint8_t hmac_result[HMAC_DIGEST_SIZE];
            const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

            if (md_info == NULL)
            {
                printf("HMAC Pub Error: SHA256 not available.\n");
                display_text_in_line("HMAC Err: SHA256", 1, 1);
                display_text_in_line("Indisponivel", 2, 1);
                sleep_ms(3000);
                current_mode = MAIN_MENU;
                first_draw_for_state = true;
                break;
            }

            int ret = mbedtls_md_hmac(md_info,
                                      (const unsigned char *)HMAC_SECRET_KEY, strlen(HMAC_SECRET_KEY),
                                      (const unsigned char *)mensagem_original, mensagem_original_len,
                                      hmac_result);

            if (ret != 0)
            {
                char error_buf[100];
                mbedtls_strerror(ret, error_buf, sizeof(error_buf));
                printf("HMAC Pub Error: mbedtls_md_hmac falhou: -0x%04X - %s\n", (unsigned int)-ret, error_buf);
                display_text_in_line("HMAC Err: Calc", 1, 1);
                snprintf(error_buf, sizeof(error_buf), "Code: -0x%04X", (unsigned int)-ret);
                display_text_in_line(error_buf, 2, 1);
                sleep_ms(3000);
                current_mode = MAIN_MENU;
                first_draw_for_state = true;
                break;
            }

            uint8_t payload_to_send[HMAC_DIGEST_SIZE + sizeof(mensagem_original)]; // Ensure enough space
            if (HMAC_DIGEST_SIZE + mensagem_original_len > sizeof(payload_to_send))
            {
                printf("HMAC Pub Error: Payload buffer muito pequeno.\n");
                display_text_in_line("HMAC Err: Buf", 1, 1);
                sleep_ms(3000);
                current_mode = MAIN_MENU;
                first_draw_for_state = true;
                break;
            }

            memcpy(payload_to_send, hmac_result, HMAC_DIGEST_SIZE);
            memcpy(payload_to_send + HMAC_DIGEST_SIZE, mensagem_original, mensagem_original_len);

            size_t total_payload_len = HMAC_DIGEST_SIZE + mensagem_original_len;

            mqtt_comm_publish(MQTT_TOPIC_SUBSCRIBE, payload_to_send, total_payload_len);

            printf("HMAC Pub: Original: %s\n", mensagem_original);
            printf("HMAC Pub: HMAC (hex): ");
            char hmac_hex_display_full[HMAC_DIGEST_SIZE * 2 + 1];
            for (int i = 0; i < HMAC_DIGEST_SIZE; i++)
            {
                sprintf(hmac_hex_display_full + i * 2, "%02x", hmac_result[i]);
            }
            hmac_hex_display_full[HMAC_DIGEST_SIZE * 2] = '\0';
            printf("%s\n", hmac_hex_display_full);

            display_text_in_line("Msg Original (HMAC):", 1, 1);
            display_text_in_line(mensagem_original, 2, 1);

            char hmac_short_display[20];
            snprintf(hmac_short_display, sizeof(hmac_short_display), "HMAC: %02x%02x%02x%02x...",
                     hmac_result[0], hmac_result[1], hmac_result[2], hmac_result[3]);
            display_text_in_line(hmac_short_display, 3, 1);
            display_text_in_line("Enviado!", 4, 1);

            sleep_ms(5000);
        }
        break;
        case AES_MODE:
        {
            if (first_draw_for_state)
            {
                display_text_in_line("Modo: AES-GCM", 0, 1);
                display_text_in_line("Enviando msg...", 1, 1);
                display_text_in_line("", 2, 1);
                display_text_in_line("", 3, 1);
                display_text_in_line("", 4, 1);
                first_draw_for_state = false;
            }

            if (button_get_pressed_and_reset())
            {
                current_mode = MAIN_MENU;
                main_menu_selected_idx = 3;
                first_draw_for_state = true;
                break;
            }

            // Prepara mensagem original
            uint64_t timestamp_us = to_us_since_boot(get_absolute_time());
            char mensagem_original[64];
            snprintf(mensagem_original, sizeof(mensagem_original), "26.5,%llu", timestamp_us);
            size_t mensagem_len = strlen(mensagem_original);

            // Prepara o IV (Initialization Vector) - 12 bytes
            // Derivado do timestamp para garantir que única por mensagem
            uint8_t iv[AES_IV_LEN];
            memcpy(iv, &timestamp_us, sizeof(uint64_t)); // Primeiros 8 bytes do timestamp
            for (size_t i = sizeof(uint64_t); i < AES_IV_LEN; ++i)
            {
                iv[i] = (uint8_t)i; // Padding
            }

            // Prepara para criptografia
            mbedtls_gcm_context aes_ctx;
            mbedtls_gcm_init(&aes_ctx);

            int ret = mbedtls_gcm_setkey(&aes_ctx, MBEDTLS_CIPHER_ID_AES, (const unsigned char *)AES_KEY, 256);
            if (ret != 0)
            {
                printf("AES Pub Error: mbedtls_gcm_setkey falhou: -0x%04X\n", (unsigned int)-ret);
                display_text_in_line("AES Err: SetKey", 1, 1);
                mbedtls_gcm_free(&aes_ctx);
                sleep_ms(3000);
                current_mode = MAIN_MENU;
                first_draw_for_state = true;
                break;
            }

            uint8_t ciphertext[sizeof(mensagem_original)]; // Ciphertext vai ser do mesmo tamanho do plaintext
            uint8_t tag[AES_TAG_LEN];

            // Encripta a mensagem
            ret = mbedtls_gcm_crypt_and_tag(&aes_ctx, MBEDTLS_GCM_ENCRYPT, mensagem_len,
                                            iv, AES_IV_LEN,
                                            NULL, 0,
                                            (const unsigned char *)mensagem_original, ciphertext,
                                            AES_TAG_LEN, tag);
            mbedtls_gcm_free(&aes_ctx);

            if (ret != 0)
            {
                printf("AES Pub Error: mbedtls_gcm_crypt_and_tag falhou: -0x%04X\n", (unsigned int)-ret);
                display_text_in_line("AES Err: Encrypt", 1, 1);
                sleep_ms(3000);
                current_mode = MAIN_MENU;
                first_draw_for_state = true;
                break;
            }

            // Constroi o payload: [IV (12)] [TAG (16)] [Ciphertext (mensagem_len)]
            uint8_t payload_to_send[AES_IV_LEN + AES_TAG_LEN + sizeof(mensagem_original)];
            if (AES_IV_LEN + AES_TAG_LEN + mensagem_len > sizeof(payload_to_send))
            {
                printf("AES Pub Error: Payload buffer too small.\n");
                display_text_in_line("AES Err: Buf", 1, 1);
                sleep_ms(3000);
                current_mode = MAIN_MENU;
                first_draw_for_state = true;
                break;
            }

            memcpy(payload_to_send, iv, AES_IV_LEN);
            memcpy(payload_to_send + AES_IV_LEN, tag, AES_TAG_LEN);
            memcpy(payload_to_send + AES_IV_LEN + AES_TAG_LEN, ciphertext, mensagem_len);
            size_t total_payload_len = AES_IV_LEN + AES_TAG_LEN + mensagem_len;

            // Publica
            mqtt_comm_publish(MQTT_TOPIC_SUBSCRIBE, payload_to_send, total_payload_len);

            printf("AES Pub: Original: %s\n", mensagem_original);
            // Informações no Display
            display_text_in_line("Msg Original (AES):", 1, 1);
            display_text_in_line(mensagem_original, 2, 1);
            char info_str[40];
            snprintf(info_str, sizeof(info_str), "IV:%02x%02x.. Tag:%02x%02x..", iv[0], iv[1], tag[0], tag[1]);
            display_text_in_line(info_str, 3, 1);
            display_text_in_line("Cripto Enviada!", 4, 1);

            sleep_ms(5000);
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
 * mosquitto -c mosquitto.conf -v
 *
 * Assina o tópico de temperatura (recebe mensagens):
 * mosquitto_sub -h localhost -p 1883 -t "escola/sala1/temperatura" -u "aluno" -P "senha123"
 *
 * Publica mensagem de teste no tópico de temperatura:
 * mosquitto_pub -h localhost -p 1883 -t "escola/sala1/temperatura" -u "aluno" -P "senha123" -m "26.6"
 */