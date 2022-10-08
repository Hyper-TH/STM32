#include "stm32l031lib.h"
#include "realm.h"

#define CPU_FREQ 16000000

int main()
{
	initClock(); // sSet the system clock running at 16MHz
	RCC->IOPENR |= 2; // enable GPIOB
	RCC->IOPENR |= 1; // enable GPIOA
	
	pinMode(GPIOA,0,1); // Make PORTA Bit 0 an output
	pinMode(GPIOA,1,1); // Make PORTA Bit 1 an output
	initSound(); // get sound system going
	SysTick->LOAD = 16000;   // 16MHz / 16000 = 1kHz
	SysTick->CTRL = 7;       // enable systick counting and interrupts, use core clock
	enable_interrupts();
	GPIOA->ODR = 1;
	
	initSerial();
	
	while(1)
	{
		runGame();
	}
}
