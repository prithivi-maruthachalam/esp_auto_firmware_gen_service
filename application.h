#ifndef APPLICATION
#define APPLICATION

#include "src/aws_iot.h"
#include "src/motion_snsr.h"
#include "src/LED.h"
void motion_snsr_event_Handler();
void start_application();
extern QueueHandle_t mqttData_Queue;
#endif /* APPLICATION */