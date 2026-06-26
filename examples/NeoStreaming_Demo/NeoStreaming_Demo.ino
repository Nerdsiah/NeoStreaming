/**
 * @file NeoStreaming_Demo.ino
 * @brief Demonstration of the NeoStreaming Library.
 * 
 * @section DESCRIPTION
 * NeoStreaming is a zero-heap streaming library for 
 * embedded systems (Arduino AVR, SAM, ESP32). It brings standard C++ 
 * <iomanip> syntax (like cout << setw(10)) to microcontrollers without 
 * causing dangerous heap memory fragmentation.
 * 
 * @section ARCHITECTURE
 * This library utilizes a "Zero-Heap Proxy Pattern". All formatting 
 * (padding, decimals, timestamps) is processed within strictly bounded 
 * stack arrays. Memory is immediately released post-execution.
 */

#include <NeoStreaming.h>

// Creates an alias pointing to the hardware Serial port
HardwareSerial& cout = Serial;

void setup() {
    Serial.begin(115200);
    
    // Wait for native USB serial to stabilize (if applicable)
    while (!Serial);

    cout << "========================================" << endl;
    cout << "           NEOSTREAMING DEMO            " << endl;
    cout << "========================================" << endl << endl;

    // --------------------------------------------------------------------
    // FLOAT FORMATTING (fixed, defaultfloat, hexfloat, setprecision)
    // --------------------------------------------------------------------
    // NOTE: Float formatting manipulators are PERSISTENT.
    
    float piValue = 3.14159265;
    float largeValue = 123456.789;

    cout << "--- Float Formatting ---" << endl;
    cout << setprecision(4); // Set formatting to 4 digits
    
    // defaultfloat: Precision applies to TOTAL significant digits
    cout << defaultfloat;
    cout << "defaultfloat (4 sig-figs) : " << piValue << endl;
    
    // fixed: Precision applies strictly to DECIMAL places
    cout << fixed;
    cout << "fixed (4 decimals)        : " << piValue << endl;

    // scientific: Outputs in standard E-notation (e.g., 12346e+05)
    // Native on ARM/ESP, mathematically mapped via dtostre on AVR
    cout << scientific;
    cout << "scientific (E-notation)   : " << largeValue << endl;

    // hexfloat: Outputs exact IEEE-754 representation
    // Works uniformly on ARM/ESP and 8-bit AVR thanks to custom unpacking!
    cout << hexfloat;
    cout << "hexfloat (exact math)     : " << piValue << endl << endl;

    // Switch back to standard fixed for the remainder of the tutorial
    cout << fixed << setprecision(2);

    // --------------------------------------------------------------------
    // PADDING & JUSTIFICATION (setw, setfill, left, right)
    // --------------------------------------------------------------------
    // NOTE: setw() is TRANSIENT. setfill() and justification are PERSISTENT.

    cout << "--- Padding & Justification ---" << endl;
    
    cout << setfill('_'); // Change pad character from space to underscore
    
    cout << right;
    cout << "Right Justified : " << setw(15) << "PAYLOAD" << endl;
    
    cout << left;
    cout << "Left Justified  : " << setw(15) << "PAYLOAD" << endl << endl;
    
    cout << setfill(' '); // Reset fill back to standard spaces

    // --------------------------------------------------------------------
    // TIME FORMATTING (put_time)
    // --------------------------------------------------------------------
    struct tm bootTime = {0};
    bootTime.tm_year = 2026 - 1900; 
    bootTime.tm_mon  = 6 - 1;       
    bootTime.tm_mday = 14;
    bootTime.tm_hour = 8;
    bootTime.tm_min  = 15;
    bootTime.tm_sec  = 0;
    
    cout << "--- Timestamp Formatting ---" << endl;
    cout << "System Boot: " << put_time(&bootTime, "%Y-%m-%d %H:%M:%S") << endl << endl;

    // --------------------------------------------------------------------
    // BUILDING A TEMP SENSOR TABLE
    // --------------------------------------------------------------------
    cout << "=======================================================" << endl;
    cout << "          STARTING MOCK TEMPRATURE SIMULATION          " << endl;
    cout << "=======================================================" << endl;
    
    cout << left 
         << setw(12) << "TIME" 
         << setw(10) << "PACKET" 
         << setw(12) << "STATUS" 
         << setw(15) << "VOLTAGE (V)" 
         << endl;
         
    cout << setfill('-') << setw(49) << "" << endl;
    cout << setfill(' '); // Ready for continuous loop padding
}

void loop() {
    static int packetId = 1;
    static struct tm liveTime = {0, 30, 14, 14, 5, 126}; // Starts at 14:30:00
	
	// Generate mock sensor data
    float voltage = 23.5 + (random(-50, 50) / 100.0);
    const char* status = (voltage < 23.2) ? "WARN" : "NOMINAL";

    // --------------------------------------------------------------------
    // COMBINING ALL FEATURES
    // --------------------------------------------------------------------
    cout << left 
         << setw(12) << put_time(&liveTime, "%H:%M:%S") 
         << right << setfill('0') 
         << setw(6) << packetId << "    "               
         << left << setfill(' ') 
         << setw(12) << status                          
         << right << fixed << setprecision(2) 
         << setw(10) << voltage                         
         << endl;

    packetId++;
    // Increment mock time for seconds only (seconds never wrap back to 0)
    // A real-time clock (RTC) is needed to provide real time updates
    liveTime.tm_sec++; 
    delay(1000); 
}
