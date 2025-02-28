#include <ESP8266WiFi.h>        // Wi-Fi library for ESP8266
#include <ESP8266HTTPClient.h>   // HTTP Client library
#include <DHT.h>                 // DHT Sensor library
#include <ThingSpeak.h>          // ThingSpeak library

// Pin Definitions
#define DHTPIN D4
#define DHTTYPE DHT11
#define RELAY_PIN D2
#define SOIL_PIN A0

// DHT object
DHT dht(DHTPIN, DHTTYPE);

// Wi-Fi Credentials
const char* ssid = "enter username";           // Your Wi-Fi SSID
const char* password = "Password";        // Your Wi-Fi Password

// Prediction API URL (Update if needed)
const char* predictionURL = "https://smart-irrigation-system-1.onrender.com/predict";

// ThingSpeak Channel Info
unsigned long myChannelNumber = 2678979;      // ThingSpeak Channel Number
const char* myWriteAPIKey = "A6ESBXKZDLLQL222"; // Write API Key

// WiFi Client object
WiFiClient client;

void setup() {
  Serial.begin(115200);   // Initialize Serial Monitor
  dht.begin();            // Initialize DHT sensor
  pinMode(RELAY_PIN, OUTPUT);  // Set relay pin as output
  digitalWrite(RELAY_PIN, LOW); // Turn off relay initially

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi!");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  // Read Sensor Data
  float temp = dht.readTemperature();  // Temperature in Celsius
  float humidity = dht.readHumidity(); // Humidity in %
  int soilMoisture = analogRead(SOIL_PIN); // Soil moisture value (0-1023)

  // Print sensor values to Serial Monitor
  Serial.print("Temperature: ");
  Serial.println(temp);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);

  // Send a prediction request to the API
  if (WiFi.status() == WL_CONNECTED) {  // Ensure Wi-Fi is connected
    HTTPClient http;  // Create HTTPClient object

    // Start HTTP connection with WiFiClient and URL
    http.begin(client, predictionURL);  

    // Set content type to JSON
    http.addHeader("Content-Type", "application/json");

    // Prepare JSON payload
    String json = "{\"temperature\":" + String(temp) + 
                  ",\"humidity\":" + String(humidity) + 
                  ",\"wind_speed\":5}";

    // Send POST request with the JSON body
    int httpResponseCode = http.POST(json);

    // Check HTTP response code
    if (httpResponseCode > 0) {
      String response = http.getString();  // Get the response from the server
      Serial.println("Response: " + response);

      // Extract rain prediction (1 = rain, 0 = no rain)
      int rainPrediction = response.substring(response.indexOf(':') + 1).toInt();
      if (rainPrediction == 1) {
        Serial.println("Rain predicted. Skipping watering.");
        digitalWrite(RELAY_PIN, LOW);  // Turn off motor
      } else {
        Serial.println("No rain predicted. Watering the soil.");
        digitalWrite(RELAY_PIN, HIGH);  // Turn on motor
      }
    } else {
      Serial.print("Error in HTTP request: ");
      Serial.println(httpResponseCode);
    }

    http.end();  // Close the HTTP connection
  } else {
    Serial.println("Wi-Fi disconnected!");
  }

  // Send data to ThingSpeak
  ThingSpeak.setField(1, temp);        // Field 1: Temperature
  ThingSpeak.setField(2, humidity);    // Field 2: Humidity
  ThingSpeak.setField(3, soilMoisture); // Field 3: Soil Moisture

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Data sent to ThingSpeak successfully.");
  } else {
    Serial.println("Failed to send data to ThingSpeak.");
  }

  delay(60000);  // Wait for 60 seconds before the next loop
}
