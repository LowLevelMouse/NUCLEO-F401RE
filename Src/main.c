#define RCC_BASE 0x40023800UL //Reset and Clock Control Base Address
#define GPIOA_BASE 0x40020000UL //GPIO Port A Base Address
#define GPIOC_BASE 0x40020800UL //GPIO Port C Base Address

#define RCC_AHB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x30UL)) //RCC Register that enables clocks for AHB1 peripherals

#define GPIOA_MODER (*(volatile unsigned int *)(GPIOA_BASE + 0x00UL)) //GPIOA pin mode register
#define GPIOA_ODR (*(volatile unsigned int *)(GPIOA_BASE + 0x14UL)) //GPIOA output data register
#define GPIOAEN (1U << 0) //GPIOA peripheral clock enable bit set to 1

#define GPIOC_MODER (*(volatile unsigned int *)(GPIOC_BASE + 0x00UL)) //GPIOC pin mode register
#define GPIOC_PUPDR (*(volatile unsigned int *)(GPIOC_BASE + 0x0CUL)) //GPIOC pull-up/pull-down register
#define GPIOC_IDR (*(volatile unsigned int *)(GPIOC_BASE + 0x10UL)) //GPIOC input data register
#define GPIOCEN (1U << 2)//GPIOC peripheral clock enable bit set to 1

#define MODER5CLEAR ~(3U << 10) //Clear mask for PA5 mode bits in GPIOA_MODER
#define MODER5OUT (1U << 10) //Set bits for PA5 mode to output in GPIOA_MODER
#define MODER13CLEAR ~(3U << 26) //Clear mask for PC13 mode bits in GPIOC_MODER
#define MODER13IN (0U << 26) //Set bits for PC13 mode to input in GPIOC_MODER (in this case it's 0 so it doesn't do anything)
#define PUPDR13CLEAR ~(3U << 26) //Clear mask for PC13 PUPD bits in GPIOC_PUPDR

#define LD2_PIN (1U << 5) //PA5 offset in GPIOA_ODR set to 1
#define USER_BUTTON_PIN (1U << 13) //PC13 offset in GPIOC_IDR set to 1

#define SYSTICK_BASE 0xE000E010UL //SysTick base address SysTick is for accurate and non blocking timing

#define STK_CTRL (*(volatile unsigned int *)(SYSTICK_BASE + 0x00UL)) //SysTick control and status register
#define STK_LOAD (*(volatile unsigned int *)(SYSTICK_BASE + 0x04UL)) //SysTick reload value register
#define STK_VAL (*(volatile unsigned int *)(SYSTICK_BASE + 0x08UL)) //SysTick current value register

#define SYST_CSR_ENABLE (1U << 0) //Enable the counter
#define SYST_CSR_TICKINT (1U << 1) //When counter goes to zero it asserts the SysTick exception request
#define SYST_CSR_CLKSOURCE (1U << 2) //Clock source is the processor

#define CPU_CLOCK_HZ 16000000U //Clock speed of our microcontroller
#define SYSTICK_HZ 1000U //How many times a second should we interrupt

#define DEBOUNCE_MS 20U // Raw button state must stay unchanged this long before being accepted.
volatile unsigned int GlobalMilliseconds = 0;


//SysTick calls us here
void SysTick_Handler()
{
	GlobalMilliseconds++;
}

static void SysTick_Init_1ms()
{
	STK_LOAD = (CPU_CLOCK_HZ / SYSTICK_HZ) - 1U;  // How many cpu ticks for one interrupt
	STK_VAL = 0U; //Reset current value
	STK_CTRL = SYST_CSR_ENABLE | SYST_CSR_TICKINT | SYST_CSR_CLKSOURCE; // Add the flags

}

int main(void)
{

	//Enable clock for GPIOA and GPIOC peripherals
	RCC_AHB1ENR |= GPIOAEN | GPIOCEN;

	//Set the mode for pin 5 in GPIOA to output
	GPIOA_MODER &= MODER5CLEAR;
	GPIOA_MODER |= MODER5OUT;

	//Set the mode for pin 13 in GPIOC to input
	GPIOC_MODER &= MODER13CLEAR;
	GPIOC_MODER |= MODER13IN;

	// Disable internal pull-up/pull-down on PC13.
	GPIOC_PUPDR &= PUPDR13CLEAR;

	SysTick_Init_1ms();

	unsigned int ButtonLastChangeMs = 0;
	unsigned int ButtonRawPrev = 0;
	unsigned int ButtonStable = 0;
	unsigned int ButtonStablePrev = 0;

	while(1)
	{
		//Pressed is 0
		//We get the raw signal whether its an intentional press or not
		unsigned int ButtonRaw = ((GPIOC_IDR & USER_BUTTON_PIN) == 0U);

		//While the signal keeps hoping aroun we keep reseting our timer
		if(ButtonRaw != ButtonRawPrev)
		{
			ButtonRawPrev = ButtonRaw;
			ButtonLastChangeMs = GlobalMilliseconds;
		}

		//If the signal has been stable for long enough accept the current one as
		//a proper button press or release
		if((GlobalMilliseconds - ButtonLastChangeMs) >= DEBOUNCE_MS)
		{
			ButtonStable = ButtonRawPrev;
		}

		//Simply an IsPressed and !WasPressed condition
		//to register the proper condition to change the light
		if(ButtonStable && !ButtonStablePrev)
		{
			GPIOA_ODR ^= LD2_PIN;
		}

		//Track the previous stable accepted button state
		ButtonStablePrev = ButtonStable;
	}

}
