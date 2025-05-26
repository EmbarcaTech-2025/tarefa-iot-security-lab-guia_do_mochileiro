/*
MIT License

Copyright (c) 2021 David Schramm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/** 
* @arquivo ssd1306.h
* 
* driver simples para displays ssd1306
*/

#ifndef _inc_ssd1306
#define _inc_ssd1306
#include <pico/stdlib.h>
#include <hardware/i2c.h>

/**
*	@brief define comandos usados no ssd1306
*/
typedef enum {
    SET_CONTRAST = 0x81,
    SET_ENTIRE_ON = 0xA4,
    SET_NORM_INV = 0xA6,
    SET_DISP = 0xAE,
    SET_MEM_ADDR = 0x20,
    SET_COL_ADDR = 0x21,
    SET_PAGE_ADDR = 0x22,
    SET_DISP_START_LINE = 0x40,
    SET_SEG_REMAP = 0xA0,
    SET_MUX_RATIO = 0xA8,
    SET_COM_OUT_DIR = 0xC0,
    SET_DISP_OFFSET = 0xD3,
    SET_COM_PIN_CFG = 0xDA,
    SET_DISP_CLK_DIV = 0xD5,
    SET_PRECHARGE = 0xD9,
    SET_VCOM_DESEL = 0xDB,
    SET_CHARGE_PUMP = 0x8D
} ssd1306_command_t;

/**
*	@brief armazena a configuração
*/
typedef struct {
    uint8_t width; 		/**< largura do display */
    uint8_t height; 	/**< altura do display */
    uint8_t pages;		/**< armazena páginas do display (calculado na inicialização)*/
    uint8_t address; 	/**< endereço i2c do display*/
    i2c_inst_t *i2c_i; 	/**< instância da conexão i2c */
    bool external_vcc; 	/**< se o display usa vcc externo */ 
    uint8_t *buffer;	/**< buffer do display */
    size_t bufsize;		/**< tamanho do buffer */
} ssd1306_t;

/**
*	@brief inicializa o display
*
*	@param[in] p : ponteiro para a instância de ssd1306_t
*	@param[in] width : largura do display
*	@param[in] height : altura do display
*	@param[in] address : endereço i2c do display
*	@param[in] i2c_instance : instância da conexão i2c
*	
* 	@return bool.
*	@retval true para Sucesso
*	@retval false se a inicialização falhou
*/
bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height, uint8_t address, i2c_inst_t *i2c_instance);

/**
*	@brief desinicializa o display
*
*	@param[in] p : instância do display
*
*/
void ssd1306_deinit(ssd1306_t *p);

/**
*	@brief desliga o display
*
*	@param[in] p : instância do display
*
*/
void ssd1306_poweroff(ssd1306_t *p);

/**
	@brief liga o display

	@param[in] p : instância do display

*/
void ssd1306_poweron(ssd1306_t *p);

/**
	@brief define o contraste do display

	@param[in] p : instância do display
	@param[in] val : contraste

*/
void ssd1306_contrast(ssd1306_t *p, uint8_t val);

/**
	@brief define a inversão do display

	@param[in] p : instância do display
	@param[in] inv : inv==0: desabilita inversão, inv!=0: inverte

*/
void ssd1306_invert(ssd1306_t *p, uint8_t inv);

/**
	@brief buffer do display, deve ser chamado na alteração

	@param[in] p : instância do display

*/
void ssd1306_show(ssd1306_t *p);

/**
	@brief limpa o buffer do display

	@param[in] p : instância do display

*/
void ssd1306_clear(ssd1306_t *p);

/**
	@brief limpa pixel no buffer

	@param[in] p : instância do display
	@param[in] x : posição x
	@param[in] y : posição y
*/
void ssd1306_clear_pixel(ssd1306_t *p, uint32_t x, uint32_t y);

/**
	@brief desenha pixel no buffer

	@param[in] p : instância do display
	@param[in] x : posição x
	@param[in] y : posição y
*/
void ssd1306_draw_pixel(ssd1306_t *p, uint32_t x, uint32_t y);

/**
	@brief desenha linha no buffer

	@param[in] p : instância do display
	@param[in] x1 : posição x do ponto inicial
	@param[in] y1 : posição y do ponto inicial
	@param[in] x2 : posição x do ponto final
	@param[in] y2 : posição y do ponto final
*/
void ssd1306_draw_line(ssd1306_t *p, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

/**
	@brief limpa quadrado na posição dada com o tamanho dado

	@param[in] p : instância do display
	@param[in] x : posição x do ponto inicial
	@param[in] y : posição y do ponto inicial
	@param[in] width : largura do quadrado
	@param[in] height : altura do quadrado
*/
void ssd1306_clear_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

/**
	@brief desenha quadrado preenchido na posição dada com o tamanho dado

	@param[in] p : instância do display
	@param[in] x : posição x do ponto inicial
	@param[in] y : posição y do ponto inicial
	@param[in] width : largura do quadrado
	@param[in] height : altura do quadrado
*/
void ssd1306_draw_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

/**
	@brief desenha quadrado vazio na posição dada com o tamanho dado

	@param[in] p : instância do display
	@param[in] x : posição x do ponto inicial
	@param[in] y : posição y do ponto inicial
	@param[in] width : largura do quadrado
	@param[in] height : altura do quadrado
*/
void ssd1306_draw_empty_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

/**
	@brief desenha bitmap monocromático com deslocamento

	@param[in] p : instância do display
	@param[in] data : dados da imagem (arquivo inteiro)
	@param[in] size : tamanho dos dados da imagem em bytes
	@param[in] x_offset : deslocamento da coordenada horizontal
	@param[in] y_offset : deslocamento da coordenada vertical
*/
void ssd1306_bmp_show_image_with_offset(ssd1306_t *p, const uint8_t *data, const long size, uint32_t x_offset, uint32_t y_offset);

/**
	@brief desenha bitmap monocromático

	@param[in] p : instância do display
	@param[in] data : dados da imagem (arquivo inteiro)
	@param[in] size : tamanho dos dados da imagem em bytes
*/
void ssd1306_bmp_show_image(ssd1306_t *p, const uint8_t *data, const long size);

/**
	@brief desenha caractere com a fonte dada

	@param[in] p : instância do display
	@param[in] x : posição x inicial do caractere
	@param[in] y : posição y inicial do caractere
	@param[in] scale : escala a fonte para n vezes o tamanho original (padrão deve ser 1)
	@param[in] font : ponteiro para a fonte
	@param[in] c : caractere a ser desenhado
*/
void ssd1306_draw_char_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, char c);

/**
	@brief desenha caractere com a fonte embutida

	@param[in] p : instância do display
	@param[in] x : posição x inicial do caractere
	@param[in] y : posição y inicial do caractere
	@param[in] scale : escala a fonte para n vezes o tamanho original (padrão deve ser 1)
	@param[in] c : caractere a ser desenhado
*/
void ssd1306_draw_char(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, char c);

/**
	@brief desenha string com a fonte dada

	@param[in] p : instância do display
	@param[in] x : posição x inicial do texto
	@param[in] y : posição y inicial do texto
	@param[in] scale : escala a fonte para n vezes o tamanho original (padrão deve ser 1)
	@param[in] font : ponteiro para a fonte
	@param[in] s : texto a ser desenhado
*/
void ssd1306_draw_string_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, const char *s );

/**
	@brief desenha string com a fonte embutida

	@param[in] p : instância do display
	@param[in] x : posição x inicial do texto
	@param[in] y : posição y inicial do texto
	@param[in] scale : escala a fonte para n vezes o tamanho original (padrão deve ser 1)
	@param[in] s : texto a ser desenhado
*/
void ssd1306_draw_string(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const char *s);

#endif