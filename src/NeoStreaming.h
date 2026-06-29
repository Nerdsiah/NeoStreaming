/**
 * @file NeoStreaming.h
 * @brief C++ Style Standard Output Stream for Arduino
 *
 * Copyright (C) 2026
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * @section ARCHITECTURE
 * Implements a Zero-Heap Proxy Pattern for stream manipulation.
 * - setw() is temporary (applies only to the next value).
 * - setprecision(), setfill(), left, right, fixed, defaultfloat, scientific,
 * and hexfloat are persistent.
 * - Architecture-aware math conversions to prevent buffer overflows.
 */

#ifndef NEOSTREAMING_H
#define NEOSTREAMING_H

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utility/util.h"

/* ========================================================================
 * COMPILE-TIME CONSTANTS
 * ======================================================================== */

/** @brief Watchdog Timer safety ceiling. Prevents infinite padding loops. */
constexpr size_t MAX_PAD_WIDTH = 128;

/** @brief Safe stack buffer size for verbose strftime outputs. */
constexpr size_t TIME_BUFFER_SIZE = 64;

/** @brief Secure stack buffer for max 32-bit/64-bit precision string
 * generation. */
constexpr size_t FLOAT_BUFFER_SIZE = 80;

/* ========================================================================
 * GLOBAL PERSISTENT STATE
 * ======================================================================== */

/**
 * @brief Holds the persistent precision state for floating-point numbers.
 * Defaults to 6 (C++ standard).
 */
inline int& precision() {
  static int p = 6;
  return p;
}

/**
 * @brief Holds the persistent fill character state.
 * Defaults to a space (' ').
 */
inline char& fillchar() {
  static char c = ' ';
  return c;
}

/**
 * @brief Holds the persistent justification state.
 * false = Right Justified (Default), true = Left Justified.
 */
inline bool& left_justify() {
  static bool lj = false;
  return lj;
}

/**
 * @brief Holds the persistent floating-point formatting state.
 * 0 = Default, 1 = Fixed, 2 = Hexfloat, 3 = Scientific
 */
inline uint8_t& float_mode() {
  static uint8_t fm = 0;
  return fm;
}

/* ========================================================================
 * MANIPULATORS & FORMATTING STRUCTURES
 * ======================================================================== */

enum EndLine { endl };
enum Justify { left, right };  // Standard C++ alignment manipulators

/**
 * @brief Standard C++ float formatting manipulators.
 */
enum FloatFormat {
  defaultfloat = 0,  // Prints using significant figures
  fixed = 1,         // Prints using exact decimal places
  hexfloat = 2,      // Prints IEEE-754 hexadecimal floating-point
  scientific = 3     // Prints using scientific notation
};

struct SetW {
  int width;
};
struct SetPrecision {
  int precision;
};
struct SetFill {
  char fill;
};

inline SetW setw(int w) {
  return {w};
}
inline SetPrecision setprecision(int p) {
  return {p};
}
inline SetFill setfill(char c) {
  return {c};
}

struct PutTime {
  const struct tm* timeStruct;
  const char* format;
};

/**
 * @brief Evaluates a tm structure into a formatted time string.
 */
inline PutTime put_time(const struct tm* tmb, const char* fmt) {
  return {tmb, fmt};
}

/* ========================================================================
 * FORMATTING HELPERS
 * ======================================================================== */

/**
 * @brief Universal float formatter. Handles dtostrf/snprintf routing,
 * trailing zero trimming (AVR), and IEEE-754 hexfloat unpacking.
 */
inline void FormatFloat(double val, char* numBuf, size_t bufSize) {
  if (float_mode() == hexfloat) {
    // UNIVERSAL IEEE-754 UNPACKER: Works on all 32-bit float architectures
    // Bypasses the need for C99 '%a' support in the underlying toolchain
    uint32_t raw;
    float fval = (float)val;
    memcpy(&raw, &fval, 4);

    if ((raw & 0x7FFFFFFF) == 0) {
      snprintf(numBuf, bufSize, (raw >> 31) ? "-0x0.0p+0" : "0x0.0p+0");
    } else if (((raw >> 23) & 0xFF) == 0xFF) {
      snprintf(numBuf, bufSize,
               (raw & 0x7FFFFF) ? "nan" : ((raw >> 31) ? "-inf" : "inf"));
    } else {
      int sign = (raw >> 31) ? 1 : 0;
      int exponent = ((raw >> 23) & 0xFF) - 127;
      uint32_t mantissa = (raw & 0x7FFFFF) << 1;
      snprintf(numBuf, bufSize, "%s0x1.%06lxp%+d", sign ? "-" : "",
               (unsigned long)mantissa, exponent);
    }
  } else {
#ifdef __AVR__
    dtostrf(val, 1, precision(), numBuf);
    if (float_mode() == defaultfloat) {
      int len = strlen(numBuf);
      if (strchr(numBuf, '.')) {
        while (len > 0 && numBuf[len - 1] == '0') {
          numBuf[len - 1] = '\0';
          len--;
        }
        if (len > 0 && numBuf[len - 1] == '.') {
          numBuf[len - 1] = '\0';
        }
      }
    }
#else
    // ARM/ESP fixed/defaultfloat
    if (float_mode() == fixed) {
      snprintf(numBuf, bufSize, "%.*f", precision(), val);
    } else if (float_mode() == scientific) {
      snprintf(numBuf, bufSize, "%.*e", precision(), val);
    } else {
      snprintf(numBuf, bufSize, "%.*g", precision(), val);
    }
#endif
  }
}

/**
 * @brief Universal time formatter. Handles strftime execution and AVR
 * workarounds.
 */
inline void FormatTime(const PutTime& arg, char* timeBuf, size_t bufSize) {
  if (arg.timeStruct && arg.format) {
#ifdef __AVR__
    // AVR WORKAROUND: AVR's native strftime is often stripped to save flash
    // memory and fails silently. We manually parse the most common time
    // formats.

    // Handle standard "%Y-%m-%d %H:%M:%S"
    if (strcmp(arg.format, "%Y-%m-%d %H:%M:%S") == 0) {
      snprintf(timeBuf, bufSize, "%04d-%02d-%02d %02d:%02d:%02d",
               arg.timeStruct->tm_year + 1900, arg.timeStruct->tm_mon + 1,
               arg.timeStruct->tm_mday, arg.timeStruct->tm_hour,
               arg.timeStruct->tm_min, arg.timeStruct->tm_sec);
    }
    // Handle time only "%H:%M:%S"
    else if (strcmp(arg.format, "%H:%M:%S") == 0) {
      snprintf(timeBuf, bufSize, "%02d:%02d:%02d", arg.timeStruct->tm_hour,
               arg.timeStruct->tm_min, arg.timeStruct->tm_sec);
    }
    // Fallback for unsupported formats on AVR
    else {
      strlcpy(timeBuf, "[AVR_FMT_ERR]", bufSize);
    }
#else
    // ARM/ESP fully support standard strftime
    if (strftime(timeBuf, bufSize, arg.format, arg.timeStruct) == 0) {
      strlcpy(timeBuf, "[TIME_ERR]", bufSize);
    }
#endif
  }
}

/* ========================================================================
 * STREAM OPERATORS (OUTSIDE PROXY)
 * ======================================================================== */

inline Print& operator<<(Print& stream, EndLine arg) {
  stream.println();
  return stream;
}

inline Print& operator<<(Print& strm, Justify arg) {
  left_justify() = (arg == left);
  return strm;
}

inline Print& operator<<(Print& strm, const SetFill& arg) {
  fillchar() = arg.fill;
  return strm;
}

inline Print& operator<<(Print& strm, const SetPrecision& arg) {
  precision() = (arg.precision > 20) ? 20 : arg.precision;
  return strm;
}

inline Print& operator<<(Print& strm, FloatFormat arg) {
  float_mode() = arg;
  return strm;
}

/**
 * @brief Architecture-safe formatting for unpadded doubles.
 */
inline Print& operator<<(Print& strm, double val) {
  char numBuf[FLOAT_BUFFER_SIZE] = {0};
  FormatFloat(val, numBuf, sizeof(numBuf));
  strm.print(numBuf);

  return strm;
}

inline Print& operator<<(Print& strm, float val) {
  return operator<<(strm, (double)val);
}

inline Print& operator<<(Print& strm, const PutTime& arg) {
  char timeBuf[TIME_BUFFER_SIZE] = {0};
  FormatTime(arg, timeBuf, sizeof(timeBuf));
  strm.print(timeBuf);

  return strm;
}

template <class T>
inline Print& operator<<(Print& stream, T arg) {
  stream.print(arg);
  return stream;
}

/* ========================================================================
 * PROXY PATTERN (TRANSIENT WIDTH STATE)
 * ======================================================================== */

/**
 * @brief Proxy object for processing setw() chains securely.
 */
struct PrintProxy {
  Print& strm;
  int width;

  PrintProxy operator<<(const SetW& arg) {
    width = (arg.width > MAX_PAD_WIDTH) ? MAX_PAD_WIDTH : arg.width;
    return *this;
  }

  PrintProxy operator<<(Justify arg) {
    left_justify() = (arg == left);
    return *this;
  }

  PrintProxy operator<<(const SetFill& arg) {
    fillchar() = arg.fill;
    return *this;
  }

  PrintProxy operator<<(const SetPrecision& arg) {
    precision() = (arg.precision > 20) ? 20 : arg.precision;
    return *this;
  }

  PrintProxy operator<<(FloatFormat arg) {
    float_mode() = arg;
    return *this;
  }

  /**
   * @brief Architecture-safe padded double overload.
   */
  Print& operator<<(double val) {
    char numBuf[FLOAT_BUFFER_SIZE] = {0};
    FormatFloat(val, numBuf, sizeof(numBuf));

    return m_printPaddedStack(numBuf);
  }

  /**
   * @brief Handles Signed/Unsigned Integer Overloads.
   */
  Print& operator<<(long val) {
    char buf[16];
    ltoa(val, buf, 10);
    return m_printPaddedStack(buf);
  }

  Print& operator<<(int val) { return this->operator<<((long)val); }

  Print& operator<<(unsigned long val) {
    char buf[16];
    ultoa(val, buf, 10);
    return m_printPaddedStack(buf);
  }

  Print& operator<<(unsigned int val) { return this->operator<<((unsigned long)val); }

  Print& operator<<(const char* val) { return m_printPaddedStack(val); }

  Print& operator<<(const PutTime& arg) {
    char timeBuf[TIME_BUFFER_SIZE] = {0};
    FormatTime(arg, timeBuf, sizeof(timeBuf));

    return m_printPaddedStack(timeBuf);
  }

  /**
   * @brief Fallback Template for all unhandled types without specific overload.
   * Width formatting is ignored here for all unknown types.
   */
  template <class T>
  Print& operator<<(const T& val) {
    strm.print(val);
    return strm;
  }

 private:
  /**
   * @brief Centralized padding engine. Handles all fill and jusification
   * evaluations.
   */
  Print& m_printPaddedStack(const char* cstr) {
    if (cstr == nullptr)
      return strm;
    int len = strlen(cstr);
    int padLen = width - len;

    if (left_justify()) {
      strm.print(cstr);
      while (padLen-- > 0)
        strm.print(fillchar());
    } else {
      while (padLen-- > 0)
        strm.print(fillchar());
      strm.print(cstr);
    }

    // Return base stream to destroy proxy transient width state
    return strm;
  }
};

/**
 * @brief Interceptor to trigger the Proxy
 */
inline PrintProxy operator<<(Print& strm, const SetW& arg) {
  int safeWidth = (arg.width > MAX_PAD_WIDTH) ? MAX_PAD_WIDTH : arg.width;
  return {strm, safeWidth};
}

#endif  // NEOSTREAMING_H
