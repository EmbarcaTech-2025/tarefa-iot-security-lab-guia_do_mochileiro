#ifndef CONFIG_H
#define CONFIG_H

// --- PINOS DOS PERIFÉRICOS ---
// Display OLED (Interface I2C)
#define I2C_SDA_PIN 14 ///< Pino SDA para comunicação I2C com o OLED.
#define I2C_SCL_PIN 15 ///< Pino SCL para comunicação I2C com o OLED.

// --- CONFIGURAÇÕES DO DISPLAY OLED ---
#define OLED_I2C_PORT i2c1               ///< Instância I2C utilizada para o OLED.
#define OLED_I2C_ADDRESS 0x3C            ///< Endereço I2C do display OLED.
#define OLED_I2C_FREQUENCY 400000        ///< Frequência de comunicação I2C (400kHz).
#define OLED_WIDTH 128                   ///< Largura do display OLED em pixels.
#define OLED_HEIGHT 64                   ///< Altura do display OLED em pixels.
#define OLED_LINE_HEIGHT 10              ///< Altura aproximada de uma linha de texto no OLED.

#endif // CONFIG_H
