#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>

ESP8266WebServer server(4000);
DHT dht14(D4, DHT11);
const int led = D6;
bool led_status = false;
const char *ntpServer = "pool.ntp.org";
WiFiClient client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");


void init_wifi(String ssid, String password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}


void setup(void) {
  pinMode(led, OUTPUT);
  dht14.begin();
  Serial.begin(115200);
  init_wifi("iPhone", "4321zszs");
  timeClient.begin();
  timeClient.setTimeOffset(7 * 3600); 


  server.on("/", HTTP_GET, []() {
    String html =
      "<html><head>"
      "<script>"
      "  setInterval(function() {"
      "    fetch('/GetData')"
      "      .then(response => response.json())"
      "      .then(data => {"
      "        fetch('http://172.20.10.3:3000/data', {"
      "          method: 'POST',"
      "          headers: {"
      "            'Content-Type': 'application/json'"
      "          },"
      "          body: JSON.stringify(data)"
      "        });"
      "      });"
      "  }, 10000);"
      "</script>"
      "</head><body>"
      "</body></html>";

    server.send(200, "text/html", html);
  });

  server.on("/GetData", HTTP_GET, []() {
    float humid = dht14.readHumidity();
    float temp = dht14.readTemperature();
 
     timeClient.update();
  String dateTime = getFormattedDateTime();

  String json = "{\"datetime\":\"" +  dateTime + "\",\"humid\":" + String(humid) + ",\"temp\":" + String(temp) + "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}

String getFormattedDateTime() {
  timeClient.update();
  time_t now = timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&now);

  char formattedDateTime[25];
  strftime(formattedDateTime, sizeof(formattedDateTime), "%Y-%m-%d %H:%M:%S", timeinfo);

  return String(formattedDateTime);
}