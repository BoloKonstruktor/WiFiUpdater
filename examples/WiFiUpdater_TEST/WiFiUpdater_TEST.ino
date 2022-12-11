#include <WiFiConnector.h>
#include <WiFiUpdater.h>

const char* HTML = {
  "<html>"
  "<head>"
  "<meta charset='utf-8'>"
  "<meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate' />"
  "<meta http-equiv='Pragma' content='no-cache' />"
  "<meta http-equiv='Expires' content='0' />"
  "<style>"
  "body {"
  "margin:0px;"
  "padding:0px;"
  "text-align:center;"
  "}"
  "div {"
  "font-family:Arial;"
  "font-size:22px;"
  "width:480px;"
  "margin:auto;"
  "margin-top:100px;"
  "padding:20px;"
  "border-radius:10px;"
  "background-color:#E0EFF0;"
  "text-align:center;"
  "}"
  "input {"
  "font-family:Arial;"
  "font-size:18px;"
  "cursor:pointer;"
  "padding:10px;"
  "border-radius:5px;"
  "}"
  "</style>"
  "<title>WiFiUpdater TEST</title>"
  "</head>"
  "<body>"
  "<div>"
  "%WUFORM%"
  "</div>"
  "</body>"
  "</html>"
};

WiFiUpdater updater( &Serial );

void WCEvents( uint8_t event, WIFIParam* wifi );

void setup() {
  Serial.begin( 115200 );
  unsigned addr = 0;
  WiFiConnector* wc = new WiFiConnector;
  wc->setTimeout( 30 );
  wc->setName( "ESP8266_AP" );
  wc->registerCallback( WCEvents );
  wc->begin( addr, NULL, 80 );
  delete wc;
  wc = NULL;

  updater.setBuildDate( __DATE__, __TIME__ );
  updater.insertHTML( HTML );
  updater.begin();
}

void loop() {
  updater.loop();
}


void WCEvents( uint8_t event, WIFIParam* wifi ){

    switch( event ){
      case WC_WIFI_SCAN:{
        Serial.println( F("Wyszukiwanie sieci ...") );
      }break;
      case WC_STA_START:{
        Serial.print( F("Łącze z siecią: ") );
        Serial.print( wifi->ssid );
        Serial.print( F(" ") );  
      }break;
      case WC_STA_CONNECTING:{
        Serial.print( F(".") );
      }break;
      case WC_STA_CONNECTED:{
        Serial.println();
        Serial.println( F("Połączony.") );
        Serial.print( F("IP: ") );
        Serial.println( wifi->ip );
      }break;
      case WC_STA_TIMEOUT:{
        Serial.println( F("Czas na połączenie z siecią: '") );
        Serial.print( wifi->ssid );
        Serial.println( F("' minął.") );
      }break;
      case WC_AP_START:{
        Serial.println( F("Uruchomiono punkt dostępowy.") );
        Serial.print( F("SSID: ") );
        Serial.println( wifi->ssid );
        Serial.print( F("IP: ") );
        Serial.println( wifi->ip );
      }break;
    }
}
