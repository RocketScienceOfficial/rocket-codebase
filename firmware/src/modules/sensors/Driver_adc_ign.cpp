#include "Driver_adc_ign.h"
#include <board_config.h>
#include <lib/maths/math_utils.h>
#include <hal/adc_driver.h>

#define ADC_CALIBRATION_SCALE 1.035f
#define ADC_CALIBRATION_OFFSET 0.036f

#define EXP_FILTER_IGN_COEFF 0.3f

void Driver_adc_ign::initialize()
{
    hal_adc_init_pin(CFG_PIN_IGN_DET_1);
    hal_adc_init_pin(CFG_PIN_IGN_DET_2);
    hal_adc_init_pin(CFG_PIN_IGN_DET_3);
    hal_adc_init_pin(CFG_PIN_IGN_DET_4);
}

void Driver_adc_ign::readAndPublish(float dt)
{
    (void)dt;
    
    m_CurrentFrame.volts[0] = exp_smoothing(readADC(CFG_PIN_IGN_DET_1), m_CurrentFrame.volts[0], EXP_FILTER_IGN_COEFF);
    m_CurrentFrame.volts[1] = exp_smoothing(readADC(CFG_PIN_IGN_DET_2), m_CurrentFrame.volts[1], EXP_FILTER_IGN_COEFF);
    m_CurrentFrame.volts[2] = exp_smoothing(readADC(CFG_PIN_IGN_DET_3), m_CurrentFrame.volts[2], EXP_FILTER_IGN_COEFF);
    m_CurrentFrame.volts[3] = exp_smoothing(readADC(CFG_PIN_IGN_DET_4), m_CurrentFrame.volts[3], EXP_FILTER_IGN_COEFF);

    m_Publisher.publish(m_CurrentFrame);
}

float Driver_adc_ign::readADC(uint8_t pin)
{
    return ADC_CALIBRATION_SCALE * (hal_adc_read_voltage(pin) - ADC_CALIBRATION_OFFSET);
}