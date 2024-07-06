// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET     -1 // Reset pin
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Replace with your network credentials

#define DHTPIN 0     // Digital pin D1 connected to the DHT sensor

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;
float feels= 0.0;
float farenheit= 0.0;


// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 10 seconds
const long interval = 10000;  

float calculateFeelsLike(float farenheit, float h) {
  float feels = -42.379 + 2.04901523*farenheit + 10.14333127*h - .22475541*farenheit*h - .00683783*farenheit*farenheit - .05481717*h*h + .00122874*farenheit*farenheit*h + .00085282*farenheit*h*h - .00000199*farenheit*farenheit*h*h;
      if (h< 13 && farenheit>= 80 && farenheit<=110) {
        // ADJUSTMENT = [(13-RH)/4]*SQRT{[17-ABS(T-95.)]/17}
        float adjust = ((13-h)/4)  * sqrt(17-abs(farenheit-95.)/17);
        feels -= adjust;
      } else if (h>85 && farenheit>80 && farenheit<87) {
        // ADJUSTMENT = [(RH-85)/10] * [(87-T)/5]
        float adjust = ((h-85)/10) * ((87-farenheit)/5);
        feels += adjust;
      } else if (farenheit<80){
        // HI = 0.5 * {T + 61.0 + [(T-68.0)*1.2] + (RH*0.094)} 
         feels = 0.5 * (farenheit + 61.0 + ((farenheit-68.0)*1.2) + (h*0.094));
      }
      feels = (feels-32)*(5.0/9.0);
      return feels;
}
void setup(){
  // Disable WiFi to save power
  WiFiMode(WIFI_STA);
  WiFi.disconnect(); 
  WiFi.mode(WIFI_OFF);
  delay(100);
  // Serial port for debugging purposes
  Serial.begin(115200);
  dht.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
    // Clear the buffer.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(60,28);
  display.println("Yo.");
  display.display();
  delay(2000);

}
 
void loop(){  
  display.clearDisplay();
  String temp = "Temp:" + String(t,1) + "C";
  String humid = "Humid:" + String(h,0) + "%";
  String heat = "Heat:" + String(feels,1) + "C";
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(heat);
  display.println(temp);
  display.print(humid);
  display.display();
  delay(8000);

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      farenheit = (t*(1.8))+32;
      feels = calculateFeelsLike(farenheit, h);
      Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      feels = calculateFeelsLike(farenheit, h);
      Serial.println(h);
    }
  }
      Serial.print("Feels Like: ");
      Serial.println(feels);
}