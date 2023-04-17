#include "LED.h"

void LED_event_Handler()
{
    str_LED_event_t msg;
    while(1)
    {
        while(!xQueueReceive(q_LED_event,&msg,portMAX_DELAY));
        switch(msg.msg_id)
        {
            case LED_ON:
                printf("LED ON\n");
                gpio_set_level(LED_PIN, 1);
                break;
            case LED_OFF:
                printf("LED OFF\n");
                gpio_set_level(LED_PIN, 0);
                break;
            case LED_BLINK:
                //printf("LED BLINK\n");
                gpio_set_level(LED_PIN, 1);
                vTaskDelay(200 / portTICK_PERIOD_MS);
                gpio_set_level(LED_PIN, 0);
                vTaskDelay(200 / portTICK_PERIOD_MS);
                msg.msg_id = LED_BLINK;
                xQueueSend(q_LED_event, &msg, (TickType_t)0 );
                break;
            default:
                break;
        }
    }

}

void install_LED(void)
{
  gpio_pad_select_gpio(LED_PIN);
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
  q_LED_event = xQueueCreate(2, sizeof(str_LED_event_t)); //Create the event message queue
  xTaskCreate(LED_event_Handler, "LED_event_handler", LED_STACK_SIZE, NULL, 2, NULL);
}

