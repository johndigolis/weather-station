#include <Wire.h>
#include <hd44780.h>                        // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header
hd44780_I2Cexp lcd;  // declare lcd object: auto locate & auto config expander chip
// LCD geometry
const int LCD_COLS = 20;
const int LCD_ROWS = 4;
#include <DHT.h>
#define DHTPIN 2  // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321
#define TEMPCALIB 1.3
DHT dht(DHTPIN, DHTTYPE);
#include <Adafruit_BMP085.h>
#define seaLevelPressure_hPa 1013.25
Adafruit_BMP085 bmp;

int32_t prevPressures[10] = { 0 };
int8_t j = 0;

void setup() {
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Weather Station");
  lcd.setCursor(4, 1);
  lcd.print("Initializing");
  lcd.setCursor(2, 4);
  lcd.print("ioanding - 58332");
  dht.begin();
  delay(2100);
  dht.readTemperature();
  dht.readHumidity();
  bmp.begin();
  delay(2100);
  summaryLCD();
}

void loop() {
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
  else if ((p - pastPressure) > 25) pressureChange = "  Rising";
  else if ((p - pastPressure) < -25) pressureChange = "  Falling";
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
}

double computeDewPoint(double celsius, double humidity) {
  double RATIO = 373.15 / (273.15 + celsius);  
  double SUM = -7.90298 * (RATIO - 1);
  SUM += 5.02808 * log10(RATIO);
  SUM += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / RATIO))) - 1);
  SUM += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1);
  SUM += log10(1013.246);
  double VP = pow(10, SUM - 3) * humidity;
  double T = log(VP / 0.61078);  
  return (241.88 * T) / (17.558 - T);
}
