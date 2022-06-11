// tweaked by Aryanicode_ - 2022
// you can use or refer to this code for any project

#include <ArduinoJson.h> // arduinojson lib v6.19.4
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_ssid_password";

const char* host = "postman-echo.com";
const char fingerprint[] = "99 c3 89 d6 54 04 82 a3 fd e2 ff 8e 9d c0 78 73 08 30 f0 68"; // web fingerprint. This is for postman-echo.com, check yours here https://iplocation.io/ssl-certificate-fingerprint
const int httpsPort = 443; // HTTPS = 443 and HTTP = 80

HTTPClient http;
WiFiClientSecure httpsClient;

enum Verb { POST_VERB, PUT_VERB, PATCH_VERB, GET_VERB };

void wifiRestart() {
  Serial.print("Turning WiFi off...");
  WiFi.mode(WIFI_OFF);
  Serial.println("Sleepping for 10 seconds...");
  delay(10000);
  Serial.println("Trying to connect to WiFi...");
  WiFi.mode(WIFI_STA);
}

void setup() {
  Serial.begin(115200);

  unsigned short count = 0;
  Serial.printf("Connecting to %s\n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    count++;
    if (count >= 30) {
      wifiRestart();
    }
  }

  Serial.println("============================");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("============================");
}

void loop() {
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
    httpsClient.setFingerprint(fingerprint);
    httpsClient.setTimeout(15000); // 15 Seconds
    delay(1000);

    Serial.print("[HTTP request] begin...\n");

    String httpsUrl = "https://postman-echo.com";
    apiRequest(POST_VERB, httpsUrl + "/post", "data to post", "", "");
    apiRequest(GET_VERB, httpsUrl + "/basic-auth", "", "basic", "postman:password");
    apiRequest(PUT_VERB, httpsUrl + "/put", "data to put", "", "");
    apiRequest(PATCH_VERB, httpsUrl + "/patch", "data to patch", "", "");
  } else {
    wifiRestart();
  }

  delay(30000);
}

void apiRequest(Verb verb, String url, String data, String authType, String authParam) {
  http.begin(httpsClient, url);

  if (authType == "basic") {
    String basicAuthData[2];
    if (authParam != "") {
      // format data "username:password"
      int index = 0;
      for (int i = 0; i <= authParam.length(); i++) {
        char delimiter = ':';
        if (authParam[i] != delimiter) {
          basicAuthData[index] += authParam[i];
        } else {
          index++;
        }
      }

      if (index == 1) {
        http.setAuthorization(basicAuthData[0].c_str(), basicAuthData[1].c_str());
      }
    }
  } else if (authType == "bearer") {
    const char* bearerAuth = authParam.c_str();
    http.setAuthorization(bearerAuth);
  }

  Serial.println("HTTPS Connecting");
  int retry = 0; // retry counter
  while ((!httpsClient.connect(host, httpsPort)) && (retry < 30)) {
    delay(100);
    Serial.print(".");
    retry++;
  }

  if (retry == 30) {
    Serial.println("Connection failed");
  } else {
    Serial.println("Connected to web");
  }

  int httpCode;

  switch (verb) {
    case POST_VERB:
      httpCode = http.POST(data);
      break;
    case GET_VERB:
      httpCode = http.GET();
      break;
    case PUT_VERB:
      httpCode = http.PUT(data);
      break;
    case PATCH_VERB:
      httpCode = http.PATCH(data);
      break;
    default:
      break;
  }

  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] ... code: %d\n", httpCode);

    // file found at server
    if (httpCode >= 200 and httpCode <= 299) {
      String jsonStringPayload = http.getString();
      Serial.println("complete payload: " + jsonStringPayload);

      const size_t docSize
        = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
      DynamicJsonDocument doc(docSize);
      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, jsonStringPayload);

      // Test if parsing succeeds.
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      const char* responseDataField;
      if (verb == GET_VERB) {
        int data = doc["authenticated"];
        responseDataField = data ? "true" : "false";
      } else {
        responseDataField = doc["data"];
      }
      Serial.printf("data: %s\n", responseDataField);
    }
  } else {
    Serial.printf("[HTTP] ... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  delay(1000);
}