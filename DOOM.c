// Inclusão das Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"

// Definição dos Pinos
#define I2C_SDA 14        // SDA display
#define I2C_SCL 15        // SCL display
#define JOYSTICK_X_PIN 26 // eixo X
#define JOYSTICK_Y_PIN 27 // eixo Y
#define JOYSTICK_PB 22    // botão do Joystick
#define BUTTON_A 5        // botão A
#define BUTTON_B 6        // botão B
#define LED_PIN_GREEN 11  // led verde
#define LED_PIN_BLUE 12   // led azul
#define LED_PIN_RED 13    // led vermelho

// Constantes
#define I2C_PORT i2c1           // porta i2c
#define endereco 0x3C           // endereço da porta idc
#define WRAP_PERIOD 4096        // Periodo de wrap
#define ROW1 8                  // Posição da linha 1 do jogo da velha
#define ROW2 30                 // Posição da linha 2 do jogo da velha
#define ROW3 52                 // Posição da linha 3 do jogo da velha
#define COLUMN1 38              // Posição da Coluna 1 do jogo da velha
#define COLUMN2 61              // Posição da Coluna 2 do jogo da velha
#define COLUMN3 84              // Posição da Coluna 3 do jogo da velha
#define INIT_SCREEN 0           // Tela Inicial
#define INSTRUCTIONS_SCREEN 1   // Tela de Instruções
#define GAME_SCREEN 2           // Tela do Jogo
#define GAME_OVER_SCREEN 3      // Tela de Game Over
static uint8_t display_y[9] = { // Vetor de Posições y para Os pontos do jogo
    ROW1, ROW1, ROW1,
    ROW2, ROW2, ROW2,
    ROW3, ROW3, ROW3};
static uint8_t display_x[9] = { // Vetor de Posições x para Os pontos do jogo
    COLUMN1, COLUMN2, COLUMN3,
    COLUMN1, COLUMN2, COLUMN3,
    COLUMN1, COLUMN2, COLUMN3};

// Variáveis Globais
static volatile uint32_t last_time = 0; // Armazena o tempo da ultima vez que o botão foi apertado (em microssegundos)
static volatile uint screen = 0;        // Indica em qual tela o jogo está
static volatile uint selected_pos;      // Indica a posição Atual selecionada
static volatile uint endgame = 0;       // Indica de que forma o jogo Acabou
static volatile bool cor = true;        // Flag para o Display
static volatile char last_mark;         // Armazena o ultimo ponto marcado 'X' ou 'O'
static volatile char game[9] = {        // Vetor que armazena os pontos no jogo
    ' ', ' ', ' ',
    ' ', ' ', ' ',
    ' ', ' ', ' '};

// Protótipos das Funções
static void gpio_irq_handler(uint gpio, uint32_t events); // Trata a interrupção Ao apertar um botão
void init();                                              // inicializa o led verde e os botões
void tic_tac_toe();                                       // Verifica se e como o jogo acabou
void reinit_game();                                       // Reinicia o Jogo após o final da partida
void win_led();                                           // Acende um LED de acordo com o vencedor do Jogo
uint get_positon(uint16_t, uint16_t);                     // recebe a posição atual do seletor
uint16_t limit_position_x(uint16_t);                      // Limita as posições em X para as do Jogo da velha
uint16_t limit_position_y(uint16_t);                      // Limita as posições em Y para as do Jogo da velha

int main()
{
    // Variáveis
    uint16_t adc_value_x; // Armazena o valor digital de x
    uint16_t adc_value_y; // Armazena o valor digital de y
    uint16_t pos_x;       // Armazena a posição de X no display
    uint16_t pos_y;       // Armazena a posição de Y no display
    char point;           // Armazena os pontos marcados no jogo individualmente para printar no display

    // inicialização entrada e saída:
    stdio_init_all();

    // Inicialização dos Pinos:
    init();

    // Inicialização do I2C:
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados  do Buffer para o display                                      // Envia os dados para o display
    ssd1306_fill(&ssd, false);                                    // Limpa o display. O display inicia com todos os pixels apagados.

    // Inicialização ADC:
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    while (true)
    {
        switch (screen)
        {
        case INIT_SCREEN:
            ssd1306_fill(&ssd, !cor);                            // limpa o buffer do display
            ssd1306_draw_string(&ssd, "JOGO DA VELHA !", 2, 12); // Escreve no Buffer do Display
            ssd1306_draw_string(&ssd, "Pressione", 20, 42);      // Escreve no Buffer do Display
            ssd1306_draw_string(&ssd, "O Joystick", 16, 52);     // Escreve no Buffer do Display
            ssd1306_send_data(&ssd);                             // Envia os dados  do Buffer para o display
            break;
        case INSTRUCTIONS_SCREEN:
            ssd1306_fill(&ssd, !cor); // limpa o buffer do display
            ssd1306_draw_string(&ssd, "Intrucoes:", 20, 2);

            ssd1306_draw_string(&ssd, "Jogo em Dupla", 10, 22); // Escreve no Buffer do Display
            ssd1306_draw_string(&ssd, "Botao A -> X", 10, 32);  // Escreve no Buffer do Display
            ssd1306_draw_string(&ssd, "Botao B -> O", 10, 42);  // Escreve no Buffer do Display
            ssd1306_draw_string(&ssd, "Joystick Jogar", 2, 52); // Escreve no Buffer do Display
            ssd1306_send_data(&ssd);                            // Envia os dados  do Buffer para o display
            break;
        case GAME_SCREEN:
            gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Ativa a Rotina de Interrupção
            gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Ativa a Rotina de Interrupção
            adc_select_input(0);                                                                       // Seleciona o ADC para eixo X. O pino 26 como entrada analógica
            adc_value_x = adc_read();                                                                  // lê o valor de X
            adc_select_input(1);                                                                       // Seleciona o ADC para eixo Y. O pino 27 como entrada analógica
            adc_value_y = adc_read();                                                                  // lê o valor de Y

            pos_x = (((adc_value_x * -1) + 4095) / 64); // configura a posição X recebida do adc para se adequar ao display
            pos_y = (adc_value_y / 64) + 32;            // configura a posição Y recebida do adc para se adequar ao display

            pos_x = limit_position_x(pos_x);          // Limita as posições possiveis do display em X para as posições do jogo da velha
            pos_y = limit_position_y(pos_y);          // Limita as posições possiveis do display em Y para as posições do jogo da velha
            selected_pos = get_positon(pos_x, pos_y); // Recebe e armazena a posição atual do seletor

            ssd1306_fill(&ssd, !cor); // limpa o buffer do display

            // DESENHA O JOGO DA VELHA
            ssd1306_hline(&ssd, 32, 96, 20, cor);
            ssd1306_hline(&ssd, 32, 96, 44, cor);
            ssd1306_line(&ssd, 52, 0, 52, 63, cor);
            ssd1306_line(&ssd, 76, 0, 76, 63, cor);

            // Desenha o que foi jogado
            for (uint i = 0; i < 9; i++)
            {
                // Verifica os Caracteres recebidos da matriz do jogo
                switch (game[i])
                {
                case 'X':
                    point = 'X';
                    break;
                case ' ':
                    point = ' ';
                    break;
                case 'O':
                    point = 'O';
                    break;
                default:
                    printf("[ERRO]");
                    break;
                }
                ssd1306_draw_char(&ssd, point, display_x[i], display_y[i]); // Desenha os caracteres do Jogo
            }

            // Verifica se deu velha e em qual posição
            switch (endgame)
            {
            case 1:
                win_led();
                ssd1306_hline(&ssd, 32, 96, ROW1 + 4, cor); // Desenha uma linha horizontal na parte superior do jogo
                ssd1306_send_data(&ssd);                    // Envia os dados  do Buffer para o display
                sleep_ms(1500);                             // Aguarda 1,5 segundos antes de reiniciar o jogo
                reinit_game();                              // reinicia o jogo
                break;
            case 2:
                win_led();
                ssd1306_hline(&ssd, 32, 96, ROW2 + 4, cor); // Desenha uma linha horizontal na parte central do jogo
                ssd1306_send_data(&ssd);                    // Envia os dados  do Buffer para o display
                sleep_ms(1500);                             // Aguarda 1,5 segundos antes de reiniciar o jogo
                reinit_game();                              // reinicia o jogo
                break;
            case 3:
                win_led();
                ssd1306_hline(&ssd, 32, 96, ROW3 + 4, cor); // Desenha uma linha horizontal na parte inferior do jogo
                ssd1306_send_data(&ssd);                    // Envia os dados  do Buffer para o display
                sleep_ms(1500);                             // Aguarda 1,5 segundos antes de reiniciar o jogo
                reinit_game();                              // reinicia o jogo
                break;
            case 4:
                win_led();
                ssd1306_vline(&ssd, COLUMN1 + 4, 0, 63, cor); // Desenha uma linha vertical na parte esquerda do jogo
                ssd1306_send_data(&ssd);                      // Envia os dados  do Buffer para o display
                sleep_ms(1500);                               // Aguarda 1,5 segundos antes de reiniciar o jogo
                reinit_game();                                // reinicia o jogo
                break;
            case 5:
                win_led();
                ssd1306_vline(&ssd, COLUMN2 + 4, 0, 63, cor); // Desenha uma linha vertical na parte central do jogo
                ssd1306_send_data(&ssd);                      // Envia os dados  do Buffer para o display
                sleep_ms(1500);                               // Aguarda 1,5 segundos antes de reiniciar o jogo
                reinit_game();                                // reinicia o jogo
                break;
            case 6:
                win_led();
                ssd1306_vline(&ssd, COLUMN3 + 4, 0, 63, cor); // Desenha uma linha vertical na parte direita do jogo
                ssd1306_send_data(&ssd);                      // Envia os dados  do Buffer para o display
                sleep_ms(1500);                               // Aguarda 1,5 segundos antes de reiniciar o jogo
                reinit_game();                                // reinicia o jogo
                break;
            case 7:
                win_led();
                ssd1306_line(&ssd, COLUMN1 - 4, ROW1 - 4, COLUMN3 + 4, ROW3 + 4, cor); // Desenha uma linha na diagonal principal do jogo
                ssd1306_send_data(&ssd);                                               // Envia os dados  do Buffer para o display
                sleep_ms(1500);                                                        // Aguarda 1,5 segundos antes de reiniciar o jogo
                reinit_game();                                                         // reinicia o jogo
                break;
            case 8:
                win_led();
                ssd1306_line(&ssd, COLUMN1 - 4, ROW3 + 8, COLUMN3 + 8, ROW1 - 4, cor); // Desenha uma linha na diagonal secundária do jogo
                ssd1306_send_data(&ssd);                                               // Envia os dados  do Buffer para o display
                sleep_ms(1500);                                                        // Aguarda 1,5 segundos antes de reiniciar o jogo
                reinit_game();                                                         // reinicia o jogo
                break;
            case 9:
                win_led();
                ssd1306_send_data(&ssd); // Envia os dados  do Buffer para o display
                sleep_ms(1500);          // Aguarda 1,5 segundos antes de reiniciar o jogo
                reinit_game();           // reinicia o jogo
                break;
            default:
                printf("Jogo rolando\n");
                break;
            }

            ssd1306_rect(&ssd, pos_x, pos_y, 8, 8, cor, false); // gera o quadrado 8x8 no buffer
            ssd1306_send_data(&ssd);                            // Envia os dados  do Buffer para o display                            // envia os dados do buffer ao display
            sleep_ms(100);                                      // aguarda 100ms antes de atualizar

            break;
        case GAME_OVER_SCREEN:
            ssd1306_fill(&ssd, !cor);                         // limpa o buffer do display
            ssd1306_draw_string(&ssd, "GAME OVER !", 16, 15); // Escreve no Buffer do Display
            ssd1306_draw_string(&ssd, "Pressione o", 15, 32); // Escreve no Buffer do Display
            ssd1306_draw_string(&ssd, "Joystick p/", 15, 42); // Escreve no Buffer do Display
            ssd1306_draw_string(&ssd, "Recomecar", 25, 52);   // Escreve no Buffer do Display
            ssd1306_send_data(&ssd);                          // Envia os dados  do Buffer para o display
            break;
        default:
            printf("[ERRO]: TELA NÃO IDENTIFICADA\n"); // Flag para identificar possível erro
            break;
        }
    }
}

void init()
{
    // inicializa o botão do joystick e a interrupção
    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB);
    gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Rotina de Interrupção

    // inicializa o botão A e a interrupção
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, false, &gpio_irq_handler); // Rotina de Interrupção

    // inicializa o botão B e a interrupção
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, false, &gpio_irq_handler); // Rotina de Interrupção

    // inicializa o LED Verde
    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_put(LED_PIN_GREEN, 0);

    // inicializa o LED Azul
    gpio_init(LED_PIN_BLUE);
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);
    gpio_put(LED_PIN_BLUE, 0);

    // inicializa o LED Vermelho
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    gpio_put(LED_PIN_RED, 0);
}
static void gpio_irq_handler(uint gpio, uint32_t events)
{
    // Configura a ação ao apertar o botão e implementa o Debouce

    // Obtém o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - last_time > 200000) // 200 ms de debouncing
    {
        last_time = current_time; // Atualiza o tempo do último evento
        // Código Função:
        if (gpio == BUTTON_A)
        {
            // Verificar se o jogador "X" foi o ultimo a jogar, e se a posição selecionada está vaga
            if (last_mark != 'X' && game[selected_pos] == ' ')
            {
                // Caso seja uma jogada válida marca no jogo e verifica se "deu velha"

                game[selected_pos] = 'X'; // Marca a Jogada no game
                last_mark = 'X';          // Atualiza a ultima jogada
                tic_tac_toe();            // Verifica se "deu velha"
            }
        }
        else if (gpio == JOYSTICK_PB)
        {
            if (screen != GAME_SCREEN)
            {
                screen++; // Atualiza a tela
                // Caso já esteja na ultima tela retorna para a tela de instruções
                if (screen > GAME_OVER_SCREEN)
                    screen = INSTRUCTIONS_SCREEN;
            }
            else
            {
                // Permite passar de tela somente após o fim do jogo
                if (endgame != 0)
                    screen++;
            }
        }
        else if (gpio == BUTTON_B)
        {
            // Verificar se o jogador "O" foi o ultimo a jogar, e se a posição selecionada está vaga
            if (last_mark != 'O' && game[selected_pos] == ' ')
            {
                // Caso seja uma jogada válida marca no jogo e verifica se "deu velha"

                game[selected_pos] = 'O'; // Marca a Jogada no game
                last_mark = 'O';          // Atualiza a ultima jogada
                tic_tac_toe();            // Verifica se "deu velha"
            }
        }
    }
}
void tic_tac_toe()
{
    bool Draw = true;
    char p1 = game[0], p2 = game[1], p3 = game[2];
    char p4 = game[3], p5 = game[4], p6 = game[5];
    char p7 = game[6], p8 = game[7], p9 = game[8];

    for (uint i = 0; i < 9; i++)
    {
        if (game[i] == ' ')
            Draw = false;
    }

    if (p1 == p2 && p2 == p3 && p3 != ' ')
    {
        endgame = 1;
    }
    else if (p4 == p5 && p5 == p6 && p6 != ' ')
    {
        endgame = 2;
    }
    else if (p7 == p8 && p8 == p9 && p9 != ' ')
    {
        endgame = 3;
    }
    else if (p1 == p4 && p4 == p7 && p7 != ' ')
    {
        endgame = 4;
    }
    else if (p2 == p5 && p5 == p8 && p8 != ' ')
    {
        endgame = 5;
    }
    else if (p3 == p6 && p6 == p9 && p9 != ' ')
    {
        endgame = 6;
    }
    else if (p1 == p5 && p5 == p9 && p9 != ' ')
    {
        endgame = 7;
    }
    else if (p3 == p5 && p5 == p7 && p7 != ' ')
    {
        endgame = 8;
    }
    else if (Draw)
    {
        endgame = 9;
    }
}
uint get_positon(uint16_t x, uint16_t y)
{
    // Recebe os Valores analógicos de X e Y e retorna a posição
    uint pos;
    switch (x)
    {
    case 8:
        if (y == 38)
            pos = 0;
        else if (y == 61)
            pos = 1;
        else if (y == 84)
            pos = 2;
        break;
    case 30:
        if (y == 38)
            pos = 3;
        else if (y == 61)
            pos = 4;
        else if (y == 84)
            pos = 5;
        break;
    case 52:
        if (y == 38)
            pos = 6;
        else if (y == 61)
            pos = 7;
        else if (y == 84)
            pos = 8;
        break;
    default:
        printf("[ERRO]: Posicao indeterminada");
        break;
    }

    return pos;
}
void reinit_game()
{
    // Reinicia o Jogo após ser encerrado
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, false, &gpio_irq_handler); // Desativa a Rotina de Interrupção
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, false, &gpio_irq_handler); // Desativa a Rotina de Interrupção
    gpio_put(LED_PIN_GREEN, false);                                                             // Apaga o LED Verde
    gpio_put(LED_PIN_BLUE, false);                                                              // Apaga o LED Azul
    gpio_put(LED_PIN_RED, false);                                                               // Apaga o LED Vermelho
    last_mark = ' ';                                                                            // Zera a última jogada
    endgame = 0;                                                                                // Zera o endgame
    for (uint i = 0; i < 9; i++)                                                                // Zera todas as posições da Matriz
        game[i] = ' ';
    screen++; // Atualiza a Tela
}
uint16_t limit_position_x(uint16_t x)
{
    // Limita as posições em X no Joystick para as linhas do Jogo
    if (x > 20)
    {
        if (x > 44)
            x = ROW3;
        else
            x = ROW2;
    }
    else
        x = ROW1;
    return x;
}
uint16_t limit_position_y(uint16_t y)
{
    // Limita as posições em Y no Joystick para as Colunas do Jogo
    if (y < 90)
    {
        if (y < 52)
            y = COLUMN1;
        else
            y = COLUMN2;
    }
    else
        y = COLUMN3;
    return y;
}
void win_led()
{
    // Se der empate -> LED Azul Aceso
    // Se o "X" Ganhar -> LED Vermelho Aceso
    // Se o "O" Ganhar -> LED Verde Aceso
    if (endgame == 9)
    {
        gpio_put(LED_PIN_BLUE, true);
    }
    else if (last_mark == 'X')
    {
        gpio_put(LED_PIN_RED, true);
    }
    else if (last_mark == 'O')
    {
        gpio_put(LED_PIN_GREEN, true);
    }
}