# TIC_TAC_TOE
O algoritmo implementa o Jogo da velha em um display ssd1306 128 x 64 utilizando o microcontrolador rp2040, faz uso também de LEDs que indicam o ganhador, além de botões e um joystick para interagir com o jogo

## Funcionalidades

- A tela inicial apresenta o jogo, ao apertar o botão do joystick segue para a Tela de instruções.
- A tela de instruções informa sobre os botões utilizados para jogar.
- Ao apertar o Joystick novamente segue para a tela de jogo.
- Caso o jogo de empate o led azul é aceso e segue para a tela de game over.
- Caso o jogador 'X' ganhe o led vermelho é aceso e segue para a tela de game over.
- Caso o jogador 'O' ganhe o led verde é aceso e segue para a tela de game over.
- Na tela de game over o botão ao apertar o botão do Joystick o jogo reinicia e retorna a tela de instruções.

## Hardware Necessário

- 1 Raspberry Pi Pico W (rp2040).
- 3 LEDs (Vermelho, Azul, Verde).
- Display ssd1306 128 x 64.
- Joystick.
- 2 Botões.
- Resistores apropriados para os LEDs.
- Fios de conexão.

## Pinagem

- Display SDA 14.
- Display SCL 15.
- Botão A 5.
- Botão B 6.
- Botão do Joystick 22.
- Joystick X 26.
- Joystick Y 27.
- LED Vermelho: Pino GPIO 13.
- LED Azul: Pino GPIO 12.
- LED Verde: Pino GPIO 11.

## Configuração do Ambiente

Antes de começar, certifique-se de que você tenha o ambiente de desenvolvimento do **Raspberry Pi Pico** configurado corretamente. Siga as instruções do [Raspberry Pi Pico SDK](https://www.raspberrypi.org/documentation/rp2040/getting-started/) para configurar o SDK e as ferramentas de compilação.
## Compilação e Upload

1. Compile o código usando o ambiente de desenvolvimento configurado.
2. Após a compilação, faça o upload do código para o seu **Raspberry Pi Pico**.

## Tecnologias Utilizadas

- **Conversor Analógico Digital**
- **Comunicação I2C**
- **C/C++**
- **Raspberry Pi Pico SDK**

## Contribuições

Contribuições são bem-vindas! Sinta-se à vontade para abrir um problema ou enviar um pull request.
