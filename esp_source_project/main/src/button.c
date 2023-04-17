#include "button.h"

void install_button()
{
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL2);     //install the ISR Service
    q_button_event = xQueueCreate(2, sizeof(str_button_event_t)); //Create the event message queue
    gpio_set_direction(button_PIN, GPIO_MODE_INPUT);   //set the interrupt pin as INPUT
    gpio_set_intr_type(button_PIN, button_TRIGGER_LEVEL); //Set the trigger level - positive or negative or both
    gpio_isr_handler_add(button_PIN, button_ISR, NULL); //add the ISR

    //commenting the following line and moving it to application.c
    //xTaskCreate(button_event_Handler, "GPIOIntrptHandler", 2048, NULL, 10, NULL);  //Event handler task create

}

void IRAM_ATTR button_ISR()
{
    str_button_event_t msg;
    msg.msg_id = BUTTON_DETECTED;
    msg.data = "Button Detected";
    xQueueSend(q_button_event, &msg , (TickType_t)0 );
}

/**
 * @brief : Function to handle messages received over MQTT. 
 * 
 */
void button_mqtt_receive_handler()
{
    str_button_event_t msg;
    while(!xQueueReceive(q_button_event,&msg,portMAX_DELAY));
}