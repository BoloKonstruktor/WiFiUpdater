#include "WiFiUpdater.h"


const char* UploadForm = {
	"<form method='POST' action='/updated' enctype='multipart/form-data'>"
	"<input type='file' accept='application/octet-stream,.bin' name='update'>"
	"<input type='submit' value='Update'>"
	"</form>"	
};

const char* RedirectScript = {
	"<script>"
	"setTimeout( function(){"
	"location.href = '%PATH%';"
	"}, %TIME% );"
	"</script>"	
};

WiFiUpdater* WiFiUpdater::int_inst = NULL;

//Metody prywatne
String WiFiUpdater::printup( const char* form ){
	
	String html = this->HTML;
	
		if( html == "" ) html = form;
		else {
			html.replace( "%WUFORM%", form );
		}

	return html;
}

void WiFiUpdater::Redirect( String& str, const char* query, uint16_t t ){
	String rs = RedirectScript;
	rs.replace( "%PATH%", this->PATH+query );
	rs.replace( "%TIME%", String( t ) );
	str += rs;
}

//Metody publiczne
WiFiUpdater::WiFiUpdater( Stream* monitor ){
	this->int_inst = this;
	this->monitor = monitor;
}

void WiFiUpdater::begin( const char* path, WebServer* server, const uint16_t port ){
	
		if( WiFi.waitForConnectResult() == WL_CONNECTED ){
				
				if( server ){
					this->server = server;
				} else {
					this->server = new WebServer( port );
					this->server->begin();
				}
			
			this->println( "[WifiUpdater] Ready" );
			this->PATH = path;
			
			this->server->on( path, HTTP_GET, [](){
					
					if( int_inst->server->hasArg( "rst" ) ){
						String result = int_inst->printup( "<p>Restart ...</p>" );
						int_inst->Redirect( result, "", 5000 );
						int_inst->server->sendHeader( "Connection", "close" );
						int_inst->server->send( 200, "text/html", result );
						delay( 1000 );
#ifdef ESP8266
						ESP.reset();
#else
						ESP.restart();
#endif
					} else {
						String form = "";
						
							if( int_inst->BUILD != "" ){
								form += "<p>Build: ";
								form += int_inst->BUILD;
								form += "</p>";
							}
							
						form += UploadForm;
						int_inst->server->sendHeader( "Connection", "close" );
						int_inst->server->send( 200, "text/html", int_inst->printup( form.c_str() ) );
					}
			});
			
			this->server->on( "/updated", HTTP_POST, [](){
				bool error = Update.hasError();
				String result = int_inst->printup( "<p>OK</p>" );
				
					if( error ){
						result = int_inst->printup( "<p>FAIL</p>" );
					}
					
				int_inst->Redirect( result, "?rst=1" );
				int_inst->server->sendHeader( "Connection", "close" );
				int_inst->server->send( 200, "text/html", result );
			}, []() {
				HTTPUpload& upload = int_inst->server->upload();
      
					if( upload.status == UPLOAD_FILE_START ){
#ifdef ESP8266
						WiFiUDP::stopAll();
#endif
						String filename = upload.filename.c_str();

							if( filename.indexOf(".bin") == -1 ){
								String errstr = int_inst->printup( ((filename == "") ? "<p>Wybierz plik!</p>" : "<p>Plik nieprawidłowy!</p>") );
								int_inst->Redirect( errstr, "", 1000 );
								int_inst->server->sendHeader( "Connection", "close" );
								int_inst->server->send( 200, "text/html", errstr );
								return; 
							}
						
						int_inst->blocked = true;
						
							if( int_inst->callback ) int_inst->callback();	

#ifdef ESP8266						
						uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        
							if( !Update.begin( maxSketchSpace ) ) {
#else
							if( !Update.begin() ) {
#endif
							  //Update.printError(Serial);
							}
      
					} else if( upload.status == UPLOAD_FILE_WRITE ){
							
							if( Update.write( upload.buf, upload.currentSize ) != upload.currentSize ){
							  //Update.printError(Serial);
							}
							
					} else if( upload.status == UPLOAD_FILE_END ) {
						
							if( Update.end( true ) ){ //true to set the size to the current progress
								//Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
							} else {
								//Update.printError(Serial);
							}
					
						//Serial.setDebugOutput( false );
					}
					
				yield();
			});	

		} else {
			this->println( "[WifiUpdater] WiFi Failed" );
		}
}

void WiFiUpdater::insertHTML( const char* html ){
	this->HTML = html;
}

void WiFiUpdater::registerStopOtherCallback( void(*callback)( void ) ){
	this->callback = callback;
}

void WiFiUpdater::setBuildDate( const char* date, const char* time ){
	this->BUILD = date;
	this->BUILD += ", ";
	this->BUILD += time;
}

void WiFiUpdater::loop( void ) {
	
		if( this->server ) {
			while( this->blocked ) this->server->handleClient();
		}
}