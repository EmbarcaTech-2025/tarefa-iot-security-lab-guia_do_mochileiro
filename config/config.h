#ifndef CONFIG_H
#define CONFIG_H

// --- PINOS DOS PERIFÉRICOS ---
// Display OLED (Interface I2C)
#define I2C_SDA_PIN 14 ///< Pino SDA para comunicação I2C com o OLED.
#define I2C_SCL_PIN 15 ///< Pino SCL para comunicação I2C com o OLED.

// Botões de Ação (A e B)
#define BTN_A_PIN 5 ///< Pino GPIO para o Botão A (geralmente Gravar).
#define BTN_B_PIN 6 ///< Pino GPIO para o Botão B (geralmente Reproduzir).

// Joystick (Analógico para Eixo Y, Digital para Botão)
#define JOYSTICK_VRY_PIN 26 ///< Pino GPIO para o eixo Y do joystick (ADC0).
#define JOYSTICK_VRX_PIN 27 ///< Pino GPIO para o eixo X do joystick (ADC1)
#define JOYSTICK_BTN_PIN 22 ///< Pino GPIO para o botão do joystick.

// LEDs RGB (Indicadores Visuais)
#define LED_VERMELHO_PIN 13 ///< Pino GPIO para o componente Vermelho do LED RGB.
#define LED_VERDE_PIN 11    ///< Pino GPIO para o componente Verde do LED RGB.
#define LED_AZUL_PIN 12     ///< Pino GPIO para o componente Azul do LED RGB.

// --- CONFIGURAÇÕES DE ENTRADA ---
#define BTN_DEBOUNCE_TIME_MS 200      ///< Tempo (ms) para debounce dos botões A e B.
#define ADC_JOYSTICK_Y_CHANNEL 0      ///< Canal ADC para o eixo Y do joystick.
// Limiares para detecção de movimento do joystick no eixo Y (valores ADC de 12 bits: 0-4095)
#define JOYSTICK_Y_MOVE_UP_THRESHOLD 3000     ///< Limiar ADC para movimento "para cima" do joystick.
#define JOYSTICK_Y_MOVE_DOWN_THRESHOLD 1000   ///< Limiar ADC para movimento "para baixo" do joystick.
#define JOYSTICK_Y_NEUTRAL_DEADZONE_HIGH 2500 ///< Limiar superior da zona morta central do joystick.
#define JOYSTICK_Y_NEUTRAL_DEADZONE_LOW 1500  ///< Limiar inferior da zona morta central do joystick.

// --- CONFIGURAÇÕES DO DISPLAY OLED ---
#define OLED_I2C_PORT i2c1               ///< Instância I2C utilizada para o OLED.
#define OLED_I2C_ADDRESS 0x3C            ///< Endereço I2C do display OLED.
#define OLED_I2C_FREQUENCY 400000        ///< Frequência de comunicação I2C (400kHz).
#define OLED_WIDTH 128                   ///< Largura do display OLED em pixels.
#define OLED_HEIGHT 64                   ///< Altura do display OLED em pixels.
#define OLED_LINE_HEIGHT 10              ///< Altura aproximada de uma linha de texto no OLED.

#endif // CONFIG_H
