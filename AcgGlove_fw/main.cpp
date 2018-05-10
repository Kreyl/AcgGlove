/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "hal.h"
#include "board.h"
#include "MsgQ.h"
#include "shell.h"
#include "uart.h"
#include "led.h"
#include "kl_lib.h"
#include "radio_lvl1.h"
#include "AcgCollector.h"
#include "Sequences.h"

#if 1 // ======================== Variables and defines ========================
// Forever
EvtMsgQ_t<EvtMsg_t, MAIN_EVT_Q_LEN> EvtQMain;
extern CmdUart_t Uart;
void OnCmd(Shell_t *PShell);
void ITask();

static void OnRadioRx();

LedBlinker_t Led {LED_PIN};

#endif

int main(void) {
    // ==== Init Clock system ====
//    Clk.SwitchToHsi48();
    Clk.UpdateFreqValues();

    // === Init OS ===
    halInit();
    chSysInit();

    // ==== Init hardware ====
    EvtQMain.Init();
    Uart.Init(115200);
    Printf("\r%S %S\r", APP_NAME, BUILD_TIME);
    Clk.PrintFreqs();

    Led.Init();

    if(AcgAllInit() != retvOk) {
        while(true) {
            Led.StartOrContinue(lbsqFailure1);
            chThdSleepMilliseconds(999);
        }
    }

    if(Radio.Init() == retvOk) Led.StartOrRestart(lbsqBlink1s);
    else Led.StartOrRestart(lbsqFailure2);

    // Adc
//    PinSetupAnalog(LUM_MEAS_PIN);
//    Adc.Init();
//    Adc.EnableVRef();
    // Main cycle
    ITask();
}

static uint8_t AcgMask = 0;
systime_t st;

__noreturn
void ITask() {
    while(true) {
        EvtMsg_t Msg = EvtQMain.Fetch(TIME_INFINITE);
        switch(Msg.ID) {
            case evtIdShellCmd:
                OnCmd((Shell_t*)Msg.Ptr);
                ((Shell_t*)Msg.Ptr)->SignalCmdProcessed();
                break;

            case evtIdRadioRx:
                OnRadioRx();
                break;

            case evtIdNewAcgRslt: {
                Acg_t *pAcg = (Acg_t*)Msg.Ptr;
                AcgMask |= 1 << pAcg->Indx;
                if(AcgMask == 0b111111) {
                    AcgMask = 0;
                    Printf("Acg %u\r", ST2MS(chVTTimeElapsedSinceX(st)));
                    st = chVTGetSystemTimeX();
                }
//                Printf("Acg: %u\r", pAcg->Indx);
//                pAcg->Read(0x3E, &pAcg->AccSpd, sizeof(AccSpd_t));
//                pAcg->AccSpd.Print();
            } break;

            case evtIdAdcRslt: {
                } break;

            default: break;
        } // switch
    } // while true
} // ITask()

void OnRadioRx() {
}

#if UART_RX_ENABLED // ================= Command processing ====================
void OnCmd(Shell_t *PShell) {
	Cmd_t *PCmd = &PShell->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
//    Uart.Printf("%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) {
        PShell->Ack(retvOk);
    }

    else PShell->Ack(retvCmdUnknown);
}
#endif
