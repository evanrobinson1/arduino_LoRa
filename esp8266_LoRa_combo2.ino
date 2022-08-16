
#include <SPI.h>
#include <LoRa.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>             // TCP Library
#include <ESPAsyncWebServer.h>       // Http and Websocket Server
#include <FS.h>                      // File System Wrapper
#include <WebSocketsServer.h>

const int csPin = 15;         // LoRa radio chip select   GPIO15(D8)->SS OF Lora module
const int resetPin = 0;       // LoRa radio reset         GPIO0(D3)->RESET OF Lora module
const int playPin = 4;      // Trigger for the player

String pstate;   // used on buttons

int rssi;
char myArray[6];  // for LoRa interpreting

int i = 0;


// Replace with your network credentials
const char* ssid = "innerstellar_Ext";
const char* password = "starbaby";
int port =  80; // default or replace with your desired port# here

// Create AsyncWebServer object on port xx
AsyncWebServer server(port); 

WebSocketsServer webSocket = WebSocketsServer(81);

//Webpage:
const char index_html[] PROGMEM = R"rawliteral(
        
<!DOCTYPE html>
<html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'/>
  <meta charset='utf-8'>
  <style>
    body     { font-size:120%;} 
    #main    { display: table; width: 300px; margin: auto;  padding: 10px 10px 10px 10px; border: 3px solid blue; border-radius: 10px; text-align:center;} 
    #BTN_LED { width:200px; height:40px; font-size: 110%;  }
    p        { font-size: 75%; }
  </style>
 
  <title>Websockets</title>
</head>
<body>
  <div id='main'>
    <h3>WEBSOCKET CONTROL</h3>
    <div id='content'>
      <p id='MOTION_status'></p>
      <button id='BTN_PLAYER'class="button">Turn on the Player</button>
    </div>
    <p>Track 1: Dogs Barking<BR><BR>Track 2: Dogs Barking </p>
     <p id='PLAYER_status'>PLAYER is off</p>
     <br />
     </div>

 </body>

<script>
 
  var Socket;
 
  function init() 
  {
    Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    Socket.onmessage = function(event) { processReceivedCommand(event); };
  }
  
function sendText(data)
  {
    Socket.send(data);
  }

function processReceivedCommand(evt) 
{
    document.getElementById('rd').innerHTML = evt.data;

    if (evt.data ==='0') 
    {  
        document.getElementById('BTN_PLAYER').innerHTML = 'Turn on the Player';  
        document.getElementById('PLAYER_status').innerHTML = 'PLAYER is off';
        document.getElementById('MOTION_status').innerHTML = '';  
    }
    if (evt.data ==='1') 
    {  
        document.getElementById('BTN_PLAYER').innerHTML = 'Turn off the Player'; 
        document.getElementById('PLAYER_status').innerHTML = 'PLAYER is on';   
    }
 if (evt.data ==='m') 
    {  
        document.getElementById('BTN_PLAYER').innerHTML = 'Turn off the Player'; 
        document.getElementById('PLAYER_status').innerHTML = 'PLAYER is on';
        document.getElementById('MOTION_status').innerHTML = 'motion was received';   
    }
  
}
 
  document.getElementById('BTN_PLAYER').addEventListener('click', buttonClicked);
  function buttonClicked()
  {   
    var btn = document.getElementById('BTN_PLAYER')
    var btnText = btn.textContent || btn.innerText;
    if (btnText ==='Turn on the PLAYER') { btn.innerHTML = 'Turn off the PLAYER'; document.getElementById('PLAYER_status').innerHTML = 'PLAYER is on';  sendText('1'); }  
    else                              { btn.innerHTML = 'Turn on the PLAYER';  document.getElementById('PLAYER_status').innerHTML = 'PLAYER is off'; sendText('0'); }
  }
 
  function sendText(data)
  {
    Socket.send(data);
  }
 
  window.onload = function(e)
  { 
    init();
  }

</script> 
</HTML>

)rawliteral";


void setup() {
  
 pinMode(playPin, OUTPUT);
 digitalWrite(playPin, LOW);

  // Serial port for debugging purposes
  Serial.begin(115200);       
  delay(500);

    // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  
  Serial.println(" ");
  Serial.print("Host name: ");
  Serial.print(WiFi.hostname());
  Serial.print(" ; MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Connecting to WiFi "); 
  Serial.println(ssid);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  
// Print ESP8266 Local IP Address 
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Port: ");
  Serial.println(port);


  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    pstate="playing";
    request->send_P(200, "text/html", index_html);
  });

 // do other things and monitor other things with server requests here


server.begin();  
 
// Start Websocket Server
  webSocket.begin();  webSocket.onEvent(webSocketEvent);

// Start LoRa Server

  while (!Serial);
  Serial.println("LoRa Receiver");
  LoRa.setPins(csPin, resetPin);

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

}

void loop() {
  
 //LoRa
 int packetSize = LoRa.parsePacket();
  if (packetSize) {
     Serial.println("Received packet");

    // read packet
    while (LoRa.available()) {     
      for (i=0; i < 6; i++){
         myArray[i] = LoRa.read();
      }
    }
    Serial.println(myArray);
    

  // Interpret LoRapacket
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());
  
  Serial.print("myArray= ");
  Serial.print(myArray[0]);
  Serial.print(myArray[1]);
  Serial.print(myArray[2]);
  Serial.print(myArray[3]);
  Serial.print(myArray[4]);
  Serial.println(myArray[5]);

 if (myArray[0] == 'm'){  
  sendBark();
  webSocket.broadcastTXT("m");
  delay(3000);
  webSocket.broadcastTXT("0");
  }  // end my array = m 
 
  }  // end packet size
  
 
 webSocket.loop();
 
 } //end loop
 
  void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length)
{
  if(type == WStype_TEXT)
  {
      if (payload[0] == '0')     // this is to make webpage buttons work
      {
          digitalWrite(playPin, LOW);
          Serial.println("PLAYER=off");        
      }
      else if (payload[0] == '1')
      {
          digitalWrite(playPin, HIGH);
          sendBark();
          Serial.println("PLAYER=on");        
      }
  }
 
  else 
  
  {
    Serial.print("WStype = ");   Serial.println(type);  
    Serial.print("WS payload = ");
    for(int i = 0; i < length; i++) { Serial.print((char) payload[i]); }
    Serial.println();
 
  }
}
 

 void sendBark(){

  Serial.println("Barking");
  digitalWrite(playPin, HIGH);  // turn d2 high
  delay(500);
  digitalWrite(playPin, LOW);  

 }
