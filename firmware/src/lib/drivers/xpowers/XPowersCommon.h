#pragma once


#ifdef _BV
#undef _BV
#endif
#define _BV(b)                          (1ULL << (uint64_t)(b))


#ifndef constrain
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#endif

#define XPOWERS_CHECK_RANGE(VAR, MIN, MAX, ERR) { if(!(((VAR) >= (MIN)) && ((VAR) <= (MAX)))) { return(ERR); } }


#define IS_BIT_SET(val,mask)            (((val)&(mask)) == (mask))

#define log_e(...)
#define log_i(...)
#define log_d(...)


template <class chipType>
class XPowersCommon
{
    typedef int (*iic_fptr_t)(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len);

public:
    bool begin(uint8_t addr, iic_fptr_t readRegCallback, iic_fptr_t writeRegCallback)
    {
        if (__has_init)return thisChip().initImpl();
        __has_init = true;
        thisReadRegCallback = readRegCallback;
        thisWriteRegCallback = writeRegCallback;
        __addr = addr;
        return thisChip().initImpl();
    }

    int readRegister(uint8_t reg)
    {
        uint8_t val = 0;
        if (thisReadRegCallback) {
            if (thisReadRegCallback(__addr, reg, &val, 1) != 0) {
                return 0;
            }
            return val;
        }
        return -1;
    }

    int writeRegister(uint8_t reg, uint8_t val)
    {
        if (thisWriteRegCallback) {
            return thisWriteRegCallback(__addr, reg, &val, 1);
        }
        return -1;
    }

    int readRegister(uint8_t reg, uint8_t *buf, uint8_t lenght)
    {
        if (thisReadRegCallback) {
            return thisReadRegCallback(__addr, reg, buf, lenght);
        }
        return -1;
    }

    int writeRegister(uint8_t reg, uint8_t *buf, uint8_t lenght)
    {
        if (thisWriteRegCallback) {
            return thisWriteRegCallback(__addr, reg, buf, lenght);
        }
        return -1;
    }


    bool inline clrRegisterBit(uint8_t registers, uint8_t bit)
    {
        int val = readRegister(registers);
        if (val == -1) {
            return false;
        }
        return  writeRegister(registers, (val & (~_BV(bit)))) == 0;
    }

    bool inline setRegisterBit(uint8_t registers, uint8_t bit)
    {
        int val = readRegister(registers);
        if (val == -1) {
            return false;
        }
        return  writeRegister(registers, (val | (_BV(bit)))) == 0;
    }

    bool inline getRegisterBit(uint8_t registers, uint8_t bit)
    {
        int val = readRegister(registers);
        if (val == -1) {
            return false;
        }
        return val & _BV(bit);
    }

    uint16_t inline readRegisterH8L4(uint8_t highReg, uint8_t lowReg)
    {
        int h8 = readRegister(highReg);
        int l4 = readRegister(lowReg);
        if (h8 == -1 || l4 == -1)return 0;
        return (h8 << 4) | (l4 & 0x0F);
    }

    uint16_t inline readRegisterH8L5(uint8_t highReg, uint8_t lowReg)
    {
        int h8 = readRegister(highReg);
        int l5 = readRegister(lowReg);
        if (h8 == -1 || l5 == -1)return 0;
        return (h8 << 5) | (l5 & 0x1F);
    }

    uint16_t inline readRegisterH6L8(uint8_t highReg, uint8_t lowReg)
    {
        int h6 = readRegister(highReg);
        int l8 = readRegister(lowReg);
        if (h6 == -1 || l8 == -1)return 0;
        return ((h6 & 0x3F) << 8) | l8;
    }

    uint16_t inline readRegisterH5L8(uint8_t highReg, uint8_t lowReg)
    {
        int h5 = readRegister(highReg);
        int l8 = readRegister(lowReg);
        if (h5 == -1 || l8 == -1)return 0;
        return ((h5 & 0x1F) << 8) | l8;
    }

    /*
     * CRTP Helper
     */
protected:

    bool begin()
    {
        return thisChip().initImpl();
    }

    void end()
    {
        (void)0;
    }


    inline const chipType &thisChip() const
    {
        return static_cast<const chipType &>(*this);
    }

    inline chipType &thisChip()
    {
        return static_cast<chipType &>(*this);
    }

protected:
    bool        __has_init              = false;
    int         __sda                   = -1;
    int         __scl                   = -1;
    uint8_t     __addr                  = 0xFF;
    iic_fptr_t  thisReadRegCallback     = NULL;
    iic_fptr_t  thisWriteRegCallback    = NULL;
};
