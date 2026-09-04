#include "InternalIgniterADCModule.h"
#include <lib/maths/math_utils.h>

#define EXP_FILTER_IGN_COEFF 0.3f

void InternalIgniterADCModule::init()
{
    hal_adc_init_channel(m_ign1);
    hal_adc_init_channel(m_ign2);
    hal_adc_init_channel(m_ign3);
    hal_adc_init_channel(m_ign4);
}

void InternalIgniterADCModule::run()
{
    m_CurrentFrame.volts[0] = exp_smoothing(readADC(m_ign1), m_CurrentFrame.volts[0], EXP_FILTER_IGN_COEFF);
    m_CurrentFrame.volts[1] = exp_smoothing(readADC(m_ign2), m_CurrentFrame.volts[1], EXP_FILTER_IGN_COEFF);
    m_CurrentFrame.volts[2] = exp_smoothing(readADC(m_ign3), m_CurrentFrame.volts[2], EXP_FILTER_IGN_COEFF);
    m_CurrentFrame.volts[3] = exp_smoothing(readADC(m_ign4), m_CurrentFrame.volts[3], EXP_FILTER_IGN_COEFF);

    m_Publisher.publish(m_CurrentFrame);
}

float InternalIgniterADCModule::readADC(hal_adc_channel_t channel)
{
    return m_CalibScale * (hal_adc_channel_read_voltage(channel) - m_CalibOffset);
}