#include "SimBridgeModule.h"
#include "sitl.h"
#include "modules/common/ModuleLogger.h"
#include <lib/debug/sys_assert.h>

SimBridgeModule::~SimBridgeModule()
{
    m_PhysicsSocket.close();
}

void SimBridgeModule::init()
{
    sitl_init_godmode();

    m_PhysicsSocket.createServer(m_Port);
    m_PhysicsSocket.setBlocking(true);
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
        LOG_INFO("Physics engine disconnected, stopping SITL simulation.");
        sitl_stop();
        return;
    }

    sitl_request_data physxData;
    int unpackResult = datalink_unpack_sitl_request_data(&physxData, &physxMsg);

    SYS_ASSERT(unpackResult == DATALINK_OK);

    if (physxData.readFlags & DATALINK_FLAGS_SITL_READ_IMU_1)
    {
        m_IMU1DataPublisher.publish({
            .dt = physxData.imu1dt,
            .acc = {
                .x = physxData.imu1AccX,
                .y = physxData.imu1AccY,
                .z = physxData.imu1AccZ,
            },
            .gyro = {
                .x = physxData.imu1GyroX,
                .y = physxData.imu1GyroY,
                .z = physxData.imu1GyroZ,
            },
            .clippingFlags = physxData.imu1ClippingFlags,
        });
    }
    if (physxData.readFlags & DATALINK_FLAGS_SITL_READ_MAG_1)
    {
        m_Mag1DataPublisher.publish({
            .mag = {
                .x = physxData.mag1X,
                .y = physxData.mag1Y,
                .z = physxData.mag1Z,
            },
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
            .pos = {
                .lat = physxData.gps1Lat,
                .lon = physxData.gps1Lon,
                .alt = physxData.gps1Alt,
            },
            .vel = {
                .x = physxData.gps1VelN,
                .y = physxData.gps1VelE,
                .z = physxData.gps1VelD,
            },
            .stddev_horizontal = physxData.gps1stddevHorizontal,
            .stddev_vertical = physxData.gps1stddevVertical,
            .stddev_speed = physxData.gps1stddevSpeed,
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
        LOG_ERROR("Cannot send physics response data: No active connection to physics engine.");

        return;
    }

    m_ResponseData.responseFlags = 0;

    if (m_EKFStateSubscriber.poll())
    {
        const auto &ekfState = m_EKFStateSubscriber.get();

        m_ResponseData.responseFlags |= DATALINK_FLAGS_SITL_RESP_EKF;
        m_ResponseData.posN = ekfState.position.x;
        m_ResponseData.posE = ekfState.position.y;
        m_ResponseData.posD = ekfState.position.z;
        m_ResponseData.velN = ekfState.velocity.x;
        m_ResponseData.velE = ekfState.velocity.y;
        m_ResponseData.velD = ekfState.velocity.z;
        m_ResponseData.qw = ekfState.orientation.w;
        m_ResponseData.qx = ekfState.orientation.x;
        m_ResponseData.qy = ekfState.orientation.y;
        m_ResponseData.qz = ekfState.orientation.z;
    }
    if (m_StateMachineStateSubscriber.poll())
    {
        const auto &stateMachineState = m_StateMachineStateSubscriber.get();

        m_ResponseData.smState = stateMachineState.state;
    }
    if (m_IGNSubscriber.poll())
    {
        const auto &ignState = m_IGNSubscriber.get();

        m_ResponseData.ignFiredFlags = 0;

        for (int i = 0; i < PubSub::Helpers::IGN_CHANNELS_COUNT; i++)
        {
            m_ResponseData.ignFiredFlags |= (ignState.fired[i] ? (1 << i) : 0);
        }
    }
    if (m_AirbrakeStateSubscriber.poll())
    {
        m_ResponseData.predictedApogee = m_AirbrakeStateSubscriber.get().predictedApogee;
    }

    datalink_message_t msg;
    datalink_pack_sitl_response_data(&m_ResponseData, &msg);

    m_PhysicsSocket.send(&msg);
}