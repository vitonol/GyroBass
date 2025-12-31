#pragma once

class DynamicStateVariableFilter{
public:
    void Init(float sample_rate) {}
    void Process(float in, float& out) { out = in; }

};