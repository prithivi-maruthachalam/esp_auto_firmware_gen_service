/**
 * @file motion_snsr.c
 * @author Vivek (vivek@denbu.io)
 * @brief 
 * @version 0.1
 * @date 2023-02-18
 * 
 * @copyright Copyright (c) 2023
 * 
 */



#include "motion_snsr.h"

void install_motion_snsr()
{
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL2);     //install the ISR Service
    q_motion_snsr_event = xQueueCreate(2, sizeof(str_motion_snsr_event_t)); //Create the event message queue
    gpio_set_direction(MOTION_SNSR_PIN, GPIO_MODE_INPUT);   //set the interrupt pin as INPUT
    gpio_set_intr_type(MOTION_SNSR_PIN, MOTION_SNSR_TRIGGER_LEVEL); //Set the trigger level - positive or negative or both
    gpio_isr_handler_add(MOTION_SNSR_PIN, motion_snsr_ISR, NULL); //add the ISR

    //commenting the following line and moving it to application.c
    //xTaskCreate(motion_snsr_event_Handler, "GPIOIntrptHandler", 2048, NULL, 10, NULL);  //Event handler task create

}

void IRAM_ATTR motion_snsr_ISR()
{
    str_motion_snsr_event_t msg;
    msg.msg_id = MOTION_DETECTED;
    msg.data = "Motion Detected";
    xQueueSend(q_motion_snsr_event, &msg , (TickType_t)0 );
}

/**
 * @brief : Function to handle messages received over MQTT. 
 * 
 */
void motion_snsr_mqtt_receive_handler()
{
    str_motion_snsr_event_t msg;
    while(!xQueueReceive(q_motion_snsr_event,&msg,portMAX_DELAY));
}