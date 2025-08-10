#include"Protocol_mqtt.h"

#define CONFIG_BROKER_URI "mqtt://192.168.56.38:1883"
 esp_mqtt_client_handle_t client;
/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD("mqtt", "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI("mqtt", "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        ESP_LOGI("mqtt", "sent publish successful, msg_id=%d", msg_id);
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        ESP_LOGI("mqtt", "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        ESP_LOGI("mqtt", "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        ESP_LOGI("mqtt", "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI("mqtt", "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI("mqtt", "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI("mqtt", "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI("mqtt", "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI("mqtt", "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI("mqtt", "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI("mqtt", "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI("mqtt", "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI("mqtt", "Other event id:%d", event->event_id);
        break;
    }
}

void Protocol_mqtt_Init(void){
    const esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = CONFIG_BROKER_URI,
        .credentials.client_id = "aaa",
        
    };
     client = esp_mqtt_client_init(&mqtt_cfg);
     esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void Protocol_mqtt_Publish(bool sw,int speed){
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"protocol","modbus");
    cJSON_AddStringToObject(root,"accesstype","write");
    cJSON_AddNumberToObject(root,"id",5);
    cJSON_AddNumberToObject(root,"switch",sw);
    if(speed>=0){
        cJSON_AddNumberToObject(root,"dir",1);
        cJSON_AddNumberToObject(root,"speed",speed);
    }else{
        cJSON_AddNumberToObject(root,"dir",0);
        cJSON_AddNumberToObject(root,"speed",-speed);
    }
    char*str = cJSON_PrintUnformatted(root);
    esp_mqtt_client_publish(client,"wangwang/W5500/Subtopic",str,0, 0, 0);
    cJSON_Delete(root);
    printf("\r\n\r\n%s\r\n\r\n",str);
    cJSON_free(str);
}