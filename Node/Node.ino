
#include <ArduinoJson.h>
#include "Sensor/SensorFactory.h"
#include <FS.h>
#include <SPIFFS.h>
#include "Communication/Communication.h"
#include "Communication/LoRaComm.h"
#include "Simpletimer.h"
#include "RTClib.h"


#define ss 5
#define rst 13
#define dio0 2
#define ledYellow 4
#define ledBlue 14
#define ledRed 27
#define MAX_SENSORS 5
#define TIME_AWAIT_ACK 3000
#define MAX_SIZE_FILE_SAVE_DATA 102400
#define MAX_NUM_FILE_DATA 3

DynamicJsonDocument myFile(1024); // file JSON config 
JsonArray sensorArray; // JSON array sensor lấy từ FFS
Analog* analogList[MAX_SENSORS];  // Tạo một mảng có thể chứa 5 đối tượng Analog*
RS485* RS485List[MAX_SENSORS];  // Tạo một mảng có thể chứa 5 đối tượng Analog*
const char* fileConfig = "/config.json";
int sendTime=10000; // default sendTime
String nodeId="1";

Simpletimer timer1{};
RTC_DS3231 rtc;
//Tao doi tuong lora tu interface Communication
Communication* lora;
SensorFactory factory;
void setup() {
  Serial.begin(9600);
  pinMode(32, OUTPUT);        
  pinMode(33, OUTPUT);
  digitalWrite(32, HIGH);       
  digitalWrite(33, HIGH);
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  offAllLED();
    if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  // rtc.begin();
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  //Load file config va luu vao myFile
  loadConfig(fileConfig);
  //Start LoRa
  lora=new LoRaComm(ss, rst, dio0);
  if(myFile.containsKey("LoRa")){
     long frequency = myFile["LoRa"]["frequency"];
     lora->begin(frequency);
     Serial.println(frequency);
  }
  else
    lora->begin(433E6);

  // Kiểm tra Sensor có tồn tại trong fileConfig không
  if(myFile.containsKey("sensor")){
    //Add AnalogSensor to array
    sensorArray = myFile["sensor"].as<JsonArray>();
    // Loop over all objects in the "sensor" array
    for(JsonVariant v : sensorArray) {
      JsonObject sensorObj = v.as<JsonObject>();
      createSensor(sensorObj);
    }
    //Serial.println("sensorArray");
    // Test println sensorArray
    //serializeJsonPretty(sensorArray, Serial);
    //Serial.println();
  }
  if(myFile.containsKey("sendTime"))
    sendTime=myFile["sendTime"];
  if(myFile.containsKey("nodeId"))
    nodeId=myFile["nodeId"].as<String>();
  //Serial.println("[APP] Free memory: " + String(esp_get_free_heap_size()/1024) );
  blinkAllLED(3,500);
}

void loop() {
  if (timer1.timer(sendTime)) {
    String tempData= readData();
    Serial.println(tempData);
    blinkLED(ledBlue,1,100);
    lora->send(tempData);
    //Serial.println("[APP] Free memory dau vong lap: " + String(esp_get_free_heap_size()/1024) );
    if (!awaitACK()){
      blinkLED(ledRed,1,100);
      saveDataSendFailed(tempData);
    }
    else{
      //gui du lieu cu
      sendDataFailed();
      }  
  }
  
  String msg = lora->receive();
  if (msg!=""){
    blinkLED(ledYellow,1,100);
    Serial.println(msg);
    saveConfig(msg);
    newSaveConfig(msg);
  }
  
  
}

void sendDataFailed(){
  DynamicJsonDocument dataDoc(1024);
  DynamicJsonDocument doc(MAX_SIZE_FILE_SAVE_DATA + 1024);
  for (int i = 1; i <= MAX_NUM_FILE_DATA; i++) {
    String fileName = "/data" + String(i) + ".json";
    if (SPIFFS.exists(fileName)) {
      File file = SPIFFS.open(fileName, "r");
      deserializeJson(doc, file);
      JsonArray dataFailed = doc["dataFailed"].as<JsonArray>();
      if (dataFailed.size() > 0) { // Kiểm tra nếu mảng dataFailed không rỗng
        Serial.println("Frist file dataFailed array not null: " + fileName);
        Serial.println("Num index dataFailed array: " + String(dataFailed.size()));
        dataFailed[0]["nodeId"]=nodeId;
        dataDoc=dataFailed[0];
        file.close();
        String output;
        serializeJson(dataDoc, output); // Chuyển mảng data thành chuỗi
        Serial.println("send old data");
        blinkLED(ledBlue,1,100);
        lora->send(output);
        if (awaitACK()){
          dataFailed.remove(0);
          Serial.println("Index dataFailed array after rmv: " + String(dataFailed.size()));
          // Mở file để ghi
          File file = SPIFFS.open(fileName, FILE_WRITE);
          if (!file) {
            Serial.println("There was an error opening the file for writing");
            return;
          }
          // Ghi đối tượng JSON vào file
          serializeJson(doc, file);
          file.close();
        }
        break;
      }
      file.close();
    }
  }
}
void saveDataSendFailed(String tempData){
      bool allFilesLargerThan100KB = true;
      DynamicJsonDocument tmp(1024);
      deserializeJson(tmp, tempData);
      //tao obj de them vao mang dataFailed
      JsonObject dataFailed = tmp["dataFailed"].createNestedObject(); // Tạo một JsonObject mới trong mảng
      dataFailed["sensor"] = tmp["sensor"]; // Thêm trường nameNode
      dataFailed["resultTime"] = tmp["resultTime"]; // Thêm trường resultTime
      // Tạo một đối tượng JSON để lưu dữ liệu đọc từ file
      DynamicJsonDocument dataSendFailed(MAX_SIZE_FILE_SAVE_DATA + 1024);
      for (int i = 1; i <= MAX_NUM_FILE_DATA; i++) {
        String filename = "/data" + String(i) + ".json";
        // Mở file để đọc
        File file = SPIFFS.open(filename, "r");
        if (!file) {
          Serial.println("Failed to open file for reading");
          continue;
        }
        size_t size = file.size();
        if(size < MAX_SIZE_FILE_SAVE_DATA){ //Neu nho hon 100KB
          Serial.println("Write file: " + filename);
          Serial.println("Size : " + String(size));
          //Serial.println("[APP] Free memory: " + String(esp_get_free_heap_size()/1024) );
          allFilesLargerThan100KB = false;
          //Luu file vao json dataSendFailed
          deserializeJson(dataSendFailed, file);
          file.close();
          // thêm vào mảng
          JsonArray dataArray = dataSendFailed["dataFailed"];
          if (!dataSendFailed.containsKey("dataFailed")){
            Serial.println("create dataFailed array ");
            dataArray=dataSendFailed.createNestedArray("dataFailed");
          }
          dataArray.add(dataFailed);
          Serial.println("Index array current: " + String(dataArray.size()));
          // Mở file để ghi
          file = SPIFFS.open(filename, FILE_WRITE);
          if (!file) {
            Serial.println("There was an error opening the file for writing");
            return;
          }
          // Ghi đối tượng JSON vào file
          serializeJson(dataSendFailed, file);
          file.close();
          dataSendFailed.clear();
          return;
        }
      }
      if(allFilesLargerThan100KB){
        
        Serial.println("All file Larger Than 100KB");
        
        String timeOld="2999-1-1T00:00:00";
        // Chuyển đổi chuỗi thời gian thành đối tượng DateTime
        DateTime oldestTime  = DateTime(timeOld.c_str());
        String oldestFile="/data1.json";
        // Mở file để đọc
        for (int i = 1; i <= MAX_NUM_FILE_DATA; i++) {
          String filename = "/data" + String(i) + ".json";
          File file = SPIFFS.open(filename, "r");
          if (!file) {
            Serial.println("Failed to open file for reading");
            continue;
          }
          //luu file vao dataSendFailed
          deserializeJson(dataSendFailed, file);
          file.close();
          //Lay vi tri dau file 
          String timeStr = dataSendFailed["dataFailed"][0]["resultTime"];
          DateTime timeTmp  = DateTime(timeStr.c_str());
          //Tim file co thoi gian cu nhat
          if(timeTmp < oldestTime){
            oldestTime=timeTmp;
            oldestFile=filename;
            Serial.println(timeStr);
            Serial.println(oldestFile);
          }    
      }
      Serial.println("Index array current: " + String(dataSendFailed["dataFailed"].size()));
      dataSendFailed["dataFailed"].remove(0);
      Serial.println("After rmv, Index array: " + String(dataSendFailed["dataFailed"].size()));
      dataSendFailed["dataFailed"].add(dataFailed);
      Serial.println("After add, Index array: " + String(dataSendFailed["dataFailed"].size()));
      // Mở file để ghi
      File file = SPIFFS.open(oldestFile, FILE_WRITE);
      if (!file) {
        Serial.println("There was an error opening the file for writing");
        return;
      }
      Serial.println("Write file: " + oldestFile);
      // Ghi đối tượng JSON vào file
      serializeJson(dataSendFailed, file);
      dataSendFailed.clear();
      file.close();
    }
}
bool awaitACK(){
  Serial.print("Node send ");
  unsigned long currentMillis = millis(); // lay mili tu luc gui
  unsigned long tempMillis = millis();
  while(tempMillis - currentMillis <=TIME_AWAIT_ACK){
    tempMillis = millis(); //mili trong lúc đợi
    String msg = lora->receive();
    if (msg!=""){
        DynamicJsonDocument doc(300);
        deserializeJson(doc, msg);
        if (doc["nodeId"]==nodeId && doc["status"]=="ACK"){
          Serial.println("complete");
          return true;
        }
    }
  }
  Serial.println("failed");
  return false;
}

void newSaveConfig(String json){
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  //Kiem tra xem phai gui cho node nay khong
  if(doc["configuration"]["nodeId"].as<String>()!=nodeId)
    return;
  String sensorName = doc["configuration"]["name"];
  String action = doc["configuration"]["action"];
  String type = doc["configuration"]["type"];
  String multiDataStreamId = doc["multiDataStreamId"];
  // Mảng chứa tất cả các số indexAnalog  có thể
  int possibleIndexAnalog[] = {0, 1, 2, 3, 4};
  // Mảng chứa tất cả các số indexRS485  có thể
  int possibleIndexRS485[] = {0, 1, 2, 3, 4};
  //bien check xem da co Sensor trong json file chua
  bool containSensor=false;
  // Loop over all objects in the "sensor" array
  for(JsonVariant v1 : sensorArray) {
    JsonObject sensorObjT = v1.as<JsonObject>();
    //Kiem tra sensor co ton tai chua
    if(sensorObjT["dataStreamId"].as<String>()==multiDataStreamId){
      Serial.println("Co dataStreamId: " + multiDataStreamId);
      containSensor=true;
    }
    //Nếu acction là add -> đánh dấu các count đã tạo đối tượng sensor
    if(action=="add"){
      //Danh dau indexAnalog,indexRS485 da co
      for (int i = 0; i < MAX_SENSORS; i++) {
        if (sensorObjT["indexAnalog"] == possibleIndexAnalog[i]) {
          possibleIndexAnalog[i] = -1;  // Đánh dấu indexAnalog này đã tồn tại
          Serial.print("indexAnalog contain: ");
          Serial.println(i);
        }
        if (sensorObjT["indexRS485"] == possibleIndexRS485[i]) {
          possibleIndexRS485[i] = -1;  // Đánh dấu indexRS485 này đã tồn tại
          Serial.print("indexRS485 contain: ");
          Serial.println(i);
        }
      }
    }
  }
  //Neu khong ton tai
  if(!containSensor){        
    //Add Sensors
    if (action=="add"){
      addSensor(doc["configuration"], type,sensorName,possibleIndexAnalog, possibleIndexRS485,multiDataStreamId );
    }
  }
  //Co ton tai
  else{
    if (action=="edit"){
      editSensor(doc["configuration"],sensorName,type,multiDataStreamId);
    }
    else if (action=="delete"){
      deleteSensor(sensorName,type,multiDataStreamId);
    }
  }
  // Mở file để ghi
  File file = SPIFFS.open("/config.json", FILE_WRITE);
  if (!file) {
    Serial.println("There was an error opening the file for writing");
    return;
  }
  // Ghi đối tượng JSON vào file
  serializeJson(myFile, file);
  file.close();
  if (doc.containsKey("multiDataStreamId")||doc.containsKey("sendTime")||doc.containsKey("LoRa"))
    ESP.restart();
}

void saveConfig(String json){
  // JSON input
  //const char* json = R"({"Sensor": [ {"type": "Analog","acction": "delete", "name": "NH4", "analog_pin": "A1", "voltage_reference": 3.3, "resolution_bit": 10},{"type": "RS485","acction": "edit", "name": "Temperature", "rx_pin": 3, "tx_pin": 4, "baud_rate": 9600}, {"type": "RS485","acction": "add", "name": "Temperature1", "rx_pin": 5, "tx_pin": 6, "baud_rate": 9600}], "sendTime": 10})";
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  //Kiem tra xem phai gui cho node nay khong
  if(doc["station"]["nodeId"]!=nodeId)
    return;
  
  //Check contain Sensor
  String gatewayId = doc["station"]["gatewayId"];
  lora->send("{\"gatewayId\":\"" + gatewayId +"\",\"status\":\"ACKCONFIG\"}");
  if(doc.containsKey("sensor")){
    Serial.println("Receive Sensor Config");
    JsonArray sensorArrayConfig = doc["sensor"].as<JsonArray>();
    // Loop over all objects in the "sensor" array
    for(JsonVariant v : sensorArrayConfig) {
      JsonObject sensorObjConfig = v.as<JsonObject>();
      String sensorName = sensorObjConfig["name"];
      String action = sensorObjConfig["action"];
      String type = sensorObjConfig["type"];
      String dataStreamId = sensorObjConfig["dataStreamId"].as<String>();
      // Mảng chứa tất cả các số indexAnalog  có thể
      int possibleIndexAnalog[] = {0, 1, 2, 3, 4};
      // Mảng chứa tất cả các số indexRS485  có thể
      int possibleIndexRS485[] = {0, 1, 2, 3, 4};
      //bien check xem da co Sensor trong json file chua
      bool containSensor=false;
      // Loop over all objects in the "sensor" array
      for(JsonVariant v1 : sensorArray) {
        JsonObject sensorObjT = v1.as<JsonObject>();
        //Kiem tra sensor co ton tai chua
        if(sensorObjT["name"]==sensorName && sensorObjT["type"]==type){
          Serial.println("Exist "+ type+sensorName);
          containSensor=true;
        }
        //Nếu acction là add -> đánh dấu các count đã tạo đối tượng sensor
        if(action=="add"){
          //Danh dau indexAnalog,indexRS485 da co
          for (int i = 0; i < MAX_SENSORS; i++) {
            if (sensorObjT["indexAnalog"] == possibleIndexAnalog[i]) {
              possibleIndexAnalog[i] = -1;  // Đánh dấu indexAnalog này đã tồn tại
              Serial.print("indexAnalog exist: ");
              Serial.println(i);
            }
            if (sensorObjT["indexRS485"] == possibleIndexRS485[i]) {
              possibleIndexRS485[i] = -1;  // Đánh dấu indexRS485 này đã tồn tại
              Serial.print("indexRS485 exist: ");
              Serial.println(i);
            }
          }
        }
      }//het for sensorArray
      Serial.print("Contains Sensor: ");
      Serial.println(containSensor);
      //Neu khong ton tai
      if(!containSensor){        
        //Add Sensors
        if (action=="add"){
          addSensor(sensorObjConfig, type,sensorName,possibleIndexAnalog, possibleIndexRS485,dataStreamId );
        }
      }
      //Co ton tai
      else{
        if (action=="edit"){
          editSensor(sensorObjConfig,sensorName,type,dataStreamId);
        }
        else if (action=="delete"){
          deleteSensor(sensorName,type,dataStreamId);
        }
      }
    }  
  }
  if (doc.containsKey("sendTime")){
    Serial.println ("Receive sendTime Config");
    int sendTime=doc["sendTime"];
    myFile["sendTime"]=sendTime;
  }
  if (doc["LoRa"].containsKey("frequency")){
    long frequency = doc["LoRa"]["frequency"];
    myFile["LoRa"]["frequency"]=frequency;
  }
  if (doc.containsKey("timeClock")){
    Serial.println ("Receive timeClock Config");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    uint16_t year= 2023;
    uint8_t month= 12;
    uint8_t day= 1;
    if (doc["timeClock"].containsKey("year") && doc["timeClock"].containsKey("month") && doc["timeClock"].containsKey("day")){
      year= doc["timeClock"]["year"];
      month= doc["timeClock"]["month"];
      day= doc["timeClock"]["day"];
    }
    uint8_t hour =0 ;
    uint8_t min = 0 ;
    uint8_t sec = 0;
    if (doc["timeClock"].containsKey("hour"))
      hour = doc["timeClock"]["hour"];
    if (doc["timeClock"].containsKey("min"))
      min = doc["timeClock"]["min"];
    if (doc["timeClock"].containsKey("sec"))
      sec = doc["timeClock"]["sec"];
    rtc.adjust(DateTime(year, month, day, hour, min, sec));
  }
  // Serial.println("myFile");
  // serializeJsonPretty(myFile, Serial);
  // Serial.println();
  // Mở file để ghi
  File file = SPIFFS.open("/config.json", FILE_WRITE);
  if (!file) {
    Serial.println("There was an error opening the file for writing");
    return;
  }
  // Ghi đối tượng JSON vào file
  serializeJson(myFile, file);
  file.close();
  
  if (doc.containsKey("sensor")||doc.containsKey("sendTime")||doc.containsKey("LoRa"))
    ESP.restart();  

}

void loadConfig(const char *fileConfig){
  // Mở file để đọc
  File file = SPIFFS.open(fileConfig, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  // Tạo một đối tượng JSON để lưu dữ liệu đọc từ file
  deserializeJson(myFile, file);
  file.close();
  // In nội dung của doc ra Serial
   Serial.println("myFile");
  serializeJsonPretty(myFile, Serial);
  Serial.println();
}
void createSensor(JsonObject sensorObj){
  String sensorName = sensorObj["name"];
  String type = sensorObj["type"];
  // Khởi tạo các sensor lưu nó vào analogList[5], RS485List[5]
  if (type=="Analog"){
     int indexAnalog= sensorObj["indexAnalog"];
     analogList[indexAnalog] = factory.createSensorAnalog(sensorName, "Analog");
     if(analogList[indexAnalog] != nullptr){
        uint8_t analogPin = sensorObj["analogPin"];
        float voltage = sensorObj["voltage"];
        int resolution= sensorObj["resolution"];
        analogList[indexAnalog]->beginSensor(analogPin,voltage,resolution);
        // Kiem tra xem sensor Analog co thong so dieu chinh khong
        if (sensorObj.containsKey("slope") && sensorObj.containsKey("intercept")){
          float slope = sensorObj["slope"];
          float intercept = sensorObj["intercept"];
          analogList[indexAnalog]->calibrate(slope,intercept);
        }
     }
  }
    else if (type=="RS485"){
         uint32_t baud_rate = sensorObj["baud_rate"];
         uint8_t rx_pin = sensorObj["rx_pin"];
         uint8_t tx_pin = sensorObj["tx_pin"];
         int indexRS485= sensorObj["indexRS485"];
         if (factory.createSensorRS485(sensorName, "RS485") != nullptr){
          RS485List[indexRS485] = factory.createSensorRS485(sensorName, "RS485");
          // RS485List[indexRS485]->setSerialPin(rx_pin,tx_pin);
          RS485List[indexRS485]->beginSensor(baud_rate);
         }
    }
}
String readData(){
  // Tạo một DynamicJsonDocument
  DynamicJsonDocument json(1024);
  DateTime now = rtc.now();
  json["resultTime"]= now.timestamp(DateTime::TIMESTAMP_FULL);
  json["nodeId"] = nodeId;
  JsonArray sArray = json.createNestedArray("sensor");
  if (sensorArray.size() > 0) {
    //Loop over all objects in the "sensor" array
    for(JsonVariant v : sensorArray) {
      JsonObject sensorObj = v.as<JsonObject>();
      // String sensorName = sensorObj["name"];
      String type = sensorObj["type"];
      JsonObject sensor = sArray.createNestedObject();
      sensor["dataStreamId"]=sensorObj["dataStreamId"];
      if (type=="Analog"){
           int indexAnalog= sensorObj["indexAnalog"];
            
           sensor["result"] = round(analogList[indexAnalog]->getSensorValue(),2); //roundf(tempValue * 1000) / 1000;
      }
      else if (type=="RS485"){
           int indexRS485= sensorObj["indexRS485"];
           //sensor["sensorName"] =type + sensorName;
           sensor["result"] = round(RS485List[indexRS485]->getSensorValue(),2);   
      }
    }
  // In nội dung của doc ra Serial
  //  serializeJsonPretty(json, Serial);
  //  Serial.println();
  }
  String msg;
  //Chuyen chuoi JSON ve String
  serializeJson(json, msg);
  return msg;
}
double round(double x, int n){ 
	int d = 0; 
	if((x * pow(10, n + 1)) - (floor(x * pow(10, n))) > 4) d = 1; 
	x = (floor(x * pow(10, n)) + d) / pow(10, n); 
	return x; 
} 
int findCount(int* possibleCount){
  // Tìm ra số Count đầu tiên không tồn tại trong mảng
  int countCurrent=MAX_SENSORS;
  for (int i = 0; i < MAX_SENSORS; i++) {
    if (possibleCount[i] != -1) {
      countCurrent=possibleCount[i];
      break;
    }
  }
  return countCurrent;
}
void addSensor(JsonObject sensorConfig, String type,String sensorName, int* possibleIndexAnalog, int* possibleIndexRS485, String dataStreamId){
  if (type=="Analog"){
    Serial.println("Sensor add "+String(type)+ String(sensorName));
    int aCountCurrent=findCount(possibleIndexAnalog);
    if(aCountCurrent<5){
      if (factory.createSensorAnalog(sensorName, "Analog") == nullptr){
        Serial.println ("Not exist Analog " + sensorName);
        return;
      }
      uint8_t analogPin = sensorConfig["analogPin"];
      float voltage = sensorConfig["voltage"];
      int resolution= sensorConfig["resolution"];
    
      //Ghi vao json file
      if (!myFile.containsKey("sensor")) {
        sensorArray = myFile.createNestedArray("sensor");
      } 
      JsonObject object = sensorArray.createNestedObject();
        object["name"] = sensorName;
        object["type"] = type;
        object["analogPin"] = analogPin;
        object["voltage"] = voltage;
        object["resolution"] = resolution;
        object["indexAnalog"] = aCountCurrent;
        object["dataStreamId"]= dataStreamId;
    }
  }
  else if (type=="RS485"){
    Serial.println("Sensor RS485 add "+ String(sensorName));
    int rCountCurrent=findCount(possibleIndexRS485);
    if(rCountCurrent<5){
      if (factory.createSensorRS485(sensorName, "RS485") == nullptr){
        Serial.println ("not exist RS485 "+ sensorName);
        return;
      }
      uint32_t baud_rate = sensorConfig["baud_rate"];
      uint8_t rx_pin = sensorConfig["rx_pin"];
      uint8_t tx_pin = sensorConfig["tx_pin"];
      //Ghi vao json file
      if (!myFile.containsKey("sensor")) {
        sensorArray = myFile.createNestedArray("sensor");
      } 
       JsonObject object = sensorArray.createNestedObject();
          object["name"] = sensorName;
          object["type"] = type;
          object["baud_rate"] = baud_rate;
          object["rx_pin"] = rx_pin;
          object["tx_pin"] = tx_pin;
          object["indexRS485"] = rCountCurrent;
          object["dataStreamId"]= dataStreamId;
    }
  }
}
void editSensor(JsonObject sensorObjConfig,String sensorName, String type, String dataStreamId){
  
  
  for (JsonVariant v : sensorArray) {
    // Nếu đối tượng có trường "name" = sensorName và "type" = type
    if (v["name"].as<String>() == sensorName && v["type"].as<String>() == type && v["dataStreamId"].as<String>() == dataStreamId) {
      // ...thì chỉnh sửa
      if(type=="Analog"){
        Serial.println("Sensor Analog Edit "+ String(sensorName));
        if (sensorObjConfig.containsKey("analogPin")){
          uint32_t analog_pin = sensorObjConfig["analogPin"];
          v["analogPin"] = analog_pin;
        }
        if (sensorObjConfig.containsKey("voltage")){
          float voltage_reference = sensorObjConfig["voltage"];
          v["voltage"] = voltage_reference;
        }
        if (sensorObjConfig.containsKey("resolution")){
          int resolution_bit = sensorObjConfig["resolution"];
          v["resolution"] = resolution_bit;
        }
        if (sensorObjConfig.containsKey("slope")){
          float slope = sensorObjConfig["slope"];
          v["slope"] = slope;
        }
        if (sensorObjConfig.containsKey("intercept")){
          float intercept = sensorObjConfig["intercept"];
          v["intercept"] = intercept;
        }
      }
      else if(type=="RS485"){
        Serial.println("Sensor RS485 Edit "+ String(sensorName));
        if (sensorObjConfig.containsKey("baud_rate")){
          uint32_t baud_rate = sensorObjConfig["baud_rate"];
          v["baud_rate"] = baud_rate;
        }
        if (sensorObjConfig.containsKey("rx_pin")){
          uint32_t rx_pin = sensorObjConfig["rx_pin"];
          v["rx_pin"] = rx_pin;
        }
        if (sensorObjConfig.containsKey("tx_pin")){
          uint32_t tx_pin = sensorObjConfig["tx_pin"];
          v["tx_pin"] = tx_pin;
        }
      }   
      break;
    }
  }
} 
void deleteSensor(String sensorName, String type, String dataStreamId){
  Serial.println("Sensor Analog Delete " + String(sensorName));
  // Duyệt qua mỗi đối tượng trong mảng
  for (JsonArray::iterator it=sensorArray.begin(); it!=sensorArray.end(); ++it) {
    JsonObject obj = it->as<JsonObject>();
    if (obj["name"].as<String>() == sensorName && obj["type"].as<String>() == type && obj["dataStreamId"].as<String>() == dataStreamId) {
      sensorArray.remove(it);
      break;
    }
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
