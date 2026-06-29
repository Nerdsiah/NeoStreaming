#ifndef UTIL_H
#define UTIL_H

// Prevent Arduino.h and its dependencies from defining min/max macros
#define NOMINMAX

// Include core Arduino functionality first
#include <Arduino.h>

#ifdef min
  #undef min
#endif
#ifdef max
  #undef max
#endif
#ifdef F
  #undef F
#endif
#ifdef B1
  #undef B1
#endif
#ifdef B2
  #undef B2
#endif
#ifdef B3
  #undef B3
#endif
#ifdef abs
  #undef abs
#endif
#ifdef round
  #undef round
#endif

// Restore the F macro safely for Arduino's Serial.print(F("..."))
#define F(string_literal) ((const __FlashStringHelper *)(string_literal))

#endif
