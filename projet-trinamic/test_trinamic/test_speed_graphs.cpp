#include "tmc4671_tests.h"
#include "UARTComm.h"

static UARTComm uartComm(USBTX, USBRX, 115200);

int random_speed()
{
    return rand() % 100;
}

void speed_graphs()
{
    Motor_tmc4671 motor1(0, Mode::Abn_encoder); //, motor2(1, Mode::Abn_encoder), motor3(2, Mode::Abn_encoder), motor4(3, Mode::Abn_encoder);
    Motor_tmc4671::init_SPI();
    motor1.init_config(0);

    float
        s1 = random_speed(),
        s2 = random_speed(),
        s3 = random_speed(),
        s4 = random_speed();

    while (1) {
        s1 = motor1.get_speed();
//        float speed2 = motor2.get_speed();
//        float speed3 = motor3.get_speed();
//        float speed4 = motor4.get_speed();
        uartComm.send_speeds(s1, s2, s3, s4);
        uartComm.read_speeds(&s1, &s2, &s3, &s4);
        motor1.set_speed(s1);
    }
}