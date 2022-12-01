#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "Erics93";
const char* password = "heejae9936";

const char* PARAM_Container = "Container";
const char* PARAM_Name = "Name";
const char* PARAM_Quantity = "Quantity";
const char* PARAM_TimeToTake = "TimeToTake";
const char* PARAM_PerDispense = "PerDispense";


// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Container: <input type="number" name="Container">
    <br>
  <form action="/get">
    Name: <input type="text" name="Name">
    <br>
  <form action="/get">
    Quantity: <input type="number" name="Quantity">
    <br>
  <form action="/get">
    Time To Take: <input type="text" name="TimeToTake">
    <br>
  <form action="/get">    
    Per Dispense: <input type="number" name="PerDispense">
    <br>
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_Container) && request->hasParam(PARAM_Name)&& request->hasParam(PARAM_Quantity) && request->hasParam(PARAM_TimeToTake) && request->hasParam(PARAM_PerDispense)){ 
      inputMessage = "Information Saved";
    }
    else {
      inputMessage = "No message sent";
    }
  Serial.println(inputMessage);
  Serial.println(request->getParam(PARAM_Container)->value());
  Serial.println(request->getParam(PARAM_Name)->value());
  Serial.println(request->getParam(PARAM_Quantity)->value());
  Serial.println(request->getParam(PARAM_TimeToTake)->value());
  Serial.println(request->getParam(PARAM_PerDispense)->value());
  request->send(200, "text/html", inputMessage + "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  
}
