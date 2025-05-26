// Bibliotecas necessárias
#include <string.h>             // Para funções de string como strlen()
#include "pico/stdlib.h"        // Biblioteca padrão do Pico (GPIO, tempo, etc.)
#include "pico/cyw43_arch.h"    // Driver WiFi para Pico W
#include "config/credentials.h" // Credenciais da rede WiFi e do broker MQTT
#include "wifi_conn.h"          // Funções personalizadas de conexão WiFi
#include "mqtt_comm.h"          // Funções personalizadas para MQTT
#include "xor_cipher.h"         // Funções de cifra XOR


// Buffer para descriptografia
#define PAYLOAD_MAX_LEN 256
uint8_t decrypted[PAYLOAD_MAX_LEN];

static uint64_t last_timestamp = 0; 

// Função handler que será chamada ao receber mensagem MQTT
void on_message(const char *topic, const uint8_t *payload, size_t len) {


    // Espera payload no formato "valor,timestamp"
    char valor[32];
    uint64_t timestamp = 0;
    //sscanf((char *)decrypted, "%31[^,],%llu", valor, &timestamp);

    // sem criptografia:
    char mensagem[64];
    memcpy(mensagem, payload, len);
    mensagem[len] = '\0';

    // Espera formato "valor,timestamp"
    sscanf(mensagem, "%31[^,],%llu", valor, &timestamp);

    static uint64_t last_timestamp = 0;
    if (timestamp > last_timestamp) {
        printf("Mensagem NOVA recebida: valor=%s, timestamp=%llu\n", valor, timestamp);
        last_timestamp = timestamp;
    } else {
        printf("Replay detectado! Ignorando mensagem: valor=%s, timestamp=%llu\n", valor, timestamp);
    }

    printf("Mensagem criptografada recebida no tópico [%s] (hex): ", topic);
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", payload[i]);
    }
    printf("\n");

    // Descriptografa
    memcpy(decrypted, payload, len);
    xor_encrypt(payload, decrypted, len, XOR_KEY); // XOR_KEY deve ser definido em xor_encrypt.h ou credentials.h
    decrypted[len] = '\0'; // Garante terminação caso texto
    

    printf("Mensagem recebida no tópico [%s]: %s\n", topic, decrypted);

}

int main() {
    stdio_init_all();
    sleep_ms(5000);

    // Conecta ao Wi-Fi
    connect_to_wifi(WIFI_SSID, WIFI_PASSWORD);
    if (!wifi_comm_is_connected()) {
        printf("Falha ao obter link da rede Wi-Fi. Abortando.\n");
        return -1;
    }
    printf("Link da rede Wi-Fi estabelecido.\n");

    // Inicializa e conecta ao MQTT
    mqtt_setup(MQTT_CLIENT_ID_SUBSCRIBER, MQTT_BROKER_IP, MQTT_USER, MQTT_PASS);

    // Define o handler para mensagens recebidas
    mqtt_comm_set_message_handler(on_message);

    // Inscreve no tópico de interesse
    mqtt_comm_subscribe(MQTT_TOPIC_SUBSCRIBE);

    printf("Aguardando mensagens no tópico: %s\n", MQTT_TOPIC_SUBSCRIBE);

    // Loop principal – mantem o programa rodando
    while (1) {
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