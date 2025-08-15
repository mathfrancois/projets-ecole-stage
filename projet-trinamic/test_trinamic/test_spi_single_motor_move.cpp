#include "tmc4671_tests.h"
#include <mbed.h>

void set_speed(int32_t speed, Motor_tmc4671 motor)
{
  motor.write_SPI(TMC4671_UQ_UD_EXT, 0x09130000);

  motor.write_SPI(TMC4671_OPENLOOP_VELOCITY_TARGET, speed);

  wait_us(500000);

  motor.write_SPI(TMC4671_OPENLOOP_VELOCITY_TARGET, 0);

  motor.write_SPI(TMC4671_UQ_UD_EXT, 0x00000000);
}

void test_spi_single_motor_move()
{
  Motor_tmc4671 motor1(0, Open_loop);

  Motor_tmc4671::init_SPI();

  motor1.init_config(0);

  motor1.switch_mode(Open_loop);

  set_speed(100, motor1);
}
