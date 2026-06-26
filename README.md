# NeoStreaming

NeoStreaming is a lightweight C++ style streaming library for Arduino. It provides `<<` stream formatting with standard manipulators such as `setw()`, `setprecision()`, `setfill()`, `left`, `right`, `fixed`, `scientific`, `defaultfloat`, and `hexfloat` without using heap memory.

## Features

- Zero-heap proxy pattern for safe embedded stream formatting
- `<<` operator support for Arduino `Print` objects like `Serial`
- Persistent manipulators: `setprecision()`, `setfill()`, `left`, `right`, `fixed`, `defaultfloat`, `scientific`, `hexfloat`
- Transient width formatting: `setw()` applies only to the next value
- Time formatting with `put_time()` and AVR-safe fallback support
- Works on AVR, SAM, ESP32 and other Arduino-compatible architectures

## Installation

1. Copy the `NeoStreaming` folder into your Arduino `libraries` directory.
2. Restart the Arduino IDE.
3. Include the library in your sketch:

```cpp
#include <NeoStreaming.h>
```

## Usage

Create an alias for a `Print` stream and use `<<` just like standard C++ output:

```cpp
HardwareSerial& cout = Serial;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  cout << "Hello NeoStreaming" << endl;
  cout << setprecision(4) << fixed << 3.14159 << endl;
}
```

## Supported manipulators

- `setw(int width)` — temporary width for the next value
- `setprecision(int precision)` — persistent precision for floating-point output
- `setfill(char fillChar)` — persistent fill character
- `left` / `right` — persistent justification
- `fixed` / `scientific` / `defaultfloat` / `hexfloat` — persistent float formatting
- `put_time(&tm, format)` — format a `struct tm` timestamp
- `endl` — print a newline

## Example

The bundled demo `examples/NeoStreaming_Demo/NeoStreaming_Demo.ino` shows:

- float formatting (`defaultfloat`, `fixed`, `scientific`, `hexfloat`)
- `setw()` padding and `setfill()`
- left/right justification
- timestamp formatting with `put_time()`
- building a formatted sensor-style output table

## Library Contents

- `src/NeoStreaming.h` — main public header
- `src/utility/util.h` — Arduino compatibility utilities
- `examples/NeoStreaming_Demo/NeoStreaming_Demo.ino` — demonstration sketch
- `library.properties` — library metadata

## License

This library is released under the GNU General Public License v3 (GPL-3.0-or-later). See `LICENSE` for details.
