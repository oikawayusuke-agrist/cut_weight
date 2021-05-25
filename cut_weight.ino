/**
 *
 * HX711 library for Arduino - example file
 * https://github.com/bogde/HX711
 *
 * MIT License
 * (c) 2018 Bogdan Necula
 *
**/
#include "HX711.h"


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif


// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 9;
const int LOADCELL_SCK_PIN = 10;

#define weights_num 16
double max_weights[weights_num];

// ピーマンのサイズ
// S < 25
// M 25 <= <= 40
// L 40 < 

// キャリブレーションしたスケール　217.75

HX711 scale;

// #define cal_scale -4.01
#define cal_scale 3158.67

void setup() {
  Serial.begin(115200);
  
  // Serial.println("HX711 Demo");

  // Serial.println("Initializing the scale");

  // Initialize library with data output pin, clock input pin and gain factor.
  // Channel selection is made by passing the appropriate gain:
  // - With a gain factor of 64 or 128, channel A is selected
  // - With a gain factor of 32, channel B is selected
  // By omitting the gain factor parameter, the library
  // default "128" (Channel A) is used here.
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // scale.set_scale(2280.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  // scale.set_scale(217.75);
  // scale.set_scale(1118.11);
  scale.set_scale(3158.67);
  scale.tare();                // reset the scale to 0

  // Serial.println("Readings:");

  delay(1000);


  setup_sh1106();

  // max_weightsの中身をすべて-1にする
  for (int i=0; i<weights_num; i++) {
    max_weights[i] = -1;
  }
  max_weights[0] = 0;

  // Serial.println(calibrate_load_cell(scale, 42.1));
}

void loop() {
  // extrude();
  static int num = 0;
  static double timeout = millis() + 3000;
  double max_weight = max(max_weights[num], measure());
  if (max_weights[num] < max_weight && max_weight > 10) {
    timeout = millis() + 3000;
    max_weights[num] = max_weight;
  }

  if (timeout < millis()) {
    num++;
    timeout = millis() + 3000;
    max_weights[num] = 0;
  }
  if (max_weights[num] == 0) {
    timeout = millis() + 3000;
  }

  display_max_weights();
//  delay(2000);
//  s_servo.write(135);
//  m_servo.write(135);
//  l_servo.write(135);
//  delay(2000);
  // Serial.println(calibrate_load_cell(scale, 228.9));
}

double measure() 
{
//  Serial.print("one reading:\t");
//  Serial.print(scale.get_units(), 1);
//   Serial.print("\t| average:\t");
  double weight_g = scale.get_units(1);
  static double weight_bf = weight_g;
  Serial.println(weight_g, 1);

  return weight_g;
}


void display_max_weights() 
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  for (int i=0; i<weights_num; i++) 
  {
    if (i>=8) display.setCursor(64,(i-8)*8);
    if (max_weights[i] < 0) break;
    display.print("[");display.print(i+1);display.print("]:");
    display.println(max_weights[i]);
  }
  display.display();
}

double calibrate_load_cell(HX711 scale, double weight_g) 
{
  scale.set_scale();
  scale.tare();
  Serial.println("put!!!!!!!!!!!!!!!!!!!!!!!!!!!"); // weight_g[g]のものを乗せる。
  delay(10000);
  double value = scale.get_units(10);
  double scale_value = value / weight_g;
  scale.set_scale(scale_value);
  return scale_value;
}
