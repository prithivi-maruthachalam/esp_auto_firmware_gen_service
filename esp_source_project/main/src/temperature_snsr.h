#ifndef _TMP_SNSR
    #define _TMP_SNSR
    #include "driver/adc.h"
    #include "esp_adc_cal.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "driver/gpio.h"
    #include "esp_log.h"
    #include "freertos/queue.h"

    #define TEMPERATURE_SENSOR_ADC_CHANNEL ADC1_CHANNEL_8
    #define temperature_snsr_STACK_SIZE 2048
    #define temperature_snsr_PRIORITY 5
    /*
    Message IDs for temperature sensor module tmpsnsr
    @note - Expand when new messages/events are required
    */
    typedef enum tmpSnsr_event_messages
    {
        TMPSNSR_NEW_MESSAGE_READ = 0,
        TEMPERATURE_THRESHOLD_CROSSED,
        TEMPERATURE_THRESHOLD_NOT_CROSSED
    }tmpSnsr_event_messages_e;

    /* Structure to hold events and corresponding data
    @note - Expand as required
     */
    typedef struct str_tmpSnsr_queue_messages
    {
        tmpSnsr_event_messages_e msg_id;
        char *data;
    }str_temperature_snsr_event_t;

    QueueHandle_t q_temperature_snsr_event;

    uint16_t adcDelayinMS;
    void tmpSnsr_init(adc_bits_width_t width, adc1_channel_t channel,adc_atten_t ADC_atten);
    void read_tmpSnsr();
    void install_temperature_snsr();
    void tmpSnsr_event_handler();
#endif