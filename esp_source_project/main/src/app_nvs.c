/**
 * @file app_nvs.c
 * @author Vivek (vivek@denbu.io)
 * @brief 
 * @version 0.1
 * @date 2023-02-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include<stdbool.h>
#include<stdio.h>
#include<string.h>

#include"esp_log.h"
#include"nvs_flash.h"

#include "app_nvs.h"
#include "app_wifi.h"

//NVS namespace used for creds
#define app_nvs_creds_namespace "wificreds"
#define device_UID_namespace "deviceID"

static const char TAG [] = "NVS_APP";

/**
 * @brief : Initialize the NVS before using it
 * @param : void
 * @return : void
 * 
 */
void NVS_init(void)
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
}

/**
 * @brief : Save the wifi creds to NVS
 * 
 * @param ssid 
 * @param password 
 * @return esp_err_t 
 */
esp_err_t app_nvs_save_wifi_creds(char *ssid, char *password)
{
    nvs_handle handle;
    esp_err_t esp_err;
    ESP_LOGI("NVS : ", "Saving credentials to flash");
    esp_err = nvs_open(app_nvs_creds_namespace, NVS_READWRITE, &handle);
    if(esp_err != ESP_OK)
    {
        printf("Error opening nvs handle : Error %d - %s",esp_err, esp_err_to_name(esp_err));
        return esp_err;
    }

    //set SSID
    esp_err = nvs_set_blob(handle, "ssid", ssid, MAX_SSID_LENGTH);
    if(esp_err != ESP_OK)
    {
        printf("Error saving SSID : Error %d - %s",esp_err, esp_err_to_name(esp_err));
        return esp_err;
    } //set SSID

    //set Password
    esp_err = nvs_set_blob(handle, "password", password, MAX_SSID_LENGTH);
    if(esp_err != ESP_OK)
    {
        printf("Error saving password : Error %d - %s",esp_err, esp_err_to_name(esp_err));
        return esp_err;
    }//set password

	//set wifi creds flag
	esp_err = nvs_set_u8(handle, "wifi_creds_flag", (uint8_t)1);
	if(esp_err != ESP_OK)
    {
        printf("Error saving flag : Error %d - %s",esp_err, esp_err_to_name(esp_err));
        return esp_err;
    }
	else{
		printf("flag saved : return %d - %s",esp_err, esp_err_to_name(esp_err));
	}
	//set flag
    //commit creds to NVS
    esp_err = nvs_commit(handle);
    if(esp_err != ESP_OK)
    {
        printf("Error commiting credentials : Error %d - %s",esp_err, esp_err_to_name(esp_err));
        return esp_err;
    }//commit creds to NVS

    nvs_close(handle);
    ESP_LOGI("NVS: ","Wrote wifi credentials to NVS");
    return ESP_OK;
}

/**
 * @brief : Load previously saved credentials from NVS
 * return : True if credentials found
 * 
 */
bool app_nvs_load_creds(void)
{
    nvs_handle handle;
	esp_err_t esp_err;

	ESP_LOGI(TAG, "app_nvs_load_sta_creds: Loading Wifi credentials from flash");

	if (nvs_open(app_nvs_creds_namespace, NVS_READONLY, &handle) == ESP_OK)
	{
		wifi_config_t *wifi_sta_config = wifi_app_get_wifi_config();

		if (wifi_sta_config == NULL)
		{
			wifi_sta_config = (wifi_config_t*)malloc(sizeof(wifi_config_t));
		}
		memset(wifi_sta_config, 0x00, sizeof(wifi_config_t));

		// Allocate buffer
		size_t wifi_config_size = sizeof(wifi_config_t);
		uint8_t *wifi_config_buff = (uint8_t*)malloc(sizeof(uint8_t) * wifi_config_size);
		memset(wifi_config_buff, 0x00, sizeof(wifi_config_size));

		// Load SSID
		wifi_config_size = sizeof(wifi_sta_config->sta.ssid);
		esp_err = nvs_get_blob(handle, "ssid", wifi_config_buff, &wifi_config_size);
		if (esp_err != ESP_OK)
		{
			free(wifi_config_buff);
			printf("app_nvs_load_sta_creds: (%s) no station SSID found in NVS\n", esp_err_to_name(esp_err));
			return false;
		}
		memcpy(wifi_sta_config->sta.ssid, wifi_config_buff, wifi_config_size);

		// Load Password
		wifi_config_size = sizeof(wifi_sta_config->sta.password);
		esp_err = nvs_get_blob(handle, "password", wifi_config_buff, &wifi_config_size);
		if (esp_err != ESP_OK)
		{
			free(wifi_config_buff);
			printf("app_nvs_load_sta_creds: (%s) retrieving password!\n", esp_err_to_name(esp_err));
			return false;
		}
		memcpy(wifi_sta_config->sta.password, wifi_config_buff, wifi_config_size);

		free(wifi_config_buff);
		nvs_close(handle);

		printf("app_nvs_load_sta_creds: SSID: %s Password: %s\n", wifi_sta_config->sta.ssid, wifi_sta_config->sta.password);
		wifi_app_send_message(WIFI_APP_MSG_LOAD_SAVED_CREDENTIALS);
		return wifi_sta_config->sta.ssid[0] != '\0';
	}
	else
	{
		return false;
	}
}

/**
 * @brief : clears wifi credentials from NVS
 * return : ESP_OK if cleared
 * 
 */
esp_err_t app_nvs_clear_wifi_creds(void)
{
	nvs_handle handle;
	esp_err_t esp_err;
	ESP_LOGI("NVS Clear:", "app_nvs_clear_sta_creds: Clearing Wifi station mode credentials from flash");

	esp_err = nvs_open(app_nvs_creds_namespace, NVS_READWRITE, &handle);
	if (esp_err != ESP_OK)
	{
		printf("app_nvs_clear_sta_creds: Error (%s) opening NVS handle!\n", esp_err_to_name(esp_err));
		return esp_err;
	}

	// Erase credentials
	esp_err = nvs_erase_all(handle);
	if (esp_err != ESP_OK)
	{
		printf("app_nvs_clear_sta_creds: Error (%s) erasing station mode credentials!\n", esp_err_to_name(esp_err));
		return esp_err;
	}

	// Commit clearing credentials from NVS
	esp_err = nvs_commit(handle);
	if (esp_err != ESP_OK)
	{
		printf("app_nvs_clear_sta_creds: Error (%s) NVS commit!\n", esp_err_to_name(esp_err));
		return esp_err;
	}
	nvs_close(handle);

	printf("app_nvs_clear_sta_creds: returned ESP_OK\n");
	return ESP_OK;
}

/**
 * @brief : Check if wifi creds are present in the NVS
 * 
 * @return true : if present
 * @return false : if not present
 */
uint8_t app_nvs_check_wifi_creds(void)
{
	
	esp_err_t err;
	nvs_handle_t my_handle;
	uint8_t creds_flag = 0; // value will default to 0, if not set yet in NVS
    err = nvs_open(app_nvs_creds_namespace, NVS_READONLY, &my_handle);
    if (err != ESP_OK) 
	{
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } 
	else 
	{
        
        err = nvs_get_u8(my_handle, "wifi_creds_flag", &creds_flag);
        nvs_close(my_handle);
    }
	return creds_flag;
}