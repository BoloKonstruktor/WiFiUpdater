#include <WiFiConnector.h>
#include <WiFiUpdater.h>

const char* UPDATER_HTML = {
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "<meta charset='utf-8'>"
  "<meta http-equiv='Cache-Control' content='no-cache,no-store,must-revalidate' />"
  "<meta http-equiv='Pragma' content='no-cache' />"
  "<meta http-equiv='Expires' content='0' />"
  "<style>"
  "body{"
  "margin:0px;"
  "padding:0px;"
  "text-align:center;"
  "}"
  "div{"
  "font-family:Arial;"
  "font-size:22px;"
  "width:480px;"
  "margin:auto;"
  "margin-top:100px;"
  "padding:20px;"
  "border-radius:10px;"
  "background-color:#D0DFE0;"
  "text-align:center;"
  "-webkit-box-shadow:0 30px 60px 0 rgba(0,0,0,0.3);"
  "box-shadow:0 30px 60px 0 rgba(0,0,0,0.3);"
  "}"
  "input{"
  "font-family:Arial;"
  "font-size:18px;"
  "cursor:pointer;"
  "padding:10px;"
  "border-radius:5px;"
  "}"
  "</style>"
  "<title>FIRMWARE UPDATE</title>"
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
  wc->registerCallback( WCEvents );
  wc->begin( addr, NULL, 80 );
  delete wc;
  wc = NULL;

  updater.setBuildName( __FILE__ );
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
