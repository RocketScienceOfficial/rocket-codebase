#include "AirbrakeModule.h"
#include "modules/common/ModuleLogger.h"
#include <lib/geo/physical_constants.h>
#include <cmath>

#define PARAM_M 9.000f
#define PARAM_CD 0.01558f
#define PARAM_AREA 0.461f
#define PARAM_K (0.5f * PARAM_CD * PARAM_AREA * SEA_LEVEL_AIR_DENSITY)

void AirbrakeModule::init()
{
}

void AirbrakeModule::run()
{
    if (m_EKFSubscriber.poll())
    {
        const auto &ekfData = m_EKFSubscriber.get();

        float h0 = ekfData.position.z;
        float v0 = ekfData.velocity.z;

        float predictedApogee = h0 + PARAM_M / (2.0f * PARAM_K) * logf(1.0f + (PARAM_K * v0 * v0) / (PARAM_M * EARTH_GRAVITY));

        if (predictedApogee > m_LastPredictedApogee)
        {
            m_LastPredictedApogee = predictedApogee;
        }

        m_AirbrakePublisher.publish({.predictedApogee = m_LastPredictedApogee});
    }
}