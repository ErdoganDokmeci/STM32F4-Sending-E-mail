// Sending e-mail using STM32F446RE-Nucleo and ESP-01
// This program was tested with Keil uVision v5.33.0.0 with DFP v2.16.0
// By default, the clock is running at 16 MHz.
// Source code for ring buffer was excerpted from GitHub-controllerstech/stm32-uart-ring-buffer and modified
// Feb 07, 2022, Eskisehir

#include "stm32f4xx.h"
#include "esp_01.h"
#include <stdio.h>


#define UART_BUFFER_SIZE 128
#define MAIN_BUFFER_SIZE 256
#define TIMEOUT_DEF 500  			// 500ms time_out


uint8_t main_buffer[MAIN_BUFFER_SIZE] = {0};

typedef struct {
	unsigned char buffer[UART_BUFFER_SIZE];
	volatile uint16_t head;
	volatile uint16_t tail;
} ring_buffer;


ring_buffer rx_buffer = { { 0 }, 0, 0};			// UART4
ring_buffer tx_buffer = { { 0 }, 0, 0};			// UART4
ring_buffer rx_buffer2 = { { 0 }, 0, 0};		// USART2
ring_buffer tx_buffer2 = { { 0 }, 0, 0};		// USART2

ring_buffer *_rx_buffer;
ring_buffer *_tx_buffer;
ring_buffer *_rx_buffer2;
ring_buffer *_tx_buffer2;

uint16_t timeout;
uint16_t main_buffer_index;

// USART/UART Functions 
void USART2_init(void);
void USART2_write(int ch);
void send_string_to_USART2(char *str);

void UART4_init(void);
void UART4_write(int ch);
void send_string_to_UART4(char *str);

void UART4_to_USART2(uint8_t rb[]);

/* Initialize the ring buffer */
void Ringbuf_init_Usart2(void);
void Ringbuf_init_Uart4(void);

// Ring Buffer Functions
/* reads the data in the rx_buffer and increment the tail count in rx_buffer */
int Uart_read_Usart2(void);		// USART2
int Uart_read(void);					// UART4

void store_char(unsigned char c, ring_buffer *buffer);
void Uart_write_Usart2(int c);
void Uart_write(int c);
void Uart_sendstring(USART_TypeDef *huart, const char *s);
int IsDataAvailable(void);
int Look_for(char *str, char *buffertolookinto);
void GetDataFromBuffer(char *startString, char *endString, char *buffertocopyfrom, char *buffertocopyinto);
void Uart_flush(void);
void Uart_flush_USART2(void);
void flush_main_buffer(void);
int Uart_peek(void);
int Copy_upto(char *string, char *buffertocopyinto);
int Get_after(char *string, uint8_t numberofchars, char *buffertosave);
int Wait_for(char *string);

void delayMs(int n);


int main(void) {
	__disable_irq();
	USART2_init();          				// initialize USART2	
	UART4_init(); 									// Initialize UART4 for ESP-01 data
	NVIC_EnableIRQ(USART2_IRQn);		// enable interrupt in NVIC
	NVIC_EnableIRQ(UART4_IRQn);			// enable interrupt in NVIC			
	__enable_irq();									// global enable IRQs
	connect_smtp2go_send_mail();		// connect to Wi-Fi/smtp2go and send mail
		
	while(1) {     	
		}
}

void delayMs(int n) {
	int i;
	SysTick->LOAD = 16000-1;
	SysTick->VAL  = 0;
	SysTick->CTRL = 0x5;
	for(i = 0; i < n; i++) {
		while((SysTick->CTRL & 0x10000) == 0);	// Wait until the COUNTFLAG is set
		}
	SysTick->CTRL = 0;										// Stop the timer
}


void USART2_IRQHandler(void) {
	/* if DR is not empty and the Rx Int is enabled */	
	if ((USART2->SR & 0x0020) && (USART2->CR1 & 0x0020)) {
		uint8_t data;
		data = USART2->DR;
		store_char (data, _rx_buffer2);			// store data in buffer					
    return;			
	}
			
	/*If interrupt is caused due to Transmit Data Register Empty */
	if ((USART2->SR & 0x0080) && (USART2->CR1 & 0x0080)) {    	
		if(tx_buffer2.head == tx_buffer2.tail) {
			// Buffer empty, so disable interrupts							
			USART2->CR1 &= ~0x0080;
		}

		else {
			// There is more data in the output buffer. Send the next byte
			unsigned char c = tx_buffer2.buffer[tx_buffer2.tail];
			tx_buffer2.tail = (tx_buffer2.tail + 1) % UART_BUFFER_SIZE;
			USART2->DR = c;
		}
		return;		
		}
}


void UART4_IRQHandler(void) {
	/* if DR is not empty and the Rx Int is enabled */	
	if ((UART4->SR & 0x0020) && (UART4->CR1 & 0x0020)) {
		uint8_t data;
		data = UART4->DR;				
		store_char (data, _rx_buffer);  // store data in buffer
		main_buffer[main_buffer_index] = data;
		main_buffer_index = (main_buffer_index + 1) % MAIN_BUFFER_SIZE;			
    return;			
	}
			
	/*If interrupt is caused due to Transmit Data Register Empty */
	if ((UART4->SR & 0x0080) && (UART4->CR1 & 0x0080)){    	
		if (tx_buffer.head == tx_buffer.tail) {
			// Buffer empty, so disable interrupts							
			UART4->CR1 &= ~0x0080;					
    }

		else {
			// There is more data in the output buffer. Send the next byte
			unsigned char c = tx_buffer.buffer[tx_buffer.tail];
			tx_buffer.tail = (tx_buffer.tail + 1) % UART_BUFFER_SIZE;
			UART4->DR = c;
			}
		return;		
	}
}

// uart.c 


/*----------------------------------------------------------------------------
  Initialize USART2 to receive and transmit at 115200 Baud
 *----------------------------------------------------------------------------*/
void USART2_init (void) {
    RCC->AHB1ENR |= 1;          // Enable GPIOA clock
    RCC->APB1ENR |= 0x20000;    // Enable USART2 clock
	
		// Configure PA2 for USART2_TX
    GPIOA->AFR[0] &= ~0x0F00;
    GPIOA->AFR[0] |=  0x0700;   // alt7 for USART2
    GPIOA->MODER  &= ~0x0030;
    GPIOA->MODER  |=  0x0020;   // enable alternate function for PA2
	
		// Configure PA3 for USART2 RX
    GPIOA->AFR[0] &= ~0xF000;
    GPIOA->AFR[0] |=  0x7000;   // alt7 for USART2
    GPIOA->MODER  &= ~0x00C0;
    GPIOA->MODER  |=  0x0080;   // enable alternate function for PA3
		
		USART2->BRR = 0x008B;       // 115200 baud @ 16 MHz	
    USART2->CR1 = 0x000C;       // enable Rx/Tx, 8-bit data
    USART2->CR2 = 0x0000;       // 1 stop bit
    USART2->CR3 = 0x0000;       // no flow control
    USART2->CR1 |= 0x2000;      // enable USART2		
}

/* Write a character to USART2 */
void USART2_write (int ch) {
	while (!(USART2->SR & 0x0080)) {}   // wait until Tx buffer empty
  USART2->DR = (ch & 0xFF);
}

void send_string_to_USART2(char *str) {
	char temp_str[80];
	strcpy(temp_str, str);
	strcat(temp_str, "\r\n");
	int i = 0;
	while(temp_str[i] != '\0')
		USART2_write(temp_str[i++]);
}

/* initialize UART4 to receive/transmit at 115200 Baud */
void UART4_init (void) {
	RCC->AHB1ENR |= 1;          // Enable GPIOA clock
  RCC->APB1ENR |= 0x80000;    // Enable UART4 clock

  /* Configure PA0, PA1 for UART4 TX, RX */
  GPIOA->AFR[0] &= ~0x00FF;
  GPIOA->AFR[0] |=  0x0088;   // alt8 for UART4
  GPIOA->MODER  &= ~0x000F;
  GPIOA->MODER  |=  0x000A;   // enable alternate function for PA0, PA1 

  UART4->BRR = 0x008B;        // 115200 baud @ 16 MHz
  UART4->CR1 = 0x000C;        // enable Tx, Rx, 8-bit data
  UART4->CR2 = 0x0000;        // 1 stop bit
  UART4->CR3 = 0x0000;        // no flow control
  UART4->CR1 |= 0x2000;       // enable UART4
}

/* Write a character to UART4 */
void UART4_write (int ch) {
	while (!(UART4->SR & 0x0080)) {}   // wait until Tx buffer empty
  UART4->DR = (ch & 0xFF);
}

void send_string_to_UART4(char *str) {
	char temp_str[80];	
	strcpy(temp_str, str);
	strcat(temp_str, "\r\n");
	int i = 0;
	while(temp_str[i] != '\0')
		UART4_write(temp_str[i++]);
}

void UART4_to_USART2(uint8_t rb[]) {
	Uart_flush_USART2();
	Uart_sendstring(USART2, (char *)rb);		
	flush_main_buffer();		
}


void Ringbuf_init_Usart2(void) {
	_rx_buffer2 = &rx_buffer2;
	_tx_buffer2 = &tx_buffer2;
	USART2->CR1 |= 0x0020;		// Enable the USART Data Register not empty Interrupt
	USART2->CR3 |= 0x0001;		// Enable the USART Error Interrupt: (Frame error, noise error, overrun error)
}

void Ringbuf_init_Uart4(void) {
	_rx_buffer = &rx_buffer;
	_tx_buffer = &tx_buffer;
	UART4->CR1 |= 0x0020;		// Enable the USART Data Register not empty Interrupt
	UART4->CR3 |= 0x0001;		// Enable the USART Error Interrupt: (Frame error, noise error, overrun error)
}

void store_char(unsigned char c, ring_buffer *buffer) {
	uint16_t i = (uint16_t)(buffer->head + 1) % UART_BUFFER_SIZE;
	// if we should be storing the received character into the location
	// just before the tail (meaning that the head would advance to the
	// current location of the tail), we're about to overflow the buffer
	// and so we don't write the character or advance the head.
	if (i != buffer->tail) {
		buffer->buffer[buffer->head] = c;
		buffer->head = i;
	}
}


int Uart_read(void) {
	// if the head isn't ahead of the tail, we don't have any characters
	if (_rx_buffer->head == _rx_buffer->tail) {
		return -1;
		}
	else {
		unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
		_rx_buffer->tail = (uint16_t)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;
		return c;
	}
}


int Uart_read_Usart2(void) {
	// if the head isn't ahead of the tail, we don't have any characters
	if(_rx_buffer2->head == _rx_buffer2->tail) {
		return -1;
	}
	else {
		unsigned char c = _rx_buffer2->buffer[_rx_buffer2->tail];
		_rx_buffer2->tail = (uint16_t)(_rx_buffer2->tail + 1) % UART_BUFFER_SIZE;
		return c;
	}
}


void Uart_write(int c) {
	if (c >= 0) {
		uint16_t i = (_tx_buffer->head + 1) % UART_BUFFER_SIZE;
		while (i == _tx_buffer->tail);

		_tx_buffer->buffer[_tx_buffer->head] = (uint8_t)c;
		_tx_buffer->head = i;			
		UART4->CR1 |= 0x0080;			// Enable UART transmission interrupt
	}
}


void Uart_write_Usart2(int c) {
	if (c >= 0) {
		uint16_t i = (_tx_buffer2->head + 1) % UART_BUFFER_SIZE;
		while (i == _tx_buffer2->tail);

		_tx_buffer2->buffer[_tx_buffer2->head] = (uint8_t)c;
		_tx_buffer2->head = i;			
		USART2->CR1 |= 0x0080;			// Enable UART transmission interrupt
	}
}



/* checks if the new data is available in the incoming buffer */
int IsDataAvailable(void) {
	return (uint16_t)(UART_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % UART_BUFFER_SIZE;
}

/* sends the string to the uart */
void Uart_sendstring(USART_TypeDef *huart, const char *s) {
	//while(*s) Uart_write(*s++);
	if (huart == UART4)
		while(*s) Uart_write(*s++);
	else
		while(*s) Uart_write_Usart2(*s++);
}

void GetDataFromBuffer(char *start_string, char *end_string, char *buffer_to_copy_from, char *buffer_to_copy_into) {
	uint16_t start_string_length = strlen(start_string);
	uint16_t end_string_length   = strlen(end_string);
	uint16_t so_far = 0;
	uint16_t index = 0;
	uint16_t start_position = 0;
	uint16_t end_position   = 0;

	repeat1:
		while (start_string[so_far] != buffer_to_copy_from[index]) 
			index++;
		if (start_string[so_far] == buffer_to_copy_from[index]) {
			while (start_string[so_far] == buffer_to_copy_from[index]) {
				so_far++;
				index++;
			}
		}

		if (so_far == start_string_length) 
			start_position = index;
		else {
			so_far = 0;
			goto repeat1;
		}

	so_far = 0;

repeat2:
	while (end_string[so_far] != buffer_to_copy_from[index]) 
		index++;
	if (end_string[so_far] == buffer_to_copy_from[index]) {
		while (end_string[so_far] == buffer_to_copy_from[index]) {
			so_far++;
			index++;
		}
	}

	if (so_far == end_string_length)
		end_position = index - end_string_length;
	else {
		so_far = 0;
		goto repeat2;
	}

	so_far = 0;
	index = 0;

	for (int i = start_position; i < end_position; i++) {
		buffer_to_copy_into[index] = buffer_to_copy_from[i];
		index++;
	}
}

void Uart_flush(void) {
	memset(_rx_buffer->buffer, '\0', UART_BUFFER_SIZE);
	_rx_buffer->head = 0;
	_rx_buffer->tail = 0;
}

void Uart_flush_USART2(void) {
	memset(_rx_buffer2->buffer, '\0', UART_BUFFER_SIZE);
	_rx_buffer2->head = 0;
	_rx_buffer2->tail = 0;
}

void flush_main_buffer(void) {
	memset(main_buffer, '\0', MAIN_BUFFER_SIZE);		
	main_buffer_index = 0;
}

int Uart_peek(void) {
	if(_rx_buffer->head == _rx_buffer->tail)
		return -1;		
	else 
		return _rx_buffer->buffer[_rx_buffer->tail];		
}

/* copies the data from the incoming buffer into our buffer
 * Must be used if you are sure that the data is being received
 * it will copy irrespective of, if the end string is there or not
 * if the end string gets copied, it returns 1 or else 0
 * Use it either after (IsDataAvailable) or after (Wait_for) functions
 */
int Copy_upto(char *string, char *buffer_to_copy_into) {
	uint16_t so_far =0;
	uint16_t len = strlen (string);
	uint16_t index = 0;

again:
	while (Uart_peek() != string[so_far]) {
		buffer_to_copy_into[index] = _rx_buffer->buffer[_rx_buffer->tail];
		_rx_buffer->tail = (uint16_t)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;
		index++;
		while (!IsDataAvailable());
	}
	while (Uart_peek() == string [so_far]) {
		so_far++;
		buffer_to_copy_into[index++] = Uart_read();
		if (so_far == len) 
			return 1;
		timeout = TIMEOUT_DEF;
		while ((!IsDataAvailable())&&timeout);
		if (timeout == 0) 
			return 0;
	}

	if (so_far != len) {
		so_far = 0;
		goto again;
	}

	if (so_far == len) 
		return 1;
	else 
		return 0;
}

/* must be used after wait_for function
 * get the entered number of characters after the entered string
 */
int Get_after(char *string, uint8_t number_of_chars, char *buffer_to_save) {
	for (uint16_t index = 0; index < number_of_chars; index++) {
		timeout = TIMEOUT_DEF;
		while ((!IsDataAvailable())&&timeout);  // wait until some data is available
		if (timeout == 0) 
			return 0;  													// if data isn't available within time, then return 0
		buffer_to_save[index] = Uart_read();  	// save the data into the buffer... increments the tail
	}
	return 1;
}

/* Waits for a particular string to arrive in the incoming buffer... It also increments the tail
 * returns 1, if the string is detected
 */
// added timeout feature so the function won't block the processing of the other functions
int Wait_for(char *string) {
	uint16_t so_far = 0;
	uint16_t len = strlen(string);

again:
	timeout = TIMEOUT_DEF;
	while ((!IsDataAvailable())&&timeout);  // let's wait for the data to show up
	if (timeout == 0) 
		return 0;
	while (Uart_peek() != string[so_far])  	// peek in the rx_buffer to see if we get the string
		{
		if (_rx_buffer->tail != _rx_buffer->head) {
			_rx_buffer->tail = (uint16_t)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;  // increment the tail
		}

		else {
			return 0;
		}
		}
	while (Uart_peek() == string [so_far]) // if we got the first letter of the string
		{
		// now we will peek for the other letters too
		so_far++;
		_rx_buffer->tail = (uint16_t)(_rx_buffer->tail + 1) % UART_BUFFER_SIZE;  // increment the tail
		if (so_far == len) 
			return 1;
		timeout = TIMEOUT_DEF;
		while ((!IsDataAvailable())&&timeout);
		if (timeout == 0) 
			return 0;
	}

	if (so_far != len) {
		so_far = 0;
		goto again;
	}

	if (so_far == len) 
		return 1;
	else 
		return 0;
}
