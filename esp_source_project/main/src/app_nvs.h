/**
 * @file app_nvs.h
 * @author Vivek (vivek@denbu.io)
 * @brief 
 * @version 0.1
 * @date 2023-02-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef APP_NVS
#define APP_NVS

    #define MAX_SSID_LENGTH				32

#include "esp_err.h"
#include "esp_log.h"
    /**
     * @brief : Initialize NVS
     * 
     */
    void NVS_init(void);
    /**
     * @brief : Save wifi credentials to NVS
     * return : ESP_OK if successful
     * 
     * 
     */
    esp_err_t app_nvs_save_wifi_creds(char *ssid, char *password);

    /**
     * @brief : Load previously saved credentials from NVS
     * return : True if credentials found
     * 
     */
    bool app_nvs_load_creds(void);

    /**
     * @brief : clears wifi credentials from NVS
     * return : ESP_OK if cleared
     * 
     */
    esp_err_t app_nvs_clear_wifi_creds(void);

    /**
     * @brief 
     * 
     */
    uint8_t app_nvs_check_wifi_creds(void);

    void app_nvs_get_deviceID(void);    //todo
    void app_nvs_set_deviceID(void);    //todo
#endif /* APP_NVS */
