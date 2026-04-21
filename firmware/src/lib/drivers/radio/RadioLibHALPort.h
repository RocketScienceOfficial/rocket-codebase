#ifndef _RADIOLIB_HAL_PORT_H
#define _RADIOLIB_HAL_PORT_H

#include <RadioLib.h>
#include <osal/systime.h>
#include <hal/gpio_driver.h>
#include <hal/spi_driver.h>
#include <hal/time_driver.h>

class RadioLibHALPort : public RadioLibHal
{
public:
    RadioLibHALPort(uint8_t spi, uint8_t misoPin, uint8_t mosiPin, uint8_t sckPin, uint32_t spiSpeed = 500 * 1000)
        : RadioLibHal(GPIO_INPUT, GPIO_OUTPUT, 0, 1, GPIO_IRQ_RISING_EDGE, GPIO_IRQ_FALLING_EDGE),
          _spiChannel(spi),
          _spiSpeed(spiSpeed),
          _misoPin(misoPin),
          _mosiPin(mosiPin),
          _sckPin(sckPin) {}

    void init() override
    {
        spiBegin();
    }

    void term() override
    {
        spiEnd();
    }

    void pinMode(uint32_t pin, uint32_t mode) override
    {
        if (pin == RADIOLIB_NC)
        {
            return;
        }

        hal_gpio_init_pin(pin, mode == GpioModeInput ? GPIO_INPUT : GPIO_OUTPUT);
    }

    void digitalWrite(uint32_t pin, uint32_t value) override
    {
        if (pin == RADIOLIB_NC)
        {
            return;
        }

        hal_gpio_set_pin_state(pin, value ? GPIO_HIGH : GPIO_LOW);
    }

    uint32_t digitalRead(uint32_t pin) override
    {
        if (pin == RADIOLIB_NC)
        {
            return 0;
        }

        return hal_gpio_get_pin_state(pin) == GPIO_HIGH ? 1 : 0;
    }

    void attachInterrupt(uint32_t interruptNum, void (*interruptCb)(void), uint32_t mode) override
    {
        hal_gpio_attach_interrupt(interruptNum, interruptCb, (hal_gpio_irq_mode_t)mode);
    }

    void detachInterrupt(uint32_t interruptNum) override
    {
        hal_gpio_detach_interrupt(interruptNum);
    }

    void delay(unsigned long ms) override
    {
        osal_task_delay_ms(ms);
    }

    void delayMicroseconds(unsigned long us) override
    {
        hal_time_sleep_us(us);
    }

    unsigned long millis() override
    {
        return hal_time_get_ms_since_boot();
    }

    unsigned long micros() override
    {
        return hal_time_get_us_since_boot();
    }

    long pulseIn(uint32_t pin, uint32_t state, unsigned long timeout) override
    {
        if (pin == RADIOLIB_NC)
        {
            return 0;
        }

        this->pinMode(pin, GPIO_INPUT);
        uint32_t start = this->micros();
        uint32_t curtick = this->micros();

        while (this->digitalRead(pin) == state)
        {
            if ((this->micros() - curtick) > timeout)
            {
                return 0;
            }
        }

        return (this->micros() - start);
    }

    void tone(uint32_t pin, unsigned int frequency, unsigned long duration = 0) override;

    void noTone(uint32_t pin) override
    {
        (void)pin;
    }

    void spiBegin()
    {
        hal_spi_init(_spiChannel, _misoPin, _mosiPin, _sckPin, _spiSpeed);
    }

    void spiBeginTransaction()
    {
    }

    void spiTransfer(uint8_t *out, size_t len, uint8_t *in)
    {
        hal_spi_transfer(_spiChannel, out, in, len);
    }

    void yield() override
    {
        osal_task_delay_ms(1);
    }

    void spiEndTransaction()
    {
    }

    void spiEnd()
    {
    }

private:
    uint8_t _spiChannel;
    uint32_t _spiSpeed;
    uint8_t _misoPin;
    uint8_t _mosiPin;
    uint8_t _sckPin;
};

#endif