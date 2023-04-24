#include <M5Core2.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

WiFiClientSecure webClient;

// WiFi constants, REMEMBER to insert values here!
const char* ssid       = "";  
const char* password   = ""; 

// Network Constants
const int port = 443;
const char* gcpHost = "api.prod.nntech.io";

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

const char* root_ca = "-----BEGIN CERTIFICATE-----\n" \
"MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \
"ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n" \
"MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n" \
"LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n" \
"RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n" \
"+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n" \
"PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n" \
"xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n" \
"Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n" \
"hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n" \
"EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n" \
"MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n" \
"FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n" \
"nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n" \
"eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n" \
"hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n" \
"Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n" \
"vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n" \
"+OkuE6N36B9K\n" \
"-----END CERTIFICATE-----\n" \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----";

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
