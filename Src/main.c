#include <stdint.h>

#define RCC_BASE 0x40023800UL //Reset and Clock Control Base Address
#define GPIOA_BASE 0x40020000UL //GPIO Port A Base Address
#define GPIOC_BASE 0x40020800UL //GPIO Port C Base Address
#define USART2_BASE 0x40004400UL //USART2 Base Address

#define RCC_AHB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x30UL)) //RCC Register that enables clocks for AHB1 peripherals
#define RCC_APB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x40UL)) //RCC Register that enables clocks for APB1 peripherals

#define GPIOA_MODER (*(volatile unsigned int *)(GPIOA_BASE + 0x00UL)) //GPIOA pin mode register
#define GPIOA_ODR (*(volatile unsigned int *)(GPIOA_BASE + 0x14UL)) //GPIOA output data register
#define GPIOA_AFRL (*(volatile unsigned int *)(GPIOA_BASE + 0x20UL)) //GPIOA alternate function low register (bits 0-7)
#define GPIOAEN (1U << 0) //GPIOA peripheral clock enable bit mask

#define GPIOC_MODER (*(volatile unsigned int *)(GPIOC_BASE + 0x00UL)) //GPIOC pin mode register
#define GPIOC_PUPDR (*(volatile unsigned int *)(GPIOC_BASE + 0x0CUL)) //GPIOC pull-up/pull-down register
#define GPIOC_IDR (*(volatile unsigned int *)(GPIOC_BASE + 0x10UL)) //GPIOC input data register
#define GPIOCEN (1U << 2)//GPIOC peripheral clock enable bit mask

#define USART2_SR (*(volatile unsigned int *)(USART2_BASE + 0x00UL)) //USART status register
#define USART2_DR (*(volatile unsigned int *)(USART2_BASE + 0x04UL)) //USART data register
#define USART2_BRR (*(volatile unsigned int *)(USART2_BASE + 0x08UL)) //USART baud rate register
#define USART2_CR1 (*(volatile unsigned int *)(USART2_BASE + 0x0CUL)) //USART control register 1
#define USART2EN (1U << 17) //USART2 peripheral clock enable bit
#define USART2_SR_TXE (1U << 7) //Transmit data register empty
#define USART2_CR1_UE (1U << 13) //USART enable
#define USART2_CR1_TE (1U << 3) //Transmitter enable
#define USART2_CR1_RE (1U << 2) //Receiver enable

#define MODER2CLEAR ~(3U << 4) //PA2 mode bits clear mask
#define MODER2AF (2U << 4) //PA2 set to alternate function mode

#define MODER3CLEAR ~(3U << 6) //PA3 mode bits clear mask
#define MODER3AF (2U << 6) //PA3 set to alternate function mode

#define MODER5CLEAR ~(3U << 10) //Clear mask for PA5 mode bits in GPIOA_MODER
#define MODER5OUT (1U << 10) //Set bits for PA5 mode to output in GPIOA_MODER
#define MODER13CLEAR ~(3U << 26) //Clear mask for PC13 mode bits in GPIOC_MODER
#define MODER13IN (0U << 26) //Set bits for PC13 mode to input in GPIOC_MODER (in this case it's 0 so it doesn't do anything)

#define PUPDR13CLEAR ~(3U << 26) //Clear mask for PC13 PUPD bits in GPIOC_PUPDR

#define AFRL2CLEAR ~(0xFU << 8) //Clear PA2 Alternate function bits
#define AFRL2AF7 (7U << 8) //Set PA2 Alternate function bits to AF7 = USART2_TX

#define AFRL3CLEAR ~(0xFU << 12)//Clear PA3 Alternate function bits
#define AFRL3AF7 (7U << 12) //Set PA3 Alternate function bits to AF7 = USART2_RX

#define LD2_PIN (1U << 5) //PA5 bit mask in GPIOA_ODR
#define USER_BUTTON_PIN (1U << 13) //PC13 bit mask in GPIOC_IDR

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

#define BME280_CTRL_MEAS 0xF4U
#define BME280_STATUS 0xF3U
#define BME280_TEMP_MSB 0xFAU
#define BME280_TEMP_LSB 0xFBU
#define BME280_TEMP_XLSB 0xFCU

#define BME280_DIG_T1_LSB 0x88U
#define BME280_DIG_T1_MSB 0x89U
#define BME280_DIG_T2_LSB 0x8AU
#define BME280_DIG_T2_MSB 0x8BU
#define BME280_DIG_T3_LSB 0x8CU
#define BME280_DIG_T3_MSB 0x8DU

#define BME280_FORCEDMODE_TEMPOVERSAMPLE1X 0x21U
#define BME280_STATUS_MEASURING (1U << 3)

volatile unsigned int GlobalMilliseconds = 0;

void I2C1_Init();
unsigned int I2C1_Scan();
unsigned int I2C1_ReadRegisterByte(unsigned int DeviceAddress, unsigned int RegisterAddress);
int I2C1_WriteRegisterByte(unsigned int DeviceAddress, unsigned int RegisterAddress, unsigned int Value);


static void UserButtonAndLD2_Init(void)
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
}

static void USART2_Init(void)
{
	//This function assumes GPIOA clock is already enabled
	//PA2 and PA3 are the pins for USART2 PA2 is TX (Transmit) PA3 is RX (Receive)
	//Enable clock for USART2 peripheral
	RCC_APB1ENR |= USART2EN;

	//Set the mode for pin 2 in GPIOA to alternate function mode
	GPIOA_MODER &= MODER2CLEAR;
	GPIOA_MODER |= MODER2AF;

	//Set the mode for pin 3 in GPIOA to alternate function mode
	GPIOA_MODER &= MODER3CLEAR;
	GPIOA_MODER |= MODER3AF;

	//Set the alternate function for PA2 to AF7 in the Alternate function register low
	GPIOA_AFRL &= AFRL2CLEAR;
	GPIOA_AFRL |= AFRL2AF7;

	//Set the alternate function for PA3 to AF7 in the Alternate function register low
	GPIOA_AFRL &= AFRL3CLEAR;
	GPIOA_AFRL |= AFRL3AF7;

	//Set the baud rate register to 139 since we want 115200 baud rate 16MHz / 115200 ~= 139
	USART2_BRR = 0x008B;

	//Enable USART2, transmitter and receiver
	USART2_CR1 = USART2_CR1_UE | USART2_CR1_TE | USART2_CR1_RE;
}

static void USART2_WriteChar(char c)
{
	//If the status register "Transmit data register empty" (TXE) bit is not set we wait until it is
	while((USART2_SR & USART2_SR_TXE) == 0U)
	{

	}
	//Then put our char into the USART2 data register
	USART2_DR = (unsigned int)c;
}

void USART2_WriteString(const char* String)
{
	while(*String)
	{
		USART2_WriteChar(*String);
		String++;
	}
}

static void UART_WriteHexNibble(unsigned int Value)
{
	//Get low 4 bits
	Value &= 0xFU;

	if(Value < 10U)
	{
		USART2_WriteChar('0' + Value); // 0 - 9
	}
	else
	{
		USART2_WriteChar('A' + (Value - 10U)); // A - F
	}

}

static void UART_WriteUnsignedDecimal(unsigned int Value)
{
	char Buffer[32];
	unsigned int Count = 0U;

	if(Value == 0U)
	{
		USART2_WriteString("0");
		return;
	}

	while(Value > 0U)
	{
		Buffer[Count] = '0' + (Value % 10);
		Value /= 10U;
		Count++;
	}

	while(Count > 0U)
	{
		char Out[2];

		Count --;

		Out[0] = Buffer[Count];
		Out[1] = '\0';

		USART2_WriteString(Out);
	}
}

static void UART_WriteSignedCentiCelsius(int TemperatureCentiC)
{
	unsigned int PositiveValue;

	if(TemperatureCentiC < 0)
	{
		USART2_WriteString("-");
		TemperatureCentiC = -TemperatureCentiC;
	}

	PositiveValue = (unsigned int)TemperatureCentiC;

	UART_WriteUnsignedDecimal(PositiveValue / 100U);
	USART2_WriteString(".");
	unsigned int DecimalPart = PositiveValue % 100U;
	if(DecimalPart < 10U)
	{
		USART2_WriteString("0");
	}
	UART_WriteUnsignedDecimal(DecimalPart);


}

//A Byte needs to be passed in here
void UART_WriteHexByte(unsigned int Value)
{
	UART_WriteHexNibble(Value >> 4);
	UART_WriteHexNibble(Value);
}


//SysTick calls us here
void SysTick_Handler(void)
{
	GlobalMilliseconds++;
}

static void SysTick_Init_1ms(void)
{
	STK_LOAD = (CPU_CLOCK_HZ / SYSTICK_HZ) - 1U;  // How many cpu ticks for one interrupt
	STK_VAL = 0U; //Reset current value
	STK_CTRL = SYST_CSR_ENABLE | SYST_CSR_TICKINT | SYST_CSR_CLKSOURCE; // Enable SysTick counter, enable SysTick interrupt, use processor clock

}

//BOSCH PROVIDED COMPENSATION CODE (slighty modified)
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23
//DegC. t_fine carries fine temperature as global value

//int32_t t_fine; We need a global t_fine for when we do pressure/humidity

int32_t BME280_compensate_T_int32(int32_t RawTemperature, uint16_t DigT1, int16_t DigT2, int16_t DigT3)
{
	int32_t var1, var2, t_fine, T;
	var1 = ((((RawTemperature>>3) - ((int32_t)DigT1<<1))) * ((int32_t)DigT2)) >> 11;
	var2 = (((((RawTemperature>>4) - ((int32_t)DigT1)) * ((RawTemperature>>4) - ((int32_t)DigT1)))
	>> 12) *
	((int32_t)DigT3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

static int BME280_ReadTemperatureCentiC(unsigned int DeviceAddress, int32_t* TemperatureCentiC)
{

		unsigned int Status;
		unsigned int TempMSB;
		unsigned int TempLSB;
		unsigned int TempXLSB;
		unsigned int RawTemperature;

		unsigned int DigT1_LSB;
		unsigned int DigT1_MSB;
		unsigned int DigT2_LSB;
		unsigned int DigT2_MSB;
		unsigned int DigT3_LSB;
		unsigned int DigT3_MSB;

		unsigned short DigT1;
		short DigT2;
		short DigT3;

		if(I2C1_WriteRegisterByte(DeviceAddress, BME280_CTRL_MEAS, BME280_FORCEDMODE_TEMPOVERSAMPLE1X))
		{
			USART2_WriteString("Temp measurement started\r\n");

			unsigned int Timeout = 1000U;
			do
			{
				Status = I2C1_ReadRegisterByte(DeviceAddress, BME280_STATUS);
				Timeout--;
			}
			while((Status & BME280_STATUS_MEASURING) && Timeout);

			if(Timeout == 0U)
			{
				USART2_WriteString("Temp measurement timed out\r\n");
				return 0;
			}

			TempMSB = I2C1_ReadRegisterByte(DeviceAddress, BME280_TEMP_MSB);
			TempLSB = I2C1_ReadRegisterByte(DeviceAddress, BME280_TEMP_LSB);
			TempXLSB = I2C1_ReadRegisterByte(DeviceAddress, BME280_TEMP_XLSB);

			//MSB[19:12] LSB[11:4] XLSB[3:0](bit 7, 6, 5, 4)
			RawTemperature = (TempMSB << 12) | (TempLSB << 4) | (TempXLSB >> 4);

			DigT1_LSB = I2C1_ReadRegisterByte(DeviceAddress, BME280_DIG_T1_LSB);
			DigT1_MSB = I2C1_ReadRegisterByte(DeviceAddress, BME280_DIG_T1_MSB);
			DigT2_LSB = I2C1_ReadRegisterByte(DeviceAddress, BME280_DIG_T2_LSB);
			DigT2_MSB = I2C1_ReadRegisterByte(DeviceAddress, BME280_DIG_T2_MSB);
			DigT3_LSB = I2C1_ReadRegisterByte(DeviceAddress, BME280_DIG_T3_LSB);
			DigT3_MSB = I2C1_ReadRegisterByte(DeviceAddress, BME280_DIG_T3_MSB);

			DigT1 = (DigT1_MSB << 8) | DigT1_LSB;
			DigT2 = (int16_t)((DigT2_MSB << 8) | DigT2_LSB);
			DigT3 = (int16_t)((DigT3_MSB << 8) | DigT3_LSB);

			*TemperatureCentiC = BME280_compensate_T_int32(RawTemperature, DigT1, DigT2, DigT3);

			USART2_WriteString("Raw Temperature bytes 0x");
			UART_WriteHexByte(TempMSB);
			UART_WriteHexByte(TempLSB);
			UART_WriteHexByte(TempXLSB);
			USART2_WriteString("\r\n");

			return 1;
		}
		else
		{
			USART2_WriteString("Temp measurement failed to start\r\n");
		    return 0;
		}
}

int main(void)
{

	UserButtonAndLD2_Init();
	SysTick_Init_1ms();
	USART2_Init();

	USART2_WriteString("STM32 USART booted \r\n");

	I2C1_Init();
	unsigned int DeviceAddress = I2C1_Scan();

	unsigned int ChipID = I2C1_ReadRegisterByte(DeviceAddress, 0xD0U);

	USART2_WriteString("BME280 chip ID: 0x");
	UART_WriteHexByte(ChipID);
	USART2_WriteString("\r\n");

	int32_t TemperatureCentiC;

	if(BME280_ReadTemperatureCentiC(DeviceAddress, &TemperatureCentiC))
	{
		USART2_WriteString("Temperature: ");
		UART_WriteSignedCentiCelsius(TemperatureCentiC);
		USART2_WriteString(" C\r\n");
	}
	else
	{
		USART2_WriteString("Failed to read temperature \r\n");
	}


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
			if((GPIOA_ODR & LD2_PIN) == 0U)
				USART2_WriteString("User button pressed, LD2 toggled off\r\n");
			else
				USART2_WriteString("User button pressed, LD2 toggled on\r\n");
		}

		//Track the previous stable accepted button state
		ButtonStablePrev = ButtonStable;
	}

}
