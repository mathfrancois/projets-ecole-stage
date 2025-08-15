#include "tmc4671_tests.h"

UnbufferedSerial serial_log(USBTX, USBRX);
#define BUFSIZE 200

#define print_value(format, ...) { \
  char buffer[BUFSIZE]; \
  memset(buffer, 0, BUFSIZE); \
  int l = sprintf(buffer, format, ##__VA_ARGS__); \
  serial_log.write(buffer, l); \
}

void print_hexa_format(uint8_t *data, char *str, size_t data_length)
{
  char buffer[BUFSIZE];
  memset(buffer, 0, BUFSIZE);
  int len = sprintf(buffer, str);
  char *data_buf = buffer + len;
  for (size_t i = 0; i < data_length; ++i)
  {
    int l = sprintf(data_buf, "0x%x ", data[i]);
    data_buf += l;
    len += l;
  }
  data_buf[0] = '\n';
  len += 1;
  serial_log.write(buffer, len);
}

void test_spi_single_motor_serial()
{
  // setup serial logging
  serial_log.baud(115200);
  while (!serial_log.writable())
    ;
  ThisThread::sleep_for(1s);

  print_value("test for intitialize\n");

  Motor_tmc4671 motor1(0, Mode::Open_loop);

  Motor_tmc4671::init_SPI();

  // uint8_t address = TMC4671_ADC_I0_SCALE_OFFSET;
  uint8_t address = 0x09;
  int32_t value = (motor1.read_SPI(address)) & 0x0000FFFF;

  //display the value returned by the read of the register
  print_value("value: %ld\n", value);

  wait_us(2000000);
  //modifying the value of the register
  motor1.write_SPI(address, 250);
  // tmc4671_writeRegister(0, address, 250 << 16); // va effacer la valeur qu'il y avait précédemment -> il faut récupérer l'ancienne valeur et rajouter ce que l'on veut en modifiant les bons bits
  wait_us(2000000);
  int32_t newvalue = (motor1.read_SPI(address)) & 0X0000FFFF;

  //printing the newvalue read
  print_value("new value: %ld\n", newvalue);

  //reseting the parameters to default
  motor1.write_SPI(address, 256);
}