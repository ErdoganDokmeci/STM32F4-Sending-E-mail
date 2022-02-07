#include "esp_01.h"
#include <string.h>

void connect_smtp2go_send_mail(void) {
	char data[80];		
	Ringbuf_init_Usart2();
	Ringbuf_init_Uart4();
			
	/********* AT+RST **********/
	Uart_flush();
	Uart_sendstring(UART4, "AT+RST\r\n");				// Reset ESP-01
	while (!(Wait_for("OK\r\n")));
	UART4_to_USART2(main_buffer);
	
	/********* AT **********/
	Uart_flush();
	Uart_flush_USART2();
	Uart_sendstring(UART4, "AT\r\n");
	while (!(Wait_for("OK\r\n")));
	UART4_to_USART2(main_buffer);
		
	/********* AT+CWMODE=3 *********/
	Uart_flush();
	Uart_flush_USART2();
	Uart_sendstring(UART4, "AT+CWMODE=3\r\n");	// Run in SAP+STATION modes
	while (!(Wait_for("OK\r\n")));
	UART4_to_USART2(main_buffer);
			
	/********* AT+CWJAP="SSID","PASSWD" **********/
	Uart_flush();
	Uart_flush_USART2();
	sprintf (data, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PSWD);
	Uart_sendstring(UART4, data);
	while (!(Wait_for("WIFI GOT IP\r\n")));
	UART4_to_USART2(main_buffer);
				
	/********* AT+CIPMUX **********/
	Uart_flush();
	Uart_flush_USART2();		
	Uart_sendstring(UART4, "AT+CIPMUX=1\r\n");
	while (!(Wait_for("OK\r\n")));		
	UART4_to_USART2(main_buffer);
		
	/********* AT+CIPSERVER **********/
	Uart_flush();
	Uart_flush_USART2();		
	Uart_sendstring(UART4, "AT+CIPSERVER=1,80\r\n");
	while (!(Wait_for("OK\r\n")));		
	UART4_to_USART2(main_buffer);
		
	Uart_flush();
	Uart_flush_USART2();
	flush_main_buffer();	
		
	Uart_sendstring(UART4, "AT+CIPSTART=4,\"TCP\",\"mail.smtp2go.com\",2525\r\n");	
	// Alternative ports are 8028, 587, 80, and 25
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	
	Uart_sendstring(UART4, "AT+CIPSEND=4,20\r\n");
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	Uart_sendstring(UART4, "EHLO 192.168.1.123\r\n");
	UART4_to_USART2(main_buffer);
	delayMs(2000);
		
	Uart_sendstring(UART4, "AT+CIPSEND=4,12\r\n");
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	Uart_sendstring(UART4, "AUTH LOGIN\r\n");
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	
	/********* AT+CIPSEND=4, "MAIL_ID" **********/
	sprintf (data, "AT+CIPSEND=4,%d\r\n", strlen(MAIL_ID)+2);
	Uart_sendstring(UART4, data);
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	sprintf (data, "%s\r\n", MAIL_ID);
	Uart_sendstring(UART4, data);
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	
	/********* AT+CIPSEND=4, "MAIL_PSWD" **********/
	sprintf (data, "AT+CIPSEND=4,%d\r\n", strlen(MAIL_PSWD)+2);
	Uart_sendstring(UART4, data);
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	sprintf (data, "%s\r\n", MAIL_PSWD);
	Uart_sendstring(UART4, data);
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	
	/********* AT+CIPSEND=4, "MAIL_FROM" **********/
	sprintf (data, "AT+CIPSEND=4,%d\r\n", strlen(MAIL_FROM)+14);
	Uart_sendstring(UART4, data);
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	sprintf (data, "MAIL FROM:<%s>\r\n", MAIL_FROM);
	Uart_sendstring(UART4, data);
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	
	/********* AT+CIPSEND=4, "MAIL_TO" **********/
	sprintf (data, "AT+CIPSEND=4,%d\r\n", strlen(MAIL_TO)+12);
	Uart_sendstring(UART4, data);
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	sprintf (data, "RCPT To:<%s>\r\n", MAIL_TO);
	Uart_sendstring(UART4, data);
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	
	/********* AT+CIPSEND=4, "DATA" **********/
	Uart_sendstring(UART4, "AT+CIPSEND=4,6\r\n");
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	Uart_sendstring(UART4, "DATA\r\n");
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	
	/********* AT+CIPSEND=4, "MAIL SUBJECT" **********/
	sprintf (data, "AT+CIPSEND=4,%d\r\n", strlen(MAIL_SUBJ)+10);
	Uart_sendstring(UART4, data);
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	sprintf (data, "Subject:%s\r\n", MAIL_SUBJ);
	Uart_sendstring(UART4, data);
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	
	/********* AT+CIPSEND=4, "MAIL BODY" **********/
	sprintf (data, "AT+CIPSEND=4,%d\r\n", strlen(MAIL_BODY)+2);
	Uart_sendstring(UART4, data);
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	sprintf (data, "%s\r\n", MAIL_BODY);
	Uart_sendstring(UART4, data);
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	
	/********* AT+CIPSEND=4,3\r\n "END MAIL" **********/
	Uart_sendstring(UART4, "AT+CIPSEND=4,3\r\n");
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	Uart_sendstring(UART4, ".\r\n");
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	
	/********* AT+CIPSEND=4, "QUIT" **********/
	Uart_sendstring(UART4, "AT+CIPSEND=4,6\r\n");
	UART4_to_USART2(main_buffer);
	delayMs(2000);
	Uart_sendstring(UART4, "QUIT\r\n");
	UART4_to_USART2(main_buffer);
	delayMs(10000);	
}
