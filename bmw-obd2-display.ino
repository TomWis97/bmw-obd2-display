#include "BluetoothSerial.h"
#include "src/ELMduino/src/ELMduino.h"


BluetoothSerial SerialBT;
#define ELM_PORT   SerialBT
#define DEBUG_PORT Serial


ELM327 myELM327;


uint32_t rpm = 0;
uint32_t coolant = 0;
uint64_t oil = 0;


// Display stuff
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// End display stuff

////////////////////
// Define icons
// 'icon_box', 16x16px
const unsigned char epd_bitmap_icon_box [] PROGMEM = {
	0xff, 0xff, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 
	0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0xff, 0xff
};
// 'icon_box_checked', 16x16px
const unsigned char epd_bitmap_icon_box_checked [] PROGMEM = {
	0xff, 0xff, 0x80, 0x07, 0x80, 0x0f, 0x80, 0x0f, 0x80, 0x1d, 0x80, 0x39, 0x80, 0x39, 0x80, 0x71, 
	0xc0, 0xe1, 0xe0, 0xe1, 0xf1, 0xc1, 0xbb, 0x81, 0x9f, 0x81, 0x8f, 0x01, 0x86, 0x01, 0xff, 0xff
};
// 'icon_turbo', 16x16px
const unsigned char epd_bitmap_icon_turbo [] PROGMEM = {
	0xf8, 0xff, 0xfd, 0xff, 0x7f, 0xff, 0x3f, 0xff, 0x3f, 0xf0, 0x7f, 0xf8, 0x7f, 0xf8, 0xfc, 0xfc, 
	0xf8, 0x7c, 0xf8, 0x7c, 0xfc, 0xfc, 0x7f, 0xf8, 0x7f, 0xf8, 0x3f, 0xf0, 0x1f, 0xe0, 0x07, 0x80
};
// 'icon_celcius', 14x14px
const unsigned char epd_bitmap_icon_celcius [] PROGMEM = {
	0xe1, 0xe0, 0xa3, 0xf0, 0xa7, 0x38, 0xe6, 0x1c, 0x0c, 0x0c, 0x0c, 0x00, 0x0c, 0x00, 0x0c, 0x00, 
	0x0c, 0x00, 0x0c, 0x0c, 0x06, 0x1c, 0x07, 0x38, 0x03, 0xf0, 0x01, 0xe0
};
// 'icon_coolant', 14x14px
const unsigned char epd_bitmap_icon_coolant [] PROGMEM = {
	0x03, 0xe0, 0x03, 0x00, 0x03, 0xe0, 0x03, 0x00, 0x03, 0xe0, 0x03, 0x00, 0x07, 0x80, 0x4f, 0xc8, 
	0xaf, 0xd4, 0x07, 0x80, 0x03, 0x00, 0x30, 0xc0, 0x49, 0x24, 0x86, 0x18
};
// 'icon_oil', 20x14px
const unsigned char epd_bitmap_icon_oil [] PROGMEM = {
	0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x1f, 0xf8, 0x00, 0x01, 0x80, 0xf0, 0x01, 0x83, 0xf0, 0xff, 
	0xff, 0xe0, 0xbf, 0xff, 0xe0, 0xbf, 0xff, 0xc0, 0xff, 0xff, 0x90, 0x3f, 0xff, 0x10, 0x3f, 0xfe, 
	0x00, 0x3f, 0xfc, 0x00, 0x3f, 0xf8, 0x10, 0x00, 0x00, 0x00
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 304)
const int epd_bitmap_allArray_LEN = 6;
const unsigned char* epd_bitmap_allArray[6] = {
	epd_bitmap_icon_box,
	epd_bitmap_icon_box_checked,
	epd_bitmap_icon_celcius,
	epd_bitmap_icon_coolant,
	epd_bitmap_icon_oil,
	epd_bitmap_icon_turbo
};


void setup()
{
  //Serial.begin(115200);

#if LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif

  // Set-up display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setRotation(2);

  // DEBUGGING
  mockup_boot();
  delay(1000);
  draw_layout();
  delay(4000);
  draw_battery(10.48);
  draw_layout();
  delay(100000);

  display.setFont(&FreeSans9pt7b);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  // Display boot text
  display.setCursor(0, 15);
  display.println("Booting...");
  display.display(); 
  // Display set-up done

  DEBUG_PORT.begin(115200);
  //SerialBT.setPin("1234");

  DEBUG_PORT.println("bmw-obd2-display by TomWis97 booting up...");
  DEBUG_PORT.println("Version: 0.1.0");

  // Second argument is whether to debug to Serial.
  ELM_PORT.begin("ArduHUD", true);
  
  if (!ELM_PORT.connect("OBDII"))
  {
    DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 1");
    while(1);
  }
  display.setCursor(0, 30);
  display.println("BT connected.");
  display.display(); 
  delay(1000);

  if (!myELM327.begin(ELM_PORT, true, 2000))
  {
    Serial.println("Couldn't connect to OBD scanner - Phase 2");
    while (1);
  }

  display.setCursor(0, 45);
  display.println("Serial connected.");
  display.display(); 
  delay(2000);
  Serial.println("Connected to ELM327");
}


void loop()
{
  Serial.println("------------------------------");
  float tempRPM = myELM327.rpm();
  if (myELM327.status == ELM_SUCCESS)
  {
    //Declare vars
    
    // RPM stuff
    rpm = (uint32_t)tempRPM;
    Serial.print("RPM: "); Serial.println(rpm);

    // Coolant temperature
    float tempCoolant = myELM327.engineCoolantTemp();
    coolant = (uint32_t)tempCoolant;
    Serial.print("Coolant temperature: "); Serial.println(coolant);

    // Turbo pressure
    float tempPressure = myELM327.manifoldPressure();
    Serial.print("Turbo pressure: "); Serial.println(tempPressure);

    // Oil temperature
    if (myELM327.queryPID(34, 17410)) {
      uint64_t tempOil = myELM327.findResponse();
      oil = tempOil * 191.25 / 255 - 48;
      Serial.print("Oil temperature:");
      Serial.println(oil);
    } else {
      Serial.println("Error while getting temperature!!");
    }

    display_normal();

    delay(2000);
  } else {
    display_noconn();
    myELM327.printError();
  }
}

void display_normal()
{
  // render base layout
  display.clearDisplay();
  //display.setfont();
  display.drawFastVLine(63, 0, 64, WHITE);
  display.setTextSize(1);
  display.setCursor(0,14);
  display.println("oil");
  display.setCursor(65,14);
  display.println("coolant");

  display.setFont(&FreeSans9pt7b);
  display.setTextSize(2);
  display.setCursor(0,55);
  display.println(oil);
  display.setCursor(64,55);
  display.println(coolant);
  
  display.display();
}

void display_noconn()
{
  // render base layout
  display.clearDisplay();
  //display.setFont();
  display.drawFastVLine(63, 0, 64, WHITE);
  display.setTextSize(1);
  display.setCursor(0,14);
  display.println("oil");
  display.setCursor(65,14);
  display.println("coolant");

  display.setFont(&FreeSans9pt7b);
  display.setTextSize(2);
  display.setCursor(0,55);
  display.println("---");
  display.setCursor(64,55);
  display.println("---");
  
  display.display();
}


//////////////////////////////////////
// Normal layout

void draw_layout()
{
  Serial.println("Display debugging: draw_layout()");
  display.clearDisplay();
  display.drawRoundRect(0, 0, 63, 45, 4, WHITE);
  display.drawRoundRect(65, 0, 63, 45, 4, WHITE);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(WHITE);

  // Draw icons
  display.drawBitmap(5, 2, epd_bitmap_icon_oil, 20, 14, WHITE);
  //display.fillRect(69, 2, 14, 14, WHITE);
  display.drawBitmap(69, 2, epd_bitmap_icon_coolant, 14, 14, WHITE);
  display.drawBitmap(47, 2, epd_bitmap_icon_celcius, 14, 14, WHITE);
  display.drawBitmap(112, 2, epd_bitmap_icon_celcius, 14, 14, WHITE);
  //display.fillRect(112, 48, 16, 16, WHITE);
  display.drawBitmap(0, 48, epd_bitmap_icon_turbo, 16, 16, WHITE);

  // Mock values
  display.setTextSize(2);
  display.setCursor(0, 41);
  display.println("126");
  display.setCursor(64, 41);
  display.println("126");
  display.setTextSize(1);
  display.setCursor(20, 60);
  display.println("0,05/0,84");

  // Labels
  //display.setTextSize(1);
  //display.setCursor(0,14);
  //display.println("Oil");
  //display.setCursor(65,14);
  //display.println("Coolant");

  display.display();
}

//////////////////////////////////////
// Battery voltage
void draw_battery(float voltage){
  display.fillRect(20, 8, 88, 46, BLACK);
  display.drawRect(20, 8, 88, 46, WHITE);
  display.setTextSize(1);
  display.setCursor(21, 22);
  display.println("Min. batt.");
  display.setCursor(95, 50);
  display.println("V");
  display.setCursor(18, 49);
  display.setTextSize(2);
  display.println(String(voltage, 1));
  display.display();
  // Wait a bit before overwriting the display.
  delay(5000);
}

//////////////////////////////////////
// Bootup layout

void mockup_boot(){
  bool status_bluetooth = false;
  bool status_serial = false;
  for (int steps = 0; steps <= 60; steps++) {
    // status mockup
    if ( steps == 20 ) {
      status_bluetooth = true;
    }
    if ( steps == 40 ) {
      status_serial = true;
    }

    display.clearDisplay();
    draw_bootup_animation(steps);
    draw_bootup(status_bluetooth, status_serial);
    display.display();
    delay(500);
  }
}

void draw_bootup(bool status_bluetooth, bool status_serial)
{
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(WHITE);
  display.setTextSize(1);

  // Labels
  display.setCursor(20, 30);
  display.println("Bluetooth");
  display.setCursor(20, 50);
  display.println("Serial");

  // Icons
  if ( status_bluetooth == true ){
    display.drawBitmap(1, 16, epd_bitmap_icon_box, 16, 16, WHITE);
  } else {
    display.drawBitmap(1, 16, epd_bitmap_icon_box_checked, 16, 16, WHITE);
  }
  if ( status_serial == true ){
    display.drawBitmap(1, 36, epd_bitmap_icon_box, 16, 16, WHITE);
  } else {
    display.drawBitmap(1, 36, epd_bitmap_icon_box_checked, 16, 16, WHITE);
  }
}

void draw_bootup_animation(int step)
{
  Serial.print("Animation step:"); Serial.println(step);
  int x_pos=114;
  int y_pos=25;
  if ( step % 4 != 0) { // Top-left
    display.fillRect(x_pos, y_pos, 5, 5, WHITE);
  }
  if ( step % 4 != 1) { // Top-right
    display.fillRect(x_pos + 8, y_pos, 5, 5, WHITE);
  }
  if ( step % 4 != 2) { // Bottom-right
    display.fillRect(x_pos + 8, y_pos + 8, 5, 5, WHITE);
  }
  if ( step % 4 != 3) { // Bottom-left
    display.fillRect(x_pos, y_pos + 8, 5, 5, WHITE);
  }
}
