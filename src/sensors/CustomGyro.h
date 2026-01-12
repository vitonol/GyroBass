#pragma once
#include "daisy_seed.h"

namespace daisy
{
    class CustomGyro {

    public:
        void Init(DaisySeed* seed);
        void Process();

        float GetTilt();

        // Debug Variables
		uint8_t device_id;      // Should be 0x69 (105) for LSM6DS3
		int     last_error;     // 0 = OK, 1 = Error
    
    private:
        I2CHandle i2c;
        float tilt_current;

        float smooth_coef;
    };
}
