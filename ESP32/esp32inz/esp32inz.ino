#include "secrets.h"
#include "WiFiClientSecure.h"
#include <MQTT.h>
#include <ArduinoJson.h>
#include "WiFi.h"

#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);
float temperature;
uint8_t cardstr[20];


void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  client.onMessage(messageHandler);
  Serial.print("Connecting to AWS IOT");
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  if(!client.connected()){
    Serial.println("AWS IoT Timeout!\n");
    return;
  }
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!\n");
}


void publishMessage()
{
    // Prepare cardstr as a hex string
    char cardstr_hex[41];  // Each byte -> 2 hex chars + 1 null terminator
    for (int i = 0; i < 20; i++) {
        sprintf(cardstr_hex + (i * 2), "%02X", cardstr[i]);
    }

    // Construct the JSON payload
    String payload = "{\"temperature\":";
    payload += String(temperature, 1);
    payload += ",\"cardstr\":\"";
    payload += cardstr_hex;
    payload += "\"}";

  client.publish(AWS_IOT_PUBLISH_TOPIC, payload.c_str());
  Serial.println("Data sent to AWS IoT: " + payload);
}

void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

//  StaticJsonDocument<200> doc;
//  deserializeJson(doc, payload);
//  const char* message = doc["message"];
}

void prepare_data() {
    char cardstr_hex[41];  
    for (int i = 0; i < 20; i++) {
        sprintf(cardstr_hex + (i * 2), "%02X", cardstr[i]);
    }
    String payload = "{\"temperature\":";
    payload += String(temperature, 2);
    payload += ",\"cardstr\":\"";
    payload += cardstr_hex;
    payload += "\"}";
    client.publish("esp32/telemetry", payload.c_str());
    Serial.println("Data sent to AWS IoT: " + payload);
}

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 16, 17);
    connectAWS();

   
}


void loop() {
    if (Serial2.available()) {
                String message = "";
        while (Serial2.available()) {
            char c = Serial2.read();
            message += c;
        }
        Serial.print("Received message: ");  // Debug print
        Serial.println(message);
        StaticJsonDocument<200> doc;

        if (message.startsWith("RFID:")) {
            message.remove(0, 5); // Remove "RFID:" prefix

            // Parse and add data to JSON document
            float temperature = message.substring(0, message.indexOf(',')).toFloat();
            String cardInfo = message.substring(message.indexOf(',') + 1);
            cardInfo.trim();

            // Fill JSON document
            doc["type"] = "RFID";
            doc["temperature"] = temperature;
            doc["cardstr"] = cardInfo;

            // Serialize JSON to String
            String jsonMessage;
            serializeJson(doc, jsonMessage);

            Serial.print("Publishing RFID JSON: ");
            Serial.println(jsonMessage);

            // Publish JSON to esp32/pub
            client.publish("esp32/pub", jsonMessage.c_str());

        } else if (message.startsWith("TELE:")) {
            message.remove(0, 5); // Remove "TELE:" prefix

            // Parse telemetry values
            float temperature = message.substring(0, message.indexOf(',')).toFloat();
            message = message.substring(message.indexOf(',') + 1);
            float pressure = message.substring(0, message.indexOf(',')).toFloat();
            message = message.substring(message.indexOf(',') + 1);
            float light_level = message.substring(0, message.indexOf(',')).toFloat();
            int people_count = message.substring(message.indexOf(',') + 1).toInt();

            // Fill JSON document
            doc["type"] = "TELE";
            doc["temperature"] = temperature;
            doc["pressure"] = pressure;
            doc["light_level"] = light_level;
            doc["people_count"] = people_count;

            // Serialize JSON to String
            String jsonMessage;
            serializeJson(doc, jsonMessage);

            Serial.print("Publishing TELE JSON: ");
            Serial.println(jsonMessage);

            // Publish JSON to esp32/telemetry
            client.publish("esp32/telemetry", jsonMessage.c_str());
        }
    }
    client.loop();
    delay(1000); // Loop delay
}
