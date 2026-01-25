/*_____________________________________________________________________________                       

        Built-in into bass/guitar distortion effect that implements LSM6DS3 gyroscope
		To allow controlling filter cutoff frequency by rotating/twisting/tilting guitar body.
___________________________________________________________________________________
*/

#include "daisy_seed.h"
#include "daisysp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sensors/CustomGyro.h"

using namespace daisy;
using namespace daisysp;

DaisySeed   hw;
CustomGyro  gyro;
Overdrive   drive;
Svf         filter;
DcBlock     dc;
Switch      button1;

// Global Variables
volatile float g_tilt = 0.5f;
volatile static int fxIncrement = 0;

extern "C" {
    // Get the FreeRTOS tick function prototype
    void xPortSysTickHandler(void);

    // custom handler
    void SysTick_Handler(void)
    {
        HAL_IncTick();
        
        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            xPortSysTickHandler();
        }
    }
}

// ================================================
//  AUDIO CALLBACK
// ===============================================
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
        float filtered_wet = 0;

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

// ===============================================
//  FREERTOS TASKS
// ================================================
// 1. Read sensors at 100Hz
void Task_Sensors(void *param)
{
    while(1){
        gyro.Process();
        g_tilt = gyro.GetTilt();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// 2. UI at 20Hz
void Task_UI(void *param)
{
    while(1){
        bool led_state = (g_tilt > 0.5f);
        button1.Debounce();
        if(button1.RisingEdge())
        {
            led_state = !led_state; // blinks led briefly on or off depending on the current tilt value
            fxIncrement = (fxIncrement + 1) % 3;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
        hw.SetLed(led_state);
    }
}

// ========================================
// MAIN
// ========================================
int main(void)
{
    // Boot Hardware
    hw.Init();
    hw.SetAudioBlockSize(4);
    float sample_rate = hw.AudioSampleRate();

    //Init Drivers and processors
    gyro.Init(&hw);
    button1.Init(hw.GetPin(28), sample_rate / 48.f);
    dc.Init(sample_rate);
    drive.Init();
    filter.Init(sample_rate);

    // Start Audio - runs on its own iterrupt
    hw.StartAudio(AudioCallback);

    // Create RTOS Tasks
    xTaskCreate(Task_Sensors, "Sensors", 256, NULL, 2, NULL); // 2 Is higher priority
    xTaskCreate(Task_UI, "UI", 256, NULL, 1, NULL); // 1 Is lower priority
    vTaskStartScheduler();
    while(1);// Should never reach this point
}

/* Currently unsused knob */
//     AdcChannelConfig adcConfig; // knob
//     adcConfig.InitSingle(hw.GetPin(21));
//     hw.adc.Init(&adcConfig, 1);
//     hw.adc.Start();