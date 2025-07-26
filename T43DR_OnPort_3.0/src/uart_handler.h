#pragma once
#include <Arduino.h>

void initUART();
bool requestFirstFault();
bool requestNextFault();
String getLastFaultResponse();
