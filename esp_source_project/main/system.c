#include "system.h"

void systemInit()
{
    NVS_init();
	app_nvs_save_wifi_creds("Vk", "AsdZxc@123"); //get the creds from the user.
	if(app_nvs_check_wifi_creds())
	{
		wifi_app_start();
	}
    else
	{
		app_nvs_save_wifi_creds("Vk", "AsdZxc@123"); //get the creds from the user.
        wifi_app_start();
	}
    aws_iot_start();
}