#define RCC_BASE 0x40023800UL //Reset and Clock Control Base Address
#define GPIOA_BASE 0x40020000UL //GPIO Port A Base Address

#define RCC_AHB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x30UL)) //RCC Register that enables clocks for AHB1 peripherals
#define GPIOA_MODER (*(volatile unsigned int *)(GPIOA_BASE + 0x00UL)) //GPIOA pin mode register
#define GPIOA_ODR (*(volatile unsigned int *)(GPIOA_BASE + 0x14UL)) //GPIOA output data register

#define GPIOAEN (1U << 0) //GPIOA peripheral clock enable bit
#define MODER5CLEAR ~(3U << 10) //Clear mask for PA5 mode bits in GPIOA_MODER
#define MODER5OUT (1U << 10) //Set bits for PA5 mode to output in GPIOA_MODER

#define LD2_PIN (1U << 5) //PA5 offset in GPIOA_ODR set to 1

#define SYSTICK_BASE 0xE000E010UL //SysTick base address SysTick is for accurate and non blocking timing

#define STK_CTRL (*(volatile unsigned int *)(SYSTICK_BASE + 0x00UL))
#define STK_LOAD (*(volatile unsigned int *)(SYSTICK_BASE + 0x04UL))
#define STK_VAL (*(volatile unsigned int *)(SYSTICK_BASE + 0x08UL))

#define SYST_CSR_ENABLE (1 << 0) //Enable the counter
#define SYST_CSR_TICKINT (1 << 1) //When counter goes to zero it asserts the SysTick exception request
#define SYST_CSR_CLICKSOURCE (1 << 2) //Clock source is the processor

#define CPU_CLOCK_HZ 16000000U
#define SYSTICK_HZ 1000U //How many times a second should we interrupt

volatile unsigned int GlobalMilliseconds = 0;


void SysTick_Handler()
{
	GlobalMilliseconds++;
}

static void SysTick_Init_1ms()
{
	STK_LOAD = (CPU_CLOCK_HZ / SYSTICK_HZ) - 1U;  // How many cpu ticks for one interrupt
	STK_VAL = 0U;
	STK_CTRL = SYST_CSR_ENABLE | SYST_CSR_TICKINT | SYST_CSR_CLICKSOURCE;

}

int main(void)
{

	RCC_AHB1ENR |= GPIOAEN;

	GPIOA_MODER &= MODER5CLEAR;
	GPIOA_MODER |= MODER5OUT;

	SysTick_Init_1ms();

	unsigned int LastBlinkTime = 0;

	while(1)
	{
		//Blink green every second so each transition is half a second
		if(GlobalMilliseconds - LastBlinkTime >= 500U)
		{
			GPIOA_ODR ^= LD2_PIN;
			LastBlinkTime = GlobalMilliseconds;
		}

	}

}
