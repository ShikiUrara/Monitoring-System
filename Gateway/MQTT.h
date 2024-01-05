#ifndef MQTT_H
#define MQTT_H

#include "Protocol.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>


class MQTT : public Protocol {
private:
    PubSubClient client;
    String clientId;
    const char* username;
    const char* password;
//    String incommingMessage;
//    String lastTopic;
    String lastMessage;
    String currentTopic;
public:
    // Khoi tao doi tuong, setServer va xu ly tin nhan 
    MQTT(Client& espClient, const char* server, const char* username, const char* password, int port) 
        : client(espClient), username(username), password(password) {
        client.setServer(server, port);
        client.setBufferSize(1024);
        
        clientId = "ESP8266Client-" + String(random(0xffff), HEX);
         // Đặt hàm callback
         
        client.setCallback([this](char* topic, byte* payload, unsigned int length) {
          // Xử lý dữ liệu nhận được ở đây
//          incommingMessage = "";
//          lastTopic=String(topic);
//          for (int i = 0; i < length; i++) 
//              incommingMessage+=(char)payload[i];  
            for (int i = 0; i < length; i++) {
              lastMessage += (char)payload[i];
           }        
        });
    }
    
    bool connectDataCenter()  {
        if (!client.connected()) {
            if (client.connect(clientId.c_str(), username, password)) {
                Serial.println("Connected to MQTT server");            
                
            } else {
              
                Serial.print("Failed to connect, rc=");
                Serial.println(client.state());
                return false;
            }
        }
        client.loop();
        return true;
    }

    void sendDataCenter(const char* topic, const char* message)  {
        client.publish(topic, message);
    }

    String receiveDataCenter(const char* topic)  {
//     client.subscribe(topic);
//     if(incommingMessage!="" && lastTopic==String(topic)){
//      String temp= incommingMessage;
//      incommingMessage="";
//      return temp;
//     }
//     return "";
    if (!currentTopic.equals(topic)) {
      if (currentTopic.length() > 0) {
        client.unsubscribe(currentTopic.c_str());
      }
      client.subscribe(topic);
      currentTopic = topic;
    }
    client.loop();
    String message = lastMessage;
    lastMessage = "";
    return message;
    }
};
#endif
