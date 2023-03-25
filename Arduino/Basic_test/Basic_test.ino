#include "Wire.h"
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

PulseOximeter pox;

Temperature temp;

uint32_t tsLastReport = 0;

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  // Initialize the MAX30105 sensor
  particleSensor.begin(Wire, I2C_SPEED_FAST);

  // Configure the MAX30105 sensor settings
  particleSensor.setup(0x1F, 4, 500, 411, 4096, 18, 4);

  // Initialize the PulseOximeter and Temperature sensors
  pox.begin();
  temp.begin();
}

void loop()
{
  // Take a reading from the MAX30105 sensor
  particleSensor.nextSample();

  // Get the heart rate and SPO2 values
  float hr = pox.getHeartRate();
  float spo2 = pox.getSpO2();

  // Check if the reading is valid
  if (hr > 0 && spo2 > 0)
  {
    Serial.print("Heart rate: ");
    Serial.print(hr);
    Serial.print(" bpm - SpO2: ");
    Serial.print(spo2);
    Serial.print("%");

    // Check if it's time to report the temperature
    if (millis() - tsLastReport > 10000)
    {
      float temperature = temp.getTemperature();
      Serial.print(" - Temperature: ");
      Serial.print(temperature);
      Serial.println("C");
      tsLastReport = millis();
    }
    else
    {
      Serial.println();
    }
  }
}
