#include <Keypad.h>
#include <Arduino.h>
#include <Servo.h>
#include "pitches.h"
#include <LiquidCrystal.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>


const int MAX = 100;


/*
KEYBOARD VARIABLES
*/
const byte ROWS = 4;
const byte COLS = 4;
 
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}==
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
int seconds = 1;
int quantity = 5;
char data[10];
int indexData = 0;

bool timerSet = false;
bool quantitySet = false;
int timer;


Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


/*
SERVO-MOTOR
*/
Servo servo;
int servoPin = 10;
int servoValue = 0;


/*
ULTRASOUND DETECTOR
*/
const int trigPin = 12; 
const int echoPin = 11; 
int catDistance = 0;
const int maxCatDistance = 10; //cm

/*
BUZZER
*/
const int buzzerPin = 13;
int melody1[] = {
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
  NOTE_E5,
  NOTE_F5, NOTE_F5, NOTE_F5, NOTE_F5,
  NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_G5
};

int durations1[] = {
  8, 8, 4,
  8, 8, 4,
  8, 8, 8, 8,
  2,
  8, 8, 8, 8,
  8, 8, 8, 16, 16,
  8, 8, 8, 8,
  4, 4
};

int melody2[] = {
  NOTE_E5, NOTE_D5, NOTE_FS4, NOTE_GS4, 
  NOTE_CS5, NOTE_B4, NOTE_D4, NOTE_E4, 
  NOTE_B4, NOTE_A4, NOTE_CS4, NOTE_E4,
  NOTE_A4
};
int durations2[] = {
  8, 8, 4, 4,
  8, 8, 4, 4,
  8, 8, 4, 4,
  2
};

int selectedMelody = 0;

/*
DISPLAY
*/
Adafruit_PCD8544 display = Adafruit_PCD8544(A0, A1, A2, A3, A4);


/*
SOUND DETECTOR
*/
const int soundPin = A5;
const int threshold = 1010;
bool soundDetect = false;
bool soundSet = false;

void setup() {
  Serial.begin(9600);

  servo.attach(servoPin);

  ResetKeypadData();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(soundPin, INPUT);

   display.begin();
   display.setContrast(60);
   display.invertDisplay(false);

   display.display();
   delay(2000);
   display.clearDisplay();

  // // Set text size and color
  display.setTextSize(1);
  display.setTextColor(BLACK);

  // // Set cursor position
 display.setCursor(0, 0);

  // // Print text
 display.println("This is my Cat Feeder. :D");

 display.display();
 delay(2000);
 display.clearDisplay();

  
  // // Set cursor position
 display.setCursor(0, 0);
 display.println("Instructions:");

 display.setCursor(0, 8);
 display.println("Set Timer + *");

 display.setCursor(0, 16);
 display.println("Set Quant + #");

 display.setCursor(0, 24);
 display.println("Sound Det: A/B");

 display.display();
 delay(1000);
 display.clearDisplay();

  // attachInterrupt(digitalPinToInterrupt(servoPin), DistanceFunc , CHANGE);


  // ca pe 6 e R4 unde e * si # ce seteaza minutele si cantitatea.

  // for(int i = 9; i >= 2; i++){
  //   attachInterrupt(digitalPinToInterrupt(i), KeypadFunc , CHANGE);
  // }

}

void loop() {


  KeypadFunc();

  // delay(1000);
  
  if(timerSet == true && quantitySet == true && soundSet == true && timer > 0){



    if(soundDetect == true){
    int soundVal = analogRead(soundPin);

          char buffer[50];
          sprintf(buffer, "%s %d", "SOUND DETECTED:", soundVal);
          Serial.println(buffer);
          
          display.println(buffer);
          display.display();

        if(soundVal >= threshold)
        {
            PourFood();
        }

      delay(1000);
      display.clearDisplay();
    }
     else
    {
      char buffer[50];
      sprintf(buffer, "%s %d", "Remaining time for next meal:", timer);
      Serial.println(buffer);

    display.println(buffer);

    display.display();

      timer--;

      if(timer == 0){
        DistanceFunc();
      }

      delay(1000);
      display.clearDisplay();
      }
    }
}


int CatDetectorFunc(){

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10000000);
  digitalWrite(trigPin,LOW);

  int duration = pulseIn(echoPin, HIGH);

  return duration * 0.017 ;

  // delay(500);
}

void DistanceFunc(){

    int dist = CatDetectorFunc(); 
    if( dist < maxCatDistance){

      char buffer[50];
      sprintf(buffer, "%s %d", "CAT IS HERE! CAN'T POUR FOOD! Distance:", dist);
      Serial.println(buffer);
      
      // display.setCursor(0, 0);
      display.println(buffer);
      display.display();

      delay(1000);
      display.clearDisplay();

    } else{
      PourFood();


    }
      timer = seconds;
  
}

void PourFood(){

  BuzzerFunc();


  for (int pos = 0; pos <= 180; pos += 1) { 
    servo.write(pos);             
    delay(15);                      
  }
  delay(1000);


    int auxQuantity = quantity;

    while(auxQuantity > 0){
      char buffer[50];
      sprintf(buffer, "%s %d", "Remaining quantity:", auxQuantity);
      Serial.println(buffer);

      // display.setCursor(0, 0);
      display.println(buffer);
      display.display();

      auxQuantity--;
      delay(1000);
      display.clearDisplay();

    }
    
    for (int pos = 180; pos >= 0; pos -= 1) {
      servo.write(pos);             
      delay(15);                       
    }

}

void KeypadFunc(){
    char customKey = customKeypad.getKey();
    const char digitsArray[] = "0123456789";

    if (customKey == '*') {
    String str = String(data);
    seconds = str.toInt();

    char buffer[50];
    sprintf(buffer, "%s %d", "Timer:", seconds);
    Serial.println(buffer);
    
    display.println(buffer);
    display.display();

    timer = seconds;

    timerSet = true;

    ResetKeypadData();

  }
  else if (customKey == '#'){
    String str = String(data);
    quantity = str.toInt();

    char buffer[50];
    sprintf(buffer, "%s %d", "Quantity:", quantity);
    Serial.println(buffer);

    display.println(buffer);
    display.display();

    quantitySet = true;

    ResetKeypadData();
  }
  else if(customKey == 'A' || customKey == 'B'){
    
    soundDetect = (customKey == 'A') ? true : false;

    char buffer[50];
    sprintf(buffer, "%s %s", "Sound Detect:", (soundDetect == true) ? "true" : "false");
    Serial.println(buffer);

    display.println(buffer);
    display.display();

    soundSet = true;
    ResetKeypadData();
  }
  
  if ( customKey && strchr(digitsArray, customKey) > 0 && indexData < 4)
  {
    

    data[indexData++] = customKey ;
    String str = String(data);
        Serial.println(str);

  }
  // delay(1000);
}


void ResetKeypadData(){
  indexData = 0;
  data[0] = 0;
  data[1] = 0;
  data[2] = 0;
  data[3] = 0;
  data[4] = 0;
}

void BuzzerFunc(){
  
  int size = sizeof(durations1) / sizeof(int);

  for (int note = 0; note < size; note++) {

    int duration;

    if(soundDetect == 0)
    {
      duration = 1000 / durations1[note];
      tone(buzzerPin, melody1[note], duration);
    }
    else
    {
      duration = 1000 / durations2[note];
      tone(buzzerPin, melody2[note], duration);
    }

    int pauseBetweenNotes = duration * 1.30;
    delay(pauseBetweenNotes);
    
    noTone(buzzerPin);
  }
}




