#if !defined(nullptr)
#define nullptr NULL
#endif

#include "FTDebouncer.h"
#include "Keyboard.h"

FTDebouncer pinDebouncer(30);

void setup(){
	Serial.begin(57600);
	pinDebouncer.addPin(2, HIGH, INPUT_PULLUP);
	pinDebouncer.addPin(3, HIGH, INPUT_PULLUP); 
	pinDebouncer.addPin(4, HIGH, INPUT_PULLUP);
	pinDebouncer.addPin(5, HIGH, INPUT_PULLUP);
	pinDebouncer.addPin(6, HIGH, INPUT_PULLUP);
	pinDebouncer.addPin(7, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(8, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(11, HIGH, INPUT_PULLUP);
	pinDebouncer.init();
  Keyboard.begin();

  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);
  digitalWrite(9,HIGH);
  digitalWrite(10,LOW);
  
}
void loop(){
	unsigned long us = micros();
	unsigned long usElapsed;
	pinDebouncer.run();
	usElapsed = micros() - us;
	//Serial.println(usElapsed);
}
void onPinActivated(uint8_t pinNr){
  switch(pinNr){
    case 2:
    Keyboard.print("q");
    delay(1000);
    digitalWrite(10,LOW);
    digitalWrite(9,HIGH);
    break;
    case 3:
    Keyboard.print("w");
    delay(1000);
    digitalWrite(9,LOW);
    digitalWrite(10,HIGH);
    break;
    case 4:
    Keyboard.print("z");
    break;
    case 5:
    Keyboard.print("x");
    break;
    case 6:
    Keyboard.print("c");
    break;
    case 7:
    Keyboard.print(" ");
    break;
    case 8:
    Keyboard.print("s");
    break;
    case 11:
    Keyboard.print("p");
    break;
  }
}
void onPinDeactivated(uint8_t pinNr){

}
