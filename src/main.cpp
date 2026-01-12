
/*
___________________________________________________________________________________                       

        Built-in into bass/guitar distortion effect that implements LSM6DS3 gyroscope
		To allow controlling filter cutoff frequency by rotating/twisting/tilting guitar body.
___________________________________________________________________________________

*/

#include "daisy_seed.h"
#include "sensors/CustomGyro.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed   hw;
Overdrive   drive;
DcBlock     dc;
Svf         filter;
CustomGyro  gyro;
Switch      button1;
// Oscillator  lfo; // was used for testing modulating freq cutoff. 

// Global Variables
volatile float g_tilt = 0.5f; 

enum SwitchFX { ONE, TWO, THREE };
static int fxIncrement = 0;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // @TODO map drive to be slightly dependent on tilt. Something between 0.7 .. 0.9
    float drive_val = 0.8f;
    drive.SetDrive(drive_val);

    // Maping Filter Frequency cutoff sensor's rotation
    float tilt_hz = 160.0f + (g_tilt * g_tilt * 3350.0f);
    
    filter.SetFreq(tilt_hz); 
    filter.SetRes(0.15f); 
    filter.SetDrive(0.0f); // Clean filter resonance

    for(size_t i = 0; i < size; i++)
    {
        float raw_input = dc.Process(in[0][i]);
        
        // Tame the input volume before distortion by allowing some headroom to prevent quick compression buildup
        float signal_to_distort = raw_input * 0.6f;

        float wet_signal = drive.Process(signal_to_distort);
        
        // Only filter the wet signal - allowing clean low end to pass through
        filter.Process(wet_signal);
        
        float filtered_wet = 0.f;
        switch (fxIncrement) {
            case 0: filtered_wet = filter.Low(); break;
            case 1: filtered_wet = filter.Notch(); break;
            case 2: filtered_wet = filter.Band(); break;
        }

        // Blend clean signal with distorted
        float master_mix = (raw_input * 0.6f) + (filtered_wet * 0.5f);
        out[0][i] = out[1][i] = master_mix;
    }
}

int main(void)
{
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    float sample_rate = hw.AudioSampleRate();

    gyro.Init(&hw);
    
    AdcChannelConfig adcConfig;
    adcConfig.InitSingle(hw.GetPin(21));
    button1.Init(hw.GetPin(28), sample_rate / 48.f);
    hw.adc.Init(&adcConfig, 1);
    hw.adc.Start();

    dc.Init(sample_rate);
    drive.Init();
    drive.SetDrive(0.f);
    filter.Init(sample_rate);

    hw.StartAudio(AudioCallback);

    while(1)
    {
        // @TODO Implement FreeRTOS
        button1.Debounce();
        if(button1.RisingEdge())
        {
            fxIncrement = (fxIncrement + 1) % 3;
            // Blink LED briefly to show mode change <-- Sometime button stays
            hw.SetLed(true);
            System::Delay(10);
            hw.SetLed(false);
        }

        gyro.Process();
        
        // Update the global variable so the Audio Callback sees it
        g_tilt = gyro.GetTilt();

        // Small delay to prevent CPU hogging
        System::Delay(5);
    }
}
