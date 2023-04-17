#ifndef MOTION_SNSR
#define MOTION_SNSR

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define MOTION_SNSR_PIN 10
#define MOTION_SNSR_TRIGGER_LEVEL GPIO_INTR_POSEDGE	//Set whethet interrupt should trigger at positive or negative edge
#define motion_snsr_PRIORITY 10
#define motion_snsr_STACK_SIZE 2048

typedef enum motion_snsr_event_messages
{
	MOTION_DETECTED = 0,
	MQTT_MESSAGE
}motion_snsr_event_messages_e;


typedef struct str_motion_snsr_event
{
	motion_snsr_event_messages_e msg_id;
	char *data;
}str_motion_snsr_event_t;

QueueHandle_t q_motion_snsr_event;

void install_motion_snsr();
void motion_snsr_event_Handler();
void IRAM_ATTR motion_snsr_ISR();

#endif /* MOTION_SNSR */
