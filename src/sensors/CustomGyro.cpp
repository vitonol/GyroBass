#include "CustomGyro.h"

#define LSM6DS3_ADDR 0x6B

// Registers
#define WHO_AM_I      0x0F
#define CTRL1_XL      0x10 // Accelerometer Control
#define OUTX_L_XL     0x28 // Accel X Low Byte

using namespace daisy;

void CustomGyro::Init(DaisySeed* seed)
{
    I2CHandle::Config i2c_config;
    i2c_config.periph = I2CHandle::Config::Peripheral::I2C_1;
    i2c_config.speed  = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_config.pin_config.scl = seed::D11; 
    i2c_config.pin_config.sda = seed::D12;
    
    i2c.Init(i2c_config);
    tilt_current = 0.5f;
    smooth_coef = 0.05f;
    last_error = 255; // Start with "Unknown" status
    device_id = 0;

    // This register is read-only and always contains 0x69 (105)
    i2c.ReadDataAtAddress(LSM6DS3_ADDR, WHO_AM_I, 1, &device_id, 1, 100);

    uint8_t val = 0x40;
    i2c.WriteDataAtAddress(LSM6DS3_ADDR, CTRL1_XL, 1, &val, 1, 100);
}

void CustomGyro::Process()
{
    uint8_t data[2];
    I2CHandle::Result res = i2c.ReadDataAtAddress(LSM6DS3_ADDR, OUTX_L_XL, 1, data, 2, 10);

    last_error = (int)res;

    if(res == I2CHandle::Result::OK)
    {
        // Combine bytes into signed 16-bit integer
        int16_t raw_x = (int16_t)((data[1] << 8) | data[0]);

        // Normalize to -1.0 to 1.0 range
        float acc_x = raw_x / 16384.0f;

        // Map to 0.0 - 1.0 range
        const float min_acc = -0.966f; // 8.5 o'clock
        const float max_acc = -0.5f;   // 11 o'clock
        float target = (acc_x - min_acc) / (max_acc - min_acc);

        // Clamp 
        if(target > 1.0f) target = 1.0f;
        if(target < 0.0f) target = 0.0f;

        // Apply simple One-Pole Smoothing
        tilt_current += smooth_coef * (target - tilt_current);
    }
}

float CustomGyro::GetTilt()
{
    return tilt_current;
}
