/*
 * acg_lsm6ds3.cpp
 *
 *  Created on: 2 рту. 2017 у.
 *      Author: Kreyl
 */

#include "acg_lsm6ds3.h"
#include "MsgQ.h"
#include "shell.h"
#include "acg_lsm6ds3_defins.h"

//Acg_t Acg;

void Acg_t::Init() {
#if 1 // ==== GPIO ====
    ICs.Init();
    IPwr.Init();
    IPwr.SetHi();
    ICs.SetHi();
    IIrq.Init(ttRising);
    chThdSleepMilliseconds(18);
#endif

#if 1 // ==== Registers ====
    // Reset
    IWriteReg(0x12, 0x81);
    chThdSleepMilliseconds(11);
    uint8_t b;
    IReadReg(0x0F, &b);
    if(b != 0x69) {
        Printf("Wrong Acg WhoAmI: %X\r", b);
        return;
    }

    // FIFO
    IWriteReg(0x06, 6); // FIFO CTRL1: FIFO thr
    IWriteReg(0x07, 0x00); // FIFO CTRL2: pedo dis, no temp, FIFO thr MSB = 0
    IWriteReg(0x08, (0b001 << 3) | (0b001)); // FIFO CTRL3: gyro and acc no decimation, both in fifo
    IWriteReg(0x09, 0x00); // FIFO CTRL4: no stop on thr, not only MSB, no fourth and third dataset
    IWriteReg(0x0A, (0b0110 << 3) | 0b110); // FIFO CTRL5: FIFO ODR = 416, FIFO mode = ovewrite old v

    IWriteReg(0x0B, 0x00); // DRDY_PULSE_CFG_G: DataReady latched mode, Wrist tilt INT2 dis
    IWriteReg(0x0D, 0x08); // INT1_CTRL: irq on FIFO threshold

    // CTRL
    b = LSM6DS3_ACC_GYRO_BW_XL_400Hz | LSM6DS3_ACC_GYRO_FS_XL_8g | LSM6DS3_ACC_GYRO_ODR_XL_104Hz;
    IWriteReg(0x10, b);
    b = LSM6DS3_ACC_GYRO_FS_G_2000dps | LSM6DS3_ACC_GYRO_ODR_G_104Hz;
    IWriteReg(0x11, b);
    IWriteReg(0x12, 0x44); // CTRL3_c: no reboot, block update, irq act hi & push-pull, spi 4w, reg addr inc, LSB first, no rst
    IWriteReg(0x13, 0x84); // CTRL4_c: DEN, no g sleep, i2c dis, no g LPF
    IWriteReg(0x14, 0x00); // CTRL5_c: no rounding, no self-test
    IWriteReg(0x15, 0x00); // CTRL6_c: no DEN, acc hi-perf en
    IWriteReg(0x16, 0x00); // CTRL7_G: g hi-perf en, g HPF dis, rounding dis
    IWriteReg(0x17, 0x00); // CTRL8_XL: no LPF2, no HP
    IWriteReg(0x1A, 0x80); // MASTER_CONFIG: DRDY on INT1, other dis
#endif
    IIrq.EnableIrq(IRQ_PRIO_MEDIUM);
    Printf("IMU Init Done\r", b);
}

void Acg_t::Shutdown() {
}

#if 1 // =========================== Low level =================================
void Acg_t::IWriteReg(uint8_t AAddr, uint8_t AValue) {
    ICsLo();
    PSpi->ReadWriteByte(AAddr);
    PSpi->ReadWriteByte(AValue);
    ICsHi();
}

void Acg_t::IReadReg(uint8_t AAddr, uint8_t *PValue) {
    ICsLo();
    PSpi->ReadWriteByte(AAddr | 0x80);   // Add "Read" bit
    *PValue = PSpi->ReadWriteByte(0);
    ICsHi();
}

void Acg_t::IRead(uint8_t AAddr, void *ptr, uint8_t Len) {
    uint8_t *p = (uint8_t*)ptr;
    ICsLo();
    PSpi->ReadWriteByte(AAddr | 0x80);   // Add "Read" bit
    while(Len-- > 0) {
        *p++ = PSpi->ReadWriteByte(0);
    }
    ICsHi();
}

void Acg_t::IReadViaDMA(uint8_t AAddr, void *ptr, uint32_t Len) {
    AAddr |= 0x80;  // Add "Read" bit
    ICsLo();
    chSysLock();
    // RX
    dmaStreamSetMemory0(ACG_DMA_RX, ptr);
    dmaStreamSetTransactionSize(ACG_DMA_RX, Len);
    dmaStreamSetMode(ACG_DMA_RX, ACG_DMA_RX_MODE);
    dmaStreamEnable(ACG_DMA_RX);
    // TX
    dmaStreamSetMemory0(ACG_DMA_TX, &AAddr);
    dmaStreamSetTransactionSize(ACG_DMA_TX, Len);
    dmaStreamSetMode(ACG_DMA_TX, ACG_DMA_TX_MODE);
    dmaStreamEnable(ACG_DMA_TX);
    chSysUnlock();
}

const uint8_t SAddr = 0x3E | 0x80; // Add "Read" bit
void Acg_t::IIrqHandler() {
//    PrintfI("i\r");
    ICsLo();
    // RX
    dmaStreamSetMemory0(ACG_DMA_RX, &AccSpd);
    dmaStreamSetTransactionSize(ACG_DMA_RX, sizeof(AccSpd_t));
    dmaStreamSetMode(ACG_DMA_RX, ACG_DMA_RX_MODE);
    dmaStreamEnable(ACG_DMA_RX);
    // TX
    dmaStreamSetMemory0(ACG_DMA_TX, &SAddr);
    dmaStreamSetTransactionSize(ACG_DMA_TX, sizeof(AccSpd_t));
    dmaStreamSetMode(ACG_DMA_TX, ACG_DMA_TX_MODE);
    dmaStreamEnable(ACG_DMA_TX);

//    chThdResumeI(&ThdRef, MSG_OK);
}
#endif
