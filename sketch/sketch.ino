

// hd44780 library see https://github.com/duinoWitchery/hd44780
// thehd44780 library is available through the IDE library manager
#include <Wire.h>
#include <hd44780.h>                        // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header

hd44780_I2Cexp lcd;  // declare lcd object: auto locate & auto config expander chip

// LCD geometry
const int LCD_COLS = 20;
const int LCD_ROWS = 4;
//Variables
// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"

#define DHTPIN 2  // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define TEMPCALIB 1.3
// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 3 (on the right) of the sensor to GROUND (if your sensor has 3 pins)
// Connect pin 4 (on the right) of the sensor to GROUND and leave the pin 3 EMPTY (if your sensor has 4 pins)
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

//bmp
#include <Adafruit_BMP085.h>
#define seaLevelPressure_hPa 1007.9
Adafruit_BMP085 bmp;

int32_t prevPressures[10] = { 0 };
int8_t j = 0;

void setup() {
  // Serial.begin(9600);
  //lcd
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Initializing");
  lcd.setCursor(2, 4);
  lcd.print("ioanding - 58332");
  //dht
  dht.begin();

  //bmp
  bmp.begin();
  delay(4000);
  summaryLCD();
}

void loop() {
  //lcd
  // updateLCD();
  //dht22
  // Wait a few seconds between measurements.
  delay(15000);
  temperatureLCD();
  delay(15000);
  humidityLCD();
  delay(15000);
  prevPressures[j] = pressureLCD(prevPressures[j]);
  j += 1;
  if (j == 10) j = 0;
  delay(15000);
  summaryLCD();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  // float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  // float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  // if (isnan(h) || isnan(t) || isnan(f)) {
  //   Serial.println(F("Failed to read from DHT sensor!"));
  //   return;
  // }

  // Compute heat index in Fahrenheit (the default)
  // float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  // float hic = dht.computeHeatIndex(t, h, false);

  // Serial.print(F("Humidity: "));
  // Serial.print(h);
  // Serial.print(F("%  Temperature: "));
  // Serial.print(t);
  // Serial.print(F("°C "));
  // Serial.print(f);
  // Serial.print(F("°F  Heat index: "));
  // Serial.print(hic);
  // Serial.print(F("°C "));
  // Serial.print(hif);
  // Serial.println(F("°F"));

  //bmp
  // Serial.print("Temperature = ");
  // Serial.print(bmp.readTemperature());
  // Serial.println(" *C");

  // Serial.print("Pressure = ");
  // Serial.print(bmp.readPressure());
  // Serial.println(" Pa");

  // Serial.print("Altitude = ");
  // Serial.print(bmp.readAltitude());
  // Serial.println(" meters");

  // Serial.print("Pressure at sealevel (calculated) = ");
  // Serial.print(bmp.readSealevelPressure());
  // Serial.println(" Pa");

  // Serial.print("Real altitude = ");
  // Serial.print(bmp.readAltitude(seaLevelPressure_hPa * 100));
  // Serial.println(" meters");

  // Serial.println();
}

int32_t pressureLCD(int32_t pastPressure) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Barometric Pressure");
  lcd.setCursor(1, 1);
  float p = (float)bmp.readPressure();
  lcd.print(p / 100, 1);
  lcd.print("hPa");
  String pressureChange;
  if (pastPressure == 0) pressureChange = " Observing";
  else if ((p - pastPressure) > 30) pressureChange = "  Rising";
  else if ((p - pastPressure) < -30) pressureChange = "  Falling";
  else pressureChange = "  Steady";
  lcd.print(pressureChange);
  lcd.setCursor(0, 2);
  lcd.print("Altitude /");             //10
  lcd.print(seaLevelPressure_hPa,0);  //4
  lcd.print("hPa@SL");         //6
  lcd.setCursor(5, 3);
  lcd.print(bmp.readAltitude(seaLevelPressure_hPa * 100), 0);
  lcd.print(" meters");
  return p;
}

void humidityLCD() {
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Dew Point");
  lcd.setCursor(1, 1);
  float t = dht.readTemperature() - TEMPCALIB;
  float h = dht.readHumidity();
  float dp = computeDewPoint(t, h);
  String dpLabel;
  if (dp < 13) dpLabel = "    Dry";
  else if (dp < 16) dpLabel = "Comfortable";
  else if (dp < 18) dpLabel = "   Humid";
  else if (dp < 21) dpLabel = "   Muggy";
  else if (dp < 24) dpLabel = " Oppressive";
  else dpLabel = "  Miserable";
  lcd.print(dp, 1);
  lcd.print((char)223);
  lcd.print("C ");
  lcd.print(dpLabel);
  lcd.setCursor(1, 2);
  lcd.print("Relative Humidity");
  lcd.setCursor(7, 3);
  lcd.print(h, 1);
  lcd.print("%");
}

void temperatureLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Real Temperature");
  lcd.setCursor(1, 1);
  float t = dht.readTemperature() - TEMPCALIB;
  String tempLabel;  //13
  if (t < -9) tempLabel = "  Frigid";
  else if (t < 0) tempLabel = " Freezing";
  else if (t < 7) tempLabel = " Very Cold";
  else if (t < 13) tempLabel = "    Cold";
  else if (t < 18) tempLabel = "    Cool";
  else if (t < 24) tempLabel = "Comfortable";
  else if (t < 29) tempLabel = "    Warm";
  else if (t < 35) tempLabel = "    Hot";
  else tempLabel = " Sweltering";
  lcd.print(t, 1);
  lcd.print((char)223);
  lcd.print("C ");
  lcd.print(tempLabel);
  lcd.setCursor(0, 2);
  lcd.print("  Heat Feels Like");
  lcd.setCursor(2, 3);
  float h = dht.readHumidity();
  float hi = dht.computeHeatIndex(t, h, false);
  String hiLabel;  //14
  if (hi < 27) hiLabel = "   Normal";
  else if (hi < 32) hiLabel = "  Elevated";
  else if (hi < 41) hiLabel = "  Attention";
  else if (hi < 54) hiLabel = "    Risk";
  else hiLabel = " High Risk";
  lcd.print(hi, 0);
  lcd.print((char)223);
  lcd.print("C  ");
  lcd.print(hiLabel);
}

void summaryLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("====< Summary >=====");
  lcd.setCursor(0, 1);
  lcd.print("Temperature: ");
  lcd.print(dht.readTemperature() - TEMPCALIB, 1);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0, 2);
  lcd.print("Rel Humidity: ");
  lcd.print(dht.readHumidity(), 1);
  lcd.print("%");
  lcd.setCursor(0, 3);
  lcd.print("Pressure:  ");
  lcd.print((float)bmp.readPressure() / 100, 1);
  lcd.print("hPa");
  // lcd.setCursor(0, 3);
  // lcd.print("Altitude:      ");
  // lcd.print(bmp.readAltitude(seaLevelPressure_hPa * 100), 0);
  // lcd.print("m");
}

double computeDewPoint(double celsius, double humidity) {
  double RATIO = 373.15 / (273.15 + celsius);  // RATIO was originally named A0, possibly confusing in Arduino context
  double SUM = -7.90298 * (RATIO - 1);
  SUM += 5.02808 * log10(RATIO);
  SUM += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / RATIO))) - 1);
  SUM += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1);
  SUM += log10(1013.246);
  double VP = pow(10, SUM - 3) * humidity;
  double T = log(VP / 0.61078);  // temp var
  return (241.88 * T) / (17.558 - T);
}