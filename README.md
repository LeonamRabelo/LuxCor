
# Projeto: Sistema de Monitoramento de Cor e Luminosidade com Matriz de LEDs

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)
![Raspberry Pi](https://img.shields.io/badge/-Raspberry_Pi-C51A4A?style=for-the-badge&logo=Raspberry-Pi)
![GitHub](https://img.shields.io/badge/github-%23121011.svg?style=for-the-badge&logo=github&logoColor=white)
![Windows 11](https://img.shields.io/badge/Windows%2011-%230079d5.svg?style=for-the-badge&logo=Windows%2011&logoColor=white)

## Descrição do Projeto

Este projeto utiliza um Raspberry Pi Pico para monitorar cor e luminosidade do ambiente usando sensores GY-33 (cor) e BH1750 (luminosidade). Os dados capturados são utilizados para:

- Controlar uma matriz de LEDs WS2812B, refletindo a cor dominante e o brilho proporcional à luminosidade.
- Exibir no display OLED SSD1306 o nome da cor detectada e o nível de luminosidade (alto, médio, baixo).
- Emitir alertas sonoros via buzzer em situações de pouca luz ou cor vermelha intensa.

## Componentes Utilizados

- **Raspberry Pi Pico (RP2040)**
- **Sensor de cor GY-33** (I2C)
- **Sensor de luminosidade BH1750** (I2C)
- **Display OLED SSD1306** (I2C)
- **Matriz de LEDs WS2812B** (GPIO 7)
- **Buzzer** (GPIO 21)
- **LEDs RGB** (GPIOs 11, 12, 13)
- **Botão A** (GPIO 5)

## Funcionalidades

- Leitura contínua dos sensores de cor e luminosidade
- Exibição do nome da cor detectada (em inglês) no display OLED
- Exibição do nível de luminosidade ("Low", "Medium", "High") no display OLED
- Controle da matriz de LEDs para refletir cor e brilho do ambiente
- Alertas sonoros automáticos para pouca luz ou cor vermelha intensa

## Como Compilar e Executar

1. Clone este repositório.
2. Abra o projeto no VS Code com a extensão da Raspberry Pi.
3. Compile usando a extensão (ou via CMake/Ninja).
4. Envie o arquivo `.uf2` para o Raspberry Pi Pico com o botão `bootsel` pressionado.
5. O sistema iniciará automaticamente, exibindo as leituras no display e controlando a matriz de LEDs.

## Testes e Validação

- Verifique se a matriz de LEDs muda de cor e brilho conforme a luz e cor do ambiente.
- Confira se o display mostra corretamente o nome da cor e o nível de luminosidade.
- Teste os alertas sonoros cobrindo o sensor de luz ou aproximando uma cor vermelha intensa do sensor de cor.

