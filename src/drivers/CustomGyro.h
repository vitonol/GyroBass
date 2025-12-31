#pragma once
#include "daisy_seed.h"

namespace daisy
{
    class CustomGyro {
    public:
        void Init(DaisySeed* seed){}
        float GetTilt() { return 0.0f; }
    };
}