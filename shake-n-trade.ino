#include <M5Core2.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "constants.h"
#include "credentials.h"

WiFiClientSecure webClient;

// WiFi constants, REMEMBER to create `credentials.h` based on `credentials.template.h`
const char* ssid       = WIFI_SSID;
const char* password   = WIFI_PASS; 

// Network Constants
const int port = HTTPS_PORT;
const char* gcpHost = GCP_HOST;

// Stocks
char* nordnet    = "17385289";
char* avanza     = "16099806";
char* ericb     = "16101929";
char* hm        = "16099811";
char* tesla     = "16119517";
char* equinor   = "16105420";
char* novonordisk = "16256554";
char* fortum = "16100802";
char* berkshireA = "16120665";

char* instruments[] = {
  // Remember to update instrumentsSize if editing this array.
  nordnet,
  avanza,
  ericb,
  hm,
  equinor,
  novonordisk,
  fortum,
  tesla,
  berkshireA
};

int instrumentsSize = 9;

int pos = 0;
float accTriggerMax = 3.0F;
float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;

float localMaxaccX = 0.0F;
float localMaxaccY = 0.0F;
float localMaxaccZ = 0.0F;

bool handlingShake = false;

bool loadingData = false;


// URL Prefixes
char* gcpPriceTimeSeriesReturns  = "/market-data/price-time-series/v2/returns/";
char* gcpInstrumentFundamentals = "/instrument/v1/fundamentals/";

// STATUS CODES
const int HTTP_FETCHING = 2000;
const int HTTP_FAILED = 2100;
const int JSON_ERROR = 3000;
// Registries
long wifiLastReconnectAttempt = 0;
long mqttLastReconnectAttempt = 0;
bool fetchDone = false;

const int width = 320; // 320
const int height = 240; // 240
const int padding = 5;
const int lineHeight = 30;
const int radius = 10;

const char* root_ca = ROOT_CA;

uint32_t getStatusColor(int type){
  switch(type){
    case WL_IDLE_STATUS: return MAROON;
    case WL_CONNECT_FAILED: return PINK;
    case WL_CONNECTION_LOST: return DARKGREY;
    case WL_DISCONNECTED: return WHITE;
    case 1000: return RED;
    case 1100: return GREEN;
    case HTTP_FETCHING: return ORANGE;
    case HTTP_FAILED: return CYAN;
    case JSON_ERROR: return GREENYELLOW;
  }
}

void printStatus(boolean active, int errorType){
  uint32_t color = getStatusColor(errorType);
  if(active){
    M5.Lcd.fillCircle(5, 5, 5, color);
  } else {
    M5.Lcd.fillCircle(5, 5, 5, BLACK);
  }
}


void ensureWifi(const char* network, const char* pw, boolean forceBegin){
  long now = millis();
  if(forceBegin){
    printStatus(true, 1000);
    WiFi.disconnect();
    WiFi.setAutoReconnect(true);
    WiFi.begin(network, pw);
    wifiLastReconnectAttempt = millis();
  }
  int currentStatus = WiFi.status();
  if(currentStatus == WL_CONNECTED && wifiLastReconnectAttempt > 0){
    printStatus(false, 1100);
    wifiLastReconnectAttempt = 0;
    // We are connected, lets do nothing.
    return;
  }
  if(now - wifiLastReconnectAttempt > 5000){
 
    if(currentStatus == WL_IDLE_STATUS){
      
      if(now - wifiLastReconnectAttempt > 15000){
          printStatus(true, 1000);
          WiFi.disconnect();
          WiFi.begin(network, pw);
          wifiLastReconnectAttempt = millis();
      } else {
        printStatus(true, currentStatus);
      }
      return;
    }
    if(currentStatus == WL_CONNECT_FAILED || currentStatus == WL_CONNECTION_LOST || currentStatus == WL_DISCONNECTED){
      printStatus(true, currentStatus);
      WiFi.disconnect();
      WiFi.begin(network, pw);
      wifiLastReconnectAttempt = millis();
      return;
    }
  }

}


String fetch(HttpClient client, char* path){
  loadingData = true;
  printStatus(true, HTTP_FETCHING);
  client.beginRequest();
  client.get(path);
  client.sendHeader("x-locale", "sv_SE");
  client.endRequest();
  int statusCode = client.responseStatusCode();
  if(statusCode != 200){
    printStatus(true, HTTP_FAILED);
    M5.Lcd.print(statusCode);
  } else {
    printStatus(false, HTTP_FETCHING);
  }
  String data = client.responseBody();
  loadingData = false;
  return data;
}

StaticJsonDocument<3000> parseJson(String data){
  StaticJsonDocument<3000> doc;

  DeserializationError error = deserializeJson(doc, data);
  M5.Lcd.print(F("Deserialized"));
  if (error) {
    printStatus(true, JSON_ERROR);
  }
  return doc;
}


String getInstrumentFundamentals(HttpClient client, char* instrumentId){
  char pathF[100] = "";
  strcat(pathF, gcpInstrumentFundamentals);
  strcat(pathF, instrumentId);

  String data = fetch(client, pathF);
  return data;
}

String getInstrumentReturns(HttpClient client, char* instrumentId){
  char pathR[100] = "";
  strcat(pathR, gcpPriceTimeSeriesReturns);
  strcat(pathR, instrumentId);
  String data = fetch(client, pathR);
  return data;
}

DynamicJsonDocument getInstrument(char* instrumentId){
  HttpClient client = HttpClient(webClient, gcpHost, port);
  // String data = fetch(webClient, host, "/market-data/price-time-series/v2/returns/77db15e3-d392-42ea-99be-6802f7be1ab4");
  String fundamentalsString = getInstrumentFundamentals(client, instrumentId);
  String returnsString = getInstrumentReturns(client, instrumentId);

  // M5.Lcd.print(fundamentalsString);
  // M5.Lcd.print(returnsString);
  DynamicJsonDocument fundamentalsDoc(1024);
  DynamicJsonDocument returnsDoc(256);

  StaticJsonDocument<200> fundamentalsFilter;
  fundamentalsFilter["company"]["name"] = true;
  fundamentalsFilter["company"]["currency"] = true;

  StaticJsonDocument<200> returnsFilter;
  returnsFilter["rows"][0]["development"] = true;
  returnsFilter["rows"][0]["close"] = true;

  DeserializationError fundamentalsError = deserializeJson(fundamentalsDoc, fundamentalsString, DeserializationOption::Filter(fundamentalsFilter));
  DeserializationError returnsError = deserializeJson(returnsDoc, returnsString, DeserializationOption::Filter(returnsFilter));

  DynamicJsonDocument result(1000);
  result["name"] = fundamentalsDoc["company"]["name"];
  result["currency"] = fundamentalsDoc["company"]["currency"];
  result["development"] = returnsDoc["rows"][0]["development"];
  result["close"] = returnsDoc["rows"][0]["close"];
  return result;
}


void printInstrument(DynamicJsonDocument instrument){
  String name = instrument["name"];
  String currency = instrument["currency"];
  double dev = instrument["development"];
  double close = instrument["close"];
  M5.Lcd.clear();
  uint32_t color = WHITE;
  if(dev > 0){
    color = GREEN;
  }
  if(dev < 0){
    color = RED;
  }

  String c ="" + String(close, 2) + " " + currency;

  String d = "(";
  if(dev > 0){
    d += "+";
  }
  d = d + String(dev, 2) + "%)";
  print(name, c, d, color);
}

void printSpinner(){
  print(F("Loading"), F(""), F("..."), WHITE);
}

void print(String txt, String close, String dev, uint32_t color){
  M5.Lcd.setTextColor(color);
  M5.Lcd.fillRoundRect(10, 10, 310, 230, radius, color);
  M5.Lcd.fillRoundRect(20, 20, 290, 210, radius, BLACK);
  M5.Lcd.setTextPadding(width / 2);
  M5.Lcd.setFreeFont(&FreeSansBold24pt7b);
  if(txt.length() > 10){
    M5.Lcd.setFreeFont(&FreeSansBold18pt7b);
  }
  if(txt.length() > 14){
    M5.Lcd.setFreeFont(&FreeSansBold12pt7b);
  }
  M5.Lcd.drawString(txt, 160, 60, 1);

  M5.Lcd.setFreeFont(&FreeSansBold24pt7b);
  if(close.length() > 10){
    M5.Lcd.setFreeFont(&FreeSansBold18pt7b);
  }
  if(close.length() > 14){
    M5.Lcd.setFreeFont(&FreeSansBold12pt7b);
  }
  M5.Lcd.drawString(close, 160, 120, 1);

  M5.Lcd.setFreeFont(&FreeSans18pt7b);
  if(dev.length() > 14){
    M5.Lcd.setFreeFont(&FreeSans12pt7b);
  }
  M5.Lcd.drawString(dev, 160, 180, 1);
}

void updateInstrument(char* instrumentId, bool showSpinner){
  if(loadingData){
    return;
  }
  if(showSpinner){
    printSpinner();
  }
  DynamicJsonDocument instrument = getInstrument(instrumentId);
  // mp3->begin(chimeId3, out);
  printInstrument(instrument);
}

void incPos(){
  pos++;
  if(pos >= instrumentsSize){
    pos = 0;
  }
}

void decPos(){
  pos--;
  if(pos < 0){
    pos = instrumentsSize - 1;
  }
}

void setPos(int position){
  pos = position;
  if(pos >= instrumentsSize){
    pos = instrumentsSize - 1;
  }
  if(pos < 0){
    pos = 0;
  }
}

void randomPos(){
  pos = int(random(instrumentsSize));
}

void resetShake(){
  localMaxaccX = 0.0F;
  localMaxaccY = 0.0F;
  localMaxaccZ = 0.0F;

  handlingShake = false;
}

bool didShake(){
  M5.IMU.getAccelData(
        &accX, &accY,
        &accZ);

  if(abs(accX) > localMaxaccX){
    localMaxaccX = abs(accX);
  }
  if(abs(accY) > localMaxaccY){
    localMaxaccY = abs(accY);
  }
  if(abs(accZ) > localMaxaccZ){
    localMaxaccZ = abs(accZ);
  }

  bool triggered = false;

  if(localMaxaccX > accTriggerMax ){
    triggered = true;
  }
  if(localMaxaccY > accTriggerMax ){
    triggered = true;
  }
  if(localMaxaccZ > accTriggerMax ){
    triggered = true;
  }
  return triggered;
}

void setup() {
  M5.begin(true, false, true, false); //Init M5Core2.
  M5.IMU.Init();
  M5.Axp.SetLed(0);
  M5.Lcd.setFreeFont(&FreeSans12pt7b);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.setTextSize(1);
  M5.Lcd.clear();
  M5.Lcd.setCursor(40,40);
  ensureWifi(ssid, password, true);
  webClient.setCACert(root_ca);
  print(F("Started"), F("Press Any Button"), F("or Shake"), WHITE);
}

void loop() {
  //M5.Lcd.clear();
  // delay(10);
  ensureWifi(ssid, password, false);
  if(WiFi.status() != WL_CONNECTED){
    return;
  }

  M5.update();

  if(didShake() && !loadingData){
    randomPos();
    updateInstrument(instruments[pos], true);
    resetShake();
  }

  if (M5.BtnA.wasPressed()) {
    decPos();
    updateInstrument(instruments[pos], true);
  }
  if (M5.BtnB.wasPressed()) {
    updateInstrument(instruments[pos], false);
  }
  if(M5.BtnC.wasPressed()){
    incPos();
    updateInstrument(instruments[pos], true);
  }
}
