#ifndef LED
#define LED
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define LED_PIN 13
#define LED_STACK_SIZE 2048
#define LED_PRIORITY 5

typedef enum LED_event_messages
{
	LED_ON = 0,
	LED_OFF,
	LED_BLINK
}LED_event_messages_e;


typedef struct str_LED_event
{
	LED_event_messages_e msg_id;
}str_LED_event_t;

QueueHandle_t q_LED_event;

void LED_event_Handler();
void install_LED();

#endif /* LED */
