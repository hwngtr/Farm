#include "arduino_secrets.h"
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include "thingProperties.h"
#include <DHT.h>

#define DHTPIN 6            // GPIO6 for DHT11 data
#define DHTTYPE DHT11
#define MOISTURE_PIN 10     // GPIO10 as ADC input for soil moisture
#define RELAY_PIN 5         // GPIO5 as digital output to relay

DHT dht(DHTPIN, DHTTYPE);

const int moistureThreshold = 2000; 
const float T_base = 10.0;         
float dailyMax = -100.0;
float dailyMin = 100.0;

unsigned long previousMillis = 0;
const unsigned long interval = 5000; 


void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
 
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); 
  
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  Serial.println("YD-ESP32-S3 Plant Monitor Starting...");
}

void loop() {
  ArduinoCloud.update();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    int moistureReading = analogRead(MOISTURE_PIN);
    moistureValue = moistureReading;
    Serial.print("Moisture Value: ");
    Serial.println(moistureReading);
    if (moistureReading > moistureThreshold) {
      Serial.println("Soil is dry. Pump ON.");
      digitalWrite(RELAY_PIN, HIGH);
    } else {
      Serial.println("Soil moisture is adequate. Pump OFF.");
      digitalWrite(RELAY_PIN, LOW);
    }
    
   float t = dht.readTemperature(); 
float h = dht.readHumidity();    
if (isnan(t) || isnan(h)) {
  Serial.println("Failed to read from DHT sensor!");
} else {
  temperature = t;
  humidity = h;
      
     Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");
      
     if (t > dailyMax) dailyMax = t;
  if (t < dailyMin) dailyMin = t;
  float dailyAvg = (dailyMax + dailyMin) / 2.0;
  float dailyGDD = dailyAvg - T_base;
  if (dailyGDD < 0) dailyGDD = 0;
  
  Serial.print("Daily Max: ");
  Serial.print(dailyMax);
  Serial.print(" °C, Daily Min: ");
  Serial.print(dailyMin);
  Serial.println(" °C");
  Serial.print("Calculated GDD: ");
  Serial.println(dailyGDD);
}
    
    Serial.println("--------------------------------");
  }
}

/*
  These functions are called when their respective cloud variables change.
  You can add code here if you want the device to respond to dashboard changes.
*/
void onMoistureValueChange() {
  Serial.print("Cloud moistureValue updated: ");
  Serial.println(moistureValue);
  // Adjust pump state based on the updated moisture value.
  if (moistureValue > moistureThreshold) {
    Serial.println("Cloud: Soil is dry. Turning pump ON.");
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    Serial.println("Cloud: Soil moisture is adequate. Turning pump OFF.");
    digitalWrite(RELAY_PIN, LOW);
  }
}

void onTemperatureChange() {
  Serial.print("Cloud temperature updated: ");
  Serial.println(temperature);
  // Update daily max and min values based on new temperature reading.
  if (temperature > dailyMax) {
    dailyMax = temperature;
    Serial.print("Updated daily maximum: ");
    Serial.println(dailyMax);
  }
  if (temperature < dailyMin) {
    dailyMin = temperature;
    Serial.print("Updated daily minimum: ");
    Serial.println(dailyMin);
  }
  // Recalculate daily Growing Degree Days (GDD) using the updated values.
  float dailyAvg = (dailyMax + dailyMin) / 2.0;
  float newDailyGDD = dailyAvg - T_base;
  if (newDailyGDD < 0) {
    newDailyGDD = 0;
  }
  dailyGDD = newDailyGDD;  // Assuming dailyGDD is a cloud variable.
  Serial.print("Calculated Daily GDD: ");
  Serial.println(dailyGDD);
}

void onHumidityChange() {
  Serial.print("Cloud humidity updated: ");
  Serial.println(humidity);
}

void onDailyGDDChange() {
  Serial.print("Cloud dailyGDD updated: ");
  Serial.println(dailyGDD);
}
