#ifndef WIFIUPDATER_H
#define WIFIUPDATER_H
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#define WebServer ESP8266WebServer
#else
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>	
#endif

#include <WiFiClient.h>
#include <Update.h>


	class WiFiUpdater {
		
		private:
			Stream* monitor = NULL;
			WebServer* server = NULL;
			void(*callback)( void ) = NULL;
			static WiFiUpdater* int_inst;
			String PATH = "", HTML = "", BUILD = "";
			bool blocked = false;
			
			template< typename V> void print( V v ){
					
					if( this->monitor ){
						this->monitor->print( v );
					}
			}
			
			template< typename V> void println( V v ){	
				this->print( v );
				this->print( "\n" );
			}
			
			String printup( const char* form );
			void Redirect( String& str, const char* query = "", uint16_t t = 1000 );
			
		public:
			WiFiUpdater( Stream* monitor = NULL );
			void begin( const char* path = "/", WebServer* server = NULL, const uint16_t port = 80 );
			void insertHTML( const char* html );
			void registerStopOtherCallback( void(*callback)( void ) );
			void setBuildDate( const char* date, const char* time );
			void loop( void );
	};

#endif