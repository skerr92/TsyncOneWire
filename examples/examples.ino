#include "TsyncOneWire.h"

void setup()
{
  TSyncOneWire test = TSyncOneWire(1,2);
  Serial.begin(9600);
}

void loop()
{
  Serial.println("hello world!");
}