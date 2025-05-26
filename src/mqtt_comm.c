#include "lwip/apps/mqtt.h"
#include "include/mqtt_comm.h"
#include "lwipopts.h"
#include "config/credentials.h"
#include <stdio.h>
#include <string.h>

static mqtt_client_t *client = NULL;
static mqtt_message_handler_t user_message_handler = NULL;

// --- Variáveis estáticas para payload buffer
static char topic_buffer[128]; // (Se quiser, pode preencher via publish_cb)
static uint8_t payload_buffer[256];
static size_t payload_len = 0;

// DECLARAÇÃO ANTECIPADA DO CALLBACK DE SUBSCRIBE
void mqtt_sub_request_cb(void *arg, err_t result);

/* Callback de recebimento de mensagem publicada (só printa o tópico) */
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    // Pode salvar o tópico em uma variável global se desejar, para usar no data_cb
    strncpy(topic_buffer, topic, sizeof(topic_buffer) - 1);
    topic_buffer[sizeof(topic_buffer) - 1] = '\0';
    payload_len = 0; // Sempre zera antes de começar a receber um novo payload!
    printf("Recebendo mensagem em tópico: %s (tamanho %ld)\n", topic, (long)tot_len);
}

/* Callback dos dados MQTT recebidos */
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    // Acumula os dados no buffer
    if (payload_len + len < sizeof(payload_buffer)) {
        memcpy(&payload_buffer[payload_len], data, len);
        payload_len += len;
    }
    // Quando termina a mensagem (MQTT_DATA_FLAG_LAST)
    if (flags & MQTT_DATA_FLAG_LAST) {
        payload_buffer[payload_len] = '\0';
        if (user_message_handler) {
            user_message_handler(topic_buffer, payload_buffer, payload_len);
        }
        payload_len = 0; // Reset para próxima mensagem!
    }
}

/* --- Inscrição em tópico --- */
void mqtt_comm_subscribe(const char *topic) {
    err_t err = mqtt_subscribe(client, topic, 0, mqtt_sub_request_cb, (void *)topic);
    if (err == ERR_OK) {
        printf("Inscrito no tópico: %s\n", topic);
    } else {
        printf("Falha ao se inscrever no tópico %s, código: %d\n", topic, err);
    }
}

/* Handler configurável para chegada de mensagem */
void mqtt_comm_set_message_handler(mqtt_message_handler_t handler) {
    user_message_handler = handler;
}

/* Callback de confirmação de inscrição (agora realmente implementada) */
void mqtt_sub_request_cb(void *arg, err_t result) {
    const char *topic = (const char *)arg;
    if (result == ERR_OK) {
        printf("Inscrição confirmada no tópico: %s\n", topic);
    } else {
        printf("Erro na inscrição do tópico %s, código: %d\n", topic, result);
    }
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("Conectado ao broker MQTT com sucesso!\n");
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);
    } else {
        printf("Falha ao conectar ao broker, código: %d\n", status);
    }
}

void mqtt_setup(const char *client_id, const char *broker_ip, const char *user, const char *pass) {
    ip_addr_t broker_addr;
    if (!ip4addr_aton(broker_ip, &broker_addr)) {
        printf("Erro no IP\n");
        return;
    }

    client = mqtt_client_new();
    if (client == NULL) {
        printf("Falha ao criar o cliente MQTT\n");
        return;
    }

    struct mqtt_connect_client_info_t ci = {
        .client_id = client_id,
        .client_user = user,
        .client_pass = pass
    };

    mqtt_client_connect(client, &broker_addr, MQTT_BROKER_PORT, mqtt_connection_cb, NULL, &ci);
}

static void mqtt_pub_request_cb(void *arg, err_t result) {
    if (result == ERR_OK) {
        printf("Publicação MQTT enviada com sucesso!\n");
    } else {
        printf("Erro ao publicar via MQTT: %d\n", result);
    }
}

void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len) {
    err_t status = mqtt_publish(
        client,
        topic,
        data,
        len,
        0,
        0,
        mqtt_pub_request_cb,
        NULL
    );

    if (status != ERR_OK) {
        printf("mqtt_publish falhou ao ser enviada: %d\n", status);
    }
}

int mqtt_comm_is_connected() {
    return mqtt_client_is_connected(client);
}
