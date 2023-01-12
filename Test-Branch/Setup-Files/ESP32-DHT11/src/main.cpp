#include <Arduino.h>
#include "SimpleDHT.h"
int pinDHT11 = 04;
SimpleDHT11 dht11(pinDHT11);

void setup() {
  Serial.begin(115200);
  Serial.print("System Booting");
}

void loop() {

  Serial.println("============================");
  Serial.println("Performing Transaction......");
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); Serial.print(SimpleDHTErrCode(err));
    Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(2000);
    return;
  }

  Serial.print("Sample Fetched: ");
  Serial.print((int)temperature); Serial.print("C, ");
  Serial.print((int)humidity); Serial.println("%");
  delay(2000);
}

