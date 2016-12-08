#include "clock.h"
#include "gpio.h"
#include "uart.h"
#include "timer.h"
#include <stm32f1xx.h>
#include "nvic.h"

typedef SystemClock<> Clock;

Pins<IO_A, 0xF> controlPins;
Pins<IO_A, (1 << 9)> txPin;
Pins<IO_A, (1 << 10)> rxPin;
Pins<IO_C, (1 << 9)> greenLed;
Pins<IO_C, (1 << 8)> blueLed;
Usart<Usart1> uart;
Timer<Timer2> timer;

const uint32_t top = 50;

void initialize()
{
    Clock::init();
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN
            | RCC_APB2ENR_ADC1EN | RCC_APB2ENR_USART1EN; // Включим порты А и C, АЦП и USART

    txPin.init<IO_AF0>();
    rxPin.init<IO_In>();
    greenLed.init<IO_Out>();
    blueLed.init<IO_Out>();

    uart.init<Clock, 115200>();

    timer.setPwmMode<0xF>();
    timer.ccr<0>(top / 2);
    timer.ccr<1>(top / 2);
    timer.ccr<2>(top / 2);
    timer.ccr<3>(top / 2);
    timer.enable(top - 1);

    setSysTick<Clock>(0.001);
}

void initializeADC()
{
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_ADCPRE) | RCC_CFGR_ADCPRE_DIV8;

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
}

enum { passive, halt, active } state = passive;

void setPassiveState()
{
    controlPins.init<IO_Analog>();
    greenLed.set();
    blueLed.clear();
    state = passive;
}

void setHaltState()
{
    timer.ccr<0>(top / 2);
    timer.ccr<1>(0);
    timer.ccr<2>(top / 2);
    timer.ccr<3>(top / 2);
    controlPins.init<IO_AF0>();
    greenLed.clear();
    blueLed.set();
    state = halt;
}

inline void setActiveState()
{
    controlPins.init<IO_AF0>();
    greenLed.set();
    blueLed.set();
    state = active;
}

int haltCounter;

void onCommand(const char * c)
{
    switch(*c)
    {
    case 'h': setHaltState(); break;
    case 'p': setPassiveState(); break;
    case 'a':
        {
            unsigned char outs[4], * op = outs;
            for(c++; op < outs + 4; c += 2)
            {
                uint32_t c1 = c[0] - '0', c2 = c[1] - '0';
                if(c1 > 9 || c2 > 9) return;
                c2 += c1 * 10;
                if(c2 > top) return;
                *(op++) = c2;
            }
            timer.ccr<0>(outs[0]);
            timer.ccr<1>(outs[1]);
            timer.ccr<2>(outs[2]);
            timer.ccr<3>(outs[3]);
            haltCounter = 100;
            if (state != active) setActiveState();
        }
    }
}

int main()
{
    /*auto rcc = RCC;
    auto a = GPIOA;
    auto adc = ADC1;
    auto tim = TIM2;
    USART_TypeDef u = uart.u();*/

    char data[16], *dp = nullptr;

    initialize();
    initializeADC();

    setPassiveState();
    while (true)
    {
        //if((ADC1->CR2 & ADC_CR2_JSWSTART)==0) ADC1->CR2 |= ADC_CR2_JSWSTART;
        if(uart.rxne()) switch(char c = uart.recv())
        {
        case '+': dp = data; break;
        case ';': if (dp) {
                *dp = 0;
                onCommand(data);
                dp = nullptr;
            } break;
        default:
            if(!dp) break;
            if (dp < data + sizeof(data) - 1) {
                *(dp++) = c;
            } else dp = 0;
        }
        if (state == active && sysTickFlag()) {
            if(haltCounter) haltCounter--;
            else setHaltState();
        }
    }
}
