#include "tmc4671_tests.h"
#include "UARTComm.h"

static UARTComm uartComm(USBTX, USBRX, 115200);


int main(char *argv[], int argc)
{
    //tmc4671_initSPI();
    // test_spi_single_motor_serial();
    //test_spi_single_motor_move();
    //test_spi_single_motor_encoders();
	speed_graphs();
//	test_spi_multiple_motors();
    return 0;
}
