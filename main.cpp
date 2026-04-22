#include "mbed.h"
#include <string.h>
#include <stdio.h>

// Initialize Serial communication (115200 baud) [cite: 18]
UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

// Hardware Definitions based on Figure 1.1 [cite: 56]
AnalogIn potentiometer(A0);   // Potentiometer on A0 [cite: 32, 47]
AnalogIn lm35(A1);            // LM35 Temperature sensor on A1 [cite: 22, 26]
DigitalInOut sirenPin(PE_10); // Buzzer/Siren on PE_10 [cite: 21, 28]
DigitalIn mq2(PE_12);         // MQ-2 Gas Sensor on PE_12 [cite: 25, 37]

// Function Prototypes
void pcSerialComStringWrite(const char* str);
float readTemperatureCelsius();

int main()
{
    // Configure Buzzer pin: OpenDrain mode is used for the siren [cite: 12]
    sirenPin.mode(OpenDrain);
    sirenPin.input(); // Initial state: Off (High Impedance)
    
    char str[160];
    pcSerialComStringWrite("--- Main Task 4: Monitoring System Started ---\r\n");

    while(true) {
        // 1. Read Sensors [cite: 141]
        int tempC = readTemperatureCelsius();
        // MQ2 Digital output is active LOW (0 when gas is detected) [cite: 11, 59]
        bool gasDetected = (mq2.read() == 0); 
        
        // 2. Dynamic Threshold Calculation [cite: 145]
        // Map potentiometer 0.0-1.0 to a 0-100 sensitivity threshold 
        int threshold = potentiometer.read() * 100.0f;
        
        // 3. Evaluate Warnings [cite: 147]
        bool tempWarning = (tempC > threshold); // [cite: 148]
        bool gasWarning = gasDetected;          // [cite: 150]

        sirenPin.input();
        // 4. Alarm and Messaging Logic [cite: 151, 155]
        if (tempWarning || gasWarning) {
            // Activate Buzzer [cite: 152]
            sirenPin.output();
            sirenPin.write(1);
            //delay(10ms);
            //sirenPin.input(); // Ring the buzzar
            //delay(40ms);

            // Construct Warning Message [cite: 153, 154]
            sprintf(str, "Buzzer ON - Cause: [%s%s%s] | Temp: %d C | Threshold: %.1d\r\n", 
                    tempWarning ? "Temperature" : "",
                    (tempWarning && gasWarning) ? " & " : "",
                    gasWarning ? "Gas" : "",
                    tempC, 
                    threshold);
        } else {
            // System Normal [cite: 157]
            sirenPin.input(); // Turn off Buzzer [cite: 156]
            
            sprintf(str, "System Normal | Temp: %.1d C | Gas: Clean | Threshold: %.1d\r\n", 
                    tempC, threshold);
        }

        // 5. Output to Serial Terminal [cite: 157]
        pcSerialComStringWrite(str);
        
        // Delay for stability and readability [cite: 62, 89]
        thread_sleep_for(500); 
    }
}

// Helper to convert LM35 analog reading to Celsius
float readTemperatureCelsius() {
    // LM35 outputs 10mV/°C. On 3.3V system: (Reading * 3.3V) / 0.01V/°C = Reading * 330
    return lm35.read()  * 3.3 / 0.01;
}

// Helper to write strings to the UART
void pcSerialComStringWrite(const char* str) {
    uartUsb.write(str, strlen(str));
}