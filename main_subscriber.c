// Bibliotecas necessárias
#include <string.h>             // Para funções de string como strlen()
#include "pico/stdlib.h"        // Biblioteca padrão do Pico (GPIO, tempo, etc.)
#include "pico/cyw43_arch.h"    // Driver WiFi para Pico W
#include "config/credentials.h" // Credenciais da rede WiFi e do broker MQTT
#include "wifi_conn.h"          // Funções personalizadas de conexão WiFi
#include "mqtt_comm.h"          // Funções personalizadas para MQTT
#include "xor_cipher.h"         // Funções de cifra XOR
#include "display.h"

// Buffer para descriptografia
#define PAYLOAD_MAX_LEN 256
uint8_t decrypted[PAYLOAD_MAX_LEN];

static uint64_t last_timestamp = 0;

// Função handler que será chamada ao receber mensagem MQTT
void on_message(const char *topic, const uint8_t *payload, size_t len)
{

    // Espera payload no formato "valor,timestamp"
    char valor[32];
    uint64_t timestamp = 0;
    // sscanf((char *)decrypted, "%31[^,],%llu", valor, &timestamp);

    // sem criptografia:
    char mensagem[64];
    memcpy(mensagem, payload, len);
    mensagem[len] = '\0';

    char hex_string_buffer[2 * strlen(payload) + 1];
    hex_string_buffer[0] = '\0'; // Inicializa o buffer como string vazia

    // Espera formato "valor,timestamp"
    sscanf(mensagem, "%31[^,],%llu", valor, &timestamp);

    static uint64_t last_timestamp = 0;
    if (timestamp > last_timestamp)
    {
        printf("Mensagem NOVA recebida: valor=%s, timestamp=%llu\n", valor, timestamp);
        last_timestamp = timestamp;
    }
    else
    {
        printf("Replay detectado! Ignorando mensagem: valor=%s, timestamp=%llu\n", valor, timestamp);
    }

    printf("Mensagem criptografada recebida no tópico [%s] (hex): ", topic);
    for (size_t i = 0; i < len; ++i)
    {
        printf("%02x", payload[i]);
        sprintf(hex_string_buffer + (i * 2), "%02x", payload[i]);
    }
    hex_string_buffer[2 * strlen(payload)] = '\0'; // Garante terminação da string hexadecimal
    printf("\n");

    // Descriptografa
    memcpy(decrypted, payload, len);
    xor_encrypt(payload, decrypted, len, XOR_KEY); // XOR_KEY deve ser definido em xor_encrypt.h ou credentials.h
    decrypted[len] = '\0';                         // Garante terminação caso texto

    printf("Mensagem recebida no tópico [%s]: %s\n", topic, decrypted);
    display_text_in_line("Msg Cript (XOR):", 1);
    display_text_in_line(hex_string_buffer, 2);
    display_text_in_line("Msg Descriptografada:", 3);
    display_text_in_line(decrypted, 4); // Exibe a string hexadecimal
}

int main()
{
    stdio_init_all();

    // Inicializa o display
    display_init();

    sleep_ms(5000);

    // Conecta ao Wi-Fi
    display_text_in_line("Conectando Wi-Fi...", 1);
    connect_to_wifi(WIFI_SSID, WIFI_PASSWORD);
    if (!wifi_comm_is_connected())
    {
        printf("Falha ao obter link da rede Wi-Fi. Abortando.\n");
        return -1;
    }
    printf("Link da rede Wi-Fi estabelecido.\n");
    display_text_in_line("Link estabecido!", 2);

    // Inicializa e conecta ao MQTT
    mqtt_setup(MQTT_CLIENT_ID_SUBSCRIBER, MQTT_BROKER_IP, MQTT_USER, MQTT_PASS);
    sleep_ms(1000);
    display_text_in_line("Conectando MQTT...", 1);

    printf("Aguardando conexao MQTT (3s)...\n"); // Tempo para o cliente MQTT conectar
    sleep_ms(3000);

    // Verifica se a conexão MQTT foi estabelecida
    if (!mqtt_comm_is_connected())
    {
        printf("Falha ao conectar ao broker MQTT. Abortando.\n");
        return -1;
    }
    printf("Conexão MQTT estabelecida.\n");
    display_text_in_line("MQTT Conectado!", 2);
    display_text_in_line(MQTT_CLIENT_ID_SUBSCRIBER, 3);
    sleep_ms(2000);

    // Define o handler para mensagens recebidas
    mqtt_comm_set_message_handler(on_message);

    // Inscreve no tópico de interesse
    mqtt_comm_subscribe(MQTT_TOPIC_SUBSCRIBE);

    printf("Aguardando mensagens no tópico: %s\n", MQTT_TOPIC_SUBSCRIBE);
    display_text_in_line("Inscrito no topico:", 1);
    display_text_in_line(MQTT_TOPIC_SUBSCRIBE, 1);

    // Loop principal – mantem o programa rodando
    while (1)
    {
        // Se necessário, adicione chamada à função de manutenção da lib MQTT
        // Exemplo: mqtt_yield(); ou mqtt_loop();
        sleep_ms(100);
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