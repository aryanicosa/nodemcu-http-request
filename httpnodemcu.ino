//tweaked by Aryanicode_ - 2022
//you can use or refer to this code for any project

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "SSID-NAME";
const char* password = "PASSWORD";

const char* host = "your-host-name.com";
const char fingerprint[] = "1B 12 5B B7 12 7D BD 7D A1 56 90 64 DB B6 E5 77 F9 65 C6 5B"; //your web fingerprint
const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80

HTTPClient http;
WiFiClientSecure httpsClient;

void setup() {
  unsigned short count = 0;

  Serial.begin(115200);

  Serial.println();

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    count++;

    if (count >= 30)
      wifiRestart();
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
    httpsClient.setFingerprint(fingerprint);
    httpsClient.setTimeout(15000); // 15 Seconds
    delay(1000);

    Serial.print("[HTTPS] begin...\n");

    GetData();
    PostData();
  }
  else {
    wifiRestart();
  }
}

void wifiRestart() {
  Serial.println("Turning WiFi off...");
  WiFi.mode(WIFI_OFF);
  Serial.println("Sleepping for 10 seconds...");
  delay(10000);
  Serial.println("Trying to connect to WiFi...");
  WiFi.mode(WIFI_STA);
}

void PostData() {
  short response_code = 0;
  String url = "https://aryanicode.free.beeceptor.com/api/post";
  http.begin(httpsClient, url);

  Serial.println("HTTPS Connecting");
  int retry = 0; //retry counter
  while ((!httpsClient.connect(host, httpsPort)) && (retry < 30)) {
    delay(100);
    Serial.print(".");
    retry++;
  }

  if (retry == 30) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected to web");
  }

  int httpCode = http.POST("{\"data\":\"fake\"}");
  if (httpCode > 0) {
    //http.writeToStream(&Serial);

    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] ... code: %d\n", httpCode);

    // file found at server
    if (httpCode >= 200 and httpCode <= 299) {
      response_code = 1;
      String payload = http.getString();
      Serial.println(payload);

      const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(payload);

      const char* data = root["data"];
      Serial.print("data: ");
      Serial.println(data);
    }
  }
  else {
    Serial.printf("[HTTP] ... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void GetData() {
  short response_code = 0;
  String url = "https://aryanicode.free.beeceptor.com/api/get";
  http.begin(httpsClient, url);

  Serial.println("HTTPS Connecting");
  int retry = 0; //retry counter
  while ((!httpsClient.connect(host, httpsPort)) && (retry < 30)) {
    delay(100);
    Serial.print(".");
    retry++;
  }

  if (retry == 30) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected to web");
  }

  int httpCode = http.GET();
  if (httpCode > 0) {
    //http.writeToStream(&Serial);

    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] ... code: %d\n", httpCode);

    // file found at server
    if (httpCode >= 200 and httpCode <= 299) {
      response_code = 1;
      String payload = http.getString();
      Serial.println(payload);

      const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(payload);

      const char* data = root["data"];
      Serial.print("data: ");
      Serial.println(data);
    }
  }
  else {
    Serial.printf("[HTTP] ... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
