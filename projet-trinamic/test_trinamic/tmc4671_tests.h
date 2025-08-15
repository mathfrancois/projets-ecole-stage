#ifndef __TMC4671_TESTS_H__
#define __TMC4671_TESTS_H__

#include "tmc4671_wrapper.h"

/**
 * Open loop hardware test. Initializes the SPI
 * connection and rotates the motor at 100 RPMe (electrical RPM, so ~= 100 / 8 RPM,
 * where 8 is the number of pole pairs of the motor).
 */
void test_spi_single_motor_move();

/**
 * Hardware SPI motor test, communicating results via Serial
 * baudrate 115200, testing read and write capability.
 * Reads a register whose value is non-null at startup and displays it.
 * Afterwards, modifies the value by setting it to 250 and reads the register
 * again, displaying the result.
 *
 * If the first result displays 0, the test has failed.
 * If the second result does not display 250, the test has failed
 */
void test_spi_single_motor_serial();

/**
 * Encoders initialization test.
 * Operations :
 *     - 1 second rotation of motor in open loop control in negative direction (to the right, wheel considered in front of us)
 *     - Stops motor
 *     - 1 second perform encoder initialization by rotating negatively again
 *     - Stops motor and lights up USER led on 6tron for 2 seconds
 *     - Led disabled
 *     - Rotates motor at 1 RPM for 4 seconds
 *     - Led enabled
 *     - Rotates motor at -1 RPM for 4 seconds
 *     - Led disabled
 *     - Stops motor
 *     - Led enabled
 */
void test_spi_single_motor_encoders();

/**
 * Interactive test mode communicating via serial to an application that
 * controls all 4 motors by changing their speed in real-time.
 */
void speed_graphs();

// TODO: write documentation of this test
void test_spi_multiple_motors();

#endif //__TMC4671_TESTS_H__
