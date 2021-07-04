#include "BluetoothSerial.h"
#include "src/ELMduino/src/ELMduino.h"


BluetoothSerial SerialBT;
#define ELM_PORT   SerialBT
#define DEBUG_PORT Serial


ELM327 myELM327;


uint32_t rpm = 0;
uint32_t coolant = 0;


void setup()
{
#if LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif

  DEBUG_PORT.begin(115200);
  //SerialBT.setPin("1234");
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
  float tempRPM = myELM327.rpm();
  if (myELM327.status == ELM_SUCCESS)
  {
    // RPM stuff
    rpm = (uint32_t)tempRPM;
    Serial.print("RPM: "); Serial.println(rpm);

    // Coolant temperature
    float tempCoolant = myELM327.engineCoolantTemp();
    coolant = (uint32_t)tempCoolant;
    Serial.print("Coolant: "); Serial.println(coolant);

    // Oil temperature
    //if (myELM327.queryPID("224402")) {
    //  uint64_t tempOil = myELM327.findResponse();
    //  Serial.print("OIL Temperature:");
    //  Serial.println(tempOil);
    //} else {
    //  Serial.println("Error while getting temperature!!");
    //}
    if (myELM327.queryPID(34, 17410)) {
      uint64_t tempOil = myELM327.findResponse();
      Serial.print("OIL Temperature:");
      Serial.println(tempOil);
    } else {
      Serial.println("Error while getting temperature!!");
    }
  } else {
    myELM327.printError();
  }
}
