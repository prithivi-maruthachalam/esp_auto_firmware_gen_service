/**
 * @file aws_iot.h
 * @author Vivek (vivek@denbu.io)
 * @brief 
 * @version 0.1
 * @date 2023-02-13
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef MAIN_AWS_IOT_H_
#define MAIN_AWS_IOT_H_

#define CONFIG_AWS_EXAMPLE_CLIENT_ID "Udemy_ESP32_Test"
#include "src/tasks_common.h"

//#ifdef LED_INSTALLED
#include "src/LED.h"
//#endif

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
#define MQTT_HOST_ADDRESS "a3lux0sh4x6pp2-ats.iot.ap-south-1.amazonaws.com"
// AWS IoT Task
#define AWS_IOT_TASK_STACK_SIZE				9216
#define AWS_IOT_TASK_PRIORITY				6
#define AWS_IOT_TASK_CORE_ID				1

typedef struct mqttData_str
{
    char *data;
}mqttData_str_t;



#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_



#endif /* MAIN_TASKS_COMMON_H_ */


/**
 * Starts AWS IoT task.
 */
void aws_iot_start(void);

#endif /* MAIN_AWS_IOT_H_ */
