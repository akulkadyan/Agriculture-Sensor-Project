#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define SOIL_PIN 34
#define FLOAT_PIN 27
#define PUMP_PIN 25

const int DRY_VALUE = 2528;
const int WET_VALUE = 880;
const int MOISTURE_THRESHOLD = 30;

Adafruit_BME280 bme;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool pumpOn = false;
bool page1 = true;

unsigned long lastPageSwitch = 0;
const unsigned long PAGE_INTERVAL = 6000;

void setup() 
{

  Serial.begin(115200);

  Wire.begin(21, 22);

  pinMode(FLOAT_PIN, INPUT_PULLUP);
  pinMode(PUMP_PIN, OUTPUT);

  digitalWrite(PUMP_PIN, LOW);

  if (!bme.begin(0x76)) 
  {
    Serial.println("BME280 not found!");
    while (1);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  {
    Serial.println("OLED not found!");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  Serial.println("System Ready");
}

void loop() 
{

  // -------- BME280 --------

  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();

  // -------- Soil Sensor --------

  long total = 0;

  for (int i = 0; i < 10; i++) 
  {
    total += analogRead(SOIL_PIN);
    delay(5);
  }

  int moistureRaw = total / 10;

  int moisturePercent = map(
    moistureRaw,
    DRY_VALUE,
    WET_VALUE,
    0,
    100
  );

  moisturePercent = constrain(
    moisturePercent,
    0,
    100
  );

  // -------- Tank Status --------

  bool tankOK = digitalRead(FLOAT_PIN);

  // -------- Pump Logic --------

  if (moisturePercent < MOISTURE_THRESHOLD && tankOK) 
  {
    digitalWrite(PUMP_PIN, HIGH);
    pumpOn = true;
  }
  else {
    digitalWrite(PUMP_PIN, LOW);
    pumpOn = false;
  }

  // -------- Serial Monitor --------

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" C  ");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%  ");

  Serial.print("Soil: ");
  Serial.print(moisturePercent);
  Serial.print("%  ");

  Serial.print("Raw: ");
  Serial.print(moistureRaw);
  Serial.print("  ");

  Serial.print("Tank: ");
  Serial.print(tankOK ? "OK" : "LOW");

  Serial.print("  Pump: ");
  Serial.println(pumpOn ? "ON" : "OFF");

  // -------- OLED Page Switching --------

  if (millis() - lastPageSwitch > PAGE_INTERVAL) 
  {
    page1 = !page1;
    lastPageSwitch = millis();
  }

  display.clearDisplay();

  if (page1) 
  {

    display.setCursor(0, 1);
    display.print("Temp: ");
    display.print(temperature, 1);
    display.print("C");

    display.setCursor(0, 11);
    display.print("Hum : ");
    display.print(humidity, 0);
    display.print("%");

    display.setCursor(0, 22);
    display.print("Soil: ");
    display.print(moisturePercent);
    display.print("%");
  }
  else 
  {

    display.setCursor(0, 1);
    display.print("Tank: ");
    display.print(tankOK ? "OK" : "LOW");

    display.setCursor(0, 11);
    display.print("Pump: ");
    display.print(pumpOn ? "ON" : "OFF");

    display.setCursor(0, 22);
    display.print("Raw : ");
    display.print(moistureRaw);
  }

  display.display();

  delay(1000);
}
