#ifndef MQTT_COMM_H
#define MQTT_COMM_H

#include <stddef.h>
#include <stdint.h>

// Tipo de função callback para tratamento de mensagens recebidas
typedef void (*mqtt_message_handler_t)(const char *topic, const uint8_t *payload, size_t len);

/**
 * Inicializa e conecta o cliente MQTT.
 * @param client_id  ID do cliente MQTT
 * @param broker_ip  Endereço IP do broker (ex: "192.168.1.1")
 * @param user       Usuário para autenticação (pode ser NULL)
 * @param pass       Senha para autenticação (pode ser NULL)
 */
void mqtt_setup(const char *client_id, const char *broker_ip, const char *user, const char *pass);

/**
 * Publica mensagem em um tópico.
 * @param topic  Nome do tópico
 * @param data   Payload (array de bytes)
 * @param len    Tamanho do payload
 */
void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len);

/**
 * Verifica se o cliente está conectado ao broker.
 * @return 1 se conectado, 0 caso contrário
 */
int mqtt_comm_is_connected(void);

/**
 * Inscreve o cliente em um tópico MQTT.
 * @param topic  Nome do tópico a ser assinado
 */
void mqtt_comm_subscribe(const char *topic);

/**
 * Registra uma função de callback para tratar mensagens recebidas.
 * @param handler  Ponteiro para a função de tratamento de mensagem
 */
void mqtt_comm_set_message_handler(mqtt_message_handler_t handler);

#endif // MQTT_COMM_H
