#include "VoltageModule.h"
#include "modules/common/ModuleLogger.h"
#include <board_config.h>
#include <hal/gpio_driver.h>

void VoltageModule::init()
{
    initPin(CFG_PIN_3V3);
    initPin(CFG_PIN_5V);
    initPin(CFG_PIN_VBAT);
}

void VoltageModule::run()
{
    using namespace PubSub::Helpers;

    if (m_RPC.requestAvailable())
    {
        const auto &req = m_RPC.getRequestData();
        uint8_t pin = req.pin == VoltagePinsFlags::VOLTAGE_PIN_3V3 ? CFG_PIN_3V3 : (req.pin == VoltagePinsFlags::VOLTAGE_PIN_5V ? CFG_PIN_5V : CFG_PIN_VBAT);

        m_CurrentPinStates = req.enabled ? (m_CurrentPinStates | req.pin) : (m_CurrentPinStates & ~req.pin);

        setPinState(req.enabled, pin);

        m_RPC.sendResponse(true);
        m_VoltageStatePublisher.publish({.pingsFlags = m_CurrentPinStates});

        OBC_DEBUG("Voltage pin %d set to %s", pin, req.enabled ? "ENABLED" : "DISABLED");
    }
}

void VoltageModule::initPin(uint8_t pin)
{
    hal_gpio_init_pin(pin, GPIO_OUTPUT);
    hal_gpio_set_pin_state(pin, GPIO_LOW);
}

void VoltageModule::setPinState(bool enable, uint8_t pin)
{
    if (hal_gpio_get_pin_state(pin) != (enable ? GPIO_HIGH : GPIO_LOW))
    {
        hal_gpio_set_pin_state(pin, enable ? GPIO_HIGH : GPIO_LOW);
    }
}