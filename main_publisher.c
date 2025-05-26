// Bibliotecas necessárias
#include <string.h>             // Para funções de string como strlen()
#include "pico/stdlib.h"        // Biblioteca padrão do Pico (GPIO, tempo, etc.)
#include "pico/cyw43_arch.h"    // Driver WiFi para Pico W
#include "config/credentials.h" // Credenciais da rede WiFi e do broker MQTT
#include "wifi_conn.h"          // Funções personalizadas de conexão WiFi
#include "mqtt_comm.h"          // Funções personalizadas para MQTT
#include "xor_cipher.h"         // Funções de cifra XOR
#include "display.h"            // Funções de exibição no display SSD1306
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

    // Loop principal do programa
    while (true)
    {

        // mensagem com timestamp
        uint64_t timestamp = to_us_since_boot(get_absolute_time());
        char mensagem[64];
        snprintf(mensagem, sizeof(mensagem), "26.5,%llu", timestamp);

        // Publica a mensagem original (não criptografada)
        // mqtt_comm_publish(MQTT_TOPIC_SUBSCRIBE, mensagem, strlen(mensagem));

        char hex_string_buffer[2 * strlen(mensagem) + 1];
        hex_string_buffer[0] = '\0'; // Inicializa o buffer como string vazia

        // Alternativa: Publica a mensagem criptografada
        uint8_t criptografada[64];
        xor_encrypt((uint8_t *)mensagem, criptografada, strlen(mensagem), XOR_KEY);

        mqtt_comm_publish(MQTT_TOPIC_SUBSCRIBE, criptografada, strlen(mensagem));
        printf("Mensagem original: %s\n", mensagem);
        printf("Mensagem criptografada (hex): ");
        for (size_t i = 0; i < strlen(mensagem); ++i)
        {
            printf("%02x", criptografada[i]);
            sprintf(hex_string_buffer + (i * 2), "%02x", criptografada[i]);
        }
        printf("\n");
        hex_string_buffer[2 * strlen(mensagem)] = '\0'; // Garante terminação nula
        display_text_in_line("Msg Original:", 1, 1);
        display_text_in_line(mensagem, 2, 1);
        display_text_in_line("Msg Cript (XOR):", 3, 1);
        display_text_in_line(hex_string_buffer, 4, 1); // Exibe a string hexadecimal

        // Aguarda 5 segundos antes da próxima publicação
        sleep_ms(5000);
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