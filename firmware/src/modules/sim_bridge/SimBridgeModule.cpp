#include "SimBridgeModule.h"
#include "sitl.h"
#include "modules/common/ModuleLogger.h"
#include <lib/debug/obc_assert.h>

#define PHYSICS_ENGINE_PORT 12345

SimBridgeModule::~SimBridgeModule()
{
    m_PhysicsSocket.close();
}

void SimBridgeModule::init()
{
    sitl_init_godmode();

    m_PhysicsSocket.createServer(PHYSICS_ENGINE_PORT, true);
}

void SimBridgeModule::run()
{
    sitl_wait_for_threads_ready();

    sendPhysicsResponseData();
    receivePhysicsData();

    sitl_time_tick();
}

void SimBridgeModule::receivePhysicsData()
{
    datalink_message_t physxMsg;
    if (!m_PhysicsSocket.receive(&physxMsg))
    {
        OBC_INFO("Physics engine disconnected, stopping SITL simulation.");
        sitl_stop();
        return;
    }

    sitl_request_data physxData;
    int unpackResult = datalink_unpack_sitl_request_data(&physxData, &physxMsg);
    
    OBC_ASSERT(unpackResult == DATALINK_OK);

    if (physxData.readFlags & DATALINK_FLAGS_SITL_READ_IMU_1)
    {
        m_IMU1DataPublisher.publish({
            .dt = physxData.imu1dt,
            .acc = {physxData.imu1AccX, physxData.imu1AccY, physxData.imu1AccZ},
            .gyro = {physxData.imu1GyroX, physxData.imu1GyroY, physxData.imu1GyroZ},
        });
    }
    if (physxData.readFlags & DATALINK_FLAGS_SITL_READ_MAG_1)
    {
        m_Mag1DataPublisher.publish({
            .mag = {physxData.mag1X, physxData.mag1Y, physxData.mag1Z},
        });
    }
    if (physxData.readFlags & DATALINK_FLAGS_SITL_READ_BARO_1)
    {
        m_Baro1DataPublisher.publish({
            .press = physxData.baro1Pressure,
            .temp = physxData.baro1Temperature,
            .baroHeight = physxData.baro1Height,
        });
    }
    if (physxData.readFlags & DATALINK_FLAGS_SITL_READ_GPS_1)
    {
        m_GPS1DataPublisher.publish({
            .pos = {physxData.gps1Lat, physxData.gps1Lon, physxData.gps1Alt},
            .gpsFix = physxData.gps1Sats > 0,
            .gpsIs3dFix = physxData.gps1Sats > 3,
            .gpsSatellitesCount = physxData.gps1Sats,
        });
    }
}

void SimBridgeModule::sendPhysicsResponseData()
{
    if (!m_PhysicsSocket.isActive())
    {
        OBC_ERROR("Cannot send physics response data: No active connection to physics engine.");
        
        return;
    }

    if (m_EKFStateSubscriber.poll())
    {
        const auto &ekfState = m_EKFStateSubscriber.get();

        m_responseData.posN = ekfState.position.x;
        m_responseData.posE = ekfState.position.y;
        m_responseData.posD = ekfState.position.z;
        m_responseData.velN = ekfState.velocity.x;
        m_responseData.velE = ekfState.velocity.y;
        m_responseData.velD = ekfState.velocity.z;
        m_responseData.accN = ekfState.acceleration.x;
        m_responseData.accE = ekfState.acceleration.y;
        m_responseData.accD = ekfState.acceleration.z;
        m_responseData.qw = ekfState.orientation.w;
        m_responseData.qx = ekfState.orientation.x;
        m_responseData.qy = ekfState.orientation.y;
        m_responseData.qz = ekfState.orientation.z;
    }
    if (m_StateMachineStateSubscriber.poll())
    {
        const auto &stateMachineState = m_StateMachineStateSubscriber.get();

        m_responseData.smState = stateMachineState.state;
    }
    if (m_IGNSubscriber.poll())
    {
        const auto &ignState = m_IGNSubscriber.get();

        m_responseData.ignFiredFlags = 0;

        for (int i = 0; i < PubSub::Helpers::IGN_CHANNELS_COUNT; i++)
        {
            m_responseData.ignFiredFlags |= (ignState.fired[i] ? (1 << i) : 0);
        }
    }

    datalink_message_t msg;
    datalink_pack_sitl_response_data(&m_responseData, &msg);

    m_PhysicsSocket.send(&msg);
}