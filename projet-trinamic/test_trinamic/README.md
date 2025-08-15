# SSL Trinamic Demo
SSL Brushless V3.0.0 Trinamic TMC4671 demo

## Requirements
### Hardware requirements
The following boards are required:
- *List SSL Trinamic Demo hardware requirements here*

### Software requirements
SSL Trinamic Demo makes use of the following libraries (automatically
imported by `mbed deploy` or `mbed import`):
- *List SSL Trinamic Demo software requirements here*

## Usage
To clone **and** deploy the project in one command, use `mbed import` and skip to the
target and toolchain definition:
```shell
mbed import https://github.com/catie-aq/ssl_trinamic-demo ssl_trinamic-demo
```

Alternatively:

- Clone to "ssl_trinamic-demo" and enter it:
  ```shell
  git clone https://github.com/catie-aq/ssl_trinamic-demo ssl_trinamic-demo
  cd ssl_trinamic-demo
  ```

- Deploy software requirements with:
  ```shell
  mbed deploy
  ```

- Clone custom target repository if necessary:
  ```shell
  git clone YOUR_CUSTOM_TARGET_REPOSITORY your-custom-target
  ```

Define your target and toolchain:
```shell
cp your-custom-target/custom_targets.json . # In case of custom target
mbed target YOUR_MBED_OS_TARGET
mbed toolchain GCC_ARM
```

Compile the project:
```shell
mbed compile
```

Program the target device with a Segger J-Link debug probe and
[`sixtron_flash`](https://gitlab.com/catie_6tron/6tron-flash) tool:
```shell
sixtron_flash YOUR_JLINK_DEVICE BUILD/YOUR_MBED_OS_TARGET/GCC_ARM/ssl_trinamic-demo.elf
```

Debug on the target device with the probe and Segger
[Ozone](https://www.segger.com/products/development-tools/ozone-j-link-debugger)
software.

You can find french test protocols for the SPI protocol in the manual_test directory.