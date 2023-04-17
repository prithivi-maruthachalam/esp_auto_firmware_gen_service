/**
 * @file aws_iot.c
 * @author Vivek (vivek@denbu.io)
 * @brief 
 * @version 0.1
 * @date 2023-02-13
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/**
 * @file subscribe_publish_sample.c
 * @brief simple MQTT publish and subscribe on the same topic
 *
 * This example takes the parameters from the build configuration and establishes a connection to the AWS IoT MQTT Platform.
 * It subscribes and publishes to the same topic - "test_topic/esp32"
 *
 * Some setup is required. See example README for details.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "freertos/queue.h"

#include "src/aws_iot.h"


#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "cJSON.h"
#include "src/OTA.h"
#include "config.h"

static const char *TAG = "aws_iot";
void aws_SubscribeToTopic(AWS_IoT_Client *client, char *topic);
void aws_PublishToTopic();
// AWS IoT task handle
static TaskHandle_t task_aws_iot = NULL;
AWS_IoT_Client client;

mqttData_str_t mqttData_e;
QueueHandle_t mqttData_Queue;

/**
 * CA Root certificate, device ("Thing") certificate and device ("Thing") key.
 * "Embedded Certs" are loaded from files in "certs/" and embedded into the app binary.
 */
extern const uint8_t aws_root_ca_pem_start[] asm("_binary_root_cert_auth_pem_start");
extern const uint8_t certificate_pem_crt_start[] asm("_binary_client_crt_start");
extern const uint8_t private_pem_key_start[] asm("_binary_client_key_start");



/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
uint32_t port = AWS_IOT_MQTT_PORT;

void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData) {
    ESP_LOGI(TAG, "Subscribe callback Test: %.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *)params->payload);
    cJSON *root = cJSON_Parse(params->payload);
    cJSON *commandFor = cJSON_GetObjectItem(root,"Command for");
    cJSON *payload = NULL;
    switch (commandFor->valueint)
    {
        case 1:
            printf("Message received for OTA");
            payload = cJSON_GetObjectItem(root,"payload");
            printf("The payload is  = %s\n", payload->valuestring);
            if(strcmp(payload->valuestring,"OTA") == 0)
            {
                OTA_Update();
            }
            break;
        #ifdef motion_snsr_INSTALLED
        case 2:
            printf("Message for motion sensor\n");
            break;
        #endif
        #ifdef LED_INSTALLED
        case 3:
            printf("Message for LED\n");
            payload = cJSON_GetObjectItem(root,"payload");
            printf("The payload is  = %s\n", payload->valuestring);
            if(strcmp(payload->valuestring,"LED_ON") == 0)
            {
                str_LED_event_t msg;
                msg.msg_id = LED_ON;
                xQueueSend(q_LED_event,&msg,portMAX_DELAY);
            }
            else if(strcmp(payload->valuestring,"LED_OFF") == 0)
            {
                str_LED_event_t msg;
                msg.msg_id = LED_OFF;
                xQueueSend(q_LED_event,&msg,portMAX_DELAY);
            }
            else
            {
                printf("Wrong message for LED\n");
            }
            break;
        #endif
        default:
            printf("message is wrong");
            break;

    }
    //printf("the message is = %s",update_URL);
    //OTA_Update(update_URL);
}

void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
    ESP_LOGW(TAG, "MQTT Disconnect");
    IoT_Error_t rc = FAILURE;

    if(NULL == pClient) {
        return;
    }

    if(aws_iot_is_autoreconnect_enabled(pClient)) {
        ESP_LOGI(TAG, "Auto Reconnect is enabled, Reconnecting attempt will start now");
    } else {
        ESP_LOGW(TAG, "Auto Reconnect not enabled. Starting manual reconnect...");
        rc = aws_iot_mqtt_attempt_reconnect(pClient);
        if(NETWORK_RECONNECTED == rc) {
            ESP_LOGW(TAG, "Manual Reconnect Successful");
        } else {
            ESP_LOGW(TAG, "Manual Reconnect Failed - %d", rc);
        }
    }
}

void aws_iot_task(void *param) {

    IoT_Error_t rc = FAILURE;
    mqttData_str_t l_mqttData_e;
    
    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

    

    ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    mqttInitParams.enableAutoReconnect = false; // We enable this later below
    mqttInitParams.pHostURL = MQTT_HOST_ADDRESS;
    mqttInitParams.port = port;

    mqttInitParams.pRootCALocation = (const char *)aws_root_ca_pem_start;
    mqttInitParams.pDeviceCertLocation = (const char *)certificate_pem_crt_start;
    mqttInitParams.pDevicePrivateKeyLocation = (const char *)private_pem_key_start;

    mqttInitParams.mqttCommandTimeout_ms = 20000;
    mqttInitParams.tlsHandshakeTimeout_ms = 5000;
    mqttInitParams.isSSLHostnameVerify = true;
    mqttInitParams.disconnectHandler = disconnectCallbackHandler;
    mqttInitParams.disconnectHandlerData = NULL;

    rc = aws_iot_mqtt_init(&client, &mqttInitParams);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "aws_iot_mqtt_init returned error : %d ", rc);
        abort();
    }

    connectParams.keepAliveIntervalInSec = 10;
    connectParams.isCleanSession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;
    /* Client ID is set in aws_iot.h and AKA your Thing's Name in AWS IoT */
    connectParams.pClientID = CONFIG_AWS_EXAMPLE_CLIENT_ID;
    connectParams.clientIDLen = (uint16_t) strlen(CONFIG_AWS_EXAMPLE_CLIENT_ID);
    connectParams.isWillMsgPresent = false;

    ESP_LOGI(TAG, "Connecting to AWS...");
    do {
        rc = aws_iot_mqtt_connect(&client, &connectParams);
        if(SUCCESS != rc) {
            ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    } while(SUCCESS != rc);

    /*
     * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
     *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
     *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
     */
    rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d", rc);
        abort();
    }

    aws_SubscribeToTopic(&client,"esp/command");


    while((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)) {

        //Max time the yield function will wait for read messages
        
        if(NETWORK_ATTEMPTING_RECONNECT == rc) {
            // If the client is attempting to reconnect we will skip the rest of the loop.
            continue;
        }

        ESP_LOGI(TAG, "Stack remaining for task '%s' is %d bytes", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL));
        
        
        l_mqttData_e.data = "I am connected";
        xQueueSend(mqttData_Queue, &l_mqttData_e , (TickType_t)0 );
        vTaskDelay(10000 / portTICK_RATE_MS);
    }

    ESP_LOGE(TAG, "An error occurred in the main loop.");
    abort();
}

void aws_iot_start(void)
{
	if (task_aws_iot == NULL)
	{
        mqttData_Queue = xQueueCreate(5, sizeof(mqttData_str_t));
        xTaskCreate(aws_PublishToTopic, "aws_PublishToTopic", 14096, NULL, 10, NULL);

        printf("aws_PublishToTopic task created");
		xTaskCreatePinnedToCore(&aws_iot_task, "aws_iot_task", AWS_IOT_TASK_STACK_SIZE, NULL, AWS_IOT_TASK_PRIORITY, &task_aws_iot, AWS_IOT_TASK_CORE_ID);
        printf("aws_iot_task task created");
	}
}



void aws_SubscribeToTopic(AWS_IoT_Client *client, char *topic)
{
    const int topic_len = strlen(topic);
    IoT_Error_t rc = FAILURE;
    ESP_LOGI(TAG, "Subscribing...");
    rc = aws_iot_mqtt_subscribe(client, topic, topic_len, QOS0, iot_subscribe_callback_handler, NULL);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Error subscribing : %d ", rc);
        abort();
    }
    rc = aws_iot_mqtt_yield(&client, 100);
}

void aws_PublishToTopic()
{
    char *topic = "esp/data";
    IoT_Publish_Message_Params publishParams;
    mqttData_str_t l_mqttData_e;
    const int topic_len = strlen(topic);
    IoT_Error_t rc = FAILURE;
    //char cPayload[100];
    while(1)
    {
        while(!xQueueReceive(mqttData_Queue,&l_mqttData_e,2000));
        printf("*************************************Publishing new data***********************************\n");
        publishParams.qos = QOS1;
        publishParams.isRetained = 0;
        //sprintf(cPayload, data);
        publishParams.payload = l_mqttData_e.data;
        
        publishParams.payloadLen = strlen(l_mqttData_e.data);
        rc = aws_iot_mqtt_publish(&client, topic, topic_len, &publishParams);
        printf(" RC value after publish = %d\n",rc);
    }
    

}


