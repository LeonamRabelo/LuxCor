#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "ws2812.pio.h"
#include "lib/ssd1306.h"
#include "lib/bh1750_light_sensor.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/matriz_leds.h"
#include "gy33.h"

#define I2C_SDA_DISPLAY 14    //Pino SDA - Dados para display SSD1306
#define I2C_SCL_DISPLAY 15    //Pino SCL - Clock para display SSD1306
#define LED_RED 13
#define LED_GREEN 12
#define LED_BLUE 11
#define WS2812_PIN 7  //Pino do WS2812
#define BUZZER_PIN 21 //Pino do buzzer
#define BOTAO_A 5       //Pino do botão A
#define IS_RGBW false //Maquina PIO para RGBW
#define LUX_MIN 100      // Limite de luminosidade em lux
#define RED_ALERT 200    // Limite para vermelho intenso

//Definições do sensor GY-33
#define GY33_I2C_ADDR 0x29 //Endereço do GY-33 no barramento I2C
#define I2C_PORT_COR i2c0
#define SDA_PIN 0
#define SCL_PIN 1

//Para o sensor de luz BH1750. Endereço 0x23
#define I2C_PORT_LUX i2c1               // i2c0 pinos 0 e 1, i2c1 pinos 2 e 3
#define I2C_SDA_LUX 2                   // 0 ou 2
#define I2C_SCL_LUX 3                   // 1 ou 3

//Variáveis globais
uint buzzer_slice;                   //Slice para o buzzer
ssd1306_t ssd;
volatile uint8_t tela_atual = 0;        //0 = GY-33, 1 = BH1750, etc
volatile uint32_t last_press_time = 0;  //Para debounce

void buzzer_beep(uint slice, int duracao_ms){
    pwm_set_enabled(slice, true);   // Liga o buzzer
    sleep_ms(duracao_ms);
    pwm_set_enabled(slice, false);  // Desliga
}

void botao_a_callback(uint gpio, uint32_t events){
    uint32_t agora = to_ms_since_boot(get_absolute_time());
    if(agora - last_press_time > 250){   // debounce de 250ms
        last_press_time = agora;
        tela_atual++;
        if(tela_atual > 1){ //verifica e alterna a tela
            tela_atual = 0;
        }
    }
}

void inicializar_componentes(){
    stdio_init_all();

    //Inicializa LED Vermelho
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_put(LED_RED, 0);
    //Inicializa LED Verde
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, 0);
    //Inicializa LED Azul
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(LED_BLUE, 0);

    //Inicializa o botão A com pull-up
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    //Interrupção no botão A
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &botao_a_callback);

    //Inicializa o pio
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    //Inicializa I2C para o display SSD1306
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(I2C_SDA_DISPLAY, GPIO_FUNC_I2C); // Dados
    gpio_set_function(I2C_SCL_DISPLAY, GPIO_FUNC_I2C); // Clock
    gpio_pull_up(I2C_SDA_DISPLAY);
    gpio_pull_up(I2C_SCL_DISPLAY);
    //Inicializa display
    ssd1306_init(&ssd, 128, 64, false, 0x3C, i2c1);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    //Inicializa buzzer
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    buzzer_slice = pwm_gpio_to_slice_num(BUZZER_PIN);              // Slice para o buzzer
    float clkdiv = 125.0f;                                         // Clock divisor
    uint16_t wrap = (uint16_t)((125000000 / (clkdiv * 1000)) - 1); // Valor do Wrap
    pwm_set_clkdiv(buzzer_slice, clkdiv);                          // Define o clock
    pwm_set_wrap(buzzer_slice, wrap);                              // Define o wrap
    pwm_set_gpio_level(BUZZER_PIN, wrap * 0.3f);                   // Define duty
    pwm_set_enabled(buzzer_slice, false);                          // Começa desligado

    //Inicializa o sensor de cor - GY33
    i2c_init(I2C_PORT_COR, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    printf("Iniciando GY-33...\n");
    gy33_init(I2C_PORT_COR);

    //Inicializa o sensor de luz BH1750
    bh1750_power_on(I2C_PORT_LUX);
}

int main(){
    inicializar_componentes();

    char str_red[5];   //Buffer para armazenar a string do sensor de cor e saturação
    char str_green[5]; 
    char str_blue[5];  
    char str_clear[5]; 
    char str_lux[10];  //Buffer para armazenar a string do sensor de luz

    while(true){
        //Leitura do sensor de cor
        uint16_t r, g, b, c;
        gy33_color_t color;
        gy33_read_color(I2C_PORT_COR, &color);
        r = color.r;
        g = color.g;
        b = color.b;

        //Leitura do sensor de Luz BH1750
        uint16_t lux = bh1750_read_measurement(I2C_PORT_LUX);
        printf("Lux = %d\n", lux);

        printf("Cor detectada - R: %d, G: %d, B: %d, Clear: %d\n", r, g, b, c);
        sprintf(str_red, "%d R", r);   // Converte o inteiro em string
        sprintf(str_green, "%d G", g); 
        sprintf(str_blue, "%d B", b);  
        sprintf(str_clear, "%d C", c);

        sprintf(str_lux, "%d Lux", lux);  // Converte o inteiro em string

        //Alerta com buzzer de pouca luz
        if(lux < LUX_MIN){
            buzzer_beep(buzzer_slice, 100);   // beep curto
        }
        //Alerta com buzzer de vermelho intenso
        if(r > RED_ALERT && r > g * 2 && r > b * 2){
            buzzer_beep(buzzer_slice, 500);   //beep longo
        }

        ssd1306_fill(&ssd, false);                          // Limpa o display
        if(tela_atual == 0){    //Tela 0 - Sensor de cor GY33
        //Atualiza o conteúdo do display com animações
        ssd1306_rect(&ssd, 3, 3, 122, 60, true, false);      // Desenha um retângulo
        ssd1306_line(&ssd, 3, 25, 123, 25, true);           // Desenha uma linha
        ssd1306_line(&ssd, 3, 37, 123, 37, true);           // Desenha uma linha
        ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6); // Desenha uma string
        ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);  // Desenha uma string
        ssd1306_draw_string(&ssd, "Cores   RGB", 10, 28);  // Desenha uma string
        ssd1306_line(&ssd, 63, 25, 63, 60, true);           // Desenha uma linha vertical
        ssd1306_draw_string(&ssd, str_red, 14, 41);        // Desenha uma string
        ssd1306_draw_string(&ssd, str_green, 14, 52);      // Desenha uma string
        ssd1306_draw_string(&ssd, str_blue, 73, 41);       // Desenha uma string
        ssd1306_draw_string(&ssd, str_clear, 73, 52);      // Desenha uma string
        }
        if(tela_atual == 1){    //Tela 1 - Sensor de luz GY302
        //Atualiza o conteúdo do display com animações
        ssd1306_rect(&ssd, 3, 3, 122, 60, true, false);       // Desenha um retângulo
        ssd1306_line(&ssd, 3, 25, 123, 25, true);            // Desenha uma linha
        ssd1306_line(&ssd, 3, 37, 123, 37, true);            // Desenha uma linha
        ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6);  // Desenha uma string
        ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);   // Desenha uma string
        ssd1306_draw_string(&ssd, "Sensor  BH1750", 10, 28);// Desenha uma string
        ssd1306_line(&ssd, 63, 25, 63, 37, true);            // Desenha uma linha vertical
        ssd1306_draw_string(&ssd, str_lux, 14, 41);         // Desenha uma string
        }

        ssd1306_send_data(&ssd);                           //Atualiza o display
        sleep_ms(500);
    }
}