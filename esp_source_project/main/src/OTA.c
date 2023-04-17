#include "esp_https_ota.h"
#include "src/OTA.h"

void OTA_Update()
{
    str_LED_event_t LED_msg;
    esp_http_client_config_t ota_client_config = {
        .url = URL,
        //.cert_pem = server_cert_pem_start,
    };
    LED_msg.msg_id = LED_BLINK;
    xQueueSend(q_LED_event, &LED_msg.msg_id, (TickType_t)0 );
    esp_err_t ret = esp_https_ota(&ota_client_config);
    if (ret == ESP_OK) {
        printf("OTA OK, restarting...\n");
        esp_restart();
    } else {
        printf("OTA failed...\n");
        }
}