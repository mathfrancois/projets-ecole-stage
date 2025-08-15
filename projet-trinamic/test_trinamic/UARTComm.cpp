#include "UARTComm.h"
#include <stdio.h>

UARTComm::UARTComm(PinName tx, PinName rx, int baudrate) 
    : uart(tx, rx, baudrate) {}


void UARTComm::send_speeds(float s1, float s2, float s3, float s4) {
    uint8_t header[2] = { 0xAA, 0xBB };  // Synchronisation
    uart.write(header, 2);

    union {
        float f[4];
        uint8_t bytes[16];
    } data;

    data.f[0] = s1;
    data.f[1] = s2;
    data.f[2] = s3;
    data.f[3] = s4;
    uart.write(data.bytes, 16);
}

void UARTComm::read_speeds(float *s1, float *s2, float *s3, float *s4) {
    // Attente du header de synchronisation 0xAA 0xBB
    uint8_t last_byte = 0;
    uint8_t current_byte = 0;

    while (true) {
        char c;
        if (uart.read(&c, 1) == 1) {
            last_byte = current_byte;
            current_byte = static_cast<uint8_t>(c);
            if (last_byte == 0xAA && current_byte == 0xBB) {
                break;  // Header trouvé
            }
        }
    }

    // Lecture des 16 octets suivants (4 floats)
    uint8_t buffer[16];
    int bytesRead = 0;
    while (bytesRead < 16) {
        char c;
        if (uart.read(&c, 1) == 1) {
            buffer[bytesRead++] = static_cast<uint8_t>(c);
        }
    }

    union {
        uint8_t bytes[16];
        float f[4];
    } data;

    for (int i = 0; i < 16; ++i) {
        data.bytes[i] = buffer[i];
    }

    *s1 = data.f[0];
    *s2 = data.f[1];
    *s3 = data.f[2];
    *s4 = data.f[3];
}
