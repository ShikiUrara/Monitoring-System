#include <ESP8266WiFi.h>
#include "Protocol.h"
#include "MQTT.h"
#include "Communication.h"
#include "LoRaComm.h"
#include <FS.h>
#include <ArduinoJson.h>
#include "WebServerESP8266.h"
#include "Simpletimer.h"

#define ss 15
#define rst 16
#define dio0 A0
#define ledYellow D4
#define ledBlue D1
#define ledRed D3
#define TIME_AWAIT_ACK 3000

DynamicJsonDocument myFile(1024); // file JSON config 
DynamicJsonDocument docData(2048); // data JSON
JsonArray data = docData.createNestedArray("data");
const char *fileConfig = "/config.json";
const IPAddress ip(192, 168, 1, 1);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);
String nameGateway="gateway"; // name gateway default
String gatewayId="1";
int sendTime = 10000;// sendTime default
unsigned long previousMillis = 0;
unsigned long previousConfigMillis = 0;
bool sendLora = false;
String msgToLoRa="";
int count = 0;
String tempConfig = "";
int isReStart =0;

Simpletimer timer{};
Simpletimer timer1{};
WiFiClient espClient;
Protocol* mqtt ;
Communication* lora;
WebServerESP8266 webServer;

void setup() {
  Serial.begin(9600);
  
  pinMode(ledBlue, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  offAllLED();
  delay(1000);
  
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  //Load file config va luu vao myFile
  loadConfig(fileConfig);
  //WebServer
  webServer.setupWebServer();
  if(myFile.containsKey("gateway")){
    nameGateway=String(myFile["gateway"]);
  }
  //Start LoRa
  lora=new LoRaComm(ss, rst, dio0);
  if(myFile.containsKey("LoRa")){
    Serial.println("Start LoRa");
     long frequency = myFile["LoRa"]["frequency"];
     lora->begin(frequency);
     //blinkLED(ledRed, 1, 500); for false
     blinkLED(ledYellow, 1, 400);
  }
  //Start WiFi
  const char* ssid = "G513";
  const char* password = "DHCT0716";
  const char* ssidAP = "ESP8266-Access-Point";
  const char* passwordAP = "123456789";
  if(myFile.containsKey("Wifi")){
    ssid = myFile["Wifi"]["ssid"];
    password = myFile["Wifi"]["password"];
  }
  if (myFile.containsKey("WifiAP")){
    ssidAP = myFile["WifiAP"]["wifiAPSSID"];
    passwordAP = myFile["WifiAP"]["wifiAPPassword"];
  }
  setupWifi(ssid,password,ssidAP,passwordAP);
  //Start MQTT
  const char* mqtt_username = "api1@Iotlab";
  const char* mqtt_server = "dev.iotlab.net.vn";
  const char* mqtt_password = "Iotlab@2023";
  int port=1883;
  if (myFile.containsKey("MQTT")){
    //Serial.println("Start MQTT");
    mqtt_username = myFile["MQTT"]["mqtt_username"];
    mqtt_server = myFile["MQTT"]["mqtt_server"];
    mqtt_password = myFile["MQTT"]["mqtt_password"];
    port=myFile["MQTT"]["mqtt_port"];
  }
  mqtt =new MQTT (espClient, mqtt_server, mqtt_username, mqtt_password, port);

  if (myFile.containsKey("sendTime"))
    sendTime= int(myFile["sendTime"]);
  Serial.println("sendTime: " + String (sendTime));
  timer.register_callback(sendData);
  if (myFile.containsKey("gatewayId")) 
    gatewayId=myFile["gatewayId"].as<String>();
  //Complete setup
  blinkAllLED(3, 500);
}

void loop() {
  timer.run(sendTime);
  webServer.loopWebServer();
  //Config from WebServer
  String configResult =webServer.getConfigResult();
  if(configResult!=""){
    blinkLED(ledBlue,1,100);
    DynamicJsonDocument json(1024);
    deserializeJson(json, configResult);
    // json["station"]["gateway"]=nameGateway;
    json["station"]["gatewayId"]=gatewayId;
    configResult="";
    serializeJson(json, configResult);
    tempConfig= configResult;
    Serial.println (configResult);
    saveConfig(configResult);
  }
  //await ACK Node
  if (sendLora){
    unsigned long currentMillis = millis();
    DynamicJsonDocument tmpConfig(2048);
    deserializeJson(tmpConfig, tempConfig);
    
    if (currentMillis - previousConfigMillis <= 3000) {
      
      //Serial.println("con thoi gian");
      String msg = lora->receiveNode();
      if (msg!=""){
        DynamicJsonDocument doc(300);
        deserializeJson(doc, msg);
        //Serial.println(msg);
        if (doc["gatewayId"]==gatewayId && doc["status"]=="ACKCONFIG"){
          Serial.println("Send Config Complete");
          sendLora = false;
          tempConfig="";
          if (tmpConfig.containsKey("sendTime"))
            ESP.restart();
        }
      }
    }
    else {
      if(count <3){
        saveConfig(tempConfig); // resend config
        count++;
        previousConfigMillis = currentMillis;
        Serial.println("Time out");
      }
      else {
        tempConfig="";
        count=0;
        sendLora = false;
        if (tmpConfig.containsKey("sendTime"))
            ESP.restart();
      }
      
    }
  }
  //Dung LoRa nhan du lieu
  String message = lora->receiveNode(); 
  if (message!=""){
    Serial.println("Receive LoRa msg");
    blinkLED(ledYellow, 1, 100);
    DynamicJsonDocument jsonObject(1024); 
    deserializeJson(jsonObject, message);
    if (!docData.containsKey("data")) 
      data = docData.createNestedArray("data"); // Thêm đối tượng vào mảng data 
    JsonArray sensor = jsonObject["sensor"]; // lấy mảng sensor
    int n = sensor.size(); // lấy số lượng phần tử
    for (int i = 0; i < n; i++) { // duyệt qua từng phần tử JsonObject 
      sensor[i]["resultTime"]= jsonObject["resultTime"]; // thêm resultTime vào phần tử 
      data.add(sensor[i]); // thêm phần tử vào mảng data 
    }
    if (mqtt->connectDataCenter()) 
      lora->sendNode("{\"nodeId\":\"" + String(jsonObject["nodeId"]) +"\",\"status\":\"ACK\"}");
  }
  //Kiem tra ket noi voi MQTT
  if (mqtt->connectDataCenter()){
    // Xu li tin nhan config
    String msg = mqtt->receiveDataCenter("data/setting");
    if (msg != "") {
      Serial.println(msg);
      blinkLED(ledBlue,1,100);
      saveConfig(msg);
      tempConfig = msg;
      // newSaveConfig(msg);
    }
    digitalWrite(ledRed, LOW);
    
  }
  else {
    if(timer1.timer(500)){

    
    // unsigned long currentMillis = millis();
    // // Kiểm tra nếu đã đến thời điểm để nhấp nháy LED
    // if (currentMillis - previousMillis >= 500) {
    //   previousMillis = currentMillis;  // Cập nhật thời gian trước đó

    //   // Đảo trạng thái của LED
      if (digitalRead(ledRed) == HIGH) {
        digitalWrite(ledRed, LOW);
      } 
      else {
        digitalWrite(ledRed, HIGH);
      }}
    // }

  }

}
void sendData(){
   //Kiem tra ket noi voi MQTT
   //Serial.println("send ne");
  if (mqtt->connectDataCenter()){
    //docData["nameGateway"]=nameGateway;
    int n = data.size(); // lấy số lượng phần tử data
    for (int i = 0; i < n; i++) { // duyệt qua từng phần tử JsonObject 
      blinkLED(ledBlue, 1, 100);
      Serial.println("Send data");
      //Gui tin len data center
      //Chuyen chuoi JSON ve String
      String msgMQTT;
      serializeJson(data[i], msgMQTT);
      mqtt->sendDataCenter("data/view",msgMQTT.c_str());
    }
    docData.clear();
    // blinkLED(ledBlue, 1, 100);
    // //Gui tin len data center
    // mqtt->sendDataCenter("test0910",msgMQTT.c_str());
    // int n = data.size(); // lấy số lượng phần tử data
    // Serial.println("size: " + String(n));
    // if (n > 0){
    //   //Chuyen chuoi JSON ve String
    //   String msgMQTT;
    //   serializeJson(data[0], msgMQTT);
    //   blinkLED(ledBlue, 1, 100);
    //   mqtt->sendDataCenter("test0910",msgMQTT.c_str());
    //   data.remove(0);
    // }
    //docData.clear();
  }
}
void loadConfig(const char* fileConfig){
  // Mở file để đọc
  File file = SPIFFS.open(fileConfig, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  //  Serial.println("File Content:");
  //
  //  while(file.available()){
  //    Serial.write(file.read());
  //  }
  // Tạo một đối tượng JSON để lưu dữ liệu đọc từ file
  deserializeJson(myFile, file);
  file.close();
}

void setupWifi(const char* ssid, const char* password, const char* ssidAP, const char* passwordAP) {
  delay(10);

  // We start by connecting to a WiFi network

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  Serial.println("Accsess Point");
  Serial.println(ssidAP);
  Serial.println(passwordAP);
      // Tạo Access Point
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(ssidAP, passwordAP);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  int isBreak=0;
  while (WiFi.status() != WL_CONNECTED) {
    //delay(500);
    if(isBreak>10){
      blinkLED(ledRed, 1, 300); //for false
      Serial.println("Can't connect Wifi");
      break;
    }
    Serial.print(".");
    isBreak++;
    blinkLED(ledYellow, 1, 500); //for false
  }

  
  randomSeed(micros());
  if(WiFi.status()== WL_CONNECTED){
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

}
//newConfig for backend API 
void newSaveConfig(String msg){
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, msg);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  }
  else{
    if (doc["stationId"].as<String>()!=gatewayId)
      return;
    Serial.println("received config");
    DynamicJsonDocument docConfig(1024);
    if (doc.containsKey("configuration") && doc.containsKey("multiDataStreamId")){
      docConfig["configuration"]=doc["configuration"];
      docConfig["multiDataStreamId"]=doc["multiDataStreamId"];
      String msgConfig;
      serializeJson(docConfig, msgConfig);
      Serial.println(msgConfig);
      lora->sendNode(msgConfig);
      //awaitACK();
    }

    
  }
}

bool awaitACK(){
  Serial.println("Gateway send");
  unsigned long currentMillis = millis(); // lay mili tu luc gui
  while(millis() - currentMillis <=TIME_AWAIT_ACK){
    
    String msg = lora->receiveNode();
    if (msg!=""){
        DynamicJsonDocument doc(300);
        deserializeJson(doc, msg);
        if (doc["gatewayId"]==gatewayId && doc["status"]=="ACK"){
          Serial.println("ok");
          return true;
        }
    }

  }
  Serial.println("That bai");
  return false;
}

void saveConfig(String msg){
  DynamicJsonDocument doc(2048);
  //Serial.println(msg);
  DeserializationError error = deserializeJson(doc, msg);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  }
  else {
    if (String(doc["station"]["gatewayId"])!=gatewayId)
      return;
    Serial.println("received config");
    if(doc.containsKey("newNameGateway")){
      String newName= doc["newNameGateway"];
      myFile["nameGateway"]= newName;
    }
    
    msgToLoRa="";
    //co sensor thi lay ten node roi gui qua lora, ben node tu xac nhan xem phai cua minh khong
    if (doc.containsKey("sensor")){
      msgToLoRa+= "{\"station\":";
      serializeJson(doc["station"], msgToLoRa);
      msgToLoRa+= ",";
      msgToLoRa+= "\"sensor\":";
      //Chuyen chuoi JSON ve String
      serializeJson(doc["sensor"], msgToLoRa);
    }
    //neu co wifi thi gateway sua wifi, mqtt tuong tu
    if (doc.containsKey("Wifi")){
      myFile["Wifi"]["ssid"] = doc["Wifi"]["ssid"];
      myFile["Wifi"]["password"] = doc["Wifi"]["password"];
    }
    if (doc.containsKey("WifiAP")){
      myFile["WifiAP"]["wifiAPSSID"] = doc["WifiAP"]["APssid"];
      myFile["WifiAP"]["wifiAPPassword"] = doc["WifiAP"]["APpassword"];
    }
    if (doc.containsKey("MQTT")){
      myFile["MQTT"]["mqtt_server"] = doc["MQTT"]["mqtt_server"];
      myFile["MQTT"]["mqtt_username"] = doc["MQTT"]["mqtt_username"];
      myFile["MQTT"]["mqtt_password"] = doc["MQTT"]["mqtt_password"];
      myFile["MQTT"]["mqtt_port"] = doc["MQTT"]["mqtt_port"];
    }
    if (doc.containsKey("newIdGateway")){
      String newIdGateway= doc["newIdGateway"];
      gatewayId= newIdGateway;
      myFile["gateway"]= gatewayId;
    }
    if(doc.containsKey("newNodeId")){
      //String newNodeId= doc["newNodeId"];
      if (msgToLoRa==""){
        msgToLoRa+= "{\"station\":";
        serializeJson(doc["station"], msgToLoRa);
        msgToLoRa+= ",";
      }
      else
      msgToLoRa+= ",";
      msgToLoRa+= "\"newNodeId\":";
      //Chuyen chuoi JSON ve String
      serializeJson(doc["newNodeId"], msgToLoRa);
    }
    //sendtime thi gui cho tat ca node va gateway cung dieu chinh sendtime
    if (doc.containsKey("sendTime")){
      if(doc["station"].containsKey("nodeId")){
        if (msgToLoRa==""){
          msgToLoRa+= "{\"station\":";
          serializeJson(doc["station"], msgToLoRa);
          msgToLoRa+= ",";
        }
        else
          msgToLoRa+= ",";
        msgToLoRa+= "\"sendTime\":";
        serializeJson(doc["sendTime"], msgToLoRa);
      }
      
      //Chuyen chuoi JSON ve String
      myFile["sendTime"]= doc["sendTime"];
      
      
    }
    if (doc.containsKey("timeClock")){
      if (msgToLoRa==""){
        msgToLoRa+= "{\"station\":";
        serializeJson(doc["station"], msgToLoRa);
        msgToLoRa+= ",";
      }
      
      else
      msgToLoRa+= ",";
      msgToLoRa+= "\"timeClock\":";
      //Chuyen chuoi JSON ve String
      serializeJson(doc["timeClock"], msgToLoRa);
    }
    if (doc.containsKey("LoRa")){
      if (msgToLoRa==""){
        msgToLoRa+= "{\"station\":";
        serializeJson(doc["station"], msgToLoRa);
        msgToLoRa+= "}";
      }
      else
      msgToLoRa+= ",";
      msgToLoRa+= "\"LoRa\":";
      //Chuyen chuoi JSON ve String
      serializeJson(doc["LoRa"], msgToLoRa);
    }
    //con thieu dong bo  timeClock ()24h gui lan
    if (msgToLoRa!=""){
      msgToLoRa+="}";
      Serial.println ("msgLoRa: " + msgToLoRa);
      previousConfigMillis=millis();
      sendLora=true;
      lora->sendNode(msgToLoRa);
    }
    // Mở file để ghi
    File file = SPIFFS.open("/config.json", "w");
    if (!file) {
      Serial.println("There was an error opening the file for writing");
      return;
    }
    // Ghi đối tượng JSON vào file
    serializeJson(myFile, file);
    file.close();
    if (doc.containsKey("Wifi")||doc.containsKey("WifiAP")||doc.containsKey("MQTT"))
    ESP.restart();  
  }
}
void blinkLED(int pin, int numBlinks, int msdelay) {
  for (int i = 0; i < numBlinks; i++) {
    digitalWrite(pin, HIGH);
    delay(msdelay);
    digitalWrite(pin, LOW);
    delay(msdelay);
  }
}
//-----------------
void blinkAllLED(int numBlinks, int msdelay) {
  for (int i = 0; i < numBlinks; i++) {
    digitalWrite(ledBlue, HIGH);
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledYellow, HIGH);
    delay(msdelay);
    digitalWrite(ledBlue, LOW);
    digitalWrite(ledRed, LOW);
    digitalWrite(ledYellow, LOW);
    delay(msdelay);
  }
}
//------------------
void offAllLED() {
  digitalWrite(ledBlue, LOW);
  digitalWrite(ledRed, LOW);
  digitalWrite(ledYellow, LOW);
}
