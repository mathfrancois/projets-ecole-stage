#include "tmc4671_tests.h"
#include "mbed.h"
#include <cmath>

extern "C"
{
#include "TMC4671.h"
}

#define ROBOT_RADIUS 0.5  // a corriger
#define WHEEL_RADIUS 0.025  // en m
#define N_MOTORS 4

void _turn_motor_abn(Motor_tmc4671 &motor, float speed)
{
    // ABN configuration
    // motor.write_SPI(TMC4671_PHI_E_SELECTION, 0x00000003);
    // motor.write_SPI(TMC4671_VELOCITY_SELECTION, 0x00000000);

    // // Velocity mode
    // motor.write_SPI(TMC4671_MODE_RAMP_MODE_MOTION, 0x00000002);

    motor.set_speed(speed);
}

void _stop_motor(Motor_tmc4671 &motor)
{
    // Mode 0: Stopped mode
    motor.write_SPI(TMC4671_MODE_RAMP_MODE_MOTION, 0x00000000);
}

void compute_motor_speed(
    float speeds[N_MOTORS], float normal_speed, float tangential_speed, float angular_speed)
{
    speeds[0] = -(sqrt(3) / 2.) * normal_speed + (1. / 2.) * tangential_speed + ROBOT_RADIUS * angular_speed;
    //motor_speed->speed1 *= 480/(2*M_PI*WHEEL_RADIUS);
    speeds[1] = -(sqrt(2) / 2.) * normal_speed - (sqrt(2) / 2.) * tangential_speed + ROBOT_RADIUS * angular_speed;
    //motor_speed->speed2 *= 480/(2*M_PI*WHEEL_RADIUS);
    speeds[2] = (sqrt(2) / 2.) * normal_speed - (sqrt(2) / 2.) * tangential_speed + ROBOT_RADIUS * angular_speed;
    //motor_speed->speed3 *= 480/(2*M_PI*WHEEL_RADIUS);
    speeds[3] = (sqrt(3) / 2.) * normal_speed + (1. / 2.) * tangential_speed + ROBOT_RADIUS * angular_speed;
    //motor_speed->speed4 *= 480/(2*M_PI*WHEEL_RADIUS);
}

void test_spi_multiple_motors() 
{
    Motor_tmc4671 motors[N_MOTORS] = {
        Motor_tmc4671(0, Abn_encoder),
        Motor_tmc4671(1, Abn_encoder),
        Motor_tmc4671(2, Abn_encoder),
        Motor_tmc4671(3, Abn_encoder)
    };

    float normal_speed, tangential_speed, angular_speed;
    float speeds[N_MOTORS];

    Motor_tmc4671::init_SPI();
    for(int i = 0; i < N_MOTORS; i++)
        motors[i].init_config(0);

    // First command: go forward
    normal_speed = 1;  // m/s
    tangential_speed = 0;  // m/s
    angular_speed = 0;  // m/s
    compute_motor_speed(speeds, normal_speed, tangential_speed, angular_speed);

    for(int i = 0; i < N_MOTORS; i++)
        _turn_motor_abn(motors[i], speeds[i]);
    
    wait_us(2*1e6);

    // First command: go right
    normal_speed = 0;  // m/s
    tangential_speed = 1;  // m/s
    angular_speed = 0;  // m/s
    compute_motor_speed(speeds, normal_speed, tangential_speed, angular_speed);

    for(int i = 0; i < N_MOTORS; i++)
        _turn_motor_abn(motors[i], speeds[i]);

    wait_us(2*1e6);

    // First command: go backwards
    normal_speed = -1;  // m/s
    tangential_speed = 0;  // m/s
    angular_speed = 0;  // m/s
    compute_motor_speed(speeds, normal_speed, tangential_speed, angular_speed);

    for(int i = 0; i < N_MOTORS; i++)
        _turn_motor_abn(motors[i], speeds[i]);

    wait_us(2*1e6);

    // First command: go left
    normal_speed = 0;  // m/s
    tangential_speed = -1;  // m/s
    angular_speed = 0;  // m/s
    compute_motor_speed(speeds, normal_speed, tangential_speed, angular_speed);

    for(int i = 0; i < N_MOTORS; i++)
        _turn_motor_abn(motors[i], speeds[i]);

    wait_us(2*1e6);

    // Stops all motors
    for(int i = 0; i < N_MOTORS; i++)
        _stop_motor(motors[i]);

}
