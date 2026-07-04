//I2C is on PB8 I2C1_SCL PB9 I2C1_SDA they also map to the Arduino style pins

#define RCC_BASE 0x40023800UL
#define I2C1_BASE 0x40005400UL
#define GPIOB_BASE 0x40020400UL

#define RCC_AHB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x30UL))
#define RCC_APB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x40UL))

#define GPIOB_MODER (*(volatile unsigned int *)(GPIOB_BASE + 0x00UL))
#define GPIOB_OTYPER (*(volatile unsigned int *)(GPIOB_BASE + 0x04UL))
#define GPIOB_OSPEEDR (*(volatile unsigned int *)(GPIOB_BASE + 0x08UL))
#define GPIOB_PUPDR (*(volatile unsigned int *)(GPIOB_BASE + 0x0CUL))
#define GPIOB_AFRH (*(volatile unsigned int *)(GPIOB_BASE + 0x24UL))

#define I2C1_CR1 (*(volatile unsigned int *)(I2C1_BASE + 0x00UL))
#define I2C1_CR2 (*(volatile unsigned int *)(I2C1_BASE + 0x04UL))
#define I2C1_OAR1 (*(volatile unsigned int *)(I2C1_BASE + 0x08UL))
#define I2C1_DR (*(volatile unsigned int *)(I2C1_BASE + 0x10UL))
#define I2C1_SR1 (*(volatile unsigned int *)(I2C1_BASE + 0x14UL))
#define I2C1_SR2 (*(volatile unsigned int *)(I2C1_BASE + 0x18UL))
#define I2C1_CCR (*(volatile unsigned int *)(I2C1_BASE + 0x1CUL))
#define I2C1_TRISE (*(volatile unsigned int *)(I2C1_BASE + 0x20UL))


#define RCC_GPIOBEN (1U << 1)
#define RCC_I2C1EN (1U << 21)

#define GPIOB_MODER8_CLEAR (3U << 16)
#define GPIOB_MODER8_SETALT (2U << 16)

#define GPIOB_MODER9_CLEAR (3U << 18)
#define GPIOB_MODER9_SETALT (2U << 18)

#define GPIOB_OTYPER8_SETOPENDRAIN (1U << 8)
#define GPIOB_OTYPER9_SETOPENDRAIN (1U << 9)

#define GPIOB_OSPEEDR8_CLEAR (3U << 16)
#define GPIOB_OSPEEDR8_SET_HIGHSPEED (2U << 16)

#define GPIOB_OSPEEDR9_CLEAR (3U << 18)
#define GPIOB_OSPEEDR9_SET_HIGHSPEED (2U << 18)

#define GPIOB_PUPDR8_CLEAR  (3U << 16)
#define GPIOB_PUPDR8_SETPULLUP  (1U << 16)

#define GPIOB_PUPDR9_CLEAR  (3U << 18)
#define GPIOB_PUPDR9_SETPULLUP  (1U << 18)

#define GPIOB_AFRH8_CLEAR (0xFU << 0)
#define GPIOB_AFRH8_SETAF4 (4U << 0)

#define GPIOB_AFRH9_CLEAR (0xFU << 4)
#define GPIOB_AFRH9_SETAF4 (4U << 4)

#define I2C_CR1_PE (1U << 0)
#define I2C_CR1_START (1U << 8)
#define I2C_CR1_STOP (1U << 9)
#define I2C_CR1_ACK (1U << 10)
#define I2C_CR1_SWRST (1U << 15)

#define I2C_SR1_SB (1U << 0)
#define I2C_SR1_ADDR (1U << 1)
#define I2C_SR1_BTF (1U << 2)
#define I2C_SR1_RXNE (1U << 6)
#define I2C_SR1_TXE (1U << 7)
#define I2C_SR1_AF (1U << 10)


void UART_WriteHexByte(unsigned int Value);
void USART2_WriteString(const char* String);

void I2C1_Init(void)
{

	RCC_AHB1ENR |= RCC_GPIOBEN; //Enable GPIOB clock

	RCC_APB1ENR |= RCC_I2C1EN; // Enable I2C1 clock

	//PB8/PB9 set to alternate function mode
	GPIOB_MODER &= ~(GPIOB_MODER8_CLEAR | GPIOB_MODER9_CLEAR);
	GPIOB_MODER |= GPIOB_MODER8_SETALT | GPIOB_MODER9_SETALT;

	//PB8/PB9 set output type to open drain
	GPIOB_OTYPER |= GPIOB_OTYPER8_SETOPENDRAIN | GPIOB_OTYPER9_SETOPENDRAIN;

	//PB8/PB9 set output speed to high speed
	GPIOB_OSPEEDR &= ~(GPIOB_OSPEEDR8_CLEAR | GPIOB_OSPEEDR9_CLEAR);
	GPIOB_OSPEEDR |= GPIOB_OSPEEDR8_SET_HIGHSPEED | GPIOB_OSPEEDR9_SET_HIGHSPEED;

	//PB8/PB9 set pupd config to pull up
	GPIOB_PUPDR &= ~(GPIOB_PUPDR8_CLEAR | GPIOB_PUPDR9_CLEAR);
	GPIOB_PUPDR |= GPIOB_PUPDR8_SETPULLUP | GPIOB_PUPDR9_SETPULLUP;

	//PB8/PB9 set alternate function to AF4 (PB8 I2C1_SCL PB9 I2C1_SDA)
	GPIOB_AFRH &= ~(GPIOB_AFRH8_CLEAR | GPIOB_AFRH9_CLEAR);
	GPIOB_AFRH |= GPIOB_AFRH8_SETAF4 | GPIOB_AFRH9_SETAF4;

	//Reset I2C1
	I2C1_CR1 |= I2C_CR1_SWRST;
	I2C1_CR1 &= ~I2C_CR1_SWRST;


	//Disable I2C1 before configuring timing registers
	I2C1_CR1 &= ~I2C_CR1_PE;

	I2C1_CR2 = 16U;
	I2C1_CCR = 80U;
	I2C1_TRISE = 17U;
	//Needs to be kept at 1
	I2C1_OAR1 = (1U << 14);

	//Enable I2C1
	I2C1_CR1 |= I2C_CR1_PE;


}

//We need the register passed in as a pointer because the register will change during this check
//if it somehow didn't already before the call
static int I2C1_WaitForSet(volatile unsigned int* Register, unsigned int Flag)
{
	unsigned int Timeout = 100000U;

	while(((*Register & Flag) == 0U) && Timeout)
	{
		Timeout --;
	}

	return Timeout != 0U;

}
static int I2C1_AddressResponds(unsigned int Address)
{
	volatile unsigned int Temp;
	unsigned int Timeout;

	I2C1_SR1 &= ~I2C_SR1_AF; //Clear acknowledge failure flag, since it may be set before

	//Generate a START condition
	I2C1_CR1 |= I2C_CR1_START;

	Timeout = 100000U;

	//Wait until the START is generated
	while(((I2C1_SR1 & I2C_SR1_SB) == 0U) && Timeout)
	{
		Timeout--;
	}

	//If START never was set lets STOP and say no external hardware detected
	if(Timeout == 0U)
	{
		I2C1_CR1 |= I2C_CR1_STOP;
		return 0;
	}

	//Put the address in the data register shifted left 1 bit so the low bit is 0
	//Making this a write
	I2C1_DR = (Address << 1);

	Timeout = 100000U;
	//Wating for ACK or NACK
	while(Timeout)
	{
		if(I2C1_SR1 & I2C_SR1_ADDR)
		{
			//The address was ACKed
			//CLEAR ADDR by reading from SR1 then SR2
			Temp = I2C1_SR1;
			Temp = I2C1_SR2;
			(void)Temp;

			I2C1_CR1 |= I2C_CR1_STOP;
			return 1;
		}

		if(I2C1_SR1 & I2C_SR1_AF)
		{
			//Address was NACKed
			I2C1_SR1 &= ~I2C_SR1_AF;
			I2C1_CR1 |= I2C_CR1_STOP;
			return 0;
		}

		Timeout--;
	}

	//Neither ACK or NACK happened before timeout
	I2C1_CR1 |= I2C_CR1_STOP;
	return 0;
}

unsigned int I2C1_ReadRegisterByte(unsigned int DeviceAddress, unsigned int RegisterAddress)
{
	volatile unsigned int Temp;
	unsigned int Value;

	//This is how we begin a serial data transfer. It causes the I2C interface to enter
	//controller mode in which it initiates a data transfer and generates the clock signal
	//The controller is now waiting for a read of the SR1 register followed by a write in the
	//DR register with the target address
	I2C1_CR1 |= I2C_CR1_START;

	//Wait for the START bit to be set
	if(!I2C1_WaitForSet(&I2C1_SR1, I2C_SR1_SB))
	{
		I2C1_CR1 |= I2C_CR1_STOP;
		return 0U;
	}

	//Write to the data register with the address of our target
	//The LSB set to 0 means we enter Transmitter mode (we want to send data over)
	I2C1_DR = DeviceAddress << 1;

	//Wait for the address to be sent to the SDA line as soon as it's sent the ADDR bit is set
	if(!I2C1_WaitForSet(&I2C1_SR1, I2C_SR1_ADDR))
	{
		I2C1_CR1 |= I2C_CR1_STOP;
		return 0U;
	}

	Temp = I2C1_SR1;
	Temp = I2C1_SR2;
	(void)Temp;

	if(!I2C1_WaitForSet(&I2C1_SR1, I2C_SR1_TXE))
	{
		I2C1_CR1 |= I2C_CR1_STOP;
		return 0U;
	}

	I2C1_DR = RegisterAddress;

	if(!I2C1_WaitForSet(&I2C1_SR1, I2C_SR1_BTF))
	{
		I2C1_CR1 |= I2C_CR1_STOP;
		return 0U;
	}

	I2C1_CR1 |= I2C_CR1_START;

	if(!I2C1_WaitForSet(&I2C1_SR1, I2C_SR1_SB))
	{
		I2C1_CR1 |= I2C_CR1_STOP;
		return 0U;
	}

	I2C1_DR = (DeviceAddress << 1) | 1U;

	if(!I2C1_WaitForSet(&I2C1_SR1, I2C_SR1_ADDR))
	{
		I2C1_CR1 |= I2C_CR1_STOP;
		return 0U;
	}

	I2C1_CR1 &= ~I2C_CR1_ACK;
	Temp = I2C1_SR1;
	Temp = I2C1_SR2;
	(void)Temp;

	I2C1_CR1 |= I2C_CR1_STOP;

	if(!I2C1_WaitForSet(&I2C1_SR1, I2C_SR1_RXNE))
	{
		I2C1_CR1 |= I2C_CR1_ACK;
		return 0U;
	}

	Value = I2C1_DR & 0xFFU;

	I2C1_CR1 |= I2C_CR1_ACK;

	return Value;

}

int I2C1_WriteRegisterByte(unsigned int DeviceAddress, unsigned int RegisterAddress, unsigned int Value)
{
	volatile unsigned int Temp;

	I2C1_CR1 |= I2C_CR1_START;

	if(!I2C1_WaitForSet(&I2C1_SR1, I2C_SR1_SB))
	{
		I2C1_CR1 |= I2C_CR1_STOP;
		return 0;
	}

	//Transmitter mode
	I2C1_DR = DeviceAddress << 1;

	if(!I2C1_WaitForSet(&I2C1_SR1, I2C_SR1_ADDR))
	{
		I2C1_CR1 |= I2C_CR1_STOP;
		return 0;
	}

	Temp = I2C1_SR1;
	Temp = I2C1_SR2;
	(void)Temp;

	//Docs state to do this TXE here
	if(!I2C1_WaitForSet(&I2C1_SR1, I2C_SR1_TXE))
	{
		I2C1_CR1 |= I2C_CR1_STOP;
		return 0;
	}

	//Lets send over the register we want to write to
	I2C1_DR = RegisterAddress;

	//Wait for the register address to be sent
	if(!I2C1_WaitForSet(&I2C1_SR1, I2C_SR1_TXE))
	{
		I2C1_CR1 |= I2C_CR1_STOP;
		return 0;
	}

	I2C1_DR = Value;

	if(!I2C1_WaitForSet(&I2C1_SR1, I2C_SR1_BTF))
	{
		I2C1_CR1 |= I2C_CR1_STOP;
		return 0;
	}

	I2C1_CR1 |= I2C_CR1_STOP;

	return 1;

}

unsigned int I2C1_Scan(void)
{
	unsigned int Address;
	unsigned int SaveAddress;
	unsigned int FoundCount = 0U;

	USART2_WriteString("I2C scan starting....\r\n");

	for(Address = 1U; Address < 128U; Address++)
	{
		if(I2C1_AddressResponds(Address))
		{
			USART2_WriteString("Found I2C device at 0x");
			UART_WriteHexByte(Address);
			USART2_WriteString("\r\n");

			SaveAddress = Address;

			FoundCount ++;
		}
	}

	if(FoundCount == 0U)
	{
		USART2_WriteString("No I2C devices found.\r\n");
	}
	else
	{
		USART2_WriteString("I2C scan done.\r\n");
	}

	return SaveAddress;
}


