#include "AirbrakeNNModule.h"
#include "modules/common/ModuleLogger.h"
#include "model/generated/model_weights.h"
#include <lib/geo/physical_constants.h>
#include <cmath>

static void dense_layer(const float *input, const float *weights, const float *bias, float *output, int in_features, int out_features)
{
    for (int i = 0; i < out_features; i++)
    {
        float sum = bias[i];
        for (int j = 0; j < in_features; j++)
        {
            sum += input[j] * weights[i * in_features + j];
        }
        output[i] = sum;
    }
}

static void tanh_activation(float *data, int size)
{
    for (int i = 0; i < size; i++)
    {
        data[i] = tanhf(data[i]);
    }
}

void AirbrakeNNModule::init()
{
}

void AirbrakeNNModule::run()
{
    if (m_EKFSubscriber.poll())
    {
        const auto &ekfData = m_EKFSubscriber.get();

        // Extract down component of position and velocity
        const float &h0 = -ekfData.position.z;
        const float &v0 = -ekfData.velocity.z;

        float predictedApogee = runNeuralNet(h0, v0);

        if (predictedApogee > m_LastPredictedApogee)
        {
            m_LastPredictedApogee = predictedApogee;
        }

        m_AirbrakePublisher.publish({.predictedApogee = m_LastPredictedApogee});
    }
}

float AirbrakeNNModule::runNeuralNet(float h0, float v0)
{
    // Input normalization
    float input[2] = {h0 / MAX_ALTITUDE, v0 / MAX_VELOCITY};

    // Layer 1
    dense_layer(input, net_0_weight, net_0_bias, m_Buffer1, 2, SIZE_NET_0_BIAS); // 32
    tanh_activation(m_Buffer1, SIZE_NET_0_BIAS);

    // Layer 2
    dense_layer(m_Buffer1, net_2_weight, net_2_bias, m_Buffer2, SIZE_NET_0_BIAS, SIZE_NET_2_BIAS); // 32
    tanh_activation(m_Buffer2, SIZE_NET_2_BIAS);

    // Layer 3
    dense_layer(m_Buffer2, net_4_weight, net_4_bias, m_Buffer1, SIZE_NET_2_BIAS, SIZE_NET_4_BIAS); // 32
    tanh_activation(m_Buffer1, SIZE_NET_4_BIAS);

    // Output layer
    dense_layer(m_Buffer1, net_6_weight, net_6_bias, m_Buffer2, SIZE_NET_4_BIAS, SIZE_NET_6_BIAS); // 1

    // Denormalize output
    return m_Buffer2[0] * MAX_ALTITUDE;
}