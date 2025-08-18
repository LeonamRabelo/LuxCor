
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include "matriz_leds.h"

//Função para ligar um LED
static inline void put_pixel(uint32_t pixel_grb){
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

//Função para converter cores RGB para um valor de 32 bits
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b){
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

bool alertas[3][NUM_PIXELS] = {
//Verde, como se fosse um OK
{
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,      
    1, 0, 0, 0, 1,  
    1, 0, 0, 0, 1,   
    1, 1, 1, 1, 1
},
//Amarelo (Exclamação de atenção)
{
    0, 0, 1, 0, 0,      
    0, 0, 0, 0, 0, 
    0, 0, 1, 0, 0,   
    0, 0, 1, 0, 0,  
    0, 0, 1, 0, 0
},
//Vermelho, um X, alerta critico
{
    1, 0, 0, 0, 1,   
    0, 1, 0, 1, 0,  
    0, 0, 1, 0, 0,
    0, 1, 0, 1, 0,      
    1, 0, 0, 0, 1
}
};

//Função para envio dos dados para a matriz de leds
void set_one_led(uint8_t r, uint8_t g, uint8_t b, int numero){
    uint32_t color = urgb_u32(r, g, b);
    for(int i = 0; i < NUM_PIXELS; i++){
        if(alertas[numero][i]){
            put_pixel(color);
        } else {
            put_pixel(0);  // Desliga
        }
    }
}

// Função para atualizar a matriz de LEDs conforme sensores
void update_led_matrix_by_sensors(uint16_t r, uint16_t g, uint16_t b, uint16_t lux) {
    // Normaliza os valores RGB para 0-255
    uint16_t max_rgb = r;
    if (g > max_rgb) max_rgb = g;
    if (b > max_rgb) max_rgb = b;
    uint8_t R = max_rgb ? (r * 255) / max_rgb : 0;
    uint8_t G = max_rgb ? (g * 255) / max_rgb : 0;
    uint8_t B = max_rgb ? (b * 255) / max_rgb : 0;

    // Ajusta brilho conforme lux (0 = escuro, 255 = máximo)
    uint8_t brightness = lux > 1000 ? 255 : (lux * 255) / 1000;
    R = (R * brightness) / 255;
    G = (G * brightness) / 255;
    B = (B * brightness) / 255;

    // Atualiza todos os LEDs da matriz (todos acesos)
    set_one_led(R, G, B, 0);
}

// Função para identificar o nome da cor a partir de valores RGB
const char* get_color_name(uint16_t r, uint16_t g, uint16_t b) {
    // Normaliza para 0-255
    uint16_t max_rgb = r;
    if (g > max_rgb) max_rgb = g;
    if (b > max_rgb) max_rgb = b;
    uint8_t R = max_rgb ? (r * 255) / max_rgb : 0;
    uint8_t G = max_rgb ? (g * 255) / max_rgb : 0;
    uint8_t B = max_rgb ? (b * 255) / max_rgb : 0;

    // Limiares simples para cores básicas
    if (R > 200 && G < 80 && B < 80) return "Red";
    if (R < 80 && G > 200 && B < 80) return "Green";
    if (R < 80 && G < 80 && B > 200) return "Blue";
    if (R > 200 && G > 200 && B < 80) return "Yellow";
    if (R < 80 && G > 200 && B > 200) return "Cyan";
    if (R > 200 && G < 80 && B > 200) return "Magenta";
    if (R > 200 && G > 200 && B > 200) return "White";
    if (R < 50 && G < 50 && B < 50) return "Black";
    if (R > 150 && G > 80 && B < 80) return "Orange";
    if (R > 180 && G > 180 && B > 100 && B < 180) return "Gray";
    return "Unknown";
}