// esp8266_webserver.h

#ifndef ESP8266_WEBSERVER_H
#define ESP8266_WEBSERVER_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "webpages.h"
#include "styles.h"
#include <ArduinoJson.h>
ESP8266WebServer server(80);
class WebServerESP8266 {

private:
String configResult="";
bool is_authenticated() {
  //Serial.println("Enter is_authenticated");
  if (server.hasHeader("Cookie")) {
    //Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    //Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      //Serial.println("Authentication Successful");
      return true;
    }
  }
  //Serial.println("Authentication Failed");
  return false;
}
void handleRoot() {
    if (is_authenticated()) {
    server.sendHeader("Location", "/config");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }
  String loginPage = FPSTR(HTML_LOGIN_PAGE);
  server.send(200, "text/html", loginPage);
}

void handleCSS() {
  String styles = FPSTR(CSS_STYLES);
  server.send(200, "text/css", styles);
}

void handleLogin() {

  String username = server.arg("username");
  String password = server.arg("password");

  // Kiểm tra thông tin đăng nhập
  if (username == "admin" && password == "1234") {
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
      server.send(302);
  } else {
    server.send(200, "text/html", "Đăng nhập không thành công");
  }
}
double round(double x, int n){ 
	int d = 0; 
	if((x * pow(10, n + 1)) - (floor(x * pow(10, n))) > 4) d = 1; 
	x = (floor(x * pow(10, n)) + d) / pow(10, n); 
	return x; 
} 
void handleSensorConfig() {
  if (!is_authenticated()) {
    server.sendHeader("Location", "/");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }
  
  // Thêm mã HTML cho thanh điều hướng và nút
  String sensorConfigPage = FPSTR(HTML_SENSOR_CONFIG_PAGE);
  sensorConfigPage.replace("{navbar}", "<a href='/config'>Wireless</a> | <a href='/sensorconfig' class='active'>Sensor</a>");
  server.send(200, "text/html", sensorConfigPage);

  DynamicJsonDocument doc(1024);
  configResult = "";
  doc["station"]["nodeId"]= server.arg("idNode");
  JsonArray sensorArray = doc["sensor"];
  if (!doc.containsKey("sensor")){
    sensorArray=doc.createNestedArray("sensor");
  }
  JsonObject sensor = sensorArray.createNestedObject();
  // Lấy giá trị từ form
  String name = server.arg("name");
  sensor["name"]=name;
  String sensorType = server.arg("sensorType");
  sensor["type"]=sensorType;
  String actionType = server.arg("actionType");
  sensor["action"]= actionType;
  String idSensor = server.arg("idSensor");
  sensor["dataStreamId"]= idSensor;
  if (server.arg("analogPin").length() > 0){
    String analogPin = server.arg("analogPin");
    sensor["analogPin"]= analogPin;
  }
  if (server.arg("voltage").length() > 0){
    float voltage = server.arg("voltage").toFloat();
    sensor["voltage"]= round(voltage,1);
  }
  if (server.arg("resolutionBit").length() > 0){
    int resolutionBit = server.arg("resolutionBit").toInt();
    sensor["resolution"]= resolutionBit;
  }
    if (server.arg("txPin").length() > 0){
    int txPin = server.arg("txPin").toInt();
    sensor["tx_pin"]= txPin;
  }
  if (server.arg("rxPin").length() > 0){
    int rxPin = server.arg("rxPin").toInt();
    sensor["rx_pin"]= rxPin;
  }
  if (server.arg("baudRate").length() > 0){
    int baudRate = server.arg("baudRate").toInt();
    sensor["baud_rate"]= baudRate;
  }
  if(sensor["dataStreamId"]!=""){
    serializeJson(doc, configResult);
    server.send(200, "text/plain", configResult);
  }
}

void handleConfig() {
   if (!is_authenticated()) {
    server.sendHeader("Location", "/");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }
  String configPage = FPSTR(HTML_CONFIG_PAGE);
  configPage.replace("{navbar}", "<a href='/config' class='active'>Wireless</a> | <a href='/sensorconfig'>Sensor</a>");
  server.send(200, "text/html", configPage);
  DynamicJsonDocument doc(1024);
  configResult="";
  // In ra Serial các thông tin khi nhấn nút Lưu
  if (server.hasArg("wifi")) {
    String wifiSSID = server.arg("wifiSSID");
    String wifiPassword = server.arg("wifiPassword");
    doc["Wifi"]["ssid"]=wifiSSID;
    doc["Wifi"]["password"]=wifiPassword;
  }

  if (server.hasArg("mqttCheckbox")) {
    String mqttServer = server.arg("mqttServer");
    String mqttUsername = server.arg("mqttUsername");
    String mqttPassword = server.arg("mqttPassword");
    String mqtt_port = server.arg("mqttPort");
    doc["MQTT"]["mqtt_server"]=mqttServer;
    doc["MQTT"]["mqtt_username"]=mqttUsername;
    doc["MQTT"]["mqtt_password"]=mqttPassword;
    doc["MQTT"]["mqtt_port"]=mqtt_port;
  }
  if (server.hasArg("wifiAPCheckbox")){
    String wifiAPSSID = server.arg("wifiAPSSID");
    String wifiAPPassword = server.arg("wifiAPPassword");
    doc["WifiAP"]["wifiAPSSID"]=wifiAPSSID;
    doc["WifiAP"]["wifiAPPassword"]=wifiAPPassword;
  }
  if(!doc.isNull()){
    serializeJson(doc, configResult);
    server.send(200, "text/plain", configResult);
  }

}

void handleNotFound() {
  String notFoundPage = FPSTR(HTML_404_PAGE);
  server.send(404, "text/html", notFoundPage);
}

void handleGetWiFiList() {
  String wifiList = getAvailableWiFi();
  server.send(200, "application/json", wifiList);
}

void handleConnectWiFi() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    if (password.length() >= 8) {
        // Thực hiện kết nối WiFi ở đây (sử dụng các hàm của thư viện WiFi)
        // Nếu kết nối thành công, có thể trả về 200
        // Nếu kết nối thất bại, có thể trả về 500 hoặc 400
        WiFi.begin(ssid.c_str(), password.c_str());
        int i=0;
        while (WiFi.status() != WL_CONNECTED) {
          delay(1000);
          Serial.println(WiFi.status());
          i++;
          if (i==10)
          break;
          Serial.println("Connecting to WiFi...");
        }
        if (WiFi.status() == WL_CONNECTED) {
            server.send(200, "text/plain", "Kết nối WiFi thành công");
        } else {
            server.send(500, "text/plain", "Kết nối WiFi thất bại");
        }
    } else {
        server.send(400, "text/plain", "Mật khẩu WiFi phải có ít nhất 8 ký tự");
    }
}

String getAvailableWiFi() {
  String wifiList = "[";
  int numNetworks = WiFi.scanNetworks();
  for (int i = 0; i < numNetworks; ++i) {
    if (i > 0) {
      wifiList += ",";
    }
    wifiList += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  wifiList += "]";
  return wifiList;
}

public:

 void setupWebServer(){
  // Đặt các đường dẫn và hàm xử lý tương ứng
  server.onNotFound(std::bind(&WebServerESP8266::handleNotFound, this));
  //server.on("/", HTTP_GET, std::bind(&WebServerESP8266::handleRoot, this));
  //lambda expression C++11
  server.on("/", HTTP_GET, [this]() {
    this->handleRoot();
  });
  server.on("/login", HTTP_POST, std::bind(&WebServerESP8266::handleLogin, this));
  server.on("/style.css", HTTP_GET, std::bind(&WebServerESP8266::handleCSS, this));
  server.on("/config", HTTP_GET, std::bind(&WebServerESP8266::handleConfig, this));
  server.on("/config", HTTP_POST, std::bind(&WebServerESP8266::handleConfig, this));  // Xử lý POST từ form
  server.on("/getWiFiList", HTTP_GET, std::bind(&WebServerESP8266::handleGetWiFiList, this));
  server.on("/connectWiFi", HTTP_POST, std::bind(&WebServerESP8266::handleConnectWiFi, this));
  server.on("/sensorconfig", HTTP_GET, std::bind(&WebServerESP8266::handleSensorConfig, this));
  server.on("/sensorconfig", HTTP_POST, std::bind(&WebServerESP8266::handleSensorConfig, this));
  server.collectHeaders("User-Agent", "Cookie");
  // Bắt đầu server
  server.begin();
  Serial.println("Server started");
}
void loopWebServer(){
  server.handleClient();
}
String getConfigResult(){
  String temp = configResult;
  configResult="";
  return temp;
}

};
#endif  // ESP8266_WEBSERVER_H
