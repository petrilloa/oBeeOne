#include "Memento.h"

Memento::Memento()
{
}

void Memento::GetMemento()
{
  Serial.println("GetMemento 1");
  delay(1000);
  EEPROM.get(0, item);

  Serial.println("GetMemento 2");
  delay(1000);
}

void Memento::SetMemento()
{
  EEPROM.put(0, item);
}
