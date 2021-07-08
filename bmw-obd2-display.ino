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

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// End display stuff


void setup()
{
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

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("bmw-obd2-display booting...");
  display.display(); 
  // Display set-up done

  DEBUG_PORT.begin(115200);
  //SerialBT.setPin("1234");

  DEBUG_PORT.println("bmw-obd2-display by TomWis97 booting up...");
  DEBUG_PORT.println("Version: 0.0.1");

  // Second argument is whether to debug to Serial.
  ELM_PORT.begin("ArduHUD", true);
  
  if (!ELM_PORT.connect("OBDII"))
  {
    DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 1");
    while(1);
  }

  if (!myELM327.begin(ELM_PORT, true, 2000))
  {
    Serial.println("Couldn't connect to OBD scanner - Phase 2");
    while (1);
  }

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

    // Render temperatures on display
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("Oil:");
    display.setCursor(0, 41);
    display.println("Coolant:");
    display.setTextSize(3);
    display.setCursor(55, 0);
    display.println(oil);
    display.setCursor(55, 32);
    display.println(coolant);
    display.display();

    delay(2000);
  } else {
    myELM327.printError();
  }
}
