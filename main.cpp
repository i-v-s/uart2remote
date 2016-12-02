#include "clock.h"
#include "gpio.h"
#include "uart.h"
#include "timer.h"
#include <stm32f1xx.h>

typedef SystemClock<> Clock;

int main()
{
    Clock::init();
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_ADC1EN | RCC_APB2ENR_USART1EN; // Включим порт А, АЦП и USART

    auto a = GPIOA;
    Pins<IO_A, 0xF> pins;
    //pins.init<IO_Analog>();

    Pins<IO_A, (1 << 9)> tx;
    int m = tx.getMask();
    tx.init<IO_AF0>();
    Pins<IO_A, (1 << 10)> rx;
    rx.init<IO_In>();
    Usart<Usart1> uart;
    uart.init<Clock, 256000>();
    USART_TypeDef u = uart.u();

    Timer<Timer2> timer;

    RCC->CFGR = RCC_CFGR_ADCPRE_DIV8;

    ADC1->CR1= 0;//ADC_CR1_JDISCEN|ADC_CR1_JAUTO;
    ADC1->CR2= ADC_CR2_EXTSEL | ADC_CR2_JEXTSEL | ADC_CR2_EXTTRIG | ADC_CR2_JEXTTRIG | ADC_CR2_CONT;
    ADC1->SMPR2=ADC_SMPR2_SMP4_1|ADC_SMPR2_SMP5_1;
    ADC1->JSQR=ADC_JSQR_JL_0 | ADC_JSQR_JSQ3_2 | ADC_JSQR_JSQ4_2 | ADC_JSQR_JSQ4_0;
    ADC1->CR2 |=ADC_CR2_ADON;
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while ((ADC1->CR2 & ADC_CR2_RSTCAL) == ADC_CR2_RSTCAL)
    {
    }
    ADC1->CR2 |= ADC_CR2_CAL;
    while ((ADC1->CR2 & ADC_CR2_RSTCAL) == ADC_CR2_CAL)
    {
    }
    ADC1->CR2 |= ADC_CR2_JSWSTART;
    auto adc = ADC1;

    pins.init<IO_AF0>();
    timer.setPwmMode<0xF>();
    timer.ccr()[0] = 30;
    timer.ccr()[1] = 30;
    timer.ccr()[2] = 30;
    timer.ccr()[3] = 30;
    timer.enable(64);
    auto tim = TIM2;
    while (true)
    {
        //if((ADC1->CR2 & ADC_CR2_JSWSTART)==0) ADC1->CR2 |= ADC_CR2_JSWSTART;
        /*uart.send('H');
        //rx.clear();
        uart.send('e');
        //rx.*set();
        uart.send('l');
        uart.send('l');
        uart.send('o');
        uart.send(13);
        uart.send(10);*/

        //uart.sendSync(ADC1->JDR1);
        uint32_t c = uart.recvSync();
        timer.ccr()[(c >> 6) & 3] = c & 63;

        //for(int x = 1000000; --x;);


    }
}
