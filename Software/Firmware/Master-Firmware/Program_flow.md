Void SETUP
System Start
Serial.begin
Connect to WiFi
Connect to Firebase
Sensors.begin


Void LOOP
Read.temp & Humidity
Read.Heart_Rate & Oxy
Read Gas
    if(gas==dangerous)
        danger = HIGH
    end
Read Body Temp

CFDB.send(temp,humid,heart rate,oxy,Gas status,Body Temp)

if (value.Z-Axis within range) Animal Not Moving
else (value.Z-Axis > range) Animal Moving

if (value.X-Axis within range) Animal Not Moving
else (value.Z-Axis > range) Animal Moving

if (value.y-Axis within range) Animal Not Moving
else (value.Z-Axis > range) Animal Moving


if(X-Movement is zero & Y & Z)
delay(5000)
CFDB.send(Warning 1 : no MOvement for short time)
delay(20000) (Warning 2 : no MOvement for Long time)
//Do not use delay, Compare with past values




