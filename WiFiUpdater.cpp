#include "WiFiUpdater.h"


const char* UploadForm = {
	"<form method='POST' enctype='multipart/form-data'>"
	"<input type='file' accept='application/octet-stream,.bin' name='f' id='f' required />"
	"<input type='button' value='Update' onclick='chk();' style='border:1px solid #999;background-image:linear-gradient(#eee,#bbb);'/>"
	"</form>"
	"<p><progress id='pB' value='0' max='100' style='width:80%;margin:auto;display:none;'></progress></p>"
	"<h3 id='st'></h3>"
	"<script type='text/javascript'>"
	"function chk(){"
	"var f=_('f').files[0];"
	"if(f.name.indexOf('%BN%')==-1)_('st').innerHTML='Invalid file!';"	
	"else gM(f,uF);"
	"}"
	"function gM(f,c){"
	"var r=new FileReader();"
	"r.onloadend=function(e){"
	"if(e.target.readyState===FileReader.DONE){"
	"var a=(new Uint8Array(e.target.result)).subarray(0,4);"
	"var h='';"
	"for(var i=0;i<a.length;i++){h+=a[i].toString(16);}"
	"c(f,h);}};r.readAsArrayBuffer(f);}"
	"function _(el){return document.getElementById(el);}"
	"function uF(f,m){"
	"if(m!='e9622f'){_('st').innerHTML='Invalid file!';"	
	"}else{var frd=new FormData();"
	"frd.append('f',f);"
	"var ax=new XMLHttpRequest();"
	"ax.upload.addEventListener('progress',pH,false);"
	"ax.addEventListener('load',cH,false);"
	"ax.addEventListener('error',eH,false);"
	"ax.addEventListener('abort',aH,false);"
	"ax.open('POST','%ACT%');"
	"ax.send(frd);}"
	"}function pH(event){"
	"var p=(event.loaded/event.total)*100;"
	"_('pB').style.display='block';"
	"_('pB').value=Math.round(p);"
	"var ps=Math.round(p)+'%';"
	"if(p==100)ps='Updating ...';"
	"_('st').innerHTML=ps;"
	"}function cH(event){"
	"_('st').innerHTML=event.target.responseText;"
	"_('pB').style.display='none';"
	"_('pB').value=0;"
	"setTimeout(function(){location.href='%PATH%?rst=1';},1500);"
	"}function eH(event){"
	"_('st').innerHTML='Upload Failed!';"
	"}function aH(event){"
	"_('st').innerHTML='Upload Aborted!';"
	"}"
	"</script>"	
};

const char* RedirectScript = {
	"<script>"
	"setTimeout( function(){"
	"location.href = '%PATH%';"
	"}, %TIME% );"
	"</script>"	
};

const char* UpdateEndPath = "/updated";

WiFiUpdater* WiFiUpdater::int_inst = NULL;

//Metody prywatne
String WiFiUpdater::printfr( const char* action ){
	String form = UploadForm;
	form.replace( "%ACT%", action );
	form.replace( "%BN%", this->BUILD_NAME );
	form.replace( "%PATH%", this->PATH );
	return form;
}

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
					this->ext_server_inst = true;
				} else {
					this->server = new WebServer( port );
					this->server->begin();
					this->ext_server_inst = false;
				}
			
			this->println( "[WifiUpdater] Ready" );
			this->PATH = path;
			
			this->server->on( path, HTTP_GET, [](){
					
					if( int_inst->server->hasArg( "rst" ) ){
						String result = int_inst->printup( "<p>Restart ...</p>" );
						int_inst->Redirect( result, "", 10000 );
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
						
							if( int_inst->BUILD_NAME != "" ){
								form += "<h3 style='text-shadow:1px 1px 1px #FFF;letter-spacing:2px;'>";
								form += int_inst->BUILD_NAME;
								form += "</h3>";
							}
						
							if( int_inst->BUILD_DATE != "" ){
								form += "<p>Build: ";
								form += int_inst->BUILD_DATE;
								form += "</p>";
							}
							
						form += int_inst->printfr( UpdateEndPath );
						int_inst->server->sendHeader( "Connection", "close" );
						int_inst->server->send( 200, "text/html", int_inst->printup( form.c_str() ) );
					}
			});
			
			this->server->on( UpdateEndPath, HTTP_POST, [](){
				bool error = Update.hasError();
				String result = "<p>Update OK!</p>";
				
					if( error ){
						result = "<p>Update FAIL!</p>";
					}

				int_inst->server->sendHeader( "Connection", "close" );
				int_inst->server->send( 200, "text/html", result );
			}, []() {
				HTTPUpload& upload = int_inst->server->upload();
      
					if( upload.status == UPLOAD_FILE_START ){
#ifdef ESP8266
						WiFiUDP::stopAll();
#endif
						String filename = upload.filename.c_str();
						String errstr = "";
						bool error = false;

							if( filename.indexOf(".bin") == -1 || int_inst->BUILD_NAME != "" && filename.indexOf( int_inst->BUILD_NAME ) == -1 ){
								errstr = int_inst->printup( "<p>Invalid file!</p>" );
								error = true;
							}
							
							if( error ){
								//int_inst->Redirect( errstr, "", 1000 );
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
								if( int_inst->monitor ) Update.printError( *int_inst->monitor );
							}
      
					} else if( upload.status == UPLOAD_FILE_WRITE ){
							
							if( Update.write( upload.buf, upload.currentSize ) != upload.currentSize ){

								if( int_inst->monitor ) Update.printError( *int_inst->monitor );
							}
							
					} else if( upload.status == UPLOAD_FILE_END ) {
						
							if( Update.end( true ) ){ //true to set the size to the current progress
								if( int_inst->monitor ) int_inst->monitor->printf( "Update Success: %u\nRebooting...\n", upload.totalSize );
							} else {
								if( int_inst->monitor ) Update.printError( *int_inst->monitor );
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
	this->BUILD_DATE = date;
	this->BUILD_DATE += ", ";
	this->BUILD_DATE += time;
}

void WiFiUpdater::setBuildName( const char* name ){
	this->BUILD_NAME = name;
	uint8_t len = this->BUILD_NAME.length();
	uint8_t i = len-1;
	uint8_t p = 0;
		
		while( i ){
				
				if( this->BUILD_NAME[i] == '\\' ) {
					p = i;
					break;
				}
			
			i--;
		}
		
	this->BUILD_NAME = this->BUILD_NAME.substring( p+1 );
	len = this->BUILD_NAME.length();
	i = len-1;
	p = 0;
	
		while( i ){
				
				if( this->BUILD_NAME[i] == '.' ) {
					p = i;
					break;
				}
			
			i--;
		}
		
	this->BUILD_NAME = this->BUILD_NAME.substring( 0, p );
}

void WiFiUpdater::loop( void ) {
	
		if( this->server ) {
			while( this->blocked ) this->server->handleClient();
			
			if( !this->ext_server_inst ) this->server->handleClient();
		}
}