/*
 * AcgCollector.cpp
 *
 *  Created on: 9 мая 2018 г.
 *      Author: Kreyl
 */

#include "AcgCollector.h"
#include "acg_lsm6ds3_defins.h"
#include "board.h"
#include "kl_lib.h"

Spi_t ISpi {ACG_SPI};
static thread_reference_t ThdRef = nullptr;

Acg_t _Acg1 {ACG_INT1, ACG_CS1, ACG_PWR1, &ISpi};
Acg_t _Acg2 {ACG_INT2, ACG_CS2, ACG_PWR2, &ISpi};
Acg_t _Acg3 {ACG_INT3, ACG_CS3, ACG_PWR3, &ISpi};
Acg_t _Acg4 {ACG_INT4, ACG_CS4, ACG_PWR4, &ISpi};
Acg_t _Acg5 {ACG_INT5, ACG_CS5, ACG_PWR5, &ISpi};
Acg_t _Acg6 {ACG_INT6, ACG_CS6, ACG_PWR6, &ISpi};
Acg_t* Acg[6] = {&_Acg1, &_Acg2, &_Acg3, &_Acg4, &_Acg5, &_Acg6};


void AcgAllInit() {
    PinSetupAlterFunc(ACG_SCK_PIN);
    PinSetupAlterFunc(ACG_MISO_PIN);
    PinSetupAlterFunc(ACG_MOSI_PIN);
#if 1 // ==== SPI ====    MSB first, master, ClkIdleHigh, FirstEdge
    uint32_t div;
#if defined STM32L1XX || defined STM32F4XX || defined STM32L4XX
    if(ACG_SPI == SPI1) div = Clk.APB2FreqHz / ACG_MAX_BAUDRATE_HZ;
    else div = Clk.APB1FreqHz / ACG_MAX_BAUDRATE_HZ;
#elif defined STM32F030 || defined STM32F0
    div = Clk.APBFreqHz / ACG_MAX_BAUDRATE_HZ;
#endif
    SpiClkDivider_t ClkDiv = sclkDiv2;
    if     (div > 128) ClkDiv = sclkDiv256;
    else if(div > 64) ClkDiv = sclkDiv128;
    else if(div > 32) ClkDiv = sclkDiv64;
    else if(div > 16) ClkDiv = sclkDiv32;
    else if(div > 8)  ClkDiv = sclkDiv16;
    else if(div > 4)  ClkDiv = sclkDiv8;
    else if(div > 2)  ClkDiv = sclkDiv4;
    ISpi.Setup(boMSB, cpolIdleHigh, cphaSecondEdge, ClkDiv);
    ISpi.EnableRxDma();
    ISpi.EnableTxDma();
    ISpi.Enable();
#endif

    for(int i=0; i<6; i++) Acg[i]->Init();


#if 0 // ==== DMA ====
    // Tx
    dmaStreamAllocate(ACG_DMA_TX, IRQ_PRIO_MEDIUM, nullptr, nullptr);
    dmaStreamSetPeripheral(ACG_DMA_TX, &ACG_SPI->DR);
    // Rx
    dmaStreamAllocate(ACG_DMA_RX, IRQ_PRIO_MEDIUM, AcgDmaRxCompIrq, nullptr);
    dmaStreamSetPeripheral(ACG_DMA_RX, &ACG_SPI->DR);
#endif

    // Thread
//    chThdCreateStatic(waAcgThread, sizeof(waAcgThread), NORMALPRIO, (tfunc_t)AcgThread, NULL);

}

// DMA reception complete
extern "C"
void AcgDmaRxCompIrq(void *p, uint32_t flags) {
    Acg_t *pacg = (Acg_t*)p;
    pacg->ICsHi();
    // Disable DMA
    dmaStreamDisable(ACG_DMA_TX);
    dmaStreamDisable(ACG_DMA_RX);
    chSysLockFromISR();
    chThdResumeI(&ThdRef, MSG_OK);
    chSysUnlockFromISR();
}

// Thread
//static THD_WORKING_AREA(waAcgThread, 512);
//__noreturn
//static void AcgThread(void *arg) {
//    chRegSetThreadName("Acg");
////    Acg.Task();
//    while(true) {
//        chThdSleepMilliseconds(999);
//    }
//}

//__noreturn
//void Acg_t::Task() {
//    while(true) {
//        chSysLock();
//        chThdSuspendS(&ThdRef); // Wait IRQ
//        chSysUnlock();
////        AccSpd.Print();
//    }
//}

