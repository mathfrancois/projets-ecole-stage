#include "tmc4671_tests.h"
#include "mbed.h"

extern "C"
{
#include "TMC4671.h"
}

void init_registers(Motor_tmc4671 &motor)
{
    // Current limit: 1000 mA (max can be up to 3210 mA)
    motor.write_SPI(TMC4671_PID_TORQUE_FLUX_LIMITS, 0x000003E8);

    // PI settings (256 for torque and velocity, 64 for position)
    motor.write_SPI(TMC4671_PID_TORQUE_P_TORQUE_I, 0x01000100);
    motor.write_SPI(TMC4671_PID_FLUX_P_FLUX_I, 0x01000100);
    motor.write_SPI(TMC4671_PID_VELOCITY_P_VELOCITY_I, 0x01000100);
    motor.write_SPI(TMC4671_PID_POSITION_P_POSITION_I, 0x00400040);

    // Motor type &  PWM configuration
    motor.write_SPI(TMC4671_MOTOR_TYPE_N_POLE_PAIRS, 0x00030008);
    motor.write_SPI(TMC4671_PWM_POLARITIES, 0x00000000);
    motor.write_SPI(TMC4671_PWM_MAXCNT, 0x00000F9F);
    motor.write_SPI(TMC4671_PWM_BBM_H_BBM_L, 0x00001919);
    motor.write_SPI(TMC4671_PWM_SV_CHOP, 0x00000007);

    // ADC configuration
    motor.write_SPI(TMC4671_ADC_I_SELECT, 0x24000100);
    motor.write_SPI(TMC4671_DSADC_MCFG_B_MCFG_A, 0x00100010);
    motor.write_SPI(TMC4671_DSADC_MCLK_A, 0x20000000);
    motor.write_SPI(TMC4671_DSADC_MCLK_B, 0x20000000);
    motor.write_SPI(TMC4671_DSADC_MDEC_B_MDEC_A, 0x014E014E);
    motor.write_SPI(TMC4671_ADC_I0_SCALE_OFFSET, 0x00328394);
    motor.write_SPI(TMC4671_ADC_I1_SCALE_OFFSET, 0x003283FE);

    // ABN encoder settings
    motor.write_SPI(TMC4671_ABN_DECODER_MODE, 0x00000000);
    motor.write_SPI(TMC4671_ABN_DECODER_PPR, 0x000007D0);    
}

void turn_motor_openloop(Motor_tmc4671 &motor, int speed)
{
    // Open loop settings
    motor.write_SPI(TMC4671_OPENLOOP_MODE, 0x00000000);
    motor.write_SPI(TMC4671_OPENLOOP_ACCELERATION, 0x0000003C);
    motor.write_SPI(TMC4671_PHI_E_SELECTION, 0x00000002);

    // Set voltage in the tangential direction
    motor.write_SPI(TMC4671_UQ_UD_EXT, 0x00000913);

    // Switch to open loop velocity mode
    motor.write_SPI(TMC4671_MODE_RAMP_MODE_MOTION, 0x00000008);

    // Rotate right
    motor.write_SPI(TMC4671_OPENLOOP_VELOCITY_TARGET, speed);
}

void turn_motor_abn(Motor_tmc4671 &motor, int speed)
{
    // ABN configuration
    motor.write_SPI(TMC4671_PHI_E_SELECTION, 0x00000003);
    motor.write_SPI(TMC4671_VELOCITY_SELECTION, 0x00000000);

    // Velocity mode
    motor.write_SPI(TMC4671_MODE_RAMP_MODE_MOTION, 0x00000002);

    motor.write_SPI(TMC4671_PID_VELOCITY_TARGET, speed);
}

void stop_motor(Motor_tmc4671 &motor)
{
    // Mode 0: Stopped mode
    motor.write_SPI(TMC4671_MODE_RAMP_MODE_MOTION, 0x00000000);
}

void test_spi_single_motor_encoders()
{
    uint16_t icID = 0; // choose motor 1
    int actualSystick; // dummy variable
    uint8_t initMode = 0;
    uint8_t initState = 0;           // initial state for initialization
    uint16_t initWaitTime;           // dummy variable
    uint16_t actualInitWaitTime;
    uint16_t startVoltage;           // dummy variable
    int16_t hall_phi_e_old;
    int16_t hall_phi_e_new;
    int16_t hall_actual_coarse_offset;

    uint16_t last_Phi_E_Selection;
    uint32_t last_UQ_UD_EXT;
    int16_t last_PHI_E_EXT;

    Motor_tmc4671 motor(0, Mode::Open_loop);
    Motor_tmc4671::init_SPI();

    init_registers(motor);
    turn_motor_openloop(motor, 0x0000003C);
    wait_us(1 * 1e6);

    // initialization for hall sensor support
    tmc4671_startEncoderInitialization(2, &initMode, &initState);
    // motor.init_config(0);

    while (initState != 0)
    {
        tmc4671_periodicJob(
            icID,
            actualSystick,
            initMode,
            &initState,
            initWaitTime,
            &actualInitWaitTime,
            startVoltage,
            &hall_phi_e_old,
            &hall_phi_e_new,
            &hall_actual_coarse_offset,
            &last_Phi_E_Selection,
            &last_UQ_UD_EXT,
            &last_PHI_E_EXT);
    }

    stop_motor(motor);
    DigitalOut led(LED1);
    led = 1;
    wait_us(2 * 1e6);
    led = 0;

    // Turn motor at 480 RPMe
    turn_motor_abn(motor, 0x000001e0);
    wait_us(4 * 1e6);
    led = 1;

    // Turn motor at -480 RPMe
    turn_motor_abn(motor, 0xFFFFFE20);
    wait_us(4 * 1e6);
    led = 0;

    stop_motor(motor);
    led = 1;
}