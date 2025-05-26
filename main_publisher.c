// Bibliotecas necessárias
#include <string.h>             // Para funções de string como strlen()
#include "pico/stdlib.h"        // Biblioteca padrão do Pico (GPIO, tempo, etc.)
#include "pico/cyw43_arch.h"    // Driver WiFi para Pico W
#include "config/credentials.h" // Credenciais da rede WiFi e do broker MQTT
#include "wifi_conn.h"          // Funções personalizadas de conexão WiFi
#include "mqtt_comm.h"          // Funções personalizadas para MQTT
#include "xor_cipher.h"         // Funções de cifra XOR
#include "pico/time.h"


int main()
{
    // Inicializa todas as interfaces de I/O padrão (USB serial, etc.)
    stdio_init_all();

    // Aguarda inicialização do terminal serial
    sleep_ms(5000);

    // Conecta à rede WiFi
    // Parâmetros em credentials.h
    connect_to_wifi(WIFI_SSID, WIFI_PASSWORD);

    // Testa o link da conexão Wi-Fi
    if (!wifi_comm_is_connected())
    {
        printf("Falha ao obter link da rede Wi-Fi. Abortando.\n");
        return -1;
    }
    printf("Link da rede Wi-Fi estabelecido.\n");

    // Configura o cliente MQTT
    // Parâmetros em credentials.h
    mqtt_setup(MQTT_CLIENT_ID_PUBLISHER, MQTT_BROKER_IP, MQTT_USER, MQTT_PASS);    

    printf("Aguardando conexao MQTT (2s)...\n"); // Tempo para o cliente MQTT conectar
    sleep_ms(2000);

    // Verifica se a conexão MQTT foi estabelecida
    if (!mqtt_comm_is_connected())
    {
        printf("Falha ao conectar ao broker MQTT. Abortando.\n");
        return -1;
    }
    printf("Conexão MQTT estabelecida.\n");

    // Mensagem original a ser enviada
    //const char *mensagem = "26.5";
    // Buffer para mensagem criptografada (16 bytes)
    //uint8_t criptografada[16];
    // Criptografa a mensagem usando XOR com chave 42
    //xor_encrypt((uint8_t *)mensagem, criptografada, strlen(mensagem), XOR_KEY);

    // Loop principal do programa
    while (true)
    {

        // mensagem com timestamp
        uint64_t timestamp = to_us_since_boot(get_absolute_time());
        char mensagem[64];
        snprintf(mensagem, sizeof(mensagem), "26.5,%llu", timestamp);

        // Publica a mensagem original (não criptografada)
        //mqtt_comm_publish(MQTT_TOPIC_SUBSCRIBE, mensagem, strlen(mensagem));

        // Alternativa: Publica a mensagem criptografada

        uint8_t criptografada[64];
        xor_encrypt((uint8_t *)mensagem, criptografada, strlen(mensagem), XOR_KEY);

        mqtt_comm_publish(MQTT_TOPIC_SUBSCRIBE, criptografada, strlen(mensagem));
        printf("Mensagem original: %s\n", mensagem);
        printf("Mensagem criptografada (hex): ");
        for (size_t i = 0; i < strlen(mensagem); ++i) {
            printf("%02x", criptografada[i]);
        }
        printf("\n");


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