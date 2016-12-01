#include "clock.h"
#include "gpio.h"
#include "uart.h"
#include <stm32f1xx.h>

typedef SystemClock<> Clock;

int main()
{
    Clock::init();
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_ADC1EN | RCC_APB2ENR_USART1EN; // Включим порт А, АЦП и USART

    auto a = GPIOA;
    Pins<IO_A, 0xF> pins;
    pins.init<IO_Analog>();

    Pins<IO_A, (1 << 9)> tx;
    int m = tx.getMask();
    tx.init<IO_AF0>();
    Pins<IO_A, (1 << 10)> rx;
    rx.init<IO_In>();
    Usart<Usart1> uart;
    uart.init<Clock, 256000>();
    USART_TypeDef u = uart.u();

    ADC1->CR1 = ADC_CR1_SCAN;
    ADC1->CR2 = ADC_CR2_CONT;
    ADC1->SQR1 = 4 << ADC_SQR1_L_Pos;
    ADC1->SQR3 = ADC_SQR3_SQ2_1 | ADC_SQR3_SQ3_2 | ADC_SQR3_SQ4_3;

    ADC1->CR2 |= ADC_CR2_ADON;

    while (true)
    {
        uart.send('H');
        //rx.clear();
        uart.send('e');
        //rx.set();
        uart.send('l');
        uart.send('l');
        uart.send('o');
        uart.send(13);
        uart.send(10);
        for(int x = 1000000; --x;);
    }
}
