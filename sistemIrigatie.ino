#include <IRremote.hpp>
#include "DHT.h"

#define DHTPIN 6
#define DHTTYPE DHT11
#define IR_RECEIVE_PIN 2

uint32_t lastCode = 0;
unsigned int timpulet = 0;
int releu = 5;
int pinSenzor = A0;
long timer = 0;

bool manual = true;

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.println("Pornit receptor IR...");

  pinMode(pinSenzor,INPUT);
  pinMode(releu, OUTPUT);
  digitalWrite(releu, HIGH);

  dht.begin();
}

void loop() {

  float temperatura = dht.readTemperature();
  float umiditate = dht.readHumidity();
  int valoare = analogRead(pinSenzor);

  Serial.println(valoare);

  if (isnan(temperatura) || isnan(umiditate)) {
    Serial.println("Eroare la citirea senzorului!");
    return;
  }

  //manual mode
  if (IrReceiver.decode()) {
    uint32_t cod = IrReceiver.decodedIRData.decodedRawData;

    //start-stop for relay
    if (cod == 0x184C) {
      digitalWrite(releu, HIGH);
    } else if (cod == 0x104C) {
      digitalWrite(releu, LOW);
    }

    //the relay is activated for 5 seconds,after it's turn off
    if (cod == 0x184D || cod == 0x104D) {
      digitalWrite(releu, LOW);
      delay(5000);
      digitalWrite(releu, HIGH);
      delay(2000);
    }

    //the relay is intermittent
    if (cod == 0x1875 || cod == 0x1075) {
      digitalWrite(releu, LOW);
      delay(2000);
      digitalWrite(releu, HIGH);
      delay(2000);
    }

    manual = true;
    timer = millis();

    
    IrReceiver.resume();
  }


  //automatic mode for irrigation
  if(manual && millis() - timer > 100000)
  {

    Serial.println("Automatic mode for irrigation: ");
       //mode one,for high temperature,and dry soil and moderate humidity
    if (temperatura > 30 && valoare >= 750 && umiditate <= 40) {
      digitalWrite(releu, LOW);
      delay(15000);
      digitalWrite(releu, HIGH);
      delay(500);
    }

    //mode two
    if (temperatura >= 20 && temperatura <= 30 && valoare > 750 && umiditate >= 40 && umiditate < 60) {
      digitalWrite(releu, LOW);
      delay(10000);
      digitalWrite(releu, HIGH);
      delay(500);
    }

    //mode 3
    if(temperatura >=30 && valoare >= 650 && valoare <= 750 && umiditate <= 40)
    {
      digitalWrite(releu, LOW);
      delay(5000);
      digitalWrite(releu, HIGH);
      delay(500);  
    }

    //mode four,the soil is very wet,so we don't need extra factors
    if(valoare < 350)
    {
      digitalWrite(releu,HIGH);
      delay(500);
    }
  }
}
