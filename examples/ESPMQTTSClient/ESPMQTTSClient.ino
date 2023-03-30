/**
 * @file      ESPMQTTSClient.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-03-30
 *
 */
#include <ETH.h>
#include "mqtt_client.h"

#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
// Pin# of the enable signal for the external crystal oscillator (-1 to disable for internal APLL source)
#define ETH_POWER_PIN -1
// Type of the Ethernet PHY (LAN8720 or TLK110)
#define ETH_TYPE      ETH_PHY_LAN8720
// I²C-address of Ethernet PHY (0 or 1 for LAN8720, 31 for TLK110)
#define ETH_ADDR      0
// Pin# of the I²C clock signal for the Ethernet PHY
#define ETH_MDC_PIN   23
// Pin# of the I²C IO signal for the Ethernet PHY
#define ETH_MDIO_PIN  18
#define NRST          5

const char *server_cert_pem =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL\r\n"
    "BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG\r\n"
    "A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU\r\n"
    "BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv\r\n"
    "by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE\r\n"
    "BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES\r\n"
    "MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp\r\n"
    "dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ\r\n"
    "KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg\r\n"
    "UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW\r\n"
    "Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA\r\n"
    "s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH\r\n"
    "3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo\r\n"
    "E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT\r\n"
    "MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV\r\n"
    "6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\r\n"
    "BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC\r\n"
    "6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf\r\n"
    "+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK\r\n"
    "sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839\r\n"
    "LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE\r\n"
    "m/XriWr/Cq4h/JfB7NTsezVslgkBaoU=\r\n"
    "-----END CERTIFICATE-----\r\n";

const char *client_cert_pem =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDiTCCAnGgAwIBAgIBADANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCR0Ix\r\n"
    "FzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTESMBAGA1UE\r\n"
    "CgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVpdHRvLm9y\r\n"
    "ZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzAeFw0yMzAyMDkwODAy\r\n"
    "NTNaFw0yMzA1MTAwODAyNTNaMGMxCzAJBgNVBAYTAkNOMQswCQYDVQQIDAJGSjEL\r\n"
    "MAkGA1UEBwwCWE0xDzANBgNVBAoMBkxpbHlHbzEPMA0GA1UEAwwGTGlseUdvMRgw\r\n"
    "FgYJKoZIhvcNAQkBFglsaWx5Z28uY2MwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAw\r\n"
    "ggEKAoIBAQCV6wb9xNQHVJh0qiY4dix9ZdC4FkY+olJ4l3NlkEwde0LMq0G6N/vl\r\n"
    "zJjVcZ4CkKdDXfoKhwexzAH7WQVMe15xaU0MfwNkuUIv89xqZkPUIdho87WSQ1nU\r\n"
    "qXcp4DzMZYanxitIa5MGwC1ihN5wrU0LGbna2gbkMb5qWloMJjLyRDZatKXTJRXK\r\n"
    "tossnmWiqgjkzQC+83rWD27Id9x6W2fRDcwX50ZUIR8zCVXP5d6db/1rXsLuro3I\r\n"
    "D9H15o0o/Aqb8r5qnIUJ1c6HpPgm9/z5DzsCkYP+wnhyE1f4/5KcYS0gdAWbhXLZ\r\n"
    "ZSEFBfh/upywgXRPtQQR7REg72l0Cbi3AgMBAAGjGjAYMAkGA1UdEwQCMAAwCwYD\r\n"
    "VR0PBAQDAgXgMA0GCSqGSIb3DQEBCwUAA4IBAQB8iF2xTNVdzLPpik0eiMy0VoNN\r\n"
    "3iVazoxSi3ctIJEnWYJjyjtzhiF9Uf7QelCzClFo/HgAeyB0b4aOGtMHoMyTtt6Q\r\n"
    "4HUUGBhiqO1dMs7z3RDtjepNPXA2gYxQkC/CGfWiJ4Ufhtfhhv9UixG96v/pBhtV\r\n"
    "vtOxjj0Rzhe2ZhygvsZZkTGKVwy2+ooKNvOXdTcFsqdVC8nt/omTfYDdUNQw/7Zq\r\n"
    "tNfqsIDmDGpnlrCRAjMU+UajGDqJLKOdtdHgY2CkR6zP527gHudAhXrAPDZ7uhKG\r\n"
    "tXU5j+BIjZZ4LQa1W5/IYKklH2Su7FKLAHM6atz4AR47vCeUB38j5qCfxxfu\r\n"
    "-----END CERTIFICATE-----\r\n";


const char *client_key_pem =
    "-----BEGIN RSA PRIVATE KEY-----\r\n"
    "MIIEpQIBAAKCAQEAlesG/cTUB1SYdKomOHYsfWXQuBZGPqJSeJdzZZBMHXtCzKtB\r\n"
    "ujf75cyY1XGeApCnQ136CocHscwB+1kFTHtecWlNDH8DZLlCL/PcamZD1CHYaPO1\r\n"
    "kkNZ1Kl3KeA8zGWGp8YrSGuTBsAtYoTecK1NCxm52toG5DG+alpaDCYy8kQ2WrSl\r\n"
    "0yUVyraLLJ5loqoI5M0AvvN61g9uyHfceltn0Q3MF+dGVCEfMwlVz+XenW/9a17C\r\n"
    "7q6NyA/R9eaNKPwKm/K+apyFCdXOh6T4Jvf8+Q87ApGD/sJ4chNX+P+SnGEtIHQF\r\n"
    "m4Vy2WUhBQX4f7qcsIF0T7UEEe0RIO9pdAm4twIDAQABAoIBAF6LyW3zWtCArmEt\r\n"
    "CUukY2x18IqYKZbIogBBa1pLBWW2Xatb+eip8+e5/0zlCYSOm53lv5IyFE0x3rIY\r\n"
    "MzpHt4kIornVxFsaZr59Ka3EbtRyv9t4jzL93MI8WdWEAHPbN2/Jj3Rtu1yPiE/Q\r\n"
    "gcRH1wXAJLD7vUregDrsXku5L3oPSal1/dgjy0y4YuZP9Mq5arncD5iTSEBFNy3Z\r\n"
    "R3iLzjo/UuBMg4iSz/nf3L5G/nsCR262kqec4OlBTDnmDONxV4dAi/MbjuYP2k5D\r\n"
    "sjRUFRvjcT2UUQ8W/YnoqSpifU8UKfpBsa8A/vlgDUyusALMXTvYLUo0aftsjAIl\r\n"
    "iE1vJPkCgYEAxQ4ZKm4fJL2Z+XaQ9x2//UIgN4ZTFruTYVRQ6qlw9wXN+8M0ndd9\r\n"
    "RXxJT6JFyj4KNakaBIy2bV6t1aH8HVwAIMgzs6edXXp15n9JQUj6D4IIIhgkaGXP\r\n"
    "UAuWkxDeK8zvDmi0R0Ns0HKZwL0HUHWNAwwyXNcBm2oP695zUsl3kwsCgYEAwsNQ\r\n"
    "yB4ci+7B2w+nf4LTWrrVHJZNxY4I+Syr9OIVY/dPKu5r/J/OjdDW/L56OVZ619BB\r\n"
    "GwelglexEpM3N7SqVW+tW3IQZ40zvdn6dG/8iyyWBuQvfKjWPaVimw3nKyDsb85A\r\n"
    "4v01fexiX3pIeJKqH0Fb4APhKXdxYA9uZ0ezfIUCgYEAixyK05NuVQes/ZmgeXP9\r\n"
    "J5NZHxmq6q99OgbGIQOBhkIIyqViLdQE0dsN/jP2xPHLT1qTzYZw+wEOA3UZgLiE\r\n"
    "NqBfL4My6t0bAp/1XSthrTUE/NzCRxkoo7+qiyQrVAyW7zsnme+VkAp3VMOduEVk\r\n"
    "STSBEV2P+uGDX+Zoz5b+3UUCgYEAkYUr64oMHEXVlF7a8LwIPUdRih2HmG8qpzrI\r\n"
    "dJ0kDGAxR8uC5etlgrKin4+WdRb3jB6vNon9ESmCRXW3kSitCE78AVJ1jVmEanre\r\n"
    "1ncCA97Zbea60HK6OK9EwMOfkKr13ggGlVNJz396dQpB8czrzZShF/zuFHSJQpPl\r\n"
    "EsgRBJ0CgYEAuzPXKnfTWpo3ucAeD+UpATiXxym6TgIsaZ2KpyUEaC9GHYBsNVrY\r\n"
    "IV7h4U0HWtRAYfTBrd5h7E70h5EkIHAuDTi1XCrB74S1C77BJnKJA85Q0EZ6Dg4n\r\n"
    "SfAFa5n69xGvwD6EDk5XbFpoVFBqL07rwqDWHXQNMB5+EPtsI5PTX1g=\r\n"
    "-----END RSA PRIVATE KEY-----\r\n";


const char *clien_id = "esp32eth";
const char *username = "empty";
const char *password = "empty";
bool eth_connected = false;

WiFiClient ethClient;

void WiFiEvent(WiFiEvent_t event)
{
    switch (event) {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        //set eth hostname here
        ETH.setHostname("esp32-ethernet");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        Serial.print(ETH.localIP());
        if (ETH.fullDuplex()) {
            Serial.print(", FULL_DUPLEX");
        }
        Serial.print(", ");
        Serial.print(ETH.linkSpeed());
        Serial.println("Mbps");
        eth_connected = true;
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }
}


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        Serial.printf("Last error %s: 0x%x\n", message, error_code);
    }
}

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
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        Serial.println("MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        Serial.printf("sent subscribe successful, msg_id=%d\n", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        Serial.printf("sent subscribe successful, msg_id=%d\n", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        Serial.printf("sent unsubscribe successful, msg_id=%d\n", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        Serial.println("MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        Serial.printf("MQTT_EVENT_SUBSCRIBED, msg_id=%d\n", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        Serial.printf("sent publish successful, msg_id=%d\n", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        Serial.printf("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        Serial.printf("MQTT_EVENT_PUBLISHED, msg_id=%d\n", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        Serial.println("MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        Serial.println("MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            Serial.printf("Last errno string (%s)\n", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        Serial.printf("Other event id:%d\n", event->event_id);
        break;
    }
}



void setup()
{
    Serial.begin(115200);

    WiFi.onEvent(WiFiEvent);

    pinMode(NRST, OUTPUT);
    digitalWrite(NRST, 0);
    delay(200);
    digitalWrite(NRST, 1);
    delay(200);
    digitalWrite(NRST, 0);
    delay(200);
    digitalWrite(NRST, 1);


    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN,
              ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);


    while (!eth_connected) {
        Serial.println("Wait eth connect..."); delay(2000);
    }


    esp_mqtt_client_config_t mqtt_cfg = {0};

#if (ESP_IDF_VERSION_MAJOR == 4) && (ESP_IDF_VERSION_MINOR == 4)

    mqtt_cfg.uri = "mqtts://test.mosquitto.org:8884";

    //Use certificate
    mqtt_cfg.client_cert_pem = (const char *)client_cert_pem;
    mqtt_cfg.client_key_pem = (const char *)client_key_pem;
    mqtt_cfg.cert_pem = (const char *)server_cert_pem;

    //Use username and password
    // mqtt_cfg.client_id = clien_id;
    // mqtt_cfg.username = username;
    // mqtt_cfg.password = password;
#else
#error "Not support this version";
//todo : Added IDF 5.0 Support
#endif

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void loop()
{
    delay(500);
}
