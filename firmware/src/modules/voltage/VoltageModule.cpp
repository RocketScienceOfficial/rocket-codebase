#include "VoltageModule.h"
#include "modules/common/ModuleLogger.h"
#include <hal/gpio_driver.h>

void VoltageModule::init()
{
    initPin(m_Pin3V3);
    initPin(m_Pin5V);
    initPin(m_PinVBAT);
}

void VoltageModule::run()
{
    using namespace PubSub::Helpers;

    if (m_RPC.requestAvailable())
    {
        const auto &req = m_RPC.getRequestData();
        hal_gpio_pin_t pin = req.pin == VoltagePinsFlags::VOLTAGE_PIN_3V3 ? m_Pin3V3 : (req.pin == VoltagePinsFlags::VOLTAGE_PIN_5V ? m_Pin5V : m_PinVBAT);

        m_CurrentPinStates = req.enabled ? (m_CurrentPinStates | req.pin) : (m_CurrentPinStates & ~req.pin);

        setPinState(req.enabled, pin);

        m_RPC.sendResponse(true);
        m_VoltageStatePublisher.publish({.pingsFlags = m_CurrentPinStates});

        LOG_DEBUG("Voltage pin %d set to %s", pin, req.enabled ? "ENABLED" : "DISABLED");
    }
}

void VoltageModule::initPin(hal_gpio_pin_t pin)
{
    hal_gpio_init_pin(pin, HAL_GPIO_OUTPUT);
    hal_gpio_set_pin_state(pin, HAL_GPIO_LOW);
}

void VoltageModule::setPinState(bool enable, hal_gpio_pin_t pin)
{
    if (hal_gpio_get_pin_state(pin) != (enable ? HAL_GPIO_HIGH : HAL_GPIO_LOW))
    {
        hal_gpio_set_pin_state(pin, enable ? HAL_GPIO_HIGH : HAL_GPIO_LOW);
    }
}