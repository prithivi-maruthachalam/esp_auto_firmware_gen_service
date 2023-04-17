#include "temperature_snsr.h"

extern uint16_t adcDelayinMS=1000;

void tmpSnsr_init(adc_bits_width_t width, adc1_channel_t channel,adc_atten_t ADC_atten)
{
    adc1_config_width(width);
    adc1_config_channel_atten(channel,ADC_atten);
    
    q_temperature_snsr_event = xQueueCreate(5, sizeof(str_temperature_snsr_event_t));
    if(q_temperature_snsr_event ==0)
    {
        ESP_LOGE("Queue: ", "Creation Error");
    }
    xTaskCreate(read_tmpSnsr, "read_tmpSnsr", 4096, NULL, 10, NULL);
}

/* Read the temperature sensor value and write into buffer */
void read_tmpSnsr()
{
    str_temperature_snsr_event_t msg;
    char data[50];
    while(1)
    {
        uint16_t val = adc1_get_raw(TEMPERATURE_SENSOR_ADC_CHANNEL);
        //printf("Raw value inside read API= %u\n",val);
        msg.data = val;
        printf("Raw value inside read API= %u\n",val);
        if(val > 2000)
        {
            //write val to msg.data
            sprintf(data,"TEMPERATURE_THRESHOLD_CROSSED - %u",val);
            //printf("data = %s",data);
            msg.msg_id = TEMPERATURE_THRESHOLD_CROSSED;
        }
        else
        {
            //write val to msg.data
            sprintf(data,"TEMPERATURE_THRESHOLD_NOT_CROSSED - %u",val);
            //printf("data = %s",data);

            msg.msg_id = TEMPERATURE_THRESHOLD_NOT_CROSSED;
        }
        //write data to msg.data
        msg.data = &data;
        xQueueSend(q_temperature_snsr_event, &msg , (TickType_t)0 );
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}


void install_temperature_snsr()
{
    tmpSnsr_init(ADC_WIDTH_BIT_12,TEMPERATURE_SENSOR_ADC_CHANNEL,ADC_ATTEN_DB_11);
}

void tmpSnsr_event_handler()
{
    str_temperature_snsr_event_t msg;
    //mqttDataStr vmqttData1;

    while(1)
    {
        while(!xQueueReceive(q_temperature_snsr_event,&msg,2000));
        switch(msg.msg_id)
        {
            case TMPSNSR_NEW_MESSAGE_READ:  //write the data to consumer buffers. One consumer is MQTT
                //ESP_LOGE("Tmp Snsr: ", "New data read");
                //ESP_LOGI("consumeADC","RAW Value = %s",msg.data);
            default:
                break;
        }

    }
}