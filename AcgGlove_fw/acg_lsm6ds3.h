/*
 * acg_lsm6ds3.h
 *
 *  Created on: 2 рту. 2017 у.
 *      Author: Kreyl
 */

#pragma once

#include "kl_lib.h"
#include "shell.h"

struct AccSpd_t {
    uint8_t __dummy;    // DMA will write here when transmitting addr
    int16_t g[3];
    int16_t a[3];
    void Print() { Printf("%d %d %d; %d %d %d\r", a[0],a[1],a[2], g[0],g[1],g[2]); }
} __packed;

class Acg_t : public IrqHandler_t {
private:
    const PinIrq_t IIrq;
    const PinOutput_t ICs, IPwr;
    Spi_t *PSpi;
    void IIrqHandler();
    void IWriteReg(uint8_t AAddr, uint8_t AValue);
    void IReadReg(uint8_t AAddr, uint8_t *PValue);
    void IRead(uint8_t AAddr, void *ptr, uint8_t Len);
    void IReadViaDMA(uint8_t AAddr, void *ptr, uint32_t Len);
public:
    AccSpd_t AccSpd;
    void Init();
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
