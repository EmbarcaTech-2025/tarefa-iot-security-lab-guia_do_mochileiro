
# Tarefa: IoT Security Lab - EmbarcaTech 2025

Autor: **Danilo Oliveira e Tífany Severo**

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Brasília, Maio de 2025

---

## 🎯 Objetivo do Projeto

Este projeto tem como foco o aprendizado prático de conceitos de segurança em sistemas embarcados com comunicação via MQTT utilizando a Raspberry Pi Pico W. São explorados mecanismos de criptografia, autenticação e comunicação assíncrona no modelo publish/subscribe.
São implementados dois programas principais:
- **main_publisher.c**: Publica mensagens em um tópico MQTT específico.
- **main_subscriber.c**: Assina um tópico MQTT e processa as mensagens recebidas.

## 🔧 Componentes Utilizados

- Raspberry Pi Pico W (RP2040)
- Display OLED SSD1306 (I2C)
- Botões físicos
- Servidor MQTT (Mosquitto)

## 📌 Pinagem do Dispositivo

| Pino RP2040 | Função               | Conexão              |
|-------------|----------------------|-----------------------|
| GPIO 2      | Botão 1              | Entrada digital       |
| GPIO 3      | Botão 2              | Entrada digital       |
| GPIO 14     | SDA (I2C)            | Display OLED          |
| GPIO 15     | SCL (I2C)            | Display OLED          |


## ⚙️ Como Compilar e Executar

### Pré-requisitos

- Raspberry Pi Pico SDK configurado (`PICO_SDK_PATH`)
- CMake e GCC para ARM
- Visual Studio Code com extensões CMake Tools
- **Mosquitto (broker MQTT)** instalado

### Compilação

```bash
git clone https://github.com/seu_usuario/seu_repositorio.git
cd tarefa-iot-security-lab-guia_do_mochileiro
export PICO_SDK_PATH=/caminho/para/o/pico-sdk
mkdir build
cd build
cmake ..
make
```

### Execução




## 🔐 Segurança Implementada
- XOR Simples – Método didático de criptografia simétrica
- Timestamp – Validação temporal para evitar ataques de replay
- AES-GCM – Criptografia simétrica robusta com autenticação integrada
- HMAC-SHA256 – Autenticação de mensagens com chave secreta



## 📊 Resultados Esperados/Observados

- O sistema executou conforme o esperado. Foram realizados testes com múltiplos esquemas de criptografia, verificando:
- Integridade e confidencialidade de mensagens trocadas via MQTT.
- AES-GCM apresentou melhor desempenho em segurança e integridade.
- Sistema de publisher/subscriber implementado com sucesso via MQTT.
- A comunicação entre os dispositivos foi segura e eficiente, reforçando o aprendizado do modelo MQTT e das técnicas de segurança implementadas.



# 🧠 Técnicas Escaláveis em MQTT para Ambientes com Múltiplas BitDogLab

- Quais técnicas são escaláveis?
- Como aplicá-las com várias BitDogLab em rede escolar?

---

## ✅ Quais técnicas são escaláveis?

As seguintes técnicas foram avaliadas como **altamente escaláveis** ou **adequadas para ambientes com múltiplos dispositivos**:

- **Protocolo MQTT**: Leve e altamente escalável, ideal para redes com muitos dispositivos IoT.
- **Autenticação MQTT com usuário/senha**: Escalável até certo ponto, pode exigir gerenciamento centralizado.
- **Timestamps**: Altamente escaláveis, essenciais para prevenir ataques de repetição e ordenar eventos.
- **HMAC (Hash-based Message Authentication Code)**: Escalável para autenticação de mensagens.
- **Criptografia AES-GCM**: Escalável para proteção do payload com autenticação integrada.
- **OTA (Over-The-Air updates)**: Permite atualização remota de firmware, essencial para manutenção em larga escala.
- **Sincronização com NTP/RTC**: Escalável para manter a precisão de tempo mesmo sem internet.
- **Monitoramento do Broker**: Escalável para diagnóstico centralizado e controle da rede MQTT.

> ⚠️ A criptografia **XOR** foi considerada **não segura** e **não recomendada**, mesmo que seja leve.

---

## 🏫 Como aplicar em uma rede escolar com várias BitDogLab?

Para garantir segurança, escalabilidade e manutenção eficiente de múltiplas BitDogLab em uma rede escolar, recomenda-se a seguinte arquitetura e práticas:

1. **Broker Dedicado**
   - Utilize um broker MQTT exclusivo para a rede da escola (ex.: Mosquitto).
   - Permite controle total sobre tópicos, autenticação e segurança.

2. **Comunicação Segura com TLS**
   - Habilite **TLS** para criptografar a comunicação MQTT entre os dispositivos e o servidor.
   - Garante confidencialidade e integridade dos dados.

3. **Autenticação Forte**
   - Configure autenticação via usuário/senha ou com **certificados digitais**/**HMAC**.
   - Reduz o risco de acesso indevido.

4. **ACLs (Listas de Controle de Acesso)**
   - Defina regras para limitar o acesso de cada BitDogLab aos tópicos MQTT.
   - Melhora a segurança e evita interferência entre dispositivos.

5. **Criptografia de Payload com AES-GCM**
   - Proteja as mensagens com **AES-GCM**, garantindo confidencialidade e autenticação.

6. **Uso de Timestamps**
   - Adicione timestamps às mensagens para garantir validade temporal e evitar replay attacks.
   - Compatível com HMAC e AES-GCM.

7. **OTA (Over-The-Air Updates)**
   - Implemente atualizações remotas de firmware para facilitar manutenção contínua.

8. **Sincronização de Tempo (NTP/RTC)**
   - Utilize **NTP** para dispositivos com internet ou **RTC** para os que operam offline.

9. **Monitoramento do Broker**
   - Configure logs e métricas do broker (ex.: conexões, tópicos, falhas).
   - Ajuda na detecção de falhas e comportamentos suspeitos.

---

Com essa abordagem, a rede escolar poderá operar com **segurança**, **eficiência** e **escalabilidade**, mesmo com dezenas ou centenas de dispositivos BitDogLab conectados simultaneamente.


---

## 📜 Licença
GNU GPL-3.0.
