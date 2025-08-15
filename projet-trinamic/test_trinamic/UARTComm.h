#ifndef UARTCOMM_H
#define UARTCOMM_H

#include <mbed.h>

class UARTComm {
public:
    UARTComm(PinName tx, PinName rx, int baudrate);
    void send_speeds(float s1, float s2, float s3, float s4);
    // void process_speed_commands();  // Traitement des commandes de vitesse
    void read_speeds(float *s1, float *s2, float *s3, float *s4);  // Lit un float depuis le port série

private:
    UnbufferedSerial uart;
};

#endif // UARTCOMM_H
