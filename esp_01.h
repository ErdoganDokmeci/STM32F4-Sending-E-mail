#include "stm32f4xx.h"

#define MAIN_BUFFER_SIZE 	256
#define WIFI_SSID 		"xxxxxxxxxxxxx"
#define WIFI_PSWD 		"xxxxxxxxx"
#define MAIL_ID				"xxxxxxxxx"
#define MAIL_PSWD			"xxxxxxxxx"
#define MAIL_FROM			"xxxxxxxxx"
#define MAIL_TO				"xxxxxxxxx"
#define MAIL_SUBJ			"Sending e-mail using STM32F4"
#define MAIL_BODY			"Chance favors the prepared mind, Louis Pasteur."

extern uint8_t main_buffer[MAIN_BUFFER_SIZE];

void connect_smtp2go_send_mail(void);
