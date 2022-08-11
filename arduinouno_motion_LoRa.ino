
#include <SPI.h>
#include <LoRa.h>


int inputPinMotion = 2;         // D1  Motion Sensor   choose the input pin (for PIR sensor) blue top middle of sensor (red top left, blk top right)
int pirState = LOW;             // Motion Sensor   we start assuming no motion detected
int val = 0;                  


void setup() {
 
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

//====================

void loop() {    
  
  val = digitalRead(inputPinMotion);  // Read input value from motion sensor
    
if (val == HIGH) {               //  if input from the motion sensor is HIGH   
   
   if (pirState == LOW) {           // Check if this is a new state change
     
    Serial.println("Motion detected!");
    sendMotionDetect();
    pirState = HIGH;               // flag this state changed as sent 
                           }
 
  } else {                         // The input pin has now gone low

    if (pirState == HIGH){         
    Serial.println("Motion ended!");
    pirState = LOW;
    }
  } 
} 
  

void sendMotionDetect() {

 // send packet of the word "motion"
  LoRa.beginPacket();
  LoRa.print("motion");
  LoRa.endPacket();
  Serial.println("Motion sent");
  
}
