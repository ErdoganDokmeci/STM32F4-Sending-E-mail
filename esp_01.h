#include "stm32f4xx.h"
//#include "uart.h"


#define MAIN_BUFFER_SIZE 	256
#define WIFI_SSID 				"TURKSAT-KABLONET-09EF-2.4G"
#define WIFI_PSWD 				"91fd704fz"
#define MAIL_ID						"ZXJkb2dhbi5kb2ttZWNpQGVtby5vcmcudHI="
#define MAIL_PSWD					"U2V2dmFsNjc2MDA="
#define MAIL_FROM					"erdogan.dokmeci@emo.org.tr"
#define MAIL_TO						"erdogan_dokmeci@yahoo.com"
#define MAIL_SUBJ			"Sending e-mail using STM32F4"
#define MAIL_BODY					"Chance favors the prepared mind Louis Pasteur."

extern uint8_t main_buffer[MAIN_BUFFER_SIZE];

void connect_smtp2go_send_mail(void);
