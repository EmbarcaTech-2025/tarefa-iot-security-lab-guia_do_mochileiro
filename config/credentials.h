#ifndef CREDENTIALS_H
#define CREDENTIALS_H

// Credenciais da rede Wi-Fi

#define WIFI_SSID "IoT_Network_Test" // Nome da rede Wi-Fi       
#define WIFI_PASSWORD "132SookieIoT" // Senha da rede Wi-Fi

// Credenciais do Broker MQTT
#define MQTT_CLIENT_ID_PUBLISHER "bitdog_publisher"
#define MQTT_CLIENT_ID_SUBSCRIBER "bitdog_subscriber"

#define MQTT_BROKER_IP "164.152.59.111" // Mudar para o IP broker (Mosquitto)

#define MQTT_BROKER_PORT 1883
#define MQTT_USER "aluno"             
#define MQTT_PASS "senha123" 

#define MQTT_TOPIC_SUBSCRIBE "escola/sala1/temperatura"

// Chave XOR para criptografia/descriptografia
#define XOR_KEY 42

#endif // CREDENTIALS_H