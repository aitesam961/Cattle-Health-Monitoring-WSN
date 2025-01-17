#include <Arduino.h>


/**
 * Project: Cattle Health Monitoring System
 * Created by Muhammad Aitesam
 *
 * Master Fimrware File
 *
 *
 */

#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#endif
// For DHT sensor
#include <DHT.h>

// For MAX30102 
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"


#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// Firebase Dependencies ================================
// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>


// INITIALIZING WIFI Credentials  ===============================
#define WIFI_SSID "aitesam961"
#define WIFI_PASSWORD "aitesam961"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

// API Key for Firebase RTDB
#define API_KEY "AIzaSyBbTQqkiBVveuM1Cf9JtTqSngrCHsi41z4"
#define DATABASE_URL "cattle-health-monitoring-wsn-default-rtdb.asia-southeast1.firebasedatabase.app" 

// Email and password for Each Cattle Node
#define USER_EMAIL "esp32-node1@CHMS.com"
#define USER_PASSWORD "esp32-node1"


#define DHTPIN 5
#define DHTTYPE DHT11
// Peripheral Objects below =====================================
// Define Firebase Data object

#define gasSensor 32
//#define gasS_trig 33
bool dngGasDet;


FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;


DHT dht(DHTPIN, DHTTYPE);

MAX30105 particleSensor;
const byte RATE_SIZE = 10; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
bool FingerON = false;


Adafruit_MPU6050 mpu;
uint8_t mpu_loop_control = 0;
bool mpu_Orinetation ;
String mpu_Activity ;





void setup()
{

  Serial.begin(115200);
  
  // Starting WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  // Start DHT Sensor
  dht.begin();
  Serial.println("DHT11 Initialized");

  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  Firebase.begin(&config, &auth);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
  Serial.println("Firebase Communication Established");

  //Firebase Corner Cases
  
  //WiFi reconnect timeout (interval) in ms (10 sec - 5 min) when WiFi disconnected.
  config.timeout.wifiReconnect = 10 * 1000;
  //Socket connection and SSL handshake timeout in ms (1 sec - 1 min).
  config.timeout.socketConnection = 10 * 1000;
  //Server response read timeout in ms (1 sec - 1 min).
  config.timeout.serverResponse = 10 * 1000;
  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;
  //RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
  config.timeout.rtdbStreamReconnect = 1 * 1000;
  //RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
  //will return false (error) when it called repeatedly in loop.
  config.timeout.rtdbStreamError = 3 * 1000;



   if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
    {
      Serial.println("MAX30105 was not found. Please check wiring/power. ");
    }

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
  particleSensor.enableDIETEMPRDY(); //Enable the temp ready interrupt.
  Serial.println("MAX30102 Initialized");


  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
  }
  else{
    Serial.println("MPU6050 Initialized");
    mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
    mpu.setMotionDetectionThreshold(1);
    mpu.setMotionDetectionDuration(20);
    mpu.setInterruptPinLatch(true);  
    mpu.setInterruptPinPolarity(true);
    mpu.setMotionInterrupt(true);
  }

  //pinMode(gasS_trig, OUTPUT);
  pinMode(gasSensor, INPUT);
  //digitalWrite(gasS_trig,LOW)
  dngGasDet = false;
  Serial.println("MQ135 Initialized");
  Serial.println("..........");
}

void loop()
{ 

  /*=====================================================
    *
    * Environment Temperature & Humidity Measurement
    *  
    */
  int humi = dht.readHumidity();
  int temp = dht.readTemperature();
  
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "Node1/Humidity", humi)){
      Serial.println("PASSED Humidity");
    }
    else {
      Serial.println("FAILED Humidity");
    }
  
  }
    if (Firebase.RTDB.setInt(&fbdo, "Node1/Environment Temperature",temp )){
      Serial.println("PASSED Environment Temperature ");
    }
    else {
      Serial.println("FAILED Environment Temperature");
    }



   
    /*=====================================================
    *
    * Heart Rate Measurement
    * 200 Iters
    */
  
   for(int hr_runtime = 0; hr_runtime <= 200;hr_runtime++){
  // Heartrate Sensor Starts Working
      Serial.println("Heart Rate Check Loop");
      long irValue = particleSensor.getIR();
  
    if (checkForBeat(irValue) == true)
    {
      //We sensed a beat!
      long delta = millis() - lastBeat;
      lastBeat = millis();
  
      beatsPerMinute = 60 / (delta / 1000.0);
  
      if (beatsPerMinute < 255 && beatsPerMinute > 20)
      {
        rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
        rateSpot %= RATE_SIZE; //Wrap variable
  
        //Take average of readings
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }
  
    Serial.print("IR=");
    Serial.print(irValue);
    Serial.print(", BPM=");
    Serial.print(beatsPerMinute);
    Serial.print(", Avg BPM=");
    Serial.print(beatAvg);
    Serial.println();
    if (irValue < 50000){
      Serial.print(" No finger?");
      FingerON = false;
    }
    else
    {
      FingerON = true;
    }
  }
  Serial.println();


    if (Firebase.RTDB.setInt(&fbdo, "Node1/Heart Rate",beatAvg)){
      Serial.println("PASSED Heart Rate");
    }
    else {
      Serial.println("FAILED Heart Rate");
    }
    if (Firebase.RTDB.setInt(&fbdo, "Node1/Body Contact",FingerON)){
      Serial.println("PASSED Body Contact");
    }
    else {
      Serial.println("FAILED Body Contact");
    }

    //Get Body Temperature
    int body_temperature = particleSensor.readTemperature();
    Serial.print("temperatureC=");
    Serial.print(body_temperature, 4);
    
    if (Firebase.RTDB.setInt(&fbdo, "Node1/Body Temperature",body_temperature)){
      Serial.println("PASSED Body Temperature");
    }
    else {
      Serial.println("FAILED Body Temperature");
    }


/*=====================================================
    *
    * MOTION Detection and Orientation Tracking

    * 20 Iters
*/
for( mpu_loop_control = 0; mpu_loop_control <= 20; mpu_loop_control++){
  if(mpu.getMotionInterruptStatus()) {
    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    if(a.acceleration.z>5 and  a.acceleration.x >-4 or a.acceleration.x > 5 ){
    mpu_Orinetation = false;
    }
    else
    {
        mpu_Orinetation = true;

        if(a.acceleration.x <-5 &&  a.acceleration.z <-5){
          mpu_Activity = "Grazing";
        }
        else if(a.acceleration.x <5 &&  a.acceleration.z >-5){
          mpu_Activity = "Standing";
        }
        else if(a.acceleration.x >-5 &&  a.acceleration.y >5)
        {
          mpu_Activity = "Loose Ear";
        }
   }
   
  }
  
}

if (Firebase.RTDB.setBool(&fbdo, "Node1/Sensor Orientation",mpu_Orinetation)){
  Serial.println("PASSED Sensor Orientation");
}
else {
  Serial.println("FAILED Sensor Orientation");
}

if (Firebase.RTDB.setString(&fbdo, "Node1/IMU Activity",mpu_Activity)){
      Serial.println("PASSED IMU Activity");
}
else {
  Serial.println("FAILED IMU Activity");
}

/*================================================
* Gas Detection
*/

//digitalWrite(gasS_trig,HIGH);
  if(analogRead(gasSensor)>1550){
    Serial.print("Dangerous Gas Detected");
    dngGasDet = true;
  }
//digitalWrite(gasS_trig,LOW)
if (Firebase.RTDB.setBool(&fbdo, "Node1/Dangerous Gas",dngGasDet)){
      Serial.println("PASSED Dangerous Gas");
}
else {
  Serial.println("FAILED Dangerous Gas");
}
  
}
