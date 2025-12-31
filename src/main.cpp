// #include "daisy_seed.h"

// using namespace daisy;

// DaisySeed hw;

// int main(void)
// {
//     hw.Init();
    
//     while(1) {
//         hw.SetLed(true);
//         System::Delay(500);
//         hw.SetLed(false);
//         System::Delay(500);
//     }
// }

#include "daisy_seed.h"
#include "daisysp.h"

/*
    Digital Symmetrical Soft Clipping Algorithm
*/

using namespace daisy;
using namespace daisysp;

DaisySeed  hw;
Overdrive  drive;
DcBlock dc;
Svf filter;

Switch button1;

enum SwitchFX
{
    ONE,
    TWO,
    THREE
};

static int fxIncrement = 0;
static SwitchFX fx = ONE;

Oscillator lfo; // , osc;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    
    float knob = hw.adc.GetFloat(0);
    float drive_amount = knob * knob;

    float tone_hz = 6000.0f - 5200.0f * drive_amount; // darker as drive increases
    tone_hz = fclamp(tone_hz, 200.f, 6000.0f);
    
    static float drive_smooth = 0.0f;
    drive_smooth += 0.002f * (drive_amount - drive_smooth);

    float mod = 0.5 + 0.5 * fabs(lfo.Process()); // 0.5..1.0
    filter.SetFreq(tone_hz * mod);
    filter.SetRes(0.3f);
    filter.SetDrive(0.2f);
    
    drive.SetDrive(drive_smooth);

    for(size_t i = 0; i < size; i++)
    {
        float input = dc.Process(in[0][i]);

        float sig = drive.Process(input);

        filter.Process(sig);

        float filteredSig = 0.f;

        switch (fxIncrement)
        {
            case 0: filteredSig = filter.Low(); break;
            case 1: filteredSig = filter.Band(); break;
            case 2: filteredSig = filter.Notch(); break;
        }

        out[0][i] = out[1][i] = filteredSig;
    }
}

int main(void)
{
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    float sample_rate = hw.AudioSampleRate();

    AdcChannelConfig adcConfig;
    adcConfig.InitSingle(hw.GetPin(21));

    //Initialize the button on pin 28
    button1.Init(hw.GetPin(28), sample_rate / 48.f);
    hw.adc.Init(&adcConfig, 1);
    hw.adc.Start();
    
    dc.Init(sample_rate);

    drive.Init();
    drive.SetDrive(0.f);

    filter.Init(sample_rate);


    // osc.Init(sample_rate);

    lfo.Init(sample_rate);
    lfo.SetAmp(.8f);
    lfo.SetWaveform(Oscillator::WAVE_TRI);
    lfo.SetFreq(.12f);

    hw.StartAudio(AudioCallback);
    while(1)
    {
        button1.Debounce();
        if(button1.RisingEdge())
        {
            fxIncrement = (fxIncrement + 1) % 3;
            System::Delay(1);
        }
    }
}
