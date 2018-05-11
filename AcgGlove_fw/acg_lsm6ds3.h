/*
 * acg_lsm6ds3.h
 *
 *  Created on: 2 рту. 2017 у.
 *      Author: Kreyl
 */

#pragma once

#include "kl_lib.h"
#include "shell.h"

union AccSpd_t {
    uint32_t DWord[3];
    struct {
        int16_t g[3];
        int16_t a[3];
    } __packed;
    void Print() { Printf("%d %d %d; %d %d %d\r", a[0],a[1],a[2], g[0],g[1],g[2]); }
    AccSpd_t& operator = (const AccSpd_t &Right) {
        DWord[0] = Right.DWord[0];
        DWord[1] = Right.DWord[1];
        DWord[2] = Right.DWord[2];
        return *this;
    }
} __packed;

typedef void (*ftVoidUint32)(uint32_t Dw);

class Acg_t : public IrqHandler_t {
private:
    const PinIrq_t IIrq;
    const PinOutput_t ICs, IPwr;
    Spi_t *PSpi;
    void IIrqHandler();
    void IWriteReg(uint8_t AAddr, uint8_t AValue);
    void IReadReg(uint8_t AAddr, uint8_t *PValue);
public:
    AccSpd_t AccSpd;
    ftVoidPVoid IHandler = nullptr;
    uint8_t Indx = 0;
    uint8_t Init();
    void Read(uint8_t AAddr, void *ptr, uint8_t Len);
    void ReadViaDMA(uint8_t AAddr, void *ptr, uint32_t Len);
    void Shutdown();
    Acg_t(GPIO_TypeDef *APGpioIrq, uint16_t APinIrq,
          GPIO_TypeDef *APGpioCs, uint16_t APinCs,
          GPIO_TypeDef *APGpioPwr, uint16_t APinPwr,
          Spi_t *APSpi) :
        IIrq(APGpioIrq, APinIrq, pudPullDown, this),
        ICs(APGpioCs, APinCs, omPushPull),
        IPwr(APGpioPwr, APinPwr, omPushPull),
        PSpi(APSpi) {}
    // Inner use
    void ICsHi() { ICs.SetHi(); }
    void ICsLo() { ICs.SetLo(); }
};
