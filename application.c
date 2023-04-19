#include "application.h"

void start_application()
{
	install_motion_snsr();
	install_LED();
	xTaskCreate(motion_snsr_event_Handler, "motion_snsr_Handler", motion_snsr_STACK_SIZE, NULL, motion_snsr_PRIORITY, NULL);
}

void motion_snsr_event_Handler()
{
	str_motion_snsr_event_t motion_snsr_msg;
	str_LED_event_t LED_msg;
	mqttData_str_t mqttData_e;
	while(1)
	{
		while(!xQueueReceive(q_motion_snsr_event,&motion_snsr_msg,portMAX_DELAY));
		switch(motion_snsr_msg.msg_id)
		{
			case MOTION_DETECTED:
				LED_msg.msg_id = LED_ON;
				xQueueSend(q_LED_event, &LED_msg, (TickType_t)0 );
				mqttData_e.data = motion_snsr_msg.data;
				xQueueSend(mqttData_Queue, &mqttData_e , (TickType_t)0 );
				break;
			default:
				break;
		}
	}
}