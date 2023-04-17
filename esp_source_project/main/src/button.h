#ifndef BUTTON
#define BUTTON

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define button_PIN 11
#define button_TRIGGER_LEVEL GPIO_INTR_POSEDGE	//Set whethet interrupt should trigger at positive or negative edge
#define button_PRIORITY 10
#define button_STACK_SIZE 2048

typedef enum BUTTON_event_messages
{
	BUTTON_DETECTED = 0,
	MQTT_MESSAGE
}button_event_messages_e;


typedef struct str_button_event
{
	button_event_messages_e msg_id;
	char *data;
}str_button_event_t;

QueueHandle_t q_button_event;

void install_button();
void button_event_Handler();
void IRAM_ATTR button_ISR();

#endif /* BUTTON */
